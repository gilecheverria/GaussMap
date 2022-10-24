#include <math.h>
#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "faces.h"
#include "normals3D.h"


// Compute the normals of each face, and then of each vertex.
// Returns an array of vertices, with length equal to the
//  number of vertices.
// The array contains the endpoints of the normal vectors to
//  each vertex.
vertexPtr normalsFromVertices (vertexPtr vertices, nodePtr faceList, int numVertices)
{
	nodePtr		pointer = faceList;
	facePtr		face = NULL;
	int			p1;
	int			p2;
	int			p3;
	int			p4;
	int			i;
	vectorPtr	faceNormal = NULL;
	vertexPtr	normals = NULL;
	int*		counters = NULL;
	double		xSquared;
	double		ySquared;
	double		zSquared;
	double		vectorLength;

	// Allocate memory for as many vertices as there exist.
	normals = (vertexPtr) xmalloc (sizeof (vertexStruct) * (numVertices + 1));
	memset (normals, 0, sizeof (vertexStruct) * (numVertices + 1));
	counters = (int*) xmalloc (sizeof (int) * (numVertices + 1));
	memset (counters, 0, sizeof (int) * (numVertices + 1));

	// Skip over the first (empty) face in the list.
	pointer = pointer->next;

	// For each face in the model.
	while (pointer != NULL)
	{
		// Get the current face.
		face = (facePtr) pointer->data;

		p1 = face->vertex[0];
		p2 = face->vertex[1];
		p3 = face->vertex[2];
		p4 = face->vertex[3];

		// Get the normal for the current triangle.
		faceNormal = getTriangleNormal (vertices[p1], vertices[p2], vertices[p3]);

		// Assign the normal to the face structure.
		face->faceNormal = faceNormal;
		// Give the normal an Id number,
		//  related to the number of the face.
		face->faceNormal->vectorId = face->faceId;
		face->faceNormal->type = NORMAL;
		// Variable to store the arc intersections
		//  of the indicatrix that occur at the
		//  location of this vector.
		// (Used in sphericalGeometry.c)
		face->faceNormal->num_intersections = 0;

		// Add the normal components to the sums
		//  for each of the vertices that belong
		//  to the current face.
		normals[p1].x += faceNormal->i;
		normals[p1].y += faceNormal->j;
		normals[p1].z += faceNormal->k;
		counters[p1] += 1;

		normals[p2].x += faceNormal->i;
		normals[p2].y += faceNormal->j;
		normals[p2].z += faceNormal->k;
		counters[p2] += 1;

		normals[p3].x += faceNormal->i;
		normals[p3].y += faceNormal->j;
		normals[p3].z += faceNormal->k;
		counters[p3] += 1;

		if (face->faceVertices == 4)
		{
			normals[p4].x += faceNormal->i;
			normals[p4].y += faceNormal->j;
			normals[p4].z += faceNormal->k;
			counters[p4] += 1;
		}

		pointer = pointer->next;
	}

	// Reduce the size of the normals.
	for (i=1; i<=numVertices; i++)
	{
		normals[i].vertexId = i;
		normals[i].x /= counters[i];
		normals[i].y /= counters[i];
		normals[i].z /= counters[i];

		xSquared = normals[i].x * normals[i].x;
		ySquared = normals[i].y * normals[i].y;
		zSquared = normals[i].z * normals[i].z;

		vectorLength = sqrtf (xSquared + ySquared + zSquared);

		normals[i].x /= vectorLength;
		normals[i].y /= vectorLength;
		normals[i].z /= vectorLength;
	}

	free (counters);

	return (normals);
}


// Compute the normal of a triangle, by using its vertices
//  to create 2 vectors, and then perform the cross product
//  on them. Finally normalize the vector length.
vectorPtr getTriangleNormal (vertexStruct vertex1, vertexStruct vertex2, vertexStruct vertex3)
{
	vectorPtr		vector1 = NULL;
	vectorPtr		vector2 = NULL;
	vectorPtr		normalVector = NULL;

	vector1 = vectorFromVertices (&vertex1, &vertex2);
	vector2 = vectorFromVertices (&vertex2, &vertex3);

	normalVector = crossProduct (vector1, vector2);

	normalizeVector (normalVector);

	free (vector1);
	free (vector2);

	return (normalVector);
}
