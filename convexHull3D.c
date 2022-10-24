#include <stdio.h>
#include <math.h>

#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "faces.h"
#include "convexHull3D.h"
#include "normals3D.h"


// Compute the convex hull of a point cloud in 3D
// Returns a list of triangles that compose the hull.
nodePtr compute_3D_Convex_Hull (vertexPtr vertexArray, int numVertices)
{
	int				i;
	int				removedFaces;
	// int				vertex_index;
	nodePtr			convexHull = NULL;
	vertexPtr		tempVertexArray = NULL;

	// Copy the array of vertices.
	tempVertexArray = (vertexPtr) xmalloc (sizeof (vertexStruct) * (numVertices + 1));
	memcpy (tempVertexArray, vertexArray, sizeof (vertexStruct) * (numVertices + 1));

	// Order the vertices by X, Y and Z coordinates.
	sort_vertex_array (tempVertexArray, numVertices);
	// Make simple convex hull with only 4 vertices.
	convexHull = compute_initial_hull (tempVertexArray, numVertices, vertexArray);
	// Sorted array no longer needed.
	free (tempVertexArray);

	// Add the rest of the vertices to the hull.
	// Skip the first and last vertices, since they
	//  have already been included in the initiall C.H.
	for (i=2; i<numVertices; i++)
	{
		// Skip the vertices already used in the convex hull.
		if (vertexArray[i].extra != 1)
			removedFaces = add_vertex_to_convex_hull (i, vertexArray, &convexHull);

// #ifndef DEBUG
		// printProgressStar (i-1, numVertices);
// #endif
	}

	return (convexHull);
}


// Update the variable: 'extra' in the array of vertices,
//  to indicate if they belong or not to the C.H.
void mark_convex_hull_vertices (nodePtr convexHull, vertexPtr vertexArray, int numVertices)
{
	int				i, j;
	int				c_h_listLenght = getListLength (convexHull);
	facePtr			face = NULL;
	vertexStruct	currentVertex;

	for (i=1; i<=numVertices; i++)
	{
		currentVertex = vertexArray[i];

		for (j=0; j<c_h_listLenght; j++)
		{
			face = (facePtr) getNodeData (convexHull, j);

			// If the vertex is part of at least one of the triangles
			//  in the C.H.
			if (	(currentVertex.vertexId == face->vertex[0]) ||
					(currentVertex.vertexId == face->vertex[1]) ||
					(currentVertex.vertexId == face->vertex[2])		)
			{
				vertexArray[i].extra = 1;
				break;
			}
		}
	}
}


// Check whether a new vertex belongs to the convex hull.
// If it does, update the convex hull accordingly.
// Returns the number of faces this vertex replaces
//  from the initial C.H..
int add_vertex_to_convex_hull (int vertex_index, vertexPtr vertexArray, nodePtr *convexHull)
{
	int			listLenght = getListLength (*convexHull);
	int			i;
	int			v1, v2, v3;
	int			outside = 0;
	double		dot;
	facePtr		face = NULL;
	vectorPtr	faceNormal = NULL;
	vectorPtr	vertexVector = NULL;
	// vectorPtr	vertexVector = (vectorPtr) xmalloc (sizeof (vectorStruct));
	nodePtr		edgeList = NULL;

	for (i=0; i<listLenght; i++)
	{
		face = (facePtr) getNodeData (*convexHull, i);
		v1 = face->vertex[0];
		v2 = face->vertex[1];
		v3 = face->vertex[2];
		faceNormal = getTriangleNormal (vertexArray[v1], vertexArray[v2], vertexArray[v3]);

		vertexVector = vectorFromVertices (&(vertexArray[v1]), &vertexArray[vertex_index]);

		dot = dotProduct (faceNormal, vertexVector);
		free (faceNormal);
		free (vertexVector);

		// If the dot product is positive, then the new vertex
		//  is outside of the hull and can see this face.
		if (dot > 0)
		{
			outside++;

			// Add the vertices of the triangle to a list.
			update_visible_edges (&edgeList, v1, v2, v3);

			// Remove the face that will lie inside of the C.H.
			face = removeNode (convexHull, i);
			free (face);
			listLenght--;
			i--;
		}
	}

	update_convex_hull (edgeList, vertex_index, convexHull);

	freeList (edgeList);

	return (outside);
}


// Add the new triangles required to the C.H.
void update_convex_hull (nodePtr edgeList, int vertex_index, nodePtr* convexHull)
{
	int			i;
	int			edgeListLength = getListLength (edgeList);
	int*		edgeTuple = NULL;
	facePtr		tempFace = NULL;

	for (i=0; i<edgeListLength; i++)
	{
		edgeTuple = (int*) getNodeData (edgeList, i);

		tempFace = newFace (-0, 3, edgeTuple[0], edgeTuple[1], vertex_index, -1);
		*convexHull = addFrontNode (*convexHull, tempFace);
	}
}


// Update the list of the vertices on the triangles
//  visible to the new vertex being inserted to
//  the convex hull.
void update_visible_edges (nodePtr* edgeList, int v1, int v2, int v3)
{
	int				i;
	int				edgeListLength = getListLength (*edgeList);
	int*			edge = NULL;
	boolean			existingEdge1 = FALSE;
	boolean			existingEdge2 = FALSE;
	boolean			existingEdge3 = FALSE;
	boolean			delete = FALSE;

	// Compare against the existing edges.
	for (i=0; i<edgeListLength; i++)
	{
		edge = (int*) getNodeData (*edgeList, i);
		delete = FALSE;

		if ( (edge[0] == v2) && edge[1] == v1)
		{
			existingEdge1 = TRUE;
			delete = TRUE;
		}
		if ( (edge[0] == v3) && edge[1] == v2)
		{
			existingEdge2 = TRUE;
			delete = TRUE;
		}
		if ( (edge[0] == v1) && edge[1] == v3)
		{
			existingEdge3 = TRUE;
			delete = TRUE;
		}

		if (delete)
		{
			edge = removeNode (edgeList, i);
			free (edge);
			edgeListLength--;
			i--;
		}
	}

	// Add the edges to the list.
	if (!existingEdge1)
	{
		edge = newTuple (v1, v2);
		*edgeList = addFrontNode (*edgeList, edge);
	}
	if (!existingEdge2)
	{
		edge = newTuple (v2, v3);
		*edgeList = addFrontNode (*edgeList, edge);
	}
	if (!existingEdge3)
	{
		edge = newTuple (v3, v1);
		*edgeList = addFrontNode (*edgeList, edge);
	}
}


// Create a very basic convex hull.
// Take the furthest points in X, and two points in the middle.
// Using these four points, create a simple tetrahedron.
nodePtr compute_initial_hull (vertexPtr vertexArray, int numVertices, vertexPtr realVertexArray)
{
	// Vertices named after the cardinal points.
	// This refers to their position among the point cloud.
	vertexStruct	vertex_E;
	vertexStruct	vertex_W;
	vertexStruct	vertex_N;
	vertexStruct	vertex_S;
	vertexStruct	real_vertex;
	vectorPtr		testVector1 = NULL;
	vectorPtr		testVector2 = NULL;
	facePtr			face1;
	facePtr			face2;
	facePtr			face3;
	facePtr			face4;
	nodePtr			convexHull = NULL;
	int				topIndex = numVertices/2;
	int				frontIndex = 2;
	int				i;
	int				W_index;
	int				E_index;
	int				N_index;
	int				S_index;
	int				tmp_index;
	double			dot;
	double			top = vertexArray[topIndex].y;
	double			front = vertexArray[frontIndex].z;

	// Get appropiate indices for the initial vertices
	//  of the C.H.
	for (i=2; i<numVertices; i++)
	{
		if (vertexArray[i].y > top)
		{
			top = vertexArray[i].y;
			topIndex = i;
		}
		else if (vertexArray[i].z > front)
		{
			front = vertexArray[i].z;
			frontIndex = i;
		}
	}

	// Set the indices in the array of each vertex.
	W_index = 1;
	E_index = numVertices;
	N_index = topIndex;
	S_index = frontIndex;
	// Get the end vertices, and two in between.
	vertex_W = vertexArray[W_index];
	vertex_E = vertexArray[E_index];
	vertex_N = vertexArray[N_index];
	vertex_S = vertexArray[S_index];

	testVector1 = vectorFromVertices (&vertex_W, &vertex_E);
	testVector2 = getTriangleNormal (vertex_E, vertex_N, vertex_S);

	dot = dotProduct (testVector1, testVector2);

	free (testVector1);
	free (testVector2);

	if (dot < 0)
	{
		tmp_index = N_index;
		N_index = S_index;
		S_index = tmp_index;
	}

	// Change to use the vertex indices on the
	//  unsorted array.
	for (i=1; i<=numVertices; i++)
	{
		real_vertex = realVertexArray[i];

		if (real_vertex.vertexId == vertex_W.vertexId)
			W_index = i;
		if (real_vertex.vertexId == vertex_E.vertexId)
			E_index = i;
		if (real_vertex.vertexId == vertex_N.vertexId)
			N_index = i;
		if (real_vertex.vertexId == vertex_S.vertexId)
			S_index = i;
	}

	// Mark the vertices as already considered for
	//  the convex hull, so that they will not be
	//  checked again.
	realVertexArray[W_index].extra = 1;
	realVertexArray[E_index].extra = 1;
	realVertexArray[N_index].extra = 1;
	realVertexArray[S_index].extra = 1;

	// face1 = newFace (-1, 3, vertex_E.vertexId, vertex_W.vertexId, vertex_N.vertexId, -1);
	face1 = newFace (-1, 3, E_index, W_index, N_index, -1);
	convexHull = addFrontNode (convexHull, face1);

	// face2 = newFace (-2, 3, vertex_W.vertexId, vertex_E.vertexId, vertex_S.vertexId, -1);
	face2 = newFace (-2, 3, W_index, E_index, S_index, -1);
	convexHull = addFrontNode (convexHull, face2);

	// face3 = newFace (-3, 3, vertex_S.vertexId, vertex_E.vertexId, vertex_N.vertexId, -1);
	face3 = newFace (-3, 3, S_index, E_index, N_index, -1);
	convexHull = addFrontNode (convexHull, face3);

	// face4 = newFace (-4, 3, vertex_W.vertexId, vertex_S.vertexId, vertex_N.vertexId, -1);
	face4 = newFace (-4, 3, W_index, S_index, N_index, -1);
	convexHull = addFrontNode (convexHull, face4);

	return (convexHull);
}


// Reorder the vertices in 3D space, so that
//  they go in increasing X, Y and Z coordinates.
void sort_vertex_array (vertexPtr vertexArray, int numVertices)
{
	int				i, j;
	int				swapIndex;
	boolean			swapVertices = FALSE;
	vertexStruct	vertex1;
	vertexStruct	vertex2;
	vertexStruct	tempVertex;

	for (i=1; i<=numVertices; i++)
	{
		vertex1 = vertexArray[i];
		swapVertices = FALSE;

		for (j=i+1; j<=numVertices; j++)
		{
			vertex2 = vertexArray[j];

			if (	(vertex2.x < vertex1.x) ||
					( (vertex2.x == vertex1.x) && (vertex2.y < vertex1.y) ) ||
					( (vertex2.x == vertex1.x) && (vertex2.y == vertex1.y) && (vertex2.z < vertex1.z) )	)
			{
				swapVertices = TRUE;
				swapIndex = j;
				vertex1 = vertex2;
			}
		}

		if (swapVertices)
		{
			tempVertex = vertexArray[i];
			vertexArray[i] = vertexArray[swapIndex];
			vertexArray[swapIndex] = tempVertex;
		}
	}
}


// Create a new tuple to store two integers.
int* newTuple (int one, int two)
{
	int*			tuple;

	tuple = (int*) xmalloc (sizeof (int) * 2);

	tuple[0] = one;
	tuple[1] = two;

	return (tuple);
}


// Print the contents of a list of triangles.
void print_convex_hull (nodePtr convexHull, vertexPtr vertexArray)
{
	int			i;
	int			listLenght = getListLength (convexHull);
	int			v1;
	int			v2;
	int			v3;
	facePtr		face = NULL;

	printf ("\nNEW CONVEX HULL:\n");

	for (i=0; i<listLenght; i++)
	{
		face = (facePtr) getNodeData (convexHull, i);
		v1 = face->vertex[0];
		v2 = face->vertex[1];
		v3 = face->vertex[2];

		printf ("\tFace %d: indices [%d] [%d] [%d]\t vertices %d, %d, %d\n", i+1, v1, v2, v3, vertexArray[v1].vertexId, vertexArray[v2].vertexId, vertexArray[v3].vertexId);
	}
}
