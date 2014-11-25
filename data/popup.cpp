/**************************************************************

popup.cpp (C-Munipack project)
Popup menu wrapper
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

#include "popup.h"
#include "utils.h"

struct tPopupMenuItemData
{
	CPopupMenu *menu;
	GtkWidget *item;
	int cmd_id;
};

// Default constructor
CPopupMenu::CPopupMenu():m_Loop(NULL), m_Data(NULL), m_hMenu(NULL),
	m_Count(0), m_Command(-1)
{
}

// Destructor
CPopupMenu::~CPopupMenu()
{
	int i;

	for (i=0; i<m_Count; i++) {
		if (m_Data[i].item) 
			g_object_unref(m_Data[i].item);
	}
	g_free(m_Data);
}

// Create and initialize the m_hBar
void CPopupMenu::Create(const tPopupMenuItem *items)
{
	m_hMenu = gtk_menu_new();

	// Compute number of items
	int count = 0;
	for (int i=0; items[i].type!=MB_END; i++)
		count++;
	if (count>0) {
		m_Data = static_cast<tPopupMenuItemData*>(g_realloc(m_Data, count*sizeof(tPopupMenuItemData)));
		for (int j=0; items[j].type!=MB_END; j++) {
			m_Data[j].item = NULL;
			switch (items[j].type)
			{
			case MB_ITEM:
				{
					m_Data[j].menu = this;
					m_Data[j].cmd_id = items[j].cmd_id;
					m_Data[j].item = gtk_image_menu_item_new_with_mnemonic(items[j].text);
					/*char *icon = get_icon_file(items[j].icon ? items[j].icon : "noicon");
					gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(m_Data[j].item), 
						gtk_image_new_from_file(icon));
					g_free(icon);*/
					gtk_menu_shell_append(GTK_MENU_SHELL(m_hMenu), GTK_WIDGET(g_object_ref(m_Data[j].item)));
					g_signal_connect(G_OBJECT(m_Data[j].item), "activate", G_CALLBACK(activate), &m_Data[j]);
					gtk_widget_show(m_Data[j].item);
				}
				break;

			case MB_SEPARATOR:
				{
					m_Data[j].item = gtk_separator_menu_item_new();
					gtk_menu_shell_append(GTK_MENU_SHELL(m_hMenu), GTK_WIDGET(g_object_ref(m_Data[j].item)));
					gtk_widget_show(m_Data[j].item);
				}
				break;

			default:
				break;
			}
		}
	}
	m_Count = count;
	g_signal_connect(G_OBJECT(m_hMenu), "selection-done", G_CALLBACK(selection_done), this);
	gtk_widget_show(m_hMenu);
}

void CPopupMenu::Enable(int cmd, bool enable)
{
	for (int i=0; i<m_Count; i++) {
		if (m_Data[i].cmd_id == cmd) 
			gtk_widget_set_sensitive(m_Data[i].item, enable);
	}
}

int CPopupMenu::Execute(const GdkEventButton *event)
{
	m_Command = -1;
	if (m_hMenu) {
		gtk_menu_popup(GTK_MENU(m_hMenu), NULL, NULL, NULL, NULL, event->button, event->time);
		m_Loop = g_main_loop_new(NULL, FALSE);
		gdk_threads_leave();  
		g_main_loop_run(m_Loop);
		gdk_threads_enter();
		g_main_loop_unref(m_Loop); 
		m_Loop = NULL;
	}
	return m_Command;
}

void CPopupMenu::selection_done(GtkWidget *widget, CPopupMenu *pMe)
{
	pMe->OnSelectionDone();
}

void CPopupMenu::OnSelectionDone(void)
{
	Callback(CB_SELECTION_DONE, m_Command);
	if (m_Loop)
		g_main_loop_quit(m_Loop);
}

void CPopupMenu::activate(GtkMenuItem *pItem, tPopupMenuItemData *pData)
{
	((tPopupMenuItemData*)pData)->menu->OnActivate(((tPopupMenuItemData*)pData)->cmd_id);
}

void CPopupMenu::OnActivate(int cmd_id)
{
	m_Command = cmd_id;
	Callback(CB_ACTIVATE, cmd_id);
}
