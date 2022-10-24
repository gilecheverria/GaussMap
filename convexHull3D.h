// Update the variable: 'extra' in the array of vertices,
//  to indicate if they belong or not to the C.H.
void mark_convex_hull_vertices (nodePtr convexHull, vertexPtr vertexArray, int numVertices);


// Compute the convex hull of a point cloud in 3D
// Returns a list of triangles that compose the hull.
nodePtr compute_3D_Convex_Hull (vertexPtr vertexArray, int numVertices);


// Check whether a new vertex belongs to the convex hull.
// If it does, update the convex hull accordingly.
// Returns the number of faces this vertex replaces
//  from the initial C.H..
int add_vertex_to_convex_hull (int vertex_index, vertexPtr vertexArray, nodePtr *convexHull);


// Add the new triangles required to the C.H.
void update_convex_hull (nodePtr edgeList, int newVertex, nodePtr* convexHull);


// Update the list of the vertices on the triangles
//  visible to the new vertex being inserted to
//  the convex hull.
void update_visible_edges (nodePtr* edgeList, int v1, int v2, int v3);


// Create a very basic convex hull.
// Take the furthest points in X, and two points in the middle.
// Using these four points, create a simple tetrahedron.
nodePtr compute_initial_hull (vertexPtr vertexArray, int numVertices, vertexPtr realVertexArray);


// Reorder the vertices in 3D space, so that
//  they go in increasing X, Y and Z coordinates.
void sort_vertex_array (vertexPtr vertexArray, int numVertices);


// Create a new tuple to store two integers.
int* newTuple (int one, int two);


void print_convex_hull (nodePtr convexHull, vertexPtr vertexArray);
