#include "SlimList.h"
#include "SlimUtil.h"
#include "SlimListDeserializer.h"
#include "SlimListSerializer.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

//static local variables
typedef struct Node Node;

struct Node {
	Node* next;
	char* string;
	SlimList* list;
};

struct SlimList {
	int length;
	Node* head;	
	Node* tail;
};

static void insertNode(SlimList* self, Node* node);

SlimList* SlimList_Create(void)
{
     SlimList* self = (SlimList*)malloc(sizeof(SlimList));
     memset(self, 0, sizeof(SlimList));
     return self;
}

void SlimList_Destroy(SlimList* self)
{
	Node * node;
	Node * next;
	for(node = self->head; node; node = next)
	{
		if (node->string) 
			free(node->string);
		
		if (node->list)
			SlimList_Destroy(node->list);
			
		next = node->next;
		free(node);
	}
    free(self);
}

void SlimList_AddBuffer(SlimList* self, char const* buffer, int length) 
{
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->next = 0;
	newNode->list = 0;
	
	insertNode(self, newNode);

	newNode->string = CSlim_BuyBuf(buffer, length);
}


void SlimList_AddString(SlimList* self, char const* string) 
{
	SlimList_AddBuffer(self, string, string ? strlen(string) : 0);
}

void SlimList_AddList(SlimList* self, SlimList* element)
{
	char * embedded = SlimList_Serialize(element);
	SlimList_AddString(self, embedded);
	free(embedded);
}

int SlimList_GetLength(SlimList* self)
{
	return self->length;
}


int SlimList_Equals(SlimList* self, SlimList* other){
	Node *p, *q;
	if (self->length != other->length)
		return 0;

	for (p = self->head, q=other->head; p; p=p->next, q=q->next) 
	{
		if (strcmp(p->string, q->string) != 0)
			return 0;	
	}
	
	return 1;
}

Node * SlimList_GetNodeAt(SlimList* self, int index)
{
	int i;
	Node* node = self->head;

	if (index >= self->length)
		return 0;

	for (i = 0; i < index; i++)
	{
		node = node->next;
	}
	return node;
}

SlimList * SlimList_GetListAt(SlimList* self, int index)
{
	Node * node = SlimList_GetNodeAt(self, index);
	if (node)
	{
		if (node->list == 0)
			node->list = SlimList_Deserialize(node->string);	
	}
	return node->list;
}

char * SlimList_GetStringAt(SlimList* self, int index)
{
	Node* node = SlimList_GetNodeAt(self, index);
	if(node == 0)
		return 0;
	return node->string;
}

void SlimList_ReplaceAt(SlimList* self, int index, char const * replacementString)
{
	Node* node = SlimList_GetNodeAt(self, index);
	if(node->list != 0){
		SlimList_Destroy(node->list);
		node->list = 0;
	}
	char * newString = CSlim_BuyString(replacementString);
	free(node->string);
	node->string = newString;
}

static void insertNode(SlimList* self, Node* node)
{
	if (self->length == 0)
	{
		self->head = node;
	}
	else
	{
		self->tail->next = node;
	}
	self->tail = node;	
	self->length++;
}

SlimList* SlimList_GetTailAt(SlimList* self, int index)
{
	SlimList * tail = SlimList_Create();
	int length = SlimList_GetLength(self);
	for(;index < length; index++) {
		SlimList_AddString(tail, SlimList_GetStringAt(self, index));
	}
	return tail;
}

char* SlimList_ToString(SlimList* self) {
	static char string[128];
	char buf[128];
	buf[0] = '\0';
	strncat(buf, "[", 128);
	int length = SlimList_GetLength(self);
	int i;
	for (i = 0; i<length; i++) {
		SlimList* sublist = SlimList_GetListAt(self, i);
		if (sublist != NULL) {
			strncat(buf, SlimList_ToString(sublist), 128);
		} else {
			strncat(buf, "\"", 128);
			strncat(buf, SlimList_GetStringAt(self, i), 128);
			strncat(buf, "\"", 128);
		}
		if (i != (length-1)) {
			strncat(buf, ", ", 128);
		}	
	}
	strncat(buf, "]", 128);
	strncpy(string, buf, 128);
	return string;	
}
