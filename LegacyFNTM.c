#include <gtk/gtk.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sched.h>
#include <wait.h>
#define HARDWARE_CONCURRENCY (4)
#define GraphPoints 180

GtkBuilder 		*builder;
GtkWidget *window, *Fixed, *draw1, *draw2, *PriorityPicker, *PriorityEntry, *core[4];
GtkTreeView		*tv1;
GtkTreeSelection	*tv1Selection;
GtkListStore		*ProcessorData;
GtkListStore		*PriorityStore;
GtkTreeViewColumn	*PID;
GtkTreeViewColumn	*Name;
GtkTreeViewColumn	*CPU;
GtkTreeViewColumn	*MEM;
GtkCellRenderer		*PID_;
GtkCellRenderer		*Name_;
GtkCellRenderer		*CPU_;
GtkCellRenderer		*MEM_;
GtkTreeIter 		SelectedRow;
GtkTreeModel 		*model;

typedef struct
{
	int *argc;
	char ***argv;
} ARG;

typedef char BYTE;
BYTE Affinity;

int PriorityLevel = 0;
int TimerOn = 1;
int cpuUtil, cpu[GraphPoints] = {0};
int memUtil[5], mem[5][GraphPoints] = {0};
double RGBGraph[5][3] = {{0.0,0.7,0.7},{1,0,0},{0,1,0},{1,1,0},{1,0.5,0}};

void Exit_Program()
{
	TimerOn = 0;
	gtk_main_quit();
}
void *ProcessorInformationHandler(void *args)
{
	char *vals[] = {NULL,"--sort=pid","--sort=%cpu", "--sort=%mem", "--sort=command"};
	char line[1024] = {0}, Info[4][1024] = {0};
	//while(TimerOn)
	if(TimerOn)
	{
		if(!fork()) execlp("/bin/sh","/bin/sh","ProcessInfo.sh", vals[2], NULL);
		else wait(NULL);
		FILE *fptr = fopen("Processes.txt", "r");
		int x = 0;
		GtkTreeIter iter;
		while(TimerOn)
		{
			if (!fgets(line,1024,fptr) || !TimerOn)
			{
				fclose(fptr);
				break;
			}
			if(!x++) continue;
			sscanf(line,"%s %s %s %[^\n]", Info[0], Info[1], Info[2], Info[3]);
			gtk_list_store_prepend(ProcessorData, &iter);
			gtk_list_store_set(ProcessorData, &iter, 0, Info[0], 1, Info[1], 2, Info[2], 3, Info[3], -1);
		}
		if(TimerOn)sleep(4);
		//gtk_list_store_clear(ProcessorData); if (ProcessorData == NULL) printf("WTF\n");
		//while(gtk_list_store_iter_is_valid(ProcessorData, &iter))gtk_list_store_remove(ProcessorData, &iter);
		//gtk_list_store_remove(ProcessorData, &iter);
//		printf("\n\nYoligigoooooh!\n\n");
	}
	pthread_exit(0);
}
void *CpuHandler(void *args)
{
	while(TimerOn)
	{
		static long time1 = 0, time2 = 0;
		static int flag = 0;
		char line[128], dummy[32];
		
		FILE *p1 = fopen("/proc/stat","r");
		fgets(line,128,p1);
		sscanf(line,"%s %ld", dummy, &time2);
		fclose(p1);
		
		if (!flag)
		{
			flag = 1;
			time1 = time2;
			sleep(1);
			continue;
		}
		
		cpuUtil = time2 - time1;
		for (int i = 0; i < GraphPoints-1 ; i++) cpu[i] = cpu[i+1];
		cpu[GraphPoints-1] = cpuUtil/(HARDWARE_CONCURRENCY);
//		printf("percent of cpu utilization = %d\n", cpuUtil );
		time1 = time2;
		gtk_widget_queue_draw(draw1);
		sleep(1);
	}
	pthread_exit(0);
}
void *MemHandler(void *args)
{
	while(TimerOn)
	{
		static long time1[5] = {0}, time2[5] = {0};
		static int flag = 0;
		char line[128], dummy[32];
		
		FILE *p1 = fopen("/proc/meminfo","r");
		for(int i = 0 ; i < 5 ; i++)
		{
			fgets(line,128,p1);
			sscanf(line,"%[^0-9] %ld", dummy, &time2[i]);
//			printf("PandeMan %s\n",dummy);
		}
		fclose(p1);
		
		if (!flag)
		{
			flag = 1;
			for(int i = 0 ; i < 5 ; i++)time1[i] = time2[i];
			sleep(1);
			continue;
		}
		for(int i = 1 ; i < 5 ; i++)
		{
			memUtil[i] = time2[i] * 100 / time1[0];
			for (int j = 0; j < GraphPoints-1 ; j++) mem[i][j] = mem[i][j+1];
			mem[i][GraphPoints-1] = memUtil[i];
//			printf("percent of mem%d utilization = %d\n", i, memUtil[i]);
			time1[i] = time2[i];
		}
		gtk_widget_queue_draw(draw2);
		sleep(1);
	}
	pthread_exit(0);
}
void *Gui(void *args)
{
	ARG *x = (ARG*)args;
	gtk_init(x->argc, x->argv);
	
	builder = gtk_builder_new_from_file("GUIFormat.glade");
	window = GTK_WIDGET(gtk_builder_get_object(builder, "MainWindow"));
	g_signal_connect(window, "destroy", G_CALLBACK(Exit_Program), NULL);
	
	gtk_builder_connect_signals(builder, NULL);
	
	draw1 = GTK_WIDGET(gtk_builder_get_object(builder, "draw1"));
	draw2 = GTK_WIDGET(gtk_builder_get_object(builder, "draw2"));
	
	PriorityStore	= GTK_LIST_STORE(gtk_builder_get_object(builder, "PriorityStore"));
	PriorityPicker 	= GTK_WIDGET(gtk_builder_get_object(builder, "PriorityPicker"));
	PriorityEntry 	= GTK_WIDGET(gtk_builder_get_object(builder, "PriorityEntry"));
	ProcessorData	= GTK_LIST_STORE(gtk_builder_get_object(builder, "ProcessorData"));
	tv1		= GTK_TREE_VIEW(gtk_builder_get_object(builder, "tv1"));
	PID		= GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "PID"));
	Name		= GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "Name"));
	CPU		= GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "CPU%"));
	MEM		= GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "MEM%"));
	PID_		= GTK_CELL_RENDERER(gtk_builder_get_object(builder, "PID_"));
	Name_		= GTK_CELL_RENDERER(gtk_builder_get_object(builder, "Name_"));
	CPU_		= GTK_CELL_RENDERER(gtk_builder_get_object(builder, "CPU%_"));
	MEM_		= GTK_CELL_RENDERER(gtk_builder_get_object(builder, "MEM%_"));
	tv1Selection	= GTK_TREE_SELECTION(gtk_builder_get_object(builder, "tv1Selection"));
	
	for(int i = 0 ; i < 4 ; i++)
	{
		char temp[10];
		sprintf(temp, "cpu%d", i);
		core[i] = GTK_WIDGET(gtk_builder_get_object(builder, temp));
	}
	
	gtk_tree_view_column_add_attribute(PID, PID_, "text", 0);
	gtk_tree_view_column_add_attribute(Name, Name_, "text", 3);
	gtk_tree_view_column_add_attribute(CPU, CPU_, "text", 1);
	gtk_tree_view_column_add_attribute(MEM, MEM_, "text", 2);
	
	tv1Selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tv1));
	
	gtk_widget_show(window);

	pthread_t Cpu, Mem, PIH;
	pthread_create(&Cpu, NULL, CpuHandler, NULL);
	pthread_create(&Mem, NULL, MemHandler, NULL);
	pthread_create(&PIH, NULL, ProcessorInformationHandler, NULL);
	
	for(int i = -20 ; i < 20 ; i++)
	{
		GtkTreeIter iter;
		gtk_list_store_prepend(PriorityStore, &iter);
		char temp[10];
		sprintf(temp, "%d", i);
		gtk_list_store_set(PriorityStore, &iter, 0, temp, -1);
	}
	
	gtk_main();
	g_object_unref(builder);
	
	pthread_join(Cpu, NULL);
	pthread_join(Mem, NULL);
	pthread_join(PIH, NULL);
	pthread_exit(0);
}
int main(int argc, char *argv[], char *envp[])
{
	ARG x = {.argc= &argc, .argv = &argv};
	pthread_t GuiThread;
	pthread_create(&GuiThread, NULL, Gui, (void*)&x);
	pthread_join(GuiThread, NULL);
	return 0;
}
gboolean on_draw1_draw (GtkWidget *widget, cairo_t *cr, gpointer data)
{
	guint width, height;
	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	cairo_set_line_width(cr, 1.0);
	for (int i = 0; i < GraphPoints-1; i++)
	{
		cairo_set_source_rgb(cr, RGBGraph[0][0], RGBGraph[0][1], RGBGraph[0][2]);
		cairo_move_to(cr, (double) ((i * 5) + 10), (double) (2 * (100 - cpu[i]) + 10));
		cairo_line_to(cr, (double) (((i + 1) * 5) + 10), (double) (2 * (100 - cpu[i + 1]) +10));
		cairo_stroke(cr);
	}
	return FALSE;
}
gboolean on_draw2_draw (GtkWidget *widget, cairo_t *cr, gpointer data)
{
	guint width, height;
	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	cairo_set_line_width(cr, 1.0);
	for (int i = 1 ; i < 5 ; i++)
	{
		for (int j = 0; j < GraphPoints-1; j++)
		{
			cairo_set_source_rgb(cr, RGBGraph[i][0], RGBGraph[i][1], RGBGraph[i][2]);
			cairo_move_to(cr, (double) ((j * 5) + 10), (double) (2 * (100 - mem[i][j]) + 10));
			cairo_line_to(cr, (double) (((j + 1) * 5) + 10), (double) (2 * (100 - mem[i][j + 1]) +10));
			cairo_stroke(cr);
		}
	}
	return FALSE;
}
void on_tv1Selection_changed(GtkWidget *c)
{
	gchar *value;
	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(c), &model, &SelectedRow) == FALSE) return;
	gtk_tree_model_get(model, &SelectedRow, 0, &value, -1);
	for(int i = 0; i < 4 ; i++) gtk_toggle_button_set_active((GtkToggleButton*)core[i], FALSE);
	cpu_set_t mask;
	Affinity = 0;
	sched_getaffinity(atoi(value), sizeof(cpu_set_t), &mask);
	for(int i = 0; i < 4 ; i++) if(CPU_ISSET(i, &mask)) gtk_toggle_button_set_active((GtkToggleButton*)core[i], TRUE);
}
void on_KillButton_clicked(GtkButton *b)
{
	if(!GTK_IS_TREE_MODEL(model))return;
	gchar *value;
	gtk_tree_model_get(model, &SelectedRow, 0, &value, -1);
	if(!kill(atoi(value), SIGKILL)) gtk_list_store_remove(ProcessorData, &SelectedRow);
}
void on_PriorityButton_clicked(GtkButton *b)
{
	if(!GTK_IS_TREE_MODEL(model))return;
	gchar *value;
	gtk_tree_model_get(model, &SelectedRow, 0, &value, -1);
	setpriority(PRIO_PROCESS, atoi(value), PriorityLevel);
}
void on_PriorityPicker_changed(GtkComboBox *c, GtkEntry *e)
{
	PriorityLevel = atoi(gtk_entry_get_text(e));
}
void on_AffinityButton_clicked(GtkButton *b)
{
	if(!GTK_IS_TREE_MODEL(model))return;
	gchar *value;
	cpu_set_t mask;
	CPU_ZERO(&mask);
	for(int i = 0 ; i < 4 ; i++) if((Affinity & 1 << i) == 1 << i) CPU_SET(i, &mask);
	gtk_tree_model_get(model, &SelectedRow, 0, &value, -1);
	sched_setaffinity(atoi(value), sizeof(mask), &mask);
}
void on_cpu_toggled(GtkCheckButton *b)
{
	int x = 0;
	sscanf(gtk_widget_get_name((GtkWidget*)b), "cpu%d", &x);
	Affinity ^= (1 << x);
}
