/***************************************************************************
 *   Copyright (C) 2004-2006 by Ilya A. Volynets-Evenbakh                  *
 *   ilya@total-knowledge.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <string>
#include <sstream>
#include <iostream>
extern "C" {
#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_main.h"
#include "http_request.h" 
#include "http_log.h"
#include "ap_mpm.h"
#include "util_script.h"
#include <time.h>
#include "scoreboard.h"
#include "http_log.h"
#include "mod_status.h"
#include "apr_strings.h"
#include "apr_network_io.h"
#include "apr_portable.h"
#include "apr_version.h"

#include <sys/un.h>
}

// Ugly compatibility layer
#if (APR_MAJOR_VERSION==0)
#define apr_socket_create apr_socket_create_ex
#endif

#define CSERV_BUF_SZ 1024
#define PORT 9004
//Pre-declare module
extern "C" module AP_MODULE_DECLARE_DATA cserv_module;
struct cserv_perdir_config
{
	const char *host;
	const char *unixpath;
	unsigned short port;
	int on;
	char* path;
};

static int connectServer(request_rec *r,apr_socket_t** con)
{
	apr_pool_t* p=r->pool;
	int ret;
	apr_socket_t* sock=0;
	cserv_perdir_config *dconf=(cserv_perdir_config *) 
			ap_get_module_config(r->per_dir_config,&cserv_module);
	if(dconf->unixpath) {
		int fd;
		struct sockaddr_un bindto={AF_UNIX,{0,}};
		if((fd=socket(PF_UNIX,SOCK_STREAM,0))==-1) {
			ap_log_rerror(APLOG_MARK,APLOG_ERR,APR_SUCCESS,r,
			    "Cannot connect to CPPSERV (%s): %s",
			    dconf->unixpath, strerror(errno));
			return HTTP_SERVICE_UNAVAILABLE;
		}
		strncpy(bindto.sun_path, dconf->unixpath, sizeof(bindto.sun_path));
		if((ret=connect(fd,(sockaddr*)&bindto,sizeof(bindto)))==-1) {
			ap_log_rerror(APLOG_MARK,APLOG_ERR,APR_SUCCESS,r,
			    "Cannot connect to CPPSERV (%s): %s",
			    dconf->unixpath, strerror(errno));
			return HTTP_SERVICE_UNAVAILABLE;
		}
		if(apr_os_sock_put(&sock,&fd,p)!=APR_SUCCESS)
			return HTTP_INTERNAL_SERVER_ERROR;
		*con=sock;
	} else {
		apr_sockaddr_t* sockaddr;
		const char* host=dconf->host?dconf->host:"localhost";
		if((ret=apr_sockaddr_info_get(&sockaddr,host,AF_INET,dconf->port,0,p))!=APR_SUCCESS) {
			ap_log_rerror(APLOG_MARK,APLOG_ERR,APR_SUCCESS,r,
			    "Cannot connect to CPPSERV (%s:%d): %s",
			    host, dconf->port, strerror(errno));
			return HTTP_SERVICE_UNAVAILABLE;
		}
		if((ret=apr_socket_create(&sock,APR_INET,SOCK_STREAM,APR_PROTO_TCP,p))!=APR_SUCCESS) {
			ap_log_rerror(APLOG_MARK,APLOG_ERR,APR_SUCCESS,r,
			    "Cannot connect to CPPSERV (%s:%d): %s",
			    host, dconf->port, strerror(errno));
			return HTTP_SERVICE_UNAVAILABLE;
		}
		if((ret=apr_socket_connect(sock,sockaddr))!=APR_SUCCESS) {
			ap_log_rerror(APLOG_MARK,APLOG_ERR,APR_SUCCESS,r,
			    "Cannot connect to CPPSERV (%s:%d): %s",
			    host, dconf->port, strerror(errno));
			return HTTP_SERVICE_UNAVAILABLE;
		}
	
		*con=sock;
	}
	return OK;
}
struct whRec
{
	request_rec *r;
	apr_socket_t* con;
};
static int writeHeader(void* rec,const char *key,const char *val)
{
	whRec* r=(whRec*)rec;
	apr_socket_t *c=r->con;
	std::string h;
	h=h+key+':'+val+'\n';
#ifdef MODCSERV_DEBUG
	ap_log_rerror(APLOG_MARK,APLOG_ERR,APR_SUCCESS,r->r,"Sending request header: %s",h.c_str());
#endif
	apr_size_t len=h.length();
	if(apr_socket_send(c,h.c_str(),&len)!=APR_SUCCESS)
		return 0;
	return 1;
}
// Iterate over headers_in table and send them all through to
// servlet
static void sendRequestHeader(request_rec *r,apr_socket_t* con)
{
	std::stringstream s;
	whRec rec={r,con};
#ifdef MODCSERV_DEBUG
	ap_log_rerror(APLOG_MARK,APLOG_ERR,APR_SUCCESS,r,">>>Sending request headers<<<");
#endif
	s<<r->method<<' '<<r->unparsed_uri<<' '<<r->protocol<<'\n';
	apr_size_t len=s.str().length();
	apr_socket_send(con,s.str().c_str(),&len);
	// TODO: check if headers were actually sent OK
	apr_table_do(writeHeader,&rec,r->headers_in,NULL);
	writeHeader(&rec, "remoteaddr", r->connection->remote_ip);
	writeHeader(&rec, "localaddr", r->connection->local_ip);
	s.clear();
	s<<"remoteport: "<<r->connection->remote_addr->port;
	s<<"\nlocalport: "<<r->connection->local_addr->port<<'\n';
	len=s.str().length();
	apr_socket_send(con, s.str().c_str(), &len);
	if(r->connection->remote_host && r->connection->remote_host[0])
		writeHeader(&rec, "remotehost", r->connection->remote_host);
	if(r->connection->local_host && r->connection->local_host[0])
		writeHeader(&rec, "localhost", r->connection->local_host);
	char endline='\n';
	len=1;
	apr_socket_send(con,&endline,&len);
#ifdef MODCSERV_DEBUG
	ap_log_rerror(APLOG_MARK,APLOG_ERR,APR_SUCCESS,r,">>>Sent request headers<<<");
#endif
}

static void discard_script_output(apr_bucket_brigade *bb)
{
    apr_bucket *e;
    const char *buf;
    apr_size_t len;
    apr_status_t rv;
    for (e = APR_BRIGADE_FIRST(bb);
         e != APR_BRIGADE_SENTINEL(bb);
         e = APR_BUCKET_NEXT(e)) {
        if (APR_BUCKET_IS_EOS(e)) {
            break;
        }
        rv = apr_bucket_read(e, &buf, &len, APR_BLOCK_READ);
        if (APR_SUCCESS == rv) {
            break;
        }
    }
}

#ifdef MODCSERV_DEBUG
static int logHeader(void* rec,const char *key,const char *val)
{
	request_rec* r=(request_rec*)rec;
	ap_log_rerror(APLOG_MARK,APLOG_ERR,APR_SUCCESS,r,"Response header %s: %s",key, val);
	return 1;
}
static void logResponseHeaders(request_rec *r)
{
	ap_log_rerror(APLOG_MARK,APLOG_ERR,APR_SUCCESS,r,"Response headers");
	apr_table_do(logHeader,r,r->headers_out,NULL);
	apr_table_do(logHeader,r,r->err_headers_out,NULL);
	ap_log_rerror(APLOG_MARK,APLOG_ERR,APR_SUCCESS,r,"Status: %i",r->status);
}
#else
static void logResponseHeaders(request_rec *)
{
}
#endif

static int dealWithResponce(request_rec *r,apr_socket_t* con)
{
    /* Handle script return... */ 
	conn_rec *c = r->connection;
	const char *location; 
	char sbuf[MAX_STRING_LEN]; 
	int ret;
	apr_bucket_brigade *bb;
	apr_bucket *b;
    
	bb = apr_brigade_create(r->pool, c->bucket_alloc);
	b = apr_bucket_socket_create(con, c->bucket_alloc);
	APR_BRIGADE_INSERT_TAIL(bb, b);
	b = apr_bucket_eos_create(c->bucket_alloc);
	APR_BRIGADE_INSERT_TAIL(bb, b);

	if ((ret = ap_scan_script_header_err_brigade(r, bb, sbuf))) { 
		return ret;
	}
	logResponseHeaders(r);

	location = apr_table_get(r->headers_out, "Location"); 

	if (location && location[0] == '/' && r->status == 200) { 

		/* Soak up all the script output */
		discard_script_output(bb);
		apr_brigade_destroy(bb);
		/* This redirect needs to be a GET no matter what the original 
			* method was. 
			*/ 
		r->method = apr_pstrdup(r->pool, "GET"); 
		r->method_number = M_GET; 

		/* We already read the message body (if any), so don't allow 
			* the redirected request to think it has one. We can ignore 
			* Transfer-Encoding, since we used REQUEST_CHUNKED_ERROR. 
			*/ 
		apr_table_unset(r->headers_in, "Content-Length"); 

		ap_internal_redirect_handler(location, r); 
		return OK; 
	} 
	else if (location && r->status == 200) { 
		/* XX Note that if a script wants to produce its own Redirect 
			* body, it now has to explicitly *say* "Status: 302" 
			*/ 
		discard_script_output(bb);
		apr_brigade_destroy(bb);
		return HTTP_MOVED_TEMPORARILY; 
	}
	ap_pass_brigade(r->output_filters, bb);

	if(r->status>=400)
		return r->status;

    return OK; /* NOT r->status, even if it has changed. */ 
}
extern "C"
{ 

static int cserv_handler(request_rec *r)
{
	cserv_perdir_config* dconf=(cserv_perdir_config *) 
			ap_get_module_config(r->per_dir_config,&cserv_module);
	if(strcmp(r->handler,"cserv-handler") && !dconf->on)
		return DECLINED;
#ifdef MODCSERV_DEBUG
	ap_log_rerror(APLOG_MARK,APLOG_ERR,APR_SUCCESS,r,"path_info: %s; file name=%s",r->path_info, r->filename);
#endif

	std::string path(r->filename);
	std::string name;
	std::string::size_type i = path.rfind('/');
	if (i == std::string::npos) {
		name = path;
	} else if(i == path.length()-1){
		name = "IndexServlet";
	} else {
		i++;
		name = path.substr(i);
	}
	if(r->path_info)
		name += r->path_info;

	// Next step: connect to server.
	apr_socket_t* tmpsock;
	int ret=connectServer(r,&tmpsock);

	if(ret!=OK)
		return ret;

	name+='\n';
	apr_size_t len=name.length();
	ret=apr_socket_send(tmpsock,name.c_str(),&len);
	if(ret!=APR_SUCCESS)
		return HTTP_INTERNAL_SERVER_ERROR;
	// Send request and header before sending the rest
	sendRequestHeader(r,tmpsock);
	// Following data retrieval code is shamelessly taken from mod_cgi.c
	apr_bucket_brigade *bb=apr_brigade_create(r->pool,r->connection->bucket_alloc);
	apr_status_t rv;
	bool seen_eos=false;
	do {
		apr_bucket *bucket;
		rv = ap_get_brigade(r->input_filters,bb,AP_MODE_READBYTES,APR_BLOCK_READ,HUGE_STRING_LEN);
		if(rv!=APR_SUCCESS)
			return rv;
		for (bucket = APR_BRIGADE_FIRST(bb);
			bucket != APR_BRIGADE_SENTINEL(bb);
			bucket = APR_BUCKET_NEXT(bucket)) {
			const char* data;
			apr_size_t len;
			if(APR_BUCKET_IS_EOS(bucket)){
				seen_eos=true;
				break;
			}
			apr_bucket_read(bucket,&data,&len,APR_BLOCK_READ);
			if(ret!=APR_SUCCESS)
				continue;
			ret=apr_socket_send(tmpsock,data,&len);
		}
		apr_brigade_cleanup(bb);
	} while(!seen_eos);
	apr_socket_shutdown(tmpsock,APR_SHUTDOWN_WRITE);
	// OK. We sent all the data. Now get responce
	ret = dealWithResponce(r,tmpsock);
	apr_socket_close(tmpsock);
	return ret;
}

static void register_hooks(apr_pool_t*)
{
	ap_hook_handler(cserv_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

static const char* cmd_set_cserv_host(cmd_parms*, void* in_dconf, const char* a1)
{
	cserv_perdir_config *dconf=(cserv_perdir_config *)in_dconf;
	dconf->host=a1;
	return NULL;
}
static const char* cmd_set_cserv_port(cmd_parms*, void* in_dconf, const char* a1)
{
	cserv_perdir_config *dconf=(cserv_perdir_config *)in_dconf;
	dconf->port=atoi(a1);
	return NULL;
}
static char* cmd_turn_cserv_on(cmd_parms*, void* in_dconf,int flag)
{
	cserv_perdir_config *c=(cserv_perdir_config *)in_dconf;
	c->on=flag;
	return NULL;
}
static const char* cmd_set_cserv_unixpath(cmd_parms*, void* in_dconf, const char* a1)
{
	cserv_perdir_config *dconf=(cserv_perdir_config *)in_dconf;
	dconf->unixpath=a1;
	return NULL;
}

static void* config_perdir_create(apr_pool_t* p,char*)
{
	cserv_perdir_config *c=(cserv_perdir_config *)apr_palloc(p,sizeof(cserv_perdir_config));
	c->host=0;
	c->port=PORT;
	c->on=0;
	c->unixpath=0;
	return c;
}

static command_rec cserv_module_cmds[] = {
	AP_INIT_FLAG ("CServ"    ,(cmd_func)cmd_turn_cserv_on ,NULL,OR_FILEINFO,"On or Off to enable or disable (default) CPPSERV"),
	AP_INIT_TAKE1("CServHost",(cmd_func)cmd_set_cserv_host,NULL,OR_FILEINFO,"Host where CPPServer is running"),
	AP_INIT_TAKE1("CServPort",(cmd_func)cmd_set_cserv_port,NULL,OR_FILEINFO,"Port where CPPServer is listening"),
	AP_INIT_TAKE1("CServUnixPath",(cmd_func)cmd_set_cserv_unixpath,NULL,OR_FILEINFO,"Unix socket path (overrides any host/port settings)"),
	AP_INIT_NO_ARGS(NULL,NULL,NULL,0,NULL)
};

module AP_MODULE_DECLARE_DATA cserv_module =
{
	STANDARD20_MODULE_STUFF,
	config_perdir_create,
	NULL,
	NULL,
	NULL,
	cserv_module_cmds,
	register_hooks
};
}

