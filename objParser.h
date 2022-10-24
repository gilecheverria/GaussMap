// Parse an obj file and create lists with the vertices
//  and faces.
objectStruct readObjFile (char* filename);

// Create a new vertex structure with the information from
//  the .obj file.
vertexPtr readVertex (FILE* OBJ_FD);

// Create a new face structure with the information from
//  the .obj file.
facePtr readFace (FILE* OBJ_FD, boolean textured, boolean normals);

// Add a new face to the array of faces per vertex.
// The 'faceId' will be inserted in the lists corresponding
//  to each of the vertices that compose the face.
nodePtr* addFaceToVertexFaceArray (nodePtr* vertexFaceArray, facePtr face, int numVertices, int* facesPerVertex);
