// Compute a list of vertices which link two points.
// The path will be the closest to a straight line 
//  possible on the surface.
nodePtr find_short_path (objectStruct ObjectData, vertexDataPtr VertexDataArray, int start_index, int end_index);

void print_path (nodePtr path_list);

// Compute the length of a path along a series of vertices.
double get_path_length (nodePtr path_list, vertexPtr vertexArray);

double get_path_horizontal_variation (nodePtr path_list, vertexPtr vertexArray);

double get_path_curvature (nodePtr path_list, vertexPtr vertexArray);

double get_flat_distance (vertexStruct vertex1, vertexStruct vertex2);
