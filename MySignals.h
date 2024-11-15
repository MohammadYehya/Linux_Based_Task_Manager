#pragma once

#include "MyData.h"

gboolean on_draw1_draw (GtkWidget *widget, cairo_t *cr, gpointer data)
{
	guint width, height;
	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	
	cairo_set_line_width(cr, 7);
	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
	cairo_move_to(cr, 0, height);
	cairo_line_to(cr, width, height);
	cairo_stroke(cr);
	
	cairo_set_line_width(cr, 0.6);
	
	int maxx = 20, maxy = 10;
	gdouble tempx = 0, tempy = 0;
	for(int i = 0 ; i <= maxy ; i++)
	{
		cairo_move_to(cr, 0, (double)(tempy));
		cairo_line_to(cr, width, (double)tempy);
		tempy += (double)height/(double)maxy;
	}
	for(int i = 0 ; i <= maxx ; i++)
	{
		cairo_move_to(cr, (double)(tempx), 0);
		cairo_line_to(cr, (double)tempx, height);
		tempx += (double)width/(double)maxx;
	}
	cairo_stroke(cr);
	
	cairo_set_line_width(cr, 2.5);
	for (int i = 0; i < GraphPoints-1; i++)
	{
		cairo_set_source_rgb(cr, TMD.RGBGraph[0][0], TMD.RGBGraph[0][1], TMD.RGBGraph[0][2]);
		cairo_move_to(cr, (double)((i*5)+10), (double)(2*(100-TMD.cpu[i])+10));
		cairo_line_to(cr, (double)(((i+1)*5)+10), (double)(2*(100-TMD.cpu[i+1])+10));
		cairo_stroke(cr);
	}
	return FALSE;
}
gboolean on_draw2_draw (GtkWidget *widget, cairo_t *cr, gpointer data)
{
	guint width, height;
	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	
	cairo_set_line_width(cr, 7);
	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
	cairo_move_to(cr, 0, 0);
	cairo_line_to(cr, width, 0);
	cairo_stroke(cr);
	
	cairo_set_line_width(cr, 0.6);
	
	int maxx = 20, maxy = 10;
	gdouble tempx = 0, tempy = 0;
	for(int i = 0 ; i <= maxy ; i++)
	{
		cairo_move_to(cr, 0, (double)(tempy));
		cairo_line_to(cr, width, (double)tempy);
		tempy += (double)height/(double)maxy;
	}
	for(int i = 0 ; i <= maxx ; i++)
	{
		cairo_move_to(cr, (double)(tempx), 0);
		cairo_line_to(cr, (double)tempx, height);
		tempx += (double)width/(double)maxx;
	}
	cairo_stroke(cr);
	
	cairo_set_line_width(cr, 2.5);
	for (int i = 1 ; i < 5 ; i++)
	{
		for (int j = 0; j < GraphPoints-1; j++)
		{
			cairo_set_source_rgb(cr, TMD.RGBGraph[i][0], TMD.RGBGraph[i][1], TMD.RGBGraph[i][2]);
			cairo_move_to(cr, (double) ((j * 5) + 10), (double) (2 * (100 - TMD.mem[i][j]) + 10));
			cairo_line_to(cr, (double) (((j + 1) * 5) + 10), (double) (2 * (100 - TMD.mem[i][j + 1]) +10));
			cairo_stroke(cr);
		}
	}
	return FALSE;
}
void on_tv1Selection_changed(GtkWidget *c)
{
	gchar *value;
	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(c), &TMD.model, &TMD.SelectedRow) == FALSE) return;
	gtk_tree_model_get(TMD.model, &TMD.SelectedRow, 0, &value, -1);
	for(int i = 0; i < 4 ; i++) gtk_toggle_button_set_active((GtkToggleButton*)TMD.core[i], FALSE);
	cpu_set_t mask;
	TMD.Affinity = 0;
	sched_getaffinity(atoi(value), sizeof(cpu_set_t), &mask);
	for(int i = 0; i < 4 ; i++) if(CPU_ISSET(i, &mask)) gtk_toggle_button_set_active((GtkToggleButton*)TMD.core[i], TRUE);
}
void on_KillButton_clicked(GtkButton *b)
{
	if(!GTK_IS_TREE_MODEL(TMD.model))return;
	gchar *value;
	gtk_tree_model_get(TMD.model, &TMD.SelectedRow, 0, &value, -1);
	if(!kill(atoi(value), SIGKILL)) gtk_list_store_remove(GTK_LIST_STORE(TMD.ProcessorData), &TMD.SelectedRow);
}
void on_PriorityButton_clicked(GtkButton *b)
{
	if(!GTK_IS_TREE_MODEL(TMD.model))return;
	gchar *value;
	gtk_tree_model_get(TMD.model, &TMD.SelectedRow, 0, &value, -1);
	setpriority(PRIO_PROCESS, atoi(value), TMD.PriorityLevel);
}
void on_PriorityPicker_changed(GtkComboBox *c, GtkEntry *e)
{
	TMD.PriorityLevel = atoi(gtk_entry_get_text(e));
}
void on_AffinityButton_clicked(GtkButton *b)
{
	if(!GTK_IS_TREE_MODEL(TMD.model))return;
	gchar *value;
	cpu_set_t mask;
	CPU_ZERO(&mask);
	for(int i = 0 ; i < 4 ; i++) if((TMD.Affinity & 1 << i) == 1 << i) CPU_SET(i, &mask);
	gtk_tree_model_get(TMD.model, &TMD.SelectedRow, 0, &value, -1);
	sched_setaffinity(atoi(value), sizeof(mask), &mask);
}
void on_cpu_toggled(GtkCheckButton *b)
{
	int x = 0;
	sscanf(gtk_widget_get_name((GtkWidget*)b), "cpu%d", &x);
	TMD.Affinity ^= (1 << x);
}
