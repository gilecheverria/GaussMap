#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "faces.h"

//Print the contents of a list of faces.
void printFaceList (nodePtr list)
{
	nodePtr		pointer = list;
	facePtr		face = NULL;

	while (pointer != NULL)
	{
		face = (facePtr) pointer->data;
		printf ("Face: %d has %d vertices: %d, %d, %d", face->faceId, face->faceVertices, face->vertex[0], face->vertex[1], face->vertex[2]);
		if (face->faceVertices == 4)
			printf (", %d\n", face->vertex[3]);
		else
			printf ("\n");
		pointer = pointer->next;
	}
}


// Convert a linked list of data into an array.
facePtr listToFaceArray (nodePtr list, int listLength)
{
	nodePtr			pointer = list;
	facePtr			faceArray = NULL;
	int				counter = 0;

	faceArray = (facePtr) xmalloc (sizeof (faceStruct) * listLength);

	while (pointer != NULL)
	{
		memcpy (&faceArray[counter], pointer->data, sizeof(faceStruct));

		pointer = pointer->next;
		counter++;
	}

	return (faceArray);
}


// Allocate memory for a face pointer
//  and fill it with null values.
facePtr	createEmptyFace (void)
{
	int			i;
	facePtr		face = NULL;

	face = (facePtr) xmalloc (sizeof (faceStruct));

	face->faceId = 0;
	face->faceVertices = 0;
	face->faceNormal = NULL;
	for (i=0; i<4; i++)
		face->vertex[i] = 0;

	return (face);
}


// Allocate memory for a face structure,
//  insert the values provided
//  and return a pointer to the structure.
facePtr newFace (int faceId, int numVertices, int v1, int v2, int v3, int v4)
{
	facePtr		face = NULL;

	face = (facePtr) xmalloc (sizeof (faceStruct));

	face->faceId = faceId;
	face->faceVertices = numVertices;
	face->faceNormal = NULL;
	face->vertex[0] = v1;
	face->vertex[1] = v2;
	face->vertex[2] = v3;
	face->vertex[3] = v4;

	return (face);
}


// Check if two faces share the same set of vertices.
// The order of the vertices does not matter.
// Return true if they are.
boolean equal_faces (faceStruct face_1, faceStruct face_2)
{
	if (	(face_1.vertex[0] == face_2.vertex[0]) ||
			(face_1.vertex[0] == face_2.vertex[1]) ||
			(face_1.vertex[0] == face_2.vertex[2]) )
		if (	(face_1.vertex[1] == face_2.vertex[0]) ||
				(face_1.vertex[1] == face_2.vertex[1]) ||
				(face_1.vertex[1] == face_2.vertex[2]) )
			if (	(face_1.vertex[2] == face_2.vertex[0]) ||
					(face_1.vertex[2] == face_2.vertex[1]) ||
					(face_1.vertex[2] == face_2.vertex[2]) )
				return (TRUE);

	return (FALSE);
}


// Look if any of the vertices that compose a face
//  matches the vertexIndex specified.
// If it does, reorder the vertices in the face,
//  so that the vertex of interest is at the beginning
//  and return TRUE.
// If the vertex is not found in the face, return FALSE.
boolean reorderFace (facePtr face, int vertexIndex)
{
	int			i;
	int			vertexLocation = 0;
	int			tempVertex;
	boolean		found = FALSE;

	// Look for the vertex in the face.
	for (i=0; i<3; i++)
	{
		if (face->vertex[i] == vertexIndex)
		{
			vertexLocation = i;
			found = TRUE;
		}
	}

	if (found)
	{
		// Swap the vertex indexes around.
		for (i=0; i<vertexLocation; i++)
		{
			tempVertex = face->vertex[0];
			face->vertex[0] = face->vertex[1];
			face->vertex[1] = face->vertex[2];
			face->vertex[2] = tempVertex;
		}
	}

	return (found);
}


// Find a given vertex inside a mesh defined
//  by triangles.
// The list of triangles holds indices that
//  correspond to an array of vertices.
// Return a boolean, indicating if the vertex
// is part of the object or not.
boolean find_vertex_in_object (int vertex_index, nodePtr triangle_list, vertexPtr vertexArray)
{
	int				list_length = getListLength (triangle_list);
	int				i, j;
	vertexStruct	vertex;
	facePtr			face = NULL;

	for (i=0; i<list_length; i++)
	{
		face = getNodeData (triangle_list, i);
		for (j=0; j<face->faceVertices; j++)
		{
			vertex = vertexArray[ face->vertex[j] ];
			if (vertex.vertexId == vertex_index)
				return (TRUE);
		}
	}

	return (FALSE);
}


// Get the area of a triangular face in 3D space.
double area3D_face (faceStruct face, vertexPtr vertex_array)
{
	double			triangle_area;
	vertexStruct    triangle_array[5];
	vertexStruct    face_normal;

	memcpy (&triangle_array[0], &vertex_array[face.vertex[0]], sizeof (vertexStruct) );
	memcpy (&triangle_array[1], &vertex_array[face.vertex[1]], sizeof (vertexStruct) );
	memcpy (&triangle_array[2], &vertex_array[face.vertex[2]], sizeof (vertexStruct) );
	memcpy (&triangle_array[3], &vertex_array[face.vertex[0]], sizeof (vertexStruct) );
	memcpy (&triangle_array[4], &vertex_array[face.vertex[1]], sizeof (vertexStruct) );

	face_normal.x = face.faceNormal->i;
	face_normal.y = face.faceNormal->j;
	face_normal.z = face.faceNormal->k;
		
	triangle_area = area3D_Polygon (3, triangle_array, face_normal);
	if (triangle_area < 0)
		triangle_area *= -1;

	return (triangle_area);
}


// Compute the area of the faces around a vertex.
double area3D_cone (nodePtr triangle_list, vertexPtr vertex_array, facePtr face_array)
{
	int				list_length = getListLength (triangle_list);
	int				i;
	int*			face_index = NULL;
	double			cone_area = 0.0;
	double			triangle_area = 0.0;
	faceStruct		face;

	for (i=0; i<list_length; i++)
	{
		face_index = (int*) getNodeData (triangle_list, i);
		face = face_array[*face_index];

		triangle_area = area3D_face (face, vertex_array);
		cone_area += triangle_area;
	}

	return (cone_area);
}


// Compute the surface area of a whole mesh.
double object_surface_area (facePtr face_array, int num_faces, vertexPtr vertex_array)
{
	int				i;
	double			object_area = 0.0;
	double			triangle_area = 0.0;
	faceStruct		face;

	for (i=1; i<=num_faces; i++)
	{
		face = face_array[i];

		triangle_area = area3D_face (face, vertex_array);
		object_area += triangle_area;
	}

	return (object_area);
}



// Compute the total length of the
//  border of a vertex star.
double get_link_length (nodePtr triangle_list, vertexPtr vertex_array, facePtr face_array, int vertex_index)
{
	int				list_length = getListLength (triangle_list);
	int				i;
	int*			face_index = NULL;
	double			segment_length = 0.0;
	double			link_length = 0.0;
	vertexStruct	vertex_1;
	vertexStruct	vertex_2;
	faceStruct		face;

	for (i=0; i<list_length; i++)
	{
		face_index = (int*) getNodeData (triangle_list, i);
		face = face_array[*face_index];

		// Make sure the segment is the one opposite
		//  the vertex being analysed.
		if (face.vertex[0] == vertex_index)
		{
			vertex_1 = vertex_array[ face.vertex[1] ];
			vertex_2 = vertex_array[ face.vertex[2] ];
		}
		else if (face.vertex[1] == vertex_index)
		{
			vertex_1 = vertex_array[ face.vertex[0] ];
			vertex_2 = vertex_array[ face.vertex[2] ];
		}
		else if (face.vertex[2] == vertex_index)
		{
			vertex_1 = vertex_array[ face.vertex[0] ];
			vertex_2 = vertex_array[ face.vertex[1] ];
		}
		else
			printf ("\nTHIS SHOULDN'T BE HAPPENING!!!\n");

		// segment_length = distance2Vertices (vertex_1, vertex_2);
		segment_length = distance2Vertices (vertex_1, vertex_2);
		link_length += segment_length;
	}

	return (link_length);
}
