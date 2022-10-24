// Data type definition
typedef struct edgeStruct
{
	int			edgeId;
	int			vertex1;
	int			vertex2;
	int			face1;
	int			opposite_vertex_1;
	int			face2;
	int			opposite_vertex_2;
	int			type;
} edgeStruct;

typedef edgeStruct* edgePtr;


// Function declarations
edgePtr newEdge (int edgeId, int edgeVertex1, int edgeVertex2);

// Print the data about the edge and its faces.
void printEdgeData (edgePtr edge, faceStruct face_1, faceStruct face_2);

// Print the contents of a list of edges.
void printEdgeList (nodePtr edgeList, int listLength);

// Extract the list of edges out of a list
//  of triangles.
nodePtr get_edge_list (nodePtr triangle_list, int num_triangles);

// Create a new edge and insert it
//  into the list.
void add_edge (int vertex1, int vertex2, int opposite_vertex, int face_id, int* num_edges, nodePtr* edge_list);


// Check if an edge has already been inserted in the list.
int existingEdge (nodePtr edgeList, edgePtr edge, int listLength);

// Takes a list of edges and drops those
//  that have only one incident triangle.
void remove_border_edges (nodePtr* edge_list, int* num_edges);


// Flip and edge and correct the resulting faces.
void flip_edge (edgePtr edge, facePtr face_1, facePtr face_2, boolean inverted);

// Modify the opposite vertices and triangles
//  of the nearby edges after a flip.
nodePtr update_neighbour_edges (faceStruct face_1, faceStruct face_2, nodePtr edge_list, int num_edges);

// Check if a face contains the edge specified as a parameter.
// The order of the vertices in the edge does not matter.
// Return true if the edge is found in the triangle.
boolean find_edge_in_triangle (edgePtr edge, faceStruct face);


// Find if an edge in a polygon is concave or convex.
// void get_edge_orientation (nodePtr edgeList, facePtr faceArray, vertexPtr vertexArray);
