/**************************************************************

about_dlg.cpp (C-Munipack project)
The 'About application' dialog
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
#include <cmunipack.h>
#include <fitsio.h>
#include <expat.h>

#include "about_dlg.h"
#include "config.h"
#include "utils.h"
#include "main.h"
#include "ctxhelp.h"

#if HAVE_LIBGSTREAMER_0_10
G_BEGIN_DECLS
gchar *gst_version_string();
G_END_DECLS
#endif

//-------------------------   STATIC DATA   --------------------------------

const char *ProjectDescription = "The application provides complete solution "
	"for reduction of images carried out by a CCD camera, "
	"oriented on a observation of variable stars.";

const char *CopyrightText = "Muniwin - graphical user intefrace to the C-Munipack package\n"
	"Copyright 2003-8 David Motl <dmotl@volny.cz>, Czech Republic\n\n"
	"This program is free software; you can redistribute it and/or modify it under "
	"the terms of the GNU General Public License as published by the Free "
	"Software Foundation; either version 2 of the License, or (at your option) any "
	"later version.\n\n"
	"This program is distributed in the hope that it will be useful, but WITHOUT "
	"ANY WARRANTY; without even the implied warranty of "
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See "
	"the GNU General Public License for more details. \n\n"
	"You should have received a copy of the GNU General Public License "
	"along with this program; if not, write to the Free Software Foundation, Inc., "
	"675 Mass Ave, Cambridge, MA 02139, USA.";

const char *CreditsText = "Project manager:\n"
	"\tDavid Motl <dmotl@volny.cz>\n\n"
	"Principle author:\n"
	"\tDavid Motl\n\n"
	"Other contributors:\n"
	"\tPetr Svoboda\n"
	"\tLuk\xC3\xA1\xC5\xA1"" Kr\xC3\xA1""l\n"
	"\tMiloslav Zejda\n"
	"\tJitka Kudrn\xC3\xA1\xC4\x8D""ov\xC3\xA1""\n"
	"\tPetr Lu\xC5\xA5""cha\n"
	"\tVolkan Bakis\n\n"
	"Third-party software used:\n"
	"\tGTK+ toolkit, http://www.gtk.org/\n"
	"\tCFITSIO library, http://heasarc.gsfc.nasa.gov/fitsio/\n"
	"\tEXPAT library, http://www.libexpat.org/\n"
	"\tMUNIPACK project, http://munipack.astronomy.cz/\n"
	"\tIJG JPEG library, http://www.ijg.org/\n"
	"\tDAOPHOT II, http://www.star.bris.ac.uk/~mbt/daophot/\n"
	"\tDCRAW, http://http://www.cybercom.net/~dcoffin/dcraw/\n";

//-------------------------   ABOUT DIALOG   --------------------------------

CAboutDlg::CAboutDlg(GtkWindow *pParent)
{
	float x;
	char buf[2048];
	GtkWidget *tbox, *label, *scrolled_window, *text_view;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("About Muniwin", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR),
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("about");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Notebook
	m_Notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), m_Notebook, TRUE, TRUE, 0);
	gtk_widget_set_size_request(m_Notebook, 400, 240);

	// Page "General"
	tbox = gtk_table_new(4, 1, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(tbox), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 8);
	gtk_notebook_append_page(GTK_NOTEBOOK(m_Notebook), tbox, gtk_label_new("General"));

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<span size=\"xx-large\" weight=\"bold\">Muniwin</span>");
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 0, 1);

	label = gtk_label_new(NULL);
	sprintf(buf, "<span size=\"x-large\" weight=\"bold\">Stable version %s</span>", VERSION);
	gtk_label_set_markup(GTK_LABEL(label), buf);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 1, 2);

	label = gtk_label_new(NULL);
	sprintf(buf, "<span size=\"large\">%s</span>", ProjectDescription);
	gtk_label_set_markup(GTK_LABEL(label), buf);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 2, 3);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<span size=\"x-large\" underline=\"single\" foreground=\"blue\">http://c-munipack.sourceforge.net</span>");
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, 3, 4);

	// Page "Versions"
	tbox = gtk_table_new(5, 2, false);
	gtk_container_set_border_width(GTK_CONTAINER(tbox), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tbox), 8);
	gtk_table_set_col_spacings(GTK_TABLE(tbox), 16);
	gtk_notebook_append_page(GTK_NOTEBOOK(m_Notebook), tbox, gtk_label_new("Versions"));
	AddVersion(GTK_TABLE(tbox), 0, g_AppTitle, VERSION);
	AddVersion(GTK_TABLE(tbox), 1, "C-Munipack", cmpack_versionid());
	sprintf(buf, "%.03f", ffvers(&x));
	AddVersion(GTK_TABLE(tbox), 2, "CFITSIO", buf);
	AddVersion(GTK_TABLE(tbox), 3, "Expat", XML_ExpatVersion());
	sprintf(buf, "%d.%d.%d", gtk_major_version, gtk_minor_version, gtk_micro_version);
	AddVersion(GTK_TABLE(tbox), 4, "GTK+", buf);
	sprintf(buf, "%d.%d.%d", glib_major_version, glib_minor_version, glib_micro_version);
	AddVersion(GTK_TABLE(tbox), 5, "GLib", buf);
#if HAVE_LIBGSTREAMER_0_10
	gchar *gstver = gst_version_string();
	AddVersion(GTK_TABLE(tbox), 6, "GStreamer", gstver);
	free(gstver);
#endif
	
	// Page "License"
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(m_Notebook), scrolled_window, gtk_label_new("License"));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), 
		GTK_SHADOW_ETCHED_IN);

	text_view = gtk_text_view_new_with_buffer(NULL);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), false);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), false);
	gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view)), 
		CopyrightText, -1);
	gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

	// Page "Credits"
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(m_Notebook), scrolled_window, gtk_label_new("Credits"));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), 
		GTK_SHADOW_ETCHED_IN);

	text_view = gtk_text_view_new_with_buffer(NULL);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), false);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), false);
	gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view)), 
		CreditsText, -1);
	gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CAboutDlg::~CAboutDlg()
{
	gtk_widget_destroy(m_pDlg);
}

void CAboutDlg::Execute()
{
	gtk_dialog_run(GTK_DIALOG(m_pDlg));
}

void CAboutDlg::AddVersion(GtkTable *tbox, int row, const char *appname, const char *version)
{
	char buf[512];

	GtkWidget *label = gtk_label_new(NULL);
	sprintf(buf, "<span size=\"large\">%s version:</span>", appname);
	gtk_label_set_markup(GTK_LABEL(label), buf);
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.0);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 0, 1, row, row+1);

	label = gtk_label_new(NULL);
	sprintf(buf, "<span size=\"large\"><b>%s</b></span>", version);
	gtk_label_set_markup(GTK_LABEL(label), buf);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.0);
	gtk_table_attach_defaults(GTK_TABLE(tbox), label, 1, 2, row, row+1);
}

void CAboutDlg::response_dialog(GtkDialog *pDlg, gint response_id, CAboutDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CAboutDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_ABOUT);
		return false;
	}
	return true;
}
