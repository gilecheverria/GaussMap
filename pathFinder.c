#include <stdio.h>
#include <math.h>

#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "faces.h"
#include "object3D.h"
#include "vertexGeometry.h"
#include "pathFinder.h"


// Compute a list of vertices which link two points.
// The path will be the closest to a straight line 
//  possible on the surface.
nodePtr find_short_path (objectStruct ObjectData, vertexDataPtr VertexDataArray, int start_index, int end_index)
{
	double			distance = 0.0;
	double			short_distance = -1.0;
	double			current_distance = 9999999.0;
	double			short_region_distance = -1.0;
	double			angle = 0.0;
	int				i;
	int				test_index;
	int				current_index;
	int				close_index;
	int				close_region_index;
	int*			face_index;
	int*			vertex_index = NULL;
	faceStruct		face;
	vertexStruct	start_vertex;
	vertexStruct	end_vertex;
	vertexStruct	test_vertex;
	vertexStruct	current_vertex;
	vertexStruct	previous_vertex;
	nodePtr			path_list = NULL;
	nodePtr			path_list_tail = NULL;

// int j;

	start_vertex = ObjectData.vertexArray[start_index];
	end_vertex = ObjectData.vertexArray[end_index];

	previous_vertex = start_vertex;
	current_vertex = start_vertex;
	current_index = start_index;

	while (current_vertex.vertexId != end_vertex.vertexId)
	// for (j=0; j<8; j++)
	{
		short_distance = -1.0;
		short_region_distance = -1.0;

		// Insert the index of the current point in the list.
		vertex_index = (int*) xmalloc ( sizeof (int) );
		*vertex_index = current_index;
		path_list_tail = addNode (path_list_tail, vertex_index);
		if (path_list == NULL)
			path_list = path_list_tail ;

		// For each of the neighbours around the current vertex
		for (i=0; i<ObjectData.facesPerVertex[current_index]; i++)
		{
			// Get the index of one of the faces.
			face_index = (int*) getNodeData (ObjectData.vertexFaceArray[current_index], i);

			// Get the face itself.
			face = ObjectData.faceArray[*face_index];
			reorderFace (&face, current_index);

			// Get the vertex appearing as the second vertex of the face.
			test_index = face.vertex[1];
			test_vertex = ObjectData.vertexArray[test_index];

			// distance = distance2Vertices (test_vertex, end_vertex);
			distance = get_flat_distance (test_vertex, end_vertex);

			// ALTERNATE MEASUREMENTS OF DISTANCE
			// distance *= distance2Vertices (test_vertex, current_vertex);
			// distance *= fabs (test_vertex.z - current_vertex.z) / distance2Vertices (test_vertex, current_vertex);
			if (previous_vertex.vertexId != start_vertex.vertexId)
			{
				// angle = angleBetweenVertices (current_vertex, test_vertex, end_vertex);
				angle = angleBetweenVertices (previous_vertex, current_vertex, test_vertex);
				angle = 1 + (angle / PI);
				distance *= angle;
			}

			// THE OPTIMISATION OF THE PATH
			// Note if the text vertex belongs to the same region.
			if (	( (distance < short_region_distance) || (short_region_distance == -1.0) ) &&
					(distance < current_distance) &&
					// (VertexDataArray[current_index].type == VertexDataArray[test_index].type)		)
					(ALL_NEG == VertexDataArray[test_index].type)										)
			{
				short_region_distance = distance;
				close_region_index = test_index;
			}
			else
				if ( ( (distance < short_distance) || (short_distance == -1.0) ) && (distance < current_distance) )
				// if ( (distance < short_distance) || (short_distance == -1.0) )
				{
					short_distance = distance;
					close_index = test_index;
				}
		}

		// If any suitable point was found within the same region,
		//  it will have preference.
		if (short_region_distance != -1.0)
		{
			short_distance = short_region_distance;
			close_index = close_region_index;
		}

printf ("GONE TO %d (distance: %lf) (curvature: %lf)\n", close_index, short_distance, VertexDataArray[test_index].curvature);

		previous_vertex = current_vertex;
		current_vertex = ObjectData.vertexArray[close_index];
		current_index = close_index;
		current_distance = short_distance;
	}

printf ("\n");

	vertex_index = (int*) xmalloc ( sizeof (int) );
	*vertex_index = end_index;
	path_list_tail = addNode (path_list_tail, vertex_index);

	return (path_list);
}


void print_path (nodePtr path_list)
{
	int			i;
	int			length = getListLength (path_list);
	int*		index = NULL;

	for (i=0; i<length; i++)
	{
		index = getNodeData (path_list, i);
		printf ("%d ", *index);
	}

	printf ("\n");
}


// Compute the length of a path along a series of vertices.
double get_path_length (nodePtr path_list, vertexPtr vertexArray)
{
	int				i;
	int				length = getListLength (path_list);
	int*			index = NULL;
	double			distance = 0.0;
	double			total_distance = 0.0;
	vertexStruct	vertex1;
	vertexStruct	vertex2;

	for (i=0; i<length-1; i++)
	{
		index = getNodeData (path_list, i);
		vertex1 = vertexArray[*index];
		index = getNodeData (path_list, i+1);
		vertex2 = vertexArray[*index];

		distance = distance2Vertices (vertex2, vertex1);
		total_distance += distance;
	}

	return (total_distance);
}


double get_path_horizontal_variation (nodePtr path_list, vertexPtr vertexArray)
{
	int				i;
	int				length = getListLength (path_list);
	int*			index = NULL;
	double			difference = 0.0;
	double			total_difference = 0.0;
	vertexStruct	vertex1;
	vertexStruct	vertex2;

	for (i=0; i<length-1; i++)
	{
		index = getNodeData (path_list, i);
		vertex1 = vertexArray[*index];
		index = getNodeData (path_list, i+1);
		vertex2 = vertexArray[*index];

		difference = fabs (vertex2.z - vertex1.z);
		total_difference += difference;
	}

	return (total_difference);
}


double get_path_curvature (nodePtr path_list, vertexPtr vertexArray)
{
	int				i;
	int				length = getListLength (path_list);
	int*			index = NULL;
	double			angle = 0.0;
	double			total_angle = 0.0;
	vertexStruct	vertex1;
	vertexStruct	vertex2;
	vertexStruct	vertex3;

	for (i=0; i<length-2; i++)
	{
		index = getNodeData (path_list, i);
		vertex1 = vertexArray[*index];
		index = getNodeData (path_list, i+1);
		vertex2 = vertexArray[*index];
		index = getNodeData (path_list, i+2);
		vertex1 = vertexArray[*index];
		index = getNodeData (path_list, i);
		vertex1 = vertexArray[*index];

		angle = angleBetweenVertices (vertex1, vertex2, vertex3);
		total_angle += angle;
	}

	return (total_angle);
}


double get_flat_distance (vertexStruct vertex1, vertexStruct vertex2)
{
	double  xDifference;
	double  yDifference;

	xDifference = vertex2.x - vertex1.x;
	yDifference = vertex2.y - vertex1.y;

	return ( (xDifference * xDifference) + (yDifference * yDifference) );
}
