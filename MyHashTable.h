#include "MyData.h"
#define TableSize 300

typedef struct Node Node;
struct Node
{
	GtkTreeIter iter;
	char name[1024];
	int flag;
	Node* next;
	Node* pre;
};
void NodeConstructor(Node* x, char val[4][1024])
{
	strcpy(x->name, val[3]);
	x->flag = 1;
	x->next = NULL;
	x->pre = NULL;
	gtk_list_store_prepend(GTK_LIST_STORE(TMD.ProcessorData), &x->iter);
	gtk_list_store_set(GTK_LIST_STORE(TMD.ProcessorData), &x->iter, 0, val[0], 1, val[1], 2, val[2], 3, val[3], -1);
}
typedef struct HashTable
{
	Node** Table;
} HashTable;
void HashTableConstructor(HashTable* a) 
{
	a->Table = (Node**)malloc(TableSize*sizeof(Node*));
	for (int i = 0; i < TableSize; i++) a->Table[i] = NULL;
}
void HashTableDestructor(HashTable *a)
{
	for(int i = 0 ; i < TableSize ; i++)
	{
		Node* temp = a->Table[i];
		while(temp != NULL)
		{
			Node* temp2 = temp;
			temp = temp->next;
			temp2->next = NULL;
			temp2->pre = NULL;
			free(temp2);
			temp2 = NULL;
		}
	}
	free(a->Table);
	a->Table = NULL;
}
int Hash(char *value) 
{
	int val = atoi(value);
	int temp1 = val;
	while(temp1 >= 10) temp1 /= 10;
	int temp2  = val%100;
	return (temp1 * 100 + temp2) % TableSize;
}
int Search(HashTable* a, char val[4][1024])
{
	int h = Hash(val[3]);
	Node* temp = a->Table[h];
	while(temp != NULL)
	{
		if(!strcmp(temp->name, val[3]))
		{
			if(gtk_list_store_iter_is_valid(GTK_LIST_STORE(TMD.ProcessorData), &temp->iter))
			{
				gtk_list_store_set(GTK_LIST_STORE(TMD.ProcessorData), &temp->iter, 0, val[0], 1, val[1], 2, val[2], 3, val[3], -1);
				temp->flag = 1;
				return h;
			}
		}
		temp = temp->next;
	}
	return -1;
}
void Insert(HashTable* a, char val[4][1024])
{
	int h = Hash(val[3]);
	Node* newnode = (Node*)malloc(sizeof(Node));
	if(TMD.TimerOn)
	{
		NodeConstructor(newnode, val);
		if (a->Table[h] == NULL) a->Table[h] = newnode;
		else
		{
			Node* temp = a->Table[h];
			while (temp->next != NULL) temp = temp->next;
			temp->next = newnode;
			newnode->pre = temp;
		}
	}
	else free(newnode);
}
void Delete(HashTable* a) 
{
	for(int i = 0 ; i < TableSize ; i++)
	{
		Node* temp = a->Table[i];
		while(temp != NULL)
		{
			if(temp->flag == 0) 
			{
				if(gtk_list_store_iter_is_valid(GTK_LIST_STORE(TMD.ProcessorData), &temp->iter))
				{
					gtk_list_store_remove(GTK_LIST_STORE(TMD.ProcessorData), &temp->iter);
					if(temp->next == NULL && temp->pre == NULL)
					{
						Node* temp2 = temp;
						temp = temp->next;
						free(temp2);
						temp2 = NULL;
					}
					else if(temp->next == NULL)
					{
						Node* temp2 = temp;
						temp = temp->next;
						temp2->pre->next = NULL;
						temp2->pre = NULL;
						free(temp2);
					}
					else if(temp->pre == NULL)
					{
						Node* temp2 = temp;
						a->Table[i] = a->Table[i]->next;
						temp = temp->next;
						temp2->next->pre = NULL;
						temp2->next = NULL;
						free(temp2);
					}
					else
					{
						Node* temp2 = temp;
						temp = temp->next;
						temp2->pre->next = temp2->next;
						temp2->next->pre = temp2->pre;
						free(temp2);
					}
				}
				else temp = temp->next;
			}
			else
			{
				temp->flag = 0;
				temp = temp->next;
			}
		}
	}
}
