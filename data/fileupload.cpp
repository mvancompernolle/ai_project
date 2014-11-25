/***************************************************************************
 *   Copyright (C) 2004 by Ilya A. Volynets-Evenbakh                       *
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
#include "fileupload.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void FileUploadServlet::service(servlet::HttpServletRequest& req, servlet::HttpServletResponse& resp)
{
	std::ostream &out=resp.getOutputStream();
	servlet::UploadedFile* file=req.getUploadedFile("theFile");
	renderHeader(out);
	if(!req.getParameter("moo").empty())
	    out<<"Something was submitted: "<<req.getParameter("moo")<<std::endl;
	if(!file)
	    renderForm(req, out);
	else
	    renderFile(out, file);
	renderFooter(out);
}

void FileUploadServlet::renderHeader(std::ostream& out)
{
	out<<"<html>\n <head>\n  <title>Sample File Upload Servlet for CPPSERV</title>\n </head>\n <body>";
}

void FileUploadServlet::renderFooter(std::ostream& out)
{
	out<<"\n </body>\n</html>";
}

void FileUploadServlet::renderForm(servlet::HttpServletRequest& req, std::ostream& out)
{
	out<<"<form method=\"post\" enctype=\"multipart/form-data\" action=\""
	   <<req.getRequestURI()<<"\"><input type=\"file\" name=\"theFile\">"
	   "<input type=\"submit\" name=\"moo\" value=\"Send File\"></form>";
}

void FileUploadServlet::renderFile(std::ostream& out, servlet::UploadedFile* file)
{
	if(file->isEmpty()) {
		out<<"The file is empty";
		return;
	}
	char fname[]="/tmp/cppserv-test.upload.XXXXXX";
	int fd;
	if((fd=::mkstemp(fname))==-1)
		throw servlet::IOError("Creating temporary file");
	::close(fd);
	file->moveTo(fname);
	struct stat st;
	::stat(fname, &st);
	out<<"<table border=0>\n<tr><td>File type:</td><td>"<<file->mimeType()<<
	"</td></tr>\n<tr><td>File Name:</td><td>"<<file->getName()<<
	"</td></tr>\n";
	out<<"<tr><td>File size:</td><td>"<<st.st_size<<"</td></tr>\n"<<std::endl;
	if(file->mimeType().find("text/")!=std::string::npos) {
		out<<"</table>\n<h1>File data</h1>\n<pre>"<<file->getStream().rdbuf()
		   <<"\n</pre>\n";
	} else {
		out<<"<tr><td>File Type:</td><td>Binary</td></tr>\n</table>";
	}
	::unlink(fname);
}

EXPORT_SERVLET(FileUploadServlet)
