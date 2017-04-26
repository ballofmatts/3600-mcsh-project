#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mcshlist.h"
#include "memory.h"


/***************************************************
Singly-Linked List declarations and functions.
Last modified: 3/20/13
***************************************************/




/************************
initList(void *data)
input: char string for label, char string for label data
output: Node struct
Initializes node struct holding data and pointer
towards the next node.
************************/
Node *initList(char *label, char *data)
{
	Node *node;
	if(!(node=mcshMalloc(sizeof(Node))))
		return 0;
	if(!(node->label=(char*)mcshMalloc(50*sizeof(char))))
		return 0;
	if(!(node->data=(char*)mcshMalloc(50*sizeof(char))))
		return 0;

	strcpy(node->label,label);
	strcpy(node->data,data);
	node->next=NULL;
	return node;
}

/************************
mcshInsert(Node **currentList, char *label, char *data)
input: reference list, char string for label, char string for label data
output: no output, however function references currentList
Takes in list and adds the label to it. It checks if the label
exists already and if so, calls the mcshUpdate function to update the
data on that label.
Otherwise, it will add the label to the current list, placing it in
its sorted order.
************************/
void mcshInsert(Node **currentList, char *label, char *data)
{
	Node *newNode, *temp;

	//initializes node with inputted data
	newNode=initList(label, data);

	//temp will traverse the list. Since currentList is referenced, outside
	//of the function, traversing will destroy it.
	temp=*currentList;

	//this checks if this is the first element added, and adds it
	if(temp==NULL)
	{
		*currentList=newNode;
	}
	else
	{
		while(temp)
		{
			//will call update function if the label already exists
			if(strcmp(temp->label, label)==0)
			{
				//printf("%s found, calling update.\n",label);
				mcshUpdate(*currentList, label, data);
				break;
			}
			//catch if reached last element of list
			else if(temp->next==NULL)
			{
				if(strcmp(temp->label,label)>0)
				{
					newNode->next=temp;
					*currentList=newNode;
				}
				else
				{
					temp->next=newNode;
				}
				break;
			}
			//this essentially checks to see if the element is smaller than
			//the current head
			else if(strcmp(temp->label, label)>0)
			{
				newNode->next=temp;
				*currentList=newNode;
				break;
			}
			//this is for if the element fits in the middle of the list
			else if(strcmp(temp->next->label, label)>0)
			{
				newNode->next=temp->next;
				temp->next=newNode;
				break;
			}
			//update traverse pointers
			temp=temp->next;
		}
	}
	//printf("%s -> %s added.\n\n",label,data);
}

/************************
mcshUpdate(Node *currentList, char *label, char *data)
input: reference list, char string for label, char string for label data
output: no output
Finds inputted label in list and updates the data in it.
************************/
void mcshUpdate(Node *currentList, char *label, char *data)
{
	Node *temp;
	temp=mcshFind(currentList,label);
	if(temp!=NULL)
	{
		//printf("Changing %s -> %s to %s\n",label,temp->data,data);
		strcpy(temp->data,data);
	}
}

/************************
mcshRemove(Node **currentList, char *label)
input: reference list, char string for label
output: no output
Finds inputted label in list and removes it if found.
************************/
void mcshRemove(Node **currentList, char *label)
{
	Node *found;
	found=mcshFind(*currentList,label);	//looks for node

    if(found) {
        Node *temp;
        temp=*currentList;
        int i, size;
        size=listSize(*currentList);
		for(i=0;i<size;i++)
		{
			//check for first element wanting to be removed
			if(temp==found)
			{
				*currentList=(*currentList)->next;
				break;
			}
			//otherwise check to the next element being the one wanting
			//to be removed
			else if(temp->next==found)
			{
				temp->next=found->next;
				break;
			}
			temp=temp->next;
		}
		//frees removed element
		mcshFree(found);
		//printf("Removed %s.\n\n", label);
    }
    else
        printf("Element not found nor removed.\n\n");

}

/************************
mcshFind(Node *currentList, char *label)
input: reference list, char string for label
output: Node pointing to where label was found
Finds inputted label in list returns it.
************************/
Node *mcshFind(Node *currentList, char *label)
{
	while(currentList)	//just rolls down the list
	{
		if(strcmp(currentList->label,label)==0)
			return currentList;		//and returns it if found
		currentList=currentList->next;
	}
	//printf("%s not found.\n",label);
	return NULL;
}


/************************
mcshHead(Node *currentList)
input: reference list
output: Node pointing to head of list
returns the head of the list
************************/
Node *mcshHead(Node *currentList)
{
	return currentList;
}


/************************
mcshNext(Node *currentNode)
input: reference Node
output: Node pointing to next node in the referenced list
returns the next node in the list
************************/
Node *mcshNext(Node *currentNode)
{
	return currentNode->next;
}

/************************
listSize(Node *currentList)
input: reference list
output: int
returns the size of the list
************************/
int listSize(Node *currentList)
{
	int i=0;
	while(currentList)
	{
		i++;
		currentList=currentList->next;
	}
	return i;
}

/************************
printList(Node *currentList)
input: reference list
output: no output
prints the contents of the list
************************/
void printList(Node *currentList)
{
	int i=0;
	while(currentList)
	{
		i++;
		printNode(currentList);
		currentList=currentList->next;
	}
	printf("\n");
}


/************************
printNode(Node *currentList)
input: reference Node
output: no output
prints the contents of the node referenced
************************/
void printNode(Node *currentNode)
{
	printf("%s -> %s\n",currentNode->label,currentNode->data);

}
