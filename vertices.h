// Data structures

// VERTEX
typedef struct vertexStruct
{
	int     vertexId;
	double  x;
	double  y;
	double  z;
	int     extra;
} vertexStruct;

typedef vertexStruct* vertexPtr;


// Types of vertices, according to their gauss map.
// typedef enum {FLAT, ALL_POS, ALL_NEG, MIXED}			vertexType;
typedef enum {FLAT, ALL_POS, ALL_NEG, MIXED, ALL_POS_CONCAVE, MIXED_CONCAVE}			vertexType;


// Function definitions

// Allocate memory for a pointer to an integer.
int* createIntPointer (int data);

// Allocate memory and store the data for a 3D coordinate.
vertexPtr createVertex (int id, double newX, double newY, double newZ);

// Print the contents of a list of vertices.
void printVertexList (vertexPtr vertexArray, nodePtr list);

// Print the contents of an array of vertices.
void printVertexArray (vertexPtr vertexArray, int length);

// Convert the data stored in an array into a linked list.
nodePtr vertexArrayToList (vertexPtr array, int length);

// Convert a linked list of data into an array.
vertexPtr listToVertexArray (nodePtr list, int listLength);

// Change the data conatained within two vertices.
void swapVertexIndex (int* index1, int* index2);

// Copies the contents of a vertex structure into another.
void copyVertex (vertexPtr source, vertexPtr destination);

// Modify the coordinates of the vertices in a list, to center them so that
//  the model will be displayed correctly.
vertexPtr normalizeVertexArray (vertexPtr vertex_array, int array_length, double maxSize);
