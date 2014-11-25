/**************************************************************

config.cpp (C-Munipack project)
Configuration file interface
Copyright (C) 2008 David Motl, dmotl@volny.cz

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

**************************************************************/

#include <stdlib.h>
#include <string.h>

#include "config.h"

#if HAVE_LIBGSTREAMER_0_10
#include <gst/gst.h>
#elif _WIN32
#include <windows.h>
#endif

#include "utils.h"

#if HAVE_LIBGSTREAMER_0_10
static GstElement *gs_play = NULL;
#endif

void SoundInit(int *argc, char **argv[])
{
#if HAVE_LIBGSTREAMER_0_10
	gst_init(argc, argv);
#endif
}

bool SoundIsSupported(void)
{
#if HAVE_LIBGSTREAMER_0_10 || _WIN32
	return true;
#else
	return false;
#endif
}

void SoundCleanup(void)
{
#if HAVE_LIBGSTREAMER_0_10
	if (gs_play) {
	  gst_element_set_state(gs_play, GST_STATE_NULL);
	  gst_object_unref(GST_OBJECT (gs_play));
	  gs_play = NULL;
	}
	gst_deinit();
#elif _WIN32
	PlaySound(0, 0, 0);
#endif
}

void SoundPlay(const gchar *sound)
{
	bool ok = false;
	
#if HAVE_LIBGSTREAMER_0_10
	if (gs_play) {
		gst_element_set_state(gs_play, GST_STATE_NULL);
		gst_object_unref(GST_OBJECT(gs_play));
		gs_play = NULL;
	}
#endif

	char *dirpath = g_build_filename(get_share_dir(), "sounds", NULL);
	GDir *dir = g_dir_open(dirpath, 0, NULL);
	if (dir) {
		const gchar *filename = g_dir_read_name(dir);
		while (filename) {
			gchar *fname = g_strdup(filename);
			gchar *ptr = strrchr(fname, '.');
			if (ptr)
				*ptr = '\0';
			if (ComparePaths(fname, sound)==0) {
#if HAVE_LIBGSTREAMER_0_10
				gchar fpath[512];
#if _WIN32
				sprintf(fpath, "file:///%s/%s", dirpath, filename);
				ptr = strchr(fpath, '\\');
				while (ptr) {
					*ptr = '/';
					ptr = strchr(ptr+1, '\\');
				}
#else
				sprintf(fpath, "file://%s/%s", dirpath, filename);
#endif
				gs_play = gst_element_factory_make ("playbin", "play");
				g_object_set(G_OBJECT (gs_play), "uri", fpath, NULL);
				gst_element_set_state(gs_play, GST_STATE_PLAYING);
#else
#if _WIN32
				gchar fpath[512];
				sprintf(fpath, "%s\\%s", dirpath, filename);
				gchar *f = g_locale_from_utf8(fpath, -1, NULL, NULL, NULL);
				if (f)
					PlaySound(f, NULL, SND_FILENAME | SND_ASYNC);
				g_free(f);
#endif
#endif
				ok = true;
				break;
			}
			g_free(fname);
			filename = g_dir_read_name(dir);
		}
		g_dir_close(dir);
	}
	g_free(dirpath);

	if (!ok) 
		g_debug("Sound file not found: %s", sound);
}
