// LIST OF STRUCTURES
typedef struct node
{
	void*	 	data;
	struct node*	next;
} node;

typedef node* nodePtr;


// Declaration of functions to manipulate lists.

nodePtr addNode (nodePtr list, void* newData);
nodePtr addFrontNode (nodePtr list, void* newData);
void insertDataAfterIndex (nodePtr list, void* newData, int index);
void insertListAfterIndex (nodePtr list, nodePtr newList, int index);
void* removeNode (nodePtr* list, int itemPosition);
void* getNodeData (nodePtr list, int itemPosition);
void* copyNode (void* node, size_t dataSize);
nodePtr copyList (nodePtr list, size_t dataSize);
nodePtr invertList (nodePtr list, size_t dataSize);
void joinLists (nodePtr* head, nodePtr tail);
nodePtr getListTail (nodePtr list);
void* rotateListNode (nodePtr* list);
int getListLength (nodePtr list);
void freeListArray (nodePtr* listArray, int numLists);
void freeListOfLists (nodePtr mainList);
void freeList (nodePtr list);
nodePtr advanceList (nodePtr pointer, char* functionName);
