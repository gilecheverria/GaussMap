// Compute the normals of each face, and then of each verte.
vertexPtr normalsFromVertices (vertexPtr vertices, nodePtr faceList, int numVertices);

// Compute the normal of a triangle, by using its vertices
//  to create 2 vectors, and then perform the cross product
//  on them. Finally normalize the vector length.
vectorPtr getTriangleNormal (vertexStruct vertex1, vertexStruct vertex2, vertexStruct vertex3);
