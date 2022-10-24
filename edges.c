#include <stdio.h>
#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "faces.h"
#include "edges.h"

// Create a new edge pointer.
edgePtr newEdge (int edgeId, int edgeVertex1, int edgeVertex2)
{
	edgePtr		new_edge = NULL;

	new_edge = (edgePtr) xmalloc ( sizeof (edgeStruct) );

	new_edge->edgeId = edgeId;
	new_edge->vertex1 = edgeVertex1;
	new_edge->vertex2 = edgeVertex2;
	new_edge->face1 = -1;
	new_edge->face2 = -1;
	new_edge->opposite_vertex_1 = -1;
	new_edge->opposite_vertex_2 = -1;
	new_edge->type = -1;

	return (new_edge);
}


// Print the data about the edge and its faces.
void printEdgeData (edgePtr edge, faceStruct face_1, faceStruct face_2)
{
	printf ("    Edge %d: %d - %d || Opps: %d, %d || Tris: %d, %d\n", edge->edgeId, edge->vertex1, edge->vertex2, edge->opposite_vertex_1, edge->opposite_vertex_2, edge->face1, edge->face2);
	printf ("    Face %d: %d, %d, %d\t|| ", face_1.faceId, face_1.vertex[0], face_1.vertex[1], face_1.vertex[2]);
	// printf ("Normal: (%lf, %lf, %lf)\n", face_1.faceNormal->i, face_1.faceNormal->j, face_1.faceNormal->k);
	printf ("\n");

	printf ("    Face %d: %d, %d, %d\t|| ", face_2.faceId, face_2.vertex[0], face_2.vertex[1], face_2.vertex[2]);
	// printf ("Normal: (%lf, %lf, %lf)\n", face_2.faceNormal->i, face_2.faceNormal->j, face_2.faceNormal->k);
	printf ("\n");
}


// Print the contents of a list of edges.
void printEdgeList (nodePtr edgeList, int listLength)
{
	edgePtr		edge = NULL;
	int			i;

	printf ("Edge list:\n");

	for (i=0; i<listLength; i++)
	{
		edge = (edgePtr) getNodeData (edgeList, i);
		printf ("Edge %d\t==>   %d - %d ||| faces %d, %d ||| opposites %d, %d ||| type %d\n", edge->edgeId, edge->vertex1, edge->vertex2, edge->face1, edge->face2, edge->opposite_vertex_1, edge->opposite_vertex_2, edge->type);
	}
}


// Extract the list of edges out of a list
//  of triangles.
nodePtr get_edge_list (nodePtr triangle_list, int num_triangles)
{
	int			i;
	int			num_edges = 0;
	int			vertex1;
	int			vertex2;
	int			vertex3;
	facePtr		face = NULL;
	nodePtr		edge_list = NULL;

	for (i=0; i<num_triangles; i++)
	{
		face = (facePtr) getNodeData (triangle_list, i);

		vertex1 = face->vertex[0];
		vertex2 = face->vertex[1];
		vertex3 = face->vertex[2];

		add_edge (vertex1, vertex2, vertex3, face->faceId, &num_edges, &edge_list);
		add_edge (vertex2, vertex3, vertex1, face->faceId, &num_edges, &edge_list);
		add_edge (vertex3, vertex1, vertex2, face->faceId, &num_edges, &edge_list);
	}

	return (edge_list);
}


// Create a new edge and insert it
//  into the list.
void add_edge (int vertex1, int vertex2, int opposite_vertex, int face_id, int* num_edges, nodePtr* edge_list)
{
	int			index;
	edgePtr		edge = NULL;

	edge = newEdge ((*num_edges)+1, vertex1, vertex2);
	edge->face1 = face_id;
	edge->opposite_vertex_1 = opposite_vertex;
	// Check that the edge is not already in.
	index = existingEdge (*edge_list, edge, *num_edges);
	// If the edge already exists.
	if (index >= 0)
	{
		// Delete the edge just created.
		free (edge);
		// Get the version of the edge in the list.
		edge = (edgePtr) removeNode (edge_list, index);
		// Update the second adjacent face.
		edge->face2 = face_id;
		edge->opposite_vertex_2 = opposite_vertex;
		// Add the edge to the final list.
		*edge_list = addFrontNode (*edge_list, edge);
	}
	else
	{
		// Add the new edge.
		*edge_list = addFrontNode (*edge_list, edge);
		*num_edges += 1;
	}
}


// Check if an edge has already been inserted in the list.
int existingEdge (nodePtr edgeList, edgePtr edge, int listLength)
{
	edgePtr		listEdge;
	int			i;

	for (i=0; i<listLength; i++)
	{
		listEdge = (edgePtr) getNodeData (edgeList, i);
		
		if ( (listEdge->vertex1 == edge->vertex2) && (listEdge->vertex2 == edge->vertex1) )
		{
			return (i);
		}
	}

	return (-1);
}


// Takes a list of edges and drops those
//  that have only one incident triangle.
void remove_border_edges (nodePtr* edge_list, int* num_edges)
{
	int			i;
	edgePtr		edge = NULL;

	// Remove edges on the borders.
	for (i=0; i<*num_edges; i++)
	{
		edge = (edgePtr) getNodeData (*edge_list, i);
		if (edge->face2 == -1)
		{
			edge = (edgePtr) removeNode (edge_list, i);
			*num_edges -= 1;
			i--;
		}
	}
}


// Flip and edge and correct the resulting faces.
void flip_edge (edgePtr edge, facePtr face_1, facePtr face_2, boolean inverted)
{
	int			temp;
	int			start_index = 0;
	int			end_index = 2;

	// Invert the rotation of the faces.
	if (inverted)
	{
		start_index = 2;
		end_index = 0;
	}

	// NOTE: The vertices of the edge have the same
	//  order as in face 1, while on face 2 they are
	//  inverted.

	// Update the faces.
	// Make sure the first vertex on the face does
	//  not lie on the edge. Rotate the vertices
	//  on the face until this condition is met.
	while (face_1->vertex[start_index] != edge->opposite_vertex_1)
	{
		temp = face_1->vertex[0];
		face_1->vertex[0] = face_1->vertex[1];
		face_1->vertex[1] = face_1->vertex[2];
		face_1->vertex[2] = temp;
	}
	// Add the opposite vertex of the other triangle.
	edge->vertex1 = face_1->vertex[start_index];
	face_1->vertex[end_index] = edge->opposite_vertex_2;

	// Do the same for the other face.
	while (face_2->vertex[start_index] != edge->opposite_vertex_2)
	{
		temp = face_2->vertex[0];
		face_2->vertex[0] = face_2->vertex[1];
		face_2->vertex[1] = face_2->vertex[2];
		face_2->vertex[2] = temp;
	}
	// Add the opposite vertex of the other triangle.
	edge->vertex2 = face_2->vertex[start_index];
	face_2->vertex[end_index] = edge->opposite_vertex_1;

	edge->opposite_vertex_1 = face_1->vertex[1];
	edge->opposite_vertex_2 = face_2->vertex[1];
}


// Modify the opposite vertices and triangles
//  of the nearby edges after a flip.
nodePtr update_neighbour_edges (faceStruct face_1, faceStruct face_2, nodePtr edge_list, int num_edges)
{
	int			i;
	edgePtr		edge = NULL;

	for (i=0; i<num_edges; i++)
	{
		edge = (edgePtr) getNodeData (edge_list, i);

		// Locate first edge of triangle 1.
		if (	(edge->vertex1 == face_1.vertex[0] && edge->vertex2 == face_1.vertex[1]) ||
				(edge->vertex2 == face_1.vertex[0] && edge->vertex1 == face_1.vertex[1])		)
		{
			if ( (edge->face1 == face_1.faceId) || (edge->face1 == face_2.faceId) )
			{
				edge->face1 = face_1.faceId;
				edge->opposite_vertex_1 = face_1.vertex[2];
				continue;
			}
			else
			{
				edge->face2 = face_1.faceId;
				edge->opposite_vertex_2 = face_1.vertex[2];
				continue;
			}
		}

		// Locate second edge of triangle 1.
		if (	(edge->vertex1 == face_1.vertex[1] && edge->vertex2 == face_1.vertex[2]) ||
				(edge->vertex2 == face_1.vertex[1] && edge->vertex1 == face_1.vertex[2])		)
		{
			if ( (edge->face1 == face_1.faceId) || (edge->face1 == face_2.faceId) )
			{
				edge->face1 = face_1.faceId;
				edge->opposite_vertex_1 = face_1.vertex[0];
				continue;
			}
			else
			{
				edge->face2 = face_1.faceId;
				edge->opposite_vertex_2 = face_1.vertex[0];
				continue;
			}
		}

		// Locate first edge of triangle 2.
		if (	(edge->vertex1 == face_2.vertex[0] && edge->vertex2 == face_2.vertex[1]) ||
				(edge->vertex2 == face_2.vertex[0] && edge->vertex1 == face_2.vertex[1])		)
		{
			if ( (edge->face1 == face_1.faceId) || (edge->face1 == face_2.faceId) )
			{
				edge->face1 = face_2.faceId;
				edge->opposite_vertex_1 = face_2.vertex[2];
				continue;
			}
			else
			{
				edge->face2 = face_2.faceId;
				edge->opposite_vertex_2 = face_2.vertex[2];
				continue;
			}
		}

		// Locate second edge of triangle 2.
		if (	(edge->vertex1 == face_2.vertex[1] && edge->vertex2 == face_2.vertex[2]) ||
				(edge->vertex2 == face_2.vertex[1] && edge->vertex1 == face_2.vertex[2])		)
		{
			if ( (edge->face1 == face_1.faceId) || (edge->face1 == face_2.faceId) )
			{
				edge->face1 = face_2.faceId;
				edge->opposite_vertex_1 = face_2.vertex[0];
				continue;
			}
			else
			{
				edge->face2 = face_2.faceId;
				edge->opposite_vertex_2 = face_2.vertex[0];
				continue;
			}
		}
	}

	return (edge_list);
}


// Check if a face contains the edge specified as a parameter.
// The order of the vertices in the edge does not matter.
// Return true if the edge is found in the triangle.
boolean find_edge_in_triangle (edgePtr edge, faceStruct face)
{
	// Check the edge in one direction.
	if (	( (edge->vertex1 == face.vertex[0]) && (edge->vertex2 == face.vertex[1]) ) ||
			( (edge->vertex1 == face.vertex[1]) && (edge->vertex2 == face.vertex[2]) ) ||
			( (edge->vertex1 == face.vertex[2]) && (edge->vertex2 == face.vertex[0]) )		)
				return (TRUE);
	// Check the edge in the opposite direction.
	if (	( (edge->vertex1 == face.vertex[0]) && (edge->vertex2 == face.vertex[1]) ) ||
			( (edge->vertex1 == face.vertex[1]) && (edge->vertex2 == face.vertex[2]) ) ||
			( (edge->vertex1 == face.vertex[2]) && (edge->vertex2 == face.vertex[0]) )		)
				return (TRUE);

	return (FALSE);
}



/*
// Find if an edge in a polygon is concave or convex.
void get_edge_orientation (nodePtr edgeList, facePtr faceArray, vertexPtr vertexArray)
{
	edgePtr			edge;
	faceStruct		face1;
	faceStruct		face2;
	vertexStruct	farVertex1;
	vertexStruct	farVertex2;
	int				i;
	int				j = 0;
	int				numEdges = getListLength (edgeList); 
	int				farVertexId1;
	int				farVertexId2;
	double			baseDistance;
	double			endDistance;

	for (i=0; i<numEdges; i++)
	{
		edge = (edgePtr) getNodeData (edgeList, i);
		face1 = faceArray[edge->face1];
		face2 = faceArray[edge->face2];

		// Get a vertex of the faces that does not belong to the edge.
		j = 0;
		do
		{
			farVertexId1 = face1.vertex[j++];
		} while ( (farVertexId1 == edge->vertex1) || (farVertexId1 == edge->vertex2) );
		j = 0;
		do
		{
			farVertexId2 = face2.vertex[j++];
		} while ( (farVertexId2 == edge->vertex1) || (farVertexId2 == edge->vertex2) );

		farVertex1 = vertexArray[farVertexId1];
		farVertex2 = vertexArray[farVertexId2];

		// Get the distance from the two vertices.
		baseDistance = distance2Vertices (farVertex1, farVertex2);

		// Update the vertices, by adding the components of 
		//  the normal vector of the face.
		farVertex1.x += (face1.faceNormal)->i;
		farVertex1.y += (face1.faceNormal)->j;
		farVertex1.z += (face1.faceNormal)->k;

		farVertex2.x += (face2.faceNormal)->i;
		farVertex2.y += (face2.faceNormal)->j;
		farVertex2.z += (face2.faceNormal)->k;

		// Get the new distance.
		endDistance = distance2Vertices (farVertex1, farVertex2);

		if (endDistance >= (baseDistance - EPSILON))
			edge->type = CONVEX;
		else
			edge->type = CONCAVE;
	}
}
*/
