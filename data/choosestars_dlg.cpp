/**************************************************************

stars_dlg.cpp (C-Munipack project)
The 'Choose stars' dialog
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
#include "choosestars_dlg.h"
#include "main.h"
#include "utils.h"
#include "configuration.h"
#include "configuration.h"
#include "profile.h"
#include "ctxhelp.h"

//-------------------------   CHOOSE STARS DIALOG   --------------------------------

//
// Constructor
//
CChooseStarsDlg::CChooseStarsDlg(GtkWindow *pParent):CEditSelectionsDlg(pParent, MAKE_LIGHT_CURVE)
{
	m_SelectionList = g_Project->SelectionList();
	m_Tags = g_Project->Tags();
}


//
// Execute the dialog
//
bool CChooseStarsDlg::Execute(CSelection &sel, bool inst_mag)
{
	int	res = 0;
	gchar *fts_file;
	GtkTreePath *refpath;

	m_Current = m_NewSelection = sel;
	m_InstMagnitudes = inst_mag;
	m_SingleComparison = !g_Project->Profile()->GetBool(CProfile::MULTIPLE_COMP_STARS);
	m_SelectionIndex = g_Project->SelectionList()->IndexOf(sel);
	m_ShowNewSelection = (m_SelectionIndex<0);
	UpdateSelectionList();
		
	cmpack_chart_view_set_model(CMPACK_CHART_VIEW(m_Chart), NULL);
	cmpack_chart_view_set_image(CMPACK_CHART_VIEW(m_Chart), NULL);
	if (m_ChartData) {
		g_object_unref(m_ChartData);
		m_ChartData = NULL;
	}
	if (m_ImageData) {
		g_object_unref(m_ImageData);
		m_ImageData = NULL;
	}
	m_Phot.Clear();
	m_Catalog.Clear();
	m_Image.Clear();

	m_RefType = g_Project->GetReferenceType();
	switch(m_RefType)
	{
	case REF_FRAME:
		refpath = g_Project->GetReferencePath();
		if (refpath) {
			gchar *pht_file = g_Project->GetPhotFile(refpath);
			if (pht_file) {
				GError *error = NULL;
				if (m_Phot.Load(pht_file, &error)) {
					m_Phot.SelectAperture(0);
					UpdateChart();
					gchar *fts_file = g_Project->GetImageFile(refpath);
					if (fts_file) {
						if (m_Image.Load(fts_file, CMPACK_BITPIX_AUTO, &error)) 
							UpdateImage();
						g_free(fts_file);
					}
				} 
				if (error) {
					ShowError(m_pParent, error->message);
					g_error_free(error);
					res = -1;
				}
				g_free(pht_file);
			}
			gtk_tree_path_free(refpath);
		}
		break;

	case REF_CATALOG_FILE:
		// Load catalog file
		{
			const char *tmp_file = g_Project->GetTempCatFile()->FullPath();
			GError *error = NULL;
			if (m_Catalog.Load(tmp_file, &error)) {
				UpdateChart();
				fts_file = SetFileExtension(tmp_file, FILE_EXTENSION_FITS);
				if (m_Image.Load(fts_file, CMPACK_BITPIX_AUTO, &error)) 
					UpdateImage();
				g_free(fts_file);
			} 
			if (error) {
				ShowError(m_pParent, error->message);
				g_error_free(error);
				res = -1;
			}
		}
		break;

	default:
		ShowError(m_pParent, "No reference file.");
		res = -1;
	}
	if (res!=0)
		return false;

	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowChart),
		m_DisplayMode == DISPLAY_CHART);
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowImage),
		m_DisplayMode == DISPLAY_IMAGE);
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(m_ShowMixed),
		m_DisplayMode == DISPLAY_FULL);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ShowImage), 
		m_Image.Width()>0 && m_Image.Height()>0);
	gtk_widget_set_sensitive(GTK_WIDGET(m_ShowMixed), 
		m_Image.Width()>0 && m_Image.Height()>0);

	UpdateControls();
	if (gtk_dialog_run(GTK_DIALOG(m_pDlg)) == GTK_RESPONSE_ACCEPT) {
		sel = m_Current;
		return true;
	}
	return false;
}

bool CChooseStarsDlg::OnCloseQuery()
{
	if (!m_InstMagnitudes) {
		if (m_Current.CountStars(CMPACK_SELECT_VAR)==0) {
			ShowError(GTK_WINDOW(m_pDlg), "Please, select a variable star.");
			return false;
		}
		if (m_Current.CountStars(CMPACK_SELECT_VAR)>1) {
			ShowError(GTK_WINDOW(m_pDlg), "It is not allowed to select more than one variable star.");
			return false;
		}
		if (m_Current.CountStars(CMPACK_SELECT_COMP)==0) {
			ShowError(GTK_WINDOW(m_pDlg), "Please, select a comparison star.");
			return false;
		} 
		if (m_SingleComparison && m_Current.CountStars(CMPACK_SELECT_COMP)>1) {
			ShowError(GTK_WINDOW(m_pDlg), "It is not allowed to select more than comparison star.");
			return false;
		}
	}
	return CEditSelectionsDlg::OnCloseQuery();
}

