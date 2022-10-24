#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "vertexGeometry.h"


// Set the structure to adequate values.
void initializeVertexDataStructure (vertexDataStruct* vertex_data)
{
	// Specific data for the vertex.
	vertex_data->type = FLAT;
	vertex_data->border_vertex = FALSE;
	vertex_data->curvature = 0.0;
	vertex_data->angle_deficit = 0.0;
	vertex_data->region_id = 0;

	// Arrays and lists.
	vertex_data->normals_around_vertex = NULL;
	vertex_data->spherical_polygon_list = NULL;
	vertex_data->spherical_polygon_orientation = NULL;

	// Data for the projection of the star on a plane.
	vertex_data->points_around_vertex = 0;
	vertex_data->vertex_neighbour_index = NULL;
	vertex_data->projected_vertices_array = NULL;

	// Data for decimation/retriangulation.
	vertex_data->feature_edge_list = NULL;

}


// Deallocate the memory of the structure elements.
void freeVertexDataStructure (vertexDataStruct* vertex_data)
{
	freeList (vertex_data->normals_around_vertex);
	freeListOfLists (vertex_data->spherical_polygon_list);
	free (vertex_data->spherical_polygon_orientation);
	free (vertex_data->vertex_neighbour_index);
	free (vertex_data->projected_vertices_array);
	freeList (vertex_data->feature_edge_list);
}
