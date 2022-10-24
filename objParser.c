#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "faces.h"
#include "object3D.h"
#include "normals3D.h"
#include "objParser.h"

// Global variable to dump useless data.
char*		string;



// Parse an obj file and create lists with the vertices and faces.
// Also creates an array with lists of faces that converge
//  on each of the vertices.
objectStruct readObjFile (char* filename)
{
	FILE*			OBJ_FD = NULL;
	char			keyword;
	nodePtr			vertexList = NULL;
	nodePtr			vertexListTail = NULL;
	nodePtr			faceList = NULL;
	nodePtr			faceListTail = NULL;
	nodePtr*		vertexFaceArray = NULL;
	vertexPtr		vertex = NULL;
	vertexPtr		vertexArray = NULL;
	vertexPtr		normalArray = NULL;
	facePtr			faceArray = NULL;
	facePtr			face = NULL;
	int				i;
	int				vertex_counter = 0;
	int				face_counter = 0;
	int*			facesPerVertex = NULL;
	boolean			textured = FALSE;
	boolean			normals = FALSE;
	objectStruct	objectData;

	OBJ_FD = xfopen (filename, "r");

	string = (char*) xmalloc (sizeof (char) * 100);

	// Add an empty vertex at the beginning of the vertex list.
	// It will be disregarded, and thus the vertices will be
	//  indexed beginning with 1, when they are transfered to
	//  an array.
	vertex = createVertex (0, 0, 0, 0);
	vertexList = addNode (NULL, vertex);
	vertexListTail = vertexList;

	// Add an empty face at the beginning of the face list.
	// It will be disregarded, and thus the faces will be
	//  indexed beginning with 1, when they are transfered to
	//  an array.
	face = createEmptyFace ();
	faceList = addNode (NULL, face);
	faceListTail = faceList;

	while (!feof (OBJ_FD))
	{
		keyword = getc (OBJ_FD);

		switch (keyword)
		{
			case 'v':	// A vertex definition
				keyword = getc (OBJ_FD);
				if (keyword == ' ' || keyword == '\t')	// Normal vertex
				{
					vertex = readVertex (OBJ_FD);
					vertexListTail = addNode (vertexListTail, vertex);
					vertex_counter++;
					vertex->vertexId = vertex_counter;
				}
				else if (keyword == 't')	// Texture vertex
				{
					textured = TRUE;
				}
				else if (keyword == 'n')	// Normal vertex
				{
					normals = TRUE;
				}
				break;
			case 'f':	// A face definition
				face = readFace (OBJ_FD, textured, normals);
				faceListTail = addNode (faceListTail, face);
				face_counter++;
				face->faceId = face_counter;

				// Initialize arrays for vertex faces.
				if (vertexFaceArray == NULL)
				{
					vertexFaceArray = (nodePtr*) xmalloc ( sizeof (nodePtr) * (vertex_counter+1) );
					facesPerVertex = (int*) xmalloc ( sizeof(int) * (vertex_counter+1) );
					for (i=0; i<=vertex_counter; i++)
					{
						vertexFaceArray[i] = NULL;
						facesPerVertex[i] = 0;
					}
				}
				vertexFaceArray = addFaceToVertexFaceArray (vertexFaceArray, face, vertex_counter, facesPerVertex);
				break;
			case '\n':	// A blank line
				break;
			default:	// Any other case
				// Read and discard the line
				fgets(string, 100, OBJ_FD);
				break;
		}
	}

	free (string);

	fclose (OBJ_FD);

	// Convert the list of vertices into an array.
	// The array will have 'numVertices'+1 elements.
	// The element at index 0 is blank, allowing the
	//  real vertices to start from index 1.
	vertexArray = listToVertexArray (vertexList, vertex_counter+1);

	// Obtain the normals for each individual vertex.
	normalArray = normalsFromVertices (vertexArray, faceList, vertex_counter);

	// Convert the list of faces into an array.
	// The array will have 'numFaces'+1 elements.
	// The element at index 0 is blank, allowing the
	//  real faces to start from index 1.
	faceArray = listToFaceArray (faceList, face_counter+1);

	// Copy the data just read into the object structure
	objectData.numVertices = vertex_counter;
	objectData.numFaces = face_counter;
	objectData.numEdges = 0;
	objectData.vertexArray = vertexArray;
	objectData.displayVertexArray = NULL;
	objectData.normalArray = normalArray;
	objectData.faceArray = faceArray;
	objectData.edgeList = NULL;
	objectData.vertexFaceArray = vertexFaceArray;
	objectData.facesPerVertex = facesPerVertex;
	objectData.surfaceArea = object_surface_area (faceArray, face_counter, vertexArray);


	freeList (vertexList);
	freeList (faceList);

	return (objectData);
}


// Create a new vertex structure with the information from
//  the .obj file.
vertexPtr readVertex (FILE* OBJ_FD)
{
	double		x;
	double		y;
	double		z;
	vertexPtr	newVertex = NULL;

	fscanf (OBJ_FD, "%lf %lf %lf", &x, &y, &z);

	newVertex = (vertexPtr) xmalloc (sizeof(vertexStruct));

	newVertex->x = x;
	newVertex->y = y;
	newVertex->z = z;
	newVertex->extra = 0;

	// Discard the rest of the line.
	fgets(string, 100, OBJ_FD);

	return (newVertex);
}


// Create a new face structure with the information from
//  the .obj file.
facePtr readFace (FILE* OBJ_FD, boolean textured, boolean normals)
{
	int			vertex[3];
	int			texture[3];
	int			normal[3];
	int			i;
	char		nextChar;
	facePtr		new_face = NULL;

	new_face = (facePtr) xmalloc (sizeof(faceStruct));

	new_face->faceVertices = 3;

	for (i=0; i<3; i++)
	{
		fscanf (OBJ_FD, "%d", &vertex[i]);
		if (textured)
		{
			fscanf (OBJ_FD, "/%d", &texture[i]);
			if (normals)
				fscanf (OBJ_FD, "/%d", &normal[i]);
		}
		else if (normals)
			fscanf (OBJ_FD, "//%d", &normal[i]);

		new_face->vertex[i] = vertex[i];
	}

	new_face->vertex[3] = -1;

	// Check to see if there is a fourth vertex in the face.
	nextChar = getc (OBJ_FD);

	// If the next character is different from:
	//  ASCII 10 = LF (line feed)
	//  ASCII 13 = CR (carriage return)
	if ( (nextChar != 10) && (nextChar != 13) )
	{
		new_face->faceVertices = 4;

		fscanf (OBJ_FD, "%d", &vertex[i]);
		if (textured)
		{
			fscanf (OBJ_FD, "/%d", &texture[i]);
			if (normals)
				fscanf (OBJ_FD, "/%d", &normal[i]);
		}
		else if (normals)
			fscanf (OBJ_FD, "//%d", &normal[i]);

		// Se the fourth vertex. (i = 3)
		new_face->vertex[i] = vertex[i];

		// Discard the rest of the line.
		fgets(string, 100, OBJ_FD);
	}

	return (new_face);
}


// Add a new face to the array of faces per vertex.
// The 'faceId' will be inserted in the lists corresponding
//  to each of the vertices that compose the face.
nodePtr* addFaceToVertexFaceArray (nodePtr* vertexFaceArray, facePtr face, int numVertices, int* facesPerVertex)
{
	int			i;
	int			currentVertex;
	int*		index;
	nodePtr		tmpList = NULL;

	// For each of the vertices delimiting this face.
	for (i=0; i<face->faceVertices; i++)
	{
		currentVertex = face->vertex[i];
		// Get the list for the current vertex.
		tmpList = vertexFaceArray[currentVertex];
		// Copy the faceId to a new int.
		index = (int*) xmalloc (sizeof (int));
		memcpy (index, &(face->faceId), sizeof (int));
		// Insert the pointer into the list.
		tmpList = addFrontNode (tmpList, index);
		vertexFaceArray[currentVertex] = tmpList;

		// Increment the number of adjacent faces
		//  to the corresponding vertices.
		facesPerVertex[currentVertex] += 1;
	}

	return (vertexFaceArray);
}
