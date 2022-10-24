#include "tools.h"
#include "lists.h"
#include "vertices.h"


// Allocate memory for a pointer to an integer.
int* createIntPointer (int data)
{
	int*    intPtr = NULL;

	intPtr = (int*) xmalloc (sizeof (int));
	*intPtr = data;

	return (intPtr);
}


// Allocate memory and store the data for a 3D coordinate.
vertexPtr createVertex (int id, double newX, double newY, double newZ)
{
	vertexPtr	newVertex = NULL;

	newVertex = (vertexPtr) xmalloc (sizeof (vertexStruct));

	newVertex->vertexId = id;
	newVertex->x = newX;
	newVertex->y = newY;
	newVertex->z = newZ;

	return (newVertex);
}


// Print the contents of a list of vertices.
void printVertexList (vertexPtr vertexArray, nodePtr list)
{
	nodePtr			pointer;
	vertexStruct	vertex;
	int*			index;

	pointer = list;

	while (pointer != NULL)
	{
		index = (int*) pointer->data;
		vertex = vertexArray[*index];
		printf ("Vertex %d:\t%.2f\t%.2f\t%.2f\t%d\n", vertex.vertexId, vertex.x, vertex.y, vertex.z, vertex.extra);
		pointer = pointer->next;
	}
}


// Print the contents of an array of vertices.
void printVertexArray (vertexPtr vertexArray, int length)
{
	int		i;

	for (i=0; i<length; i++)
		printf ("Vertex %d:\t%.2f\t%.2f\t%.2f\t%d\n", vertexArray[i].vertexId, vertexArray[i].x, vertexArray[i].y, vertexArray[i].z, vertexArray[i].extra);
}


// Convert the data stored in an array into a linked list.
nodePtr vertexArrayToList (vertexPtr array, int length)
{
	nodePtr		newList = NULL;
	nodePtr		listTail = NULL;
	int*		index = NULL;
	int			i;

	// Start from index 1, to skip over the 
	//  blank vertex in element 0 of the array.
	for (i=1; i<=length; i++)
	{
		index = createIntPointer (array[i].vertexId);
		listTail = addNode (listTail, index);
		if (newList == NULL)
			newList = listTail;
	}

	return (newList);
}


// Convert a linked list of data into an array.
vertexPtr listToVertexArray (nodePtr list, int listLength)
{
	nodePtr			pointer = list;
	vertexPtr		vertexArray = NULL;
	int				counter = 0;

	vertexArray = (vertexPtr) xmalloc (sizeof (vertexStruct) * listLength);

	while (pointer != NULL)
	{
		memcpy (&vertexArray[counter], pointer->data, sizeof(vertexStruct));

		pointer = pointer->next;
		counter++;
	}

	return (vertexArray);
}


// Change the data conatained within two vertices.
void swapVertexIndex (int* index1, int* index2)
{
	int		tempIndex;

	tempIndex = (int) *index1;
	*index1 = (int) *index2;
	*index2 = tempIndex;
}


// Copies the contents of a vertex structure into another.
void copyVertex (vertexPtr source, vertexPtr destination)
{
	destination->vertexId = source->vertexId;
	destination->x = source->x;
	destination->y = source->y;
	destination->z = source->z;
}


// Modify the coordinates of the vertices in an array, to centre them so that
//  the model will be displayed correctly.
// Return another vertex array, with the modified coordinates.
vertexPtr normalizeVertexArray (vertexPtr vertex_array, int array_length, double maxSize)
{
	int				i;
	double 			size[3];
	double			max[3];
	double			min[3];
	double			largestSize;
	vertexStruct	vertex;
	vertexPtr		new_array = NULL;

	new_array = (vertexPtr) xmalloc (sizeof (vertexStruct) * (array_length+1));

	// Begin from index 1 of the array,
	//  since the first element is a blank vertex,
	//  to correct the indexing in the arrays.
	vertex = vertex_array[1];

	// Initialise the location of the largest values.
	max[X] = vertex.x;
	max[Y] = vertex.y;
	max[Z] = vertex.z;

	min[X] = vertex.x;
	min[Y] = vertex.y;
	min[Z] = vertex.z;

	// Locate the largest values in the
	//  three coordinates.
	for (i=2; i<=array_length; i++)
	{
		vertex = vertex_array[i];

		if (vertex.x > max[X])
			max[X] = vertex.x;
		if (vertex.y > max[Y])
			max[Y] = vertex.y;
		if (vertex.z > max[Z])
			max[Z] = vertex.z;

		if (vertex.x < min[X])
			min[X] = vertex.x;
		if (vertex.y < min[Y])
			min[Y] = vertex.y;
		if (vertex.z < min[Z])
			min[Z] = vertex.z;
	}

	// Find the overall largest value
	//  to use for the normalisation.
	size[X] = (max[X] - min[X]) / 2;
	largestSize = size[X];
	size[Y] = (max[Y] - min[Y]) / 2;
	if (size[Y] > largestSize)
		largestSize = size[Y];
	size[Z] = (max[Z] - min[Z]) / 2;
	if (size[Z] > largestSize)
		largestSize = size[Z];

	// Fill in the new array.
	new_array[0].vertexId = 0;
	new_array[0].x = 0.0;
	new_array[0].y = 0.0;
	new_array[0].z = 0.0;

	for (i=1; i<=array_length; i++)
	{
		new_array[i].vertexId = vertex_array[i].vertexId;

		// Translate to the centre.
		new_array[i].x = vertex_array[i].x - (min[X] + size[X]);
		new_array[i].y = vertex_array[i].y - (min[Y] + size[Y]);
		new_array[i].z = vertex_array[i].z - (min[Z] + size[Z]);

		// Scaling
		new_array[i].x = new_array[i].x * (maxSize / largestSize);
		new_array[i].y = new_array[i].y * (maxSize / largestSize);
		new_array[i].z = new_array[i].z * (maxSize / largestSize);
	}

	return (new_array);
}


// Change the data conatained within two points.
void swapPointIndex (int* index1, int* index2)
{
	int		tempIndex;

	tempIndex = (int) *index1;
	*index1 = (int) *index2;
	*index2 = tempIndex;
}
