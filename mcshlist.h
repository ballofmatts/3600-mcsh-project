#ifndef _MCSHLIST_H_
#define _MCSHLIST_H_

/***************************************************
Singly-Linked List declarations and functions.
Last modified: 3/20/13
***************************************************/

typedef struct node_d {
	char *label;
	char *data;
	struct node_d *next;
} Node;

/************************
initList(void *data)
input: char string for label, char string for label data
output: Node struct
Initializes node struct holding data and pointer
towards the next node.
************************/
Node *initList(char *label, char *data);


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
void mcshInsert(Node **currentList, char *label, char *data);


/************************
mcshUpdate(Node *currentList, char *label, char *data)
input: reference list, char string for label, char string for label data
output: no output
Finds inputted label in list and updates the data in it.
************************/
void mcshUpdate(Node *currentList, char *label, char *data);


/************************
mcshRemove(Node **currentList, char *label)
input: reference list, char string for label
output: no output
Finds inputted label in list and removes it if found.
************************/
void mcshRemove(Node **currentList, char *label);


/************************
mcshFind(Node *currentList, char *label)
input: reference list, char string for label
output: Node pointing to where label was found
Finds inputted label in list returns it.
************************/
Node *mcshFind(Node *currentList, char *label);


/************************
mcshHead(Node *currentList)
input: reference list
output: Node pointing to head of list
returns the head of the list
************************/
Node *mcshHead(Node *currentList);


/************************
mcshNext(Node *currentNode)
input: reference Node
output: Node pointing to next node in the referenced list
returns the next node in the list
************************/
Node *mcshNext(Node *currentNode);


/************************
listSize(Node *currentList)
input: reference list
output: int
returns the size of the list
************************/
int listSize(Node *currentList);


/************************
printList(Node *currentList)
input: reference list
output: no output
prints the contents of the list
************************/
void printList(Node *currentList);


/************************
printNode(Node *currentList)
input: reference Node
output: no output
prints the contents of the node referenced
************************/
void printNode(Node *currentNode);

#endif
