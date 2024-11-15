#pragma once

#include "MyMacros.h"

typedef char BYTE;

typedef struct
{
	GtkBuilder 		*builder;
	GtkWidget 		*window, *Fixed, *draw1, *draw2, 
					*PriorityPicker, *PriorityEntry, 
					*core[4];
	GtkTreeView		*tv1;
	GtkTreeSelection	*tv1Selection;
	GtkListStore		*ProcessorData, *PriorityStore;
	GtkTreeViewColumn	*PID, *Name, *CPU, *MEM;
	GtkCellRenderer		*PID_, *Name_, *CPU_, *MEM_;
	GtkTreeIter 		SelectedRow;
	GtkTreeModel 		*model;
	
	BYTE Affinity;
	int PriorityLevel;
	int TimerOn;
	int cpuUtil, cpu[GraphPoints];
	int memUtil[5], mem[5][GraphPoints];
	double RGBGraph[5][3];
} TMData;
TMData TMD;

typedef struct
{
	int *argc;
	char ***argv;
} ARG;

sem_t ProcessMutex;
sem_t CPUMutex;
sem_t MEMMutex;
