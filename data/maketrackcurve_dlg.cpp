/**************************************************************

tracklist_dlg.cpp (C-Munipack project)
The 'Plot track-list' dialog
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

#include "project.h"
#include "configuration.h"
#include "progress_dlg.h"
#include "maketrackcurve_dlg.h"
#include "trackcurve_dlg.h"
#include "main.h"
#include "utils.h"
#include "proc_classes.h"
#include "ctxhelp.h"

//-------------------------   MAKE TRACK CURVE DIALOG   --------------------------------

CMakeTrackCurveDlg::CMakeTrackCurveDlg(GtkWindow *pParent):m_pParent(pParent)
{
	GtkWidget *vbox;
	GSList *group;

	// Dialog with buttons
	m_pDlg = gtk_dialog_new_with_buttons("Make track curve", pParent, 
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
	gtk_dialog_widget_standard_tooltips(GTK_DIALOG(m_pDlg));
	gtk_dialog_set_tooltip_by_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_ACCEPT, "Use the entered values and continue");
	g_signal_connect(G_OBJECT(m_pDlg), "response", G_CALLBACK(response_dialog), this);

	// Dialog icon
	gchar *icon = get_icon_file("tracklist");
	gtk_window_set_icon(GTK_WINDOW(m_pDlg), gdk_pixbuf_new_from_file(icon, NULL));
	g_free(icon);

	// Dialog layout
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(m_pDlg)->vbox), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);

	// Frame selection
	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>Process</b>");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	m_AllBtn = gtk_radio_button_new_with_label(NULL, "all files in current project");
	gtk_widget_set_tooltip_text(m_AllBtn, "Process all frames in the current project");
	gtk_box_pack_start(GTK_BOX(vbox), m_AllBtn, TRUE, TRUE, 0);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_AllBtn));
	m_SelBtn = gtk_radio_button_new_with_label(group, "selected files only");
	gtk_widget_set_tooltip_text(m_SelBtn, "Process frames that are selected in the main window");
	gtk_box_pack_start(GTK_BOX(vbox), m_SelBtn, TRUE, TRUE, 0);

	gtk_widget_show_all(GTK_DIALOG(m_pDlg)->vbox);
}

CMakeTrackCurveDlg::~CMakeTrackCurveDlg()
{
	gtk_widget_destroy(m_pDlg);
}

void CMakeTrackCurveDlg::Execute(void)
{
	if (g_Project->GetReferenceType()==REF_UNDEFINED) {
		ShowError(m_pParent, "Please, execute the matching procedure first.");
		return;
	}

	GtkTreeSelection *pSel = g_MainWnd->GetSelection();
	gtk_widget_set_sensitive(m_AllBtn, TRUE);
	gtk_widget_set_sensitive(m_SelBtn, 
		gtk_tree_selection_count_selected_rows(pSel)>0);
	if (gtk_tree_selection_count_selected_rows(pSel)>1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_SelBtn), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_AllBtn), TRUE);

	// Show the dialog
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg))!=GTK_RESPONSE_ACCEPT) 
		return;

	bool selected_files = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_SelBtn))!=0;

	gtk_widget_hide(m_pDlg);
	CTrackCurveDlg *dlg = new CTrackCurveDlg();
	if (dlg->Make(m_pParent, selected_files))
		dlg->Show();
	else
		dlg->Close();
}

void CMakeTrackCurveDlg::response_dialog(GtkDialog *pDlg, gint response_id, CMakeTrackCurveDlg *pMe)
{
	if (!pMe->OnResponseDialog(response_id))
		g_signal_stop_emission_by_name(pDlg, "response");
}

bool CMakeTrackCurveDlg::OnResponseDialog(gint response_id)
{
	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		// Show context help
		g_MainWnd->ShowHelp(GTK_WINDOW(m_pDlg), IDH_MAKE_TRACK_CURVE);
		return false;
	}
	return true;
}
