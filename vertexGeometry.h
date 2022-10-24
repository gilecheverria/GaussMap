typedef struct vertexDataStruct 
{
	// Specific data for the vertex.
	vertexType		type;
	boolean			border_vertex;
	double			curvature;
	double			angle_deficit;
	int				region_id;

	// Arrays and lists.
	nodePtr			normals_around_vertex;		// Normals of the faces around vertex
	nodePtr			spherical_polygon_list;
	spinType*		spherical_polygon_orientation;

	// Data for the projection of the star on a plane.
	vectorStruct	artificial_normal;
	int				points_around_vertex;
	int*			vertex_neighbour_index;		// Indices of the neighbour vertices
	vertexPtr		projected_vertices_array;	// Neighbours projected on a plane

	// Data for decimation/retriangulation
	nodePtr			feature_edge_list;
} vertexDataStruct;

typedef vertexDataStruct* vertexDataPtr;


// Set the structure to adequate values.
void initializeVertexDataStructure (vertexDataStruct* vertex_data);


// Deallocate the memory of the structure elements.
void freeVertexDataStructure (vertexDataStruct* vertex_data);
