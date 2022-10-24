// Extract the normals from a list of faces.
// Return a list with only the normals of the faces.
nodePtr getFaceNormalList (nodePtr faceList, facePtr faceArray, int* numNormals, int vertex_index, nodePtr* feature_edge_list);


// Add extra vectors between pairs of opposite vectors.
// When two faces lie on the same plane, but in contrary directions.
void check_opposite_vectors (nodePtr normal_list, facePtr faceArray, vertexPtr vertexArray, int* numNormals);


// Delete normals from a list, whenever they are
//  equal to the previous item.
void remove_duplicate_normals (nodePtr* normal_list);


// Locate loops in the normal star of a vertex.
// This will be the case mostly of saddle type vertices.
// Test whether the pattern of vertices repeats itself
//  when traversing the star.
// Each repeated loop is stored as an individual list.
// Returns a list containing each of these individual polygons.
nodePtr findLoopNormals (nodePtr normalList, int numNormals, int* numBasicPolygons);


// Determine if the vertices of a spherical polygon intersect
//  each other at the vertices themselves.
// Returns a list of lists, containing each individual polygon.
nodePtr findBasicIntersections (nodePtr normalList, int numNormals, int* numBasicPolygons);


// Compare a new intersection against those already
//  stored in a list.
void matchIntersectionPairs (int vectorId_1, int vectorId_2, int vectorId_3, int vectorId_4, int new_Id);


// Set the list of intersection of a single segment
//  in the correct order.
void orderIntersectionsList (nodePtr* tmpList, int tmpLength, vectorPtr vector1, vectorPtr intersection);


// Free the memory used for the
//  matching of intersection pairs.
void free_pairs_list (void);


// Divide a self intersecting spherical polygon into 
//  several simple polygons.
// Finds the vectors where the arcs that define a polygon
//  have self intersections.
// Returns a list of the normals of the object, plus
//  the vectors where the arcs intersect.
nodePtr findArcIntersections (nodePtr faceList, boolean* arcsIntersect);


// Do an extra check, to see if the neighbours are really
//  separated by an intersection.
// In the case of intersections at the same location as
//  two face normals.
boolean double_check_intersection (vectorPtr vector1_1, vectorPtr vector1, vectorPtr vector2, vectorPtr vector3_1, vectorPtr vector3, vectorPtr vector4);


// Cycle through the list of normals and call
//  'splitSphericalPolygon' for each possibility
// Return the list of polygons that contains the
//  polygon with the longest perimeter.
nodePtr testPolygonSplit (nodePtr polygonList, nodePtr normalList);


// Measure the length of the arcs that delimit a
//  spherical polygon.
double measure_spherical_polygon_perimeter (nodePtr polygon_list);


// Determine the possible orientation of a spherical
//  polygon, based on the number of angles in each
//  orientation, in the curve that delimits its perimeter.
angleType find_spherical_polygon_orientation (nodePtr polygon_list, int* orientation_changes);


// Separate a complex spherical polygon into several
//  simple ones, removing any self intersections.
// Return a list of lists of normals. Each list will
//  represent a simple polygon.
nodePtr splitSphericalPolygon (nodePtr polygonList, nodePtr normalList);


// Delete from the list of pairs those for which the
//  vectors involved are no longer present in the list.
void discard_redundant_pairs (int* remainingPairs, nodePtr normalList, int listLength);


// Compute the area of a polygon that lies on the surface of
//  a sphere.
// The polygon is defined by the normals of a 3D object.
// The normals are unit vectors, all beginning at the origin.
double findSphericalPolygonArea (nodePtr normalList, spinType* orientation, boolean c_h_vertex, vertexPtr coneVertexArray, int coneVertexArrayLength);


// Find out if the orientatio of an angle formed by three points on
//  the surface of a sphere.
// Returns COL, CO or CCW.
spinType sphereAngleOrientation (vectorPtr vector1, vectorPtr vector2, vectorPtr vector3, int type);


// Compute the angle between two great circles in a sphere.
// The arcs are defined by 3 vectors, and the angle is computed
//  at the middle vector.
double findSphericalAngle (vectorPtr vector1, vectorPtr vector2, vectorPtr vector3, spinType polygonOrientation, angleType* angleOrientation);


// Find a vector tangent to a sphere, that lies on the same plane
//  as two radius vectors of the unit sphere.
vectorPtr findSphereTangent (vectorPtr mainVector, vectorPtr referenceVector);


// Determine the vector where two great circles of a sphere intersect.
// The great circles are defined by arcs, which in turn are given by
//  by two vectors each.
// Returns the vector where the two arcs intersect, or NULL if the
//  arcs do not intersect.
vectorPtr arcIntersection (vectorPtr arc1vector1, vectorPtr arc1vector2, vectorPtr arc2vector1, vectorPtr arc2vector2, boolean* intersect, int* endpoint);


// Remove the vertices from a polygon which are
//  collinear with the adjacent vertices.
// Returns a copy of the parameter list, minus
//  the useless vertices.
nodePtr remove_collinear_vertices (nodePtr polygonList);


// Divide a spherical polygon into triangles,
//  to facilitate its rendering.
void triangulateSphericalPolygon (nodePtr polygonList, spinType polygonOrientation);


// Divide a spherical polygon into two simpler polygons.
// The division is done by drawing a diagonal between two
//  vertices of the polygon. These two vertices will now
//  belong to both of the polygons.
void divideSphericalPolygon (nodePtr polygonList, nodePtr* part1List, nodePtr* part2List, spinType polygonOrientation);


// Determine if a vertex is inside of a spherical triangle.
// Returns true if the point is inside.
boolean pointInsideSphericalTriangle (vectorPtr vertex1, vectorPtr vertex2, vectorPtr vertex3, vectorPtr vertex4, spinType polygonOrientation);
