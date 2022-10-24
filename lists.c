#include "tools.h"
#include "lists.h"


// Add a new node structure to a list, at the end.
// Returns a pointer to the new tail of the list.
nodePtr addNode (nodePtr list, void* newData)
{
	nodePtr		newNode = NULL;

	newNode = (nodePtr) xmalloc (sizeof (node) );
	newNode->data = newData;
	newNode->next = NULL;

	// Check that there is a list already.
	//  Otherwise, the list will be initialized with the newNode.
	if (list != NULL)
		list->next = newNode;

	return (newNode);
}


// Add a new node structure to a list, at the front.
// Returns a pointer to the head of the list.
nodePtr addFrontNode (nodePtr list, void* newData)
{
	nodePtr		newNode = NULL;

	newNode = (nodePtr) xmalloc (sizeof (node) );
	newNode->data = newData;
	newNode->next = list;

	return (newNode);
}


// Insert a new node at the position after the index provided as
//  the argument.
// If the index is 0, the element will be inserted in the second position.
// If the index specified is less than the length of the list, the item
//  will be inserted at the end of the list.
void insertDataAfterIndex (nodePtr list, void* newData, int index)
{
	nodePtr		pointer = list;
	nodePtr		newNode = NULL;
	int		counter = 0;

	while ( (pointer != NULL) && (counter < index) )
	{
		if (pointer->next == NULL)
			break;
		pointer = pointer->next;
		counter++;
	}

	newNode = (nodePtr) xmalloc (sizeof (node) );
	newNode->data = newData;
	newNode->next = pointer->next;
	pointer->next = newNode;
}


// Insert a list at the position after the index provided as
//  the argument.
// If the index is 0, the list will be inserted in the second position.
// If the index specified is less than the length of the list, the new list
//  will be inserted at the end of the list.
void insertListAfterIndex (nodePtr list, nodePtr newList, int index)
{
	nodePtr		pointer1 = list;
	nodePtr		tail = NULL;
	int		counter = 0;

	while ( (pointer1 != NULL) && (counter < index) )
	{
		if (pointer1->next == NULL)
			break;
		pointer1 = pointer1->next;
		counter++;
	}

	tail = getListTail (newList);

	tail->next = pointer1->next;
	pointer1->next = newList;
}


// Remove an item from a list, located at the possition used as a parameter.
// Return the pointer to the data in the deleted node.
void* removeNode (nodePtr* list, int itemPosition)
{
	nodePtr		pointer = *list;
	nodePtr		deleted;
	void*		item;
	int		i;

	// For the first item in the list.
	if (itemPosition == 0)
	{
		deleted = *list;
		*list = deleted->next;
	}
	// For any other item.
	else
	{
		for (i=0; i<itemPosition-1; i++)
			pointer = advanceList (pointer, "removeNode");

		deleted = pointer->next;
		pointer->next = deleted->next;
	}

	deleted->next = NULL;
	item = deleted->data;
	free (deleted);

	return (item);
}


// Returns the pointer to the data structure stored in the list,
//  at the position indicated as a parameter.
void* getNodeData (nodePtr list, int itemPosition)
{
	nodePtr		pointer = list;
	int		i;

	for (i=0; i<itemPosition; i++)
		pointer = advanceList (pointer, "getNodeData");

	return (pointer->data);
}


// Allocate memory for a duplicate of a node pointer.
// Make a copy of the data stored in the pointer.
void* copyNode (void* node, size_t dataSize)
{
	void*		newData = NULL;

	newData = xmalloc (dataSize);
	memcpy (newData, node, dataSize);

	return (newData);
}


// Copy the list into another, without messing the pointers.
// Returns the pointer to the new list.
nodePtr copyList (nodePtr list, size_t dataSize)
{
	nodePtr		pointer = list;
	nodePtr		newList = NULL;
	nodePtr		listTail = NULL;
	void*		item = NULL;
	void*		newItem = NULL;
	int			counter = 0;

	while (pointer != NULL)
	{
		item = getNodeData (list, counter);
		newItem = copyNode (item, dataSize);
		listTail = addNode (listTail, newItem);

		// Initialize the head of the list.
		if (newList == NULL)
			newList = listTail;

		pointer = pointer->next;
		counter++;
	}

	return (newList);
}


// Return a new list with the nodes in the original in the
//  inverse order.
nodePtr invertList (nodePtr list, size_t dataSize)
{
	nodePtr		pointer = list;
	nodePtr		newList = NULL;
	void*		item = NULL;
	void*		newItem = NULL;
	int		counter = 0;

	while (pointer != NULL)
	{
		item = getNodeData (list, counter);
		newItem = copyNode (item, dataSize);
		newList = addFrontNode (newList, newItem);

		pointer = pointer->next;
		counter++;
	}

	return (newList);
}


// Concatenate two lists, by setting the last pointer of
//  the 'head' list equal to the 'tail' list.
void joinLists (nodePtr* head, nodePtr tail)
{
	nodePtr 	pointer = *head;

	if (*head == NULL)
		*head = tail;
	else
	{
		while (pointer->next != NULL)
			pointer = pointer->next;

		pointer->next = tail;
	}
}


// Return a pointer to the last item in a list.
nodePtr getListTail (nodePtr list)
{
	nodePtr		tail = NULL;

	if (list != NULL)
	{
		tail = list;

		while (tail->next != NULL)
			tail = tail->next;
	}

	return (tail);

}


// Put the first element of a list at the end.
// Returns a pointer to the element that was moved.
void* rotateListNode (nodePtr* list)
{
	void*	tempData = NULL;
	nodePtr	tail = NULL;

	tail = getListTail (*list);

	// Don't do anything if the list has only
	//  one element. If tail and head are the same.
	if (tail == *list)
		return (*list);

	tempData = removeNode (list, 0);
	tail = addNode (tail, tempData);

	return (tempData);
}


// Return the number of nodes in a list.
int getListLength (nodePtr list)
{
	nodePtr		pointer = list;
	int		counter = 0;

	while (pointer != NULL)
	{
		pointer = pointer->next;
		counter++;
	}

	return (counter);
}


// Free allocated memery for an array of linked lists
//  and for the pointers in the data variable.
void freeListArray (nodePtr* listArray, int numLists)
{
	int		i;

	for (i=0; i<numLists; i++)
		freeList (listArray[i]);

	free (listArray);
}


// Free allocated memory for a list, containing lists
//  as its data.
void freeListOfLists (nodePtr mainList)
{
	nodePtr		pointer = mainList;
	nodePtr		nextNode = NULL;

	while (pointer != NULL)
	{
		nextNode = pointer->next;
		freeList ( (nodePtr) pointer->data);
		free (pointer);
		pointer = nextNode;
	}

	mainList = NULL;
}


// Free allocated memery for the linked lists
//  and for the pointers in the data variable.
void freeList (nodePtr list)
{
	nodePtr		pointer = list;
	nodePtr 	nextNode = NULL;

	while (pointer != NULL)
	{
		nextNode = pointer->next;
		free (pointer->data);
		free (pointer);
		pointer = nextNode;
	}
}


nodePtr advanceList (nodePtr pointer, char* functionName)
{
	if (pointer->next != NULL)
		return (pointer->next);
	else
	{
		printf ("ERROR when advancing the list, in '%s':\nNot enough items in list\n", functionName);
		exit (1);
	}
}
