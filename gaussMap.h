#include <stdio.h>
#include <math.h>

#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "faces.h"
#include "edges.h"
#include "object3D.h"
#include "objParser.h"
#include "vertexGeometry.h"
#include "sphericalGeometry.h"
#include "convexHull3D.h"
#include "normals3D.h"


// Read the input file and generate an Object structure.
void generateObject (char* inputFile);

// Process the object data to obtain the Gauss Map.
void generateGaussMap ();

// Deallocate the memory used for the global variables.
void freeObjectMemory ();

// Do all of the checks for the vertex indicated.
// Fills all the data structures for that vertex.
void process_vertex (int vertex_index);

void print_faces_around_vertex (int vertex_id);
