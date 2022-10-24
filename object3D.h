// Data structures

// OBJECT

typedef struct objectStruct
{
	int			numVertices;
	int			numFaces;
	int			numNormals;
	int			numEdges;

	vertexPtr	vertexArray;
	vertexPtr	displayVertexArray;
	vertexPtr	normalArray;
	facePtr		faceArray;
	nodePtr		edgeList;

	// Array of lists of face indexes.
	nodePtr*	vertexFaceArray;
	// Array of integers.
	int*		facesPerVertex;

	double		surfaceArea;
} objectStruct;

typedef objectStruct* objectPtr;
