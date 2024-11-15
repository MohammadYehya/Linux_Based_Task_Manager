#pragma once

#include "MySignals.h"
#include "MyHashTable.h"

void Exit_Program()
{
	TMD.TimerOn = 0;
	gtk_main_quit();
}
void *ProcessHandler(void *args)
{
	sem_wait(&ProcessMutex);
	
	char *vals[] = {NULL,"--sort=pid","--sort=%cpu", "--sort=%mem", "--sort=command"};
	char line[1024] = {0}, Info[4][1024] = {0};
	gtk_list_store_clear(TMD.ProcessorData);
	HashTable HT;
	HashTableConstructor(&HT);
#ifndef Update
#define Update 1
	while(TMD.TimerOn)
#else
	if(TMD.TimerOn)
#endif
	{
		if(!fork()) execlp("/bin/sh","/bin/sh","ProcessInfo.sh", vals[2], NULL);
		else wait(NULL);
		FILE *fptr = fopen("Processes.txt", "r");
		int x = 0;
		GtkTreeIter iter;
		while(TMD.TimerOn)
		{
			if (!fgets(line,1024,fptr) || !TMD.TimerOn)
			{
				fclose(fptr);				
				break;
			}
			if(!x++) continue;
			int check = sscanf(line,"%s %s %s %[^\n]", Info[0], Info[1], Info[2], Info[3]);
			if(TMD.TimerOn && Search(&HT, Info) == -1) Insert(&HT, Info);
		}
		if(TMD.TimerOn)
		{
			sleep(1);
			Delete(&HT);
		}
	}
	while(TMD.TimerOn);
	HashTableDestructor(&HT);
}
void *CpuHandler(void *args)
{
	sem_wait(&CPUMutex);
	
	while(TMD.TimerOn)
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
		
		TMD.cpuUtil = time2 - time1;
		for (int i = 0; i < GraphPoints-1 ; i++) TMD.cpu[i] = TMD.cpu[i+1];
		TMD.cpu[GraphPoints-1] = TMD.cpuUtil/(HARDWARE_CONCURRENCY);
		time1 = time2;
		gtk_widget_queue_draw(TMD.draw1);
		sleep(1);
	}
	pthread_exit(0);
}
void *MemHandler(void *args)
{
	sem_wait(&MEMMutex);
	
	while(TMD.TimerOn)
	{
		static long time1[5] = {0}, time2[5] = {0};
		static int flag = 0;
		char line[128], dummy[32];
		
		FILE *p1 = fopen("/proc/meminfo","r");
		for(int i = 0 ; i < 5 ; i++)
		{
			fgets(line,128,p1);
			sscanf(line,"%[^0-9] %ld", dummy, &time2[i]);
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
			TMD.memUtil[i] = time2[i] * 100 / time1[0];
			for (int j = 0; j < GraphPoints-1 ; j++) TMD.mem[i][j] = TMD.mem[i][j+1];
			TMD.mem[i][GraphPoints-1] = TMD.memUtil[i];
			time1[i] = time2[i];
		}
		gtk_widget_queue_draw(TMD.draw2);
		sleep(1);
	}
	pthread_exit(0);
}
void *Gui(void *args)
{
	ARG *x = (ARG*)args;
	gtk_init(x->argc, x->argv);
	
	TMD.builder = gtk_builder_new_from_file("GUIFormat.glade");
	TMD.window = GTK_WIDGET(gtk_builder_get_object(TMD.builder, "MainWindow"));
	g_signal_connect(TMD.window, "destroy", G_CALLBACK(Exit_Program), NULL);
	
	gtk_builder_connect_signals(TMD.builder, NULL);
	
	TMD.draw1 = GTK_WIDGET(gtk_builder_get_object(TMD.builder, "draw1"));
	TMD.draw2 = GTK_WIDGET(gtk_builder_get_object(TMD.builder, "draw2"));
	
	TMD.PriorityStore	= GTK_LIST_STORE(gtk_builder_get_object(TMD.builder, "PriorityStore"));
	TMD.PriorityPicker 	= GTK_WIDGET(gtk_builder_get_object(TMD.builder, "PriorityPicker"));
	TMD.PriorityEntry 	= GTK_WIDGET(gtk_builder_get_object(TMD.builder, "PriorityEntry"));
	TMD.ProcessorData	= GTK_LIST_STORE(gtk_builder_get_object(TMD.builder, "ProcessorData"));
	TMD.tv1			= GTK_TREE_VIEW(gtk_builder_get_object(TMD.builder, "tv1"));
	TMD.PID			= GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(TMD.builder, "PID"));
	TMD.Name		= GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(TMD.builder, "Name"));
	TMD.CPU			= GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(TMD.builder, "CPU%"));
	TMD.MEM			= GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(TMD.builder, "MEM%"));
	TMD.PID_		= GTK_CELL_RENDERER(gtk_builder_get_object(TMD.builder, "PID_"));
	TMD.Name_		= GTK_CELL_RENDERER(gtk_builder_get_object(TMD.builder, "Name_"));
	TMD.CPU_		= GTK_CELL_RENDERER(gtk_builder_get_object(TMD.builder, "CPU%_"));
	TMD.MEM_		= GTK_CELL_RENDERER(gtk_builder_get_object(TMD.builder, "MEM%_"));
	TMD.tv1Selection	= GTK_TREE_SELECTION(gtk_builder_get_object(TMD.builder, "tv1Selection"));
	
	for(int i = 0 ; i < 4 ; i++)
	{
		char temp[10];
		sprintf(temp, "cpu%d", i);
		TMD.core[i] = GTK_WIDGET(gtk_builder_get_object(TMD.builder, temp));
	}
	
	gtk_tree_view_column_add_attribute(TMD.PID, TMD.PID_, "text", 0);
	gtk_tree_view_column_add_attribute(TMD.Name, TMD.Name_, "text", 3);
	gtk_tree_view_column_add_attribute(TMD.CPU, TMD.CPU_, "text", 1);
	gtk_tree_view_column_add_attribute(TMD.MEM, TMD.MEM_, "text", 2);
	
	TMD.tv1Selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(TMD.tv1));
	
	gtk_widget_show_all(TMD.window);

	pthread_t Cpu, Mem, PIH;
	pthread_create(&Cpu, NULL, CpuHandler, NULL);
	pthread_create(&Mem, NULL, MemHandler, NULL);
	pthread_create(&PIH, NULL, ProcessHandler, NULL);
	
	for(int i = -20 ; i < 20 ; i++)
	{
		GtkTreeIter iter;
		gtk_list_store_prepend(GTK_LIST_STORE(TMD.PriorityStore), &iter);
		char temp[10];
		sprintf(temp, "%d", i);
		gtk_list_store_set(GTK_LIST_STORE(TMD.PriorityStore), &iter, 0, temp, -1);
	}
	
	sem_post(&ProcessMutex);
	sem_post(&CPUMutex);
	sem_post(&MEMMutex);
	
	gtk_main();
	g_object_unref(TMD.builder);
	
	pthread_join(Cpu, NULL);
	pthread_join(Mem, NULL);
	pthread_join(PIH, NULL);
	
	pthread_exit(0);
}
int TaskManagerStart(ARG *x)
{
	sem_init(&ProcessMutex, 0, 0);
	sem_init(&CPUMutex, 0, 0);
	sem_init(&MEMMutex, 0, 0);
	TMD.PriorityLevel = 0;
	TMD.TimerOn = 1;
	for(int i = 0 ; i < GraphPoints ; i++) TMD.cpu[i] = 0;
	for(int i = 0 ; i < 5 ; i++) for(int j = 0 ; j < GraphPoints ; j++) TMD.mem[i][j] = 0;
	double temp[5][3] = {{0.0,0.7,0.7},{1,0,0},{0,1,0},{1,1,0},{1,0.5,0}};
	for(int i = 0 ; i < 5 ; i++) for(int j = 0 ; j < 3 ; j++) TMD.RGBGraph[i][j] = temp[i][j];
	pthread_t GuiThread;
	pthread_create(&GuiThread, NULL, Gui, (void*)x);
	pthread_join(GuiThread, NULL);
	return 0;
}
