// Data structures

// FACE
typedef struct faceStruct
{
	int			faceId;
	int			faceVertices;
	int			vertex[4];
	vectorPtr	faceNormal;
} faceStruct;

typedef faceStruct* facePtr;


// Function definitions
void printFaceList (nodePtr list);

// Convert a linked list of data into an array.
facePtr listToFaceArray (nodePtr list, int listLength);

// Allocate memory for a face pointer
//  and fill it with null values.
facePtr createEmptyFace (void);

// Allocate memory for a face structure,
//  insert the values provided
//  and return a pointer to the structure.
facePtr newFace (int faceId, int numVertices, int v1, int v2, int v3, int v4);

// Check if two faces share the same set of vertices.
// The order of the vertices does not matter.
// Return true if they are.
boolean equal_faces (faceStruct face_1, faceStruct face_2);

// Look if any of the vertices that compose a face
//  matches the vertexIndex specified.
// If it does, reorder the vertices in the face,
//  so that the vertex of interest is at the beginning
//  and return TRUE.
// If the vertex is not found in the face, return FALSE.
boolean reorderFace (facePtr face, int vertexIndex);

// Find a given vertex inside a mesh defined
//  by triangles.
// The list of triangles holds indices that
//  correspond to an array of vertices.
// Return a boolean, indicating if the vertex
// is part of the object or not.
boolean find_vertex_in_object (int vertex_index, nodePtr triangle_list, vertexPtr vertexArray);

// Get the area of a triangular face in 3D space.
double area3D_face (faceStruct face, vertexPtr vertex_array);

// Compute the area of the faces around a vertex.
double area3D_cone (nodePtr triangle_list, vertexPtr vertex_array, facePtr face_array);

// Compute the surface area of a whole mesh.
double object_surface_area (facePtr face_array, int num_faces, vertexPtr vertex_array);

// Compute the total length of the
//  border of a vertex star.
double get_link_length (nodePtr triangle_list, vertexPtr vertex_array, facePtr face_array, int vertex_index);
