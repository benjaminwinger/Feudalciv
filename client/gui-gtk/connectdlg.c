/********************************************************************** 
 Freeciv - Copyright (C) 1996 - A Kjeldberg, L Gregersen, P Unold
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
***********************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "version.h"

#include "chatline.h"
#include "clinet.h"
#include "colors.h"
#include "dialogs.h"
#include "gui_stuff.h"

#include "connectdlg.h"

extern char name[];
extern char server_host[];
extern int  server_port;

static GtkWidget *iname, *ihost, *iport;
static GtkWidget *connw, *quitw;

extern GtkWidget *toplevel;

static GtkWidget *dialog;

/* meta Server */
int  update_meta_dialog(GtkWidget *meta_list);
void meta_list_callback(GtkWidget *w, gint row, gint column);
void meta_update_callback(GtkWidget *w, gpointer data);


/**************************************************************************
...
**************************************************************************/
void connect_callback(GtkWidget *w, gpointer data)
{
  char errbuf [512];

  strcpy(name, gtk_entry_get_text(GTK_ENTRY(iname)));
  strcpy(server_host, gtk_entry_get_text(GTK_ENTRY(ihost)));
  server_port=atoi(gtk_entry_get_text(GTK_ENTRY(iport)));
  
  if(connect_to_server(name, server_host, server_port, errbuf)!=-1) {
    gtk_widget_destroy(dialog);
    gtk_widget_set_sensitive(toplevel,TRUE);
  }
  else
    append_output_window(errbuf);
}

/**************************************************************************
...
**************************************************************************/
int update_meta_dialog(GtkWidget *meta_list)
{
  char errbuf[128];

  if(get_meta_list(meta_list,errbuf)!=-1)  {
    return 1;
  } else {
    append_output_window(errbuf);
    return 0;
  }
}

/**************************************************************************
...
**************************************************************************/
void meta_update_callback(GtkWidget *w, gpointer data)
{
  update_meta_dialog(GTK_WIDGET(data));
}

/**************************************************************************
...
**************************************************************************/
void meta_list_callback(GtkWidget *w, gint row, gint column)
{
  gchar *name, *port;

  gtk_clist_get_text(GTK_CLIST(w), row, 0, &name);
  gtk_entry_set_text(GTK_ENTRY(ihost), name);
  gtk_clist_get_text(GTK_CLIST(w), row, 1, &port);
  gtk_entry_set_text(GTK_ENTRY(iport), port);
}

/**************************************************************************
...
**************************************************************************/
gint connect_deleted_callback(GtkWidget *w, GdkEvent *ev, gpointer data)
{
  gtk_main_quit();
  return FALSE;
}

/**************************************************************************
...
**************************************************************************/
void gui_server_connect(void)
{
  GtkWidget *label, *table, *book, *scrolled, *list, *vbox, *update, *label2;
  char *titles[6]= {"Server Name", "Port", "Version", "Status", "Players",
		    "Comment"};
  char buf [256];
  int i;

  gtk_widget_set_sensitive(toplevel, FALSE);

  dialog=gtk_dialog_new();
  gtk_signal_connect(GTK_OBJECT(dialog),"delete_event",
	GTK_SIGNAL_FUNC(connect_deleted_callback), NULL);
  
  gtk_window_set_title(GTK_WINDOW(dialog), " Connect to FreeCiv Server");

  book = gtk_notebook_new ();
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), book, TRUE, TRUE, 0);


  label=gtk_label_new("Freeciv Server Selection");

  vbox=gtk_vbox_new(FALSE, 2);
  gtk_notebook_append_page (GTK_NOTEBOOK (book), vbox, label);

  table = gtk_table_new (4, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_border_width (GTK_CONTAINER (table), 5);
  gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, TRUE, 0);

  label=gtk_label_new("Name:");
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, 0, 0, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);

  iname=gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(iname), name);
  gtk_table_attach_defaults (GTK_TABLE (table), iname, 1, 2, 0, 1);

  label=gtk_label_new("Host:");
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, 0, 0, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);

  ihost=gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(ihost), server_host);
  gtk_table_attach_defaults (GTK_TABLE (table), ihost, 1, 2, 1, 2);

  label=gtk_label_new("Port:");
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3, 0, 0, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);

  sprintf(buf, "%d", server_port);

  iport=gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(iport), buf);
  gtk_table_attach_defaults (GTK_TABLE (table), iport, 1, 2, 2, 3);

  if (IS_BETA_VERSION)
  {
    GtkStyle *style;

    label2=gtk_label_new ("THIS IS A BETA RELEASE\n"
			  "Freeciv 1.8 will be released third\n"
			  "week of March at http://www.freeciv.org");

    style=gtk_style_copy (label2->style);
    style->fg[GTK_STATE_NORMAL]=*colors_standard[COLOR_STD_RED];
    gtk_widget_set_style (label2, style);
    gtk_table_attach_defaults (GTK_TABLE (table), label2, 0, 2, 3, 4);
  }


  label=gtk_label_new("Metaserver");

  vbox=gtk_vbox_new(FALSE, 2);
  gtk_notebook_append_page (GTK_NOTEBOOK (book), vbox, label);

  list=gtk_clist_new_with_titles(6, titles);
  gtk_clist_column_titles_passive(GTK_CLIST(list));

  for(i=0; i<6; i++)
    gtk_clist_set_column_auto_resize(GTK_CLIST(list), i, TRUE);

  scrolled=gtk_scrolled_window_new(NULL,NULL);
  gtk_container_add(GTK_CONTAINER(scrolled), list);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
				 GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

  update=gtk_button_new_with_label("Update");
  gtk_box_pack_start(GTK_BOX(vbox), update, FALSE, FALSE, 2);

  gtk_signal_connect(GTK_OBJECT(list), "select_row",
			GTK_SIGNAL_FUNC(meta_list_callback), NULL);
  gtk_signal_connect(GTK_OBJECT(update), "clicked",
			GTK_SIGNAL_FUNC(meta_update_callback), (gpointer)list);

  connw=gtk_button_new_with_label("Connect");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), connw,
	TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS(connw, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(connw);

  quitw=gtk_button_new_with_label("Quit");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), quitw,
	TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS(quitw, GTK_CAN_DEFAULT);

  gtk_widget_grab_focus (iname);

  gtk_signal_connect(GTK_OBJECT(iname), "activate",
        	      GTK_SIGNAL_FUNC(connect_callback), NULL);
  gtk_signal_connect(GTK_OBJECT(ihost), "activate",
        	      GTK_SIGNAL_FUNC(connect_callback), NULL);
  gtk_signal_connect(GTK_OBJECT(iport), "activate",
        	      GTK_SIGNAL_FUNC(connect_callback), NULL);
  gtk_signal_connect(GTK_OBJECT(connw), "clicked",
        	      GTK_SIGNAL_FUNC(connect_callback), NULL);
  gtk_signal_connect(GTK_OBJECT(quitw), "clicked",
        	      GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

  gtk_widget_show_all(GTK_DIALOG(dialog)->vbox);
  gtk_widget_show_all(GTK_DIALOG(dialog)->action_area);

  gtk_widget_set_usize(dialog, 450, 200);
  gtk_set_relative_position(toplevel, dialog, 50, 50);
  gtk_widget_show(dialog);
}
