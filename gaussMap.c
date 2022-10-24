#include "gaussMap.h"

// External functions
extern int glMain ();

// Global variables associated with the objects generated
objectStruct	ObjectData;
int				ShowVertex = 0;
int				Begin;
int				End;
double*			VertexAngleArray = NULL;
double			TotalGaussMapArea = 0.0;
double			PositiveGaussMapArea = 0.0;
double			NegativeGaussMapArea = 0.0;
double			GaussianCurvature = 0.0;

double			PositiveGaussian = 0.0;
double			NegativeGaussian = 0.0;
double			Roughness = 0.0;

double			MAX_TAC = 0.0;

// Structure to keep information gathered
//  about each individual vertex.
vertexDataStruct*	VertexDataArray = NULL;


// Variable to be used for the display of the 
//  spherical polygons as sets of triangles.
nodePtr			PolygonTriangles = NULL;

// Variable to keep the vertices of the 
//  Convex Hull of the dataset.
vertexPtr		C_H_NormalArray = NULL;
nodePtr			ConvexHull = NULL;

// Function declarations
void usage (char* programName);
void generateObject (char* inputFile);
void generateGaussMap ();
void process_vertex (int vertex_index);
void classify_vertex (vertexDataStruct* current_vertex_data, int numPolygons, int positive_polys, int flat_polys, int negative_polys);
void find_concave_vertices (vertexDataStruct* current_vertex_data, int vertex_index);
void split_mixed_vertex (int vertex_index, vertexPtr coneVertexArray, int coneVertexArrayLength);
nodePtr faces_per_vertex (objectStruct objectData);
nodePtr get_vertex_faces (vertexStruct currentVertex, objectStruct objectData);
nodePtr order_faces_around_vertex (nodePtr tempList, facePtr faceArray, int vertex_index);
double getAngleAroundVertex (nodePtr faceIndexList, facePtr faceArray, vertexPtr vertexArray, int vertexIndex, boolean* border);
vertexPtr getConeVertices (nodePtr faceList, facePtr faceArray, vertexPtr vertexArray, vertexDataPtr current_vertex_data);
vectorStruct get_average_normal (vertexStruct vertex_normal);
vectorStruct getArtificialNormal (nodePtr faceList, facePtr faceArray, vertexPtr vertexArray);
// double getConeStarAngle (nodePtr faceList, facePtr faceArray, vertexPtr vertexArray);
void project_cone_on_plane (vertexPtr vertexArray, vertexDataPtr current_vertex_data);
void freeObjectMemory ();

void testTriangulation (nodePtr polygonList, spinType* orientations);
void findFacesIntersection (int faceIndex1, int faceIndex2);

void herisson_swap (objectStruct ObjectData);


#ifndef DECIMATOR
// program entry point
int main (int argc, char* argv[])
{
	char		inputFile[60];
	time_t		start;
	time_t		stop;

	if (argc < 2)
	{
		usage (argv[0]);
		exit (1);
	}

	strcpy (inputFile, argv[1]);

	if (argc == 3)
		ShowVertex = atoi(argv[2]);

	////////	OBJECT ACQUISITION	////////
	printf ("\n[STAGE 1] -- READING DATA: '%s'\n", inputFile); fflush (stdout);
	(void) time (&start);
	generateObject (inputFile);
	(void) time (&stop);
	printf ("[done] (%ld seconds)\n", (int) stop - start); fflush (stdout);

	////////	OBJECT PROCESSING	////////
	printf ("\n[STAGE 2] -- COMPUTING GAUSS MAP\n"); fflush (stdout);
	(void) time (&start);
	generateGaussMap ();
	(void) time (&stop);
	printf ("[done] (%ld seconds)\n", (int) stop - start); fflush (stdout);

	////////	RENDERING WITH OpenGL	////////
	printf ("\n[STAGE 3] -- GRAPHICAL DISPLAY\n"); fflush (stdout);
	(void) time (&start);
	glMain ();
	(void) time (&stop);
	printf ("[done] (%ld seconds)\n", (int) stop - start); fflush (stdout);

	freeObjectMemory ();

	return (0);
}


// Prints information about the arguments to supply to the program.
void usage (char* programName)
{
	printf ("Usage: %s input_file_name [vertex_number]\n", programName);
	printf ("\tinput_file_name:\tobj file\n");
	printf ("\tvertex_number:\t\tonly one vertex analysed\n");
	exit (1);
}
#endif


// Read the input file and generate an Object structure.
void generateObject (char* inputFile)
{
	// Read the input file and store the information in a list.
	ObjectData = readObjFile (inputFile);

	printf ("\tObject read has %d vertices and %d faces and %d edges\n", ObjectData.numVertices, ObjectData.numFaces, ObjectData.numEdges);

	// Change the normals of faces with concave edges.
	// herisson_swap (ObjectData);
}


// Process the object data to obtain the Gauss Map.
void generateGaussMap ()
{
	int			i;
	int			numVertices = 0;

	numVertices = ObjectData.numVertices;

	// Check whether to do the computations for only one vertex
	//  or for all.
	if (ShowVertex == 0)
	{
		Begin = 0;
		End = numVertices;
	}
	else
	{
		// Check that the Vertex selected is within range.
		if (ShowVertex > numVertices)
		{
			printf ("Sample has only %d vertices. Can't display Vertex %d.\nExiting.\n", numVertices, ShowVertex);
			exit (1);
		}

		Begin = ShowVertex - 1;
		End = ShowVertex;
	}

	/*
	printf ("\tComputing the Convex Hull:\t"); fflush (stdout);
	// Get the Convex Hull of the dataset.
	ConvexHull = compute_3D_Convex_Hull (ObjectData.vertexArray, ObjectData.numVertices);
	C_H_NormalArray = normalsFromVertices (ObjectData.vertexArray, ConvexHull, ObjectData.numVertices);
	printf (" Done\n"); fflush (stdout);
	*/

	// Allocate memory for the structure of data for each vertex.
	VertexDataArray = (vertexDataStruct*) xmalloc (sizeof (vertexDataStruct) * (numVertices + 1));

	// Reset values in the vertex structure.
	for (i=0; i<=numVertices; i++)
		initializeVertexDataStructure (&VertexDataArray[i]);

	printf ("\tProcessing the vertices:\t"); fflush (stdout);
    for (i=Begin; i<End; i++)
	{
#if (DEBUG >= 1)
	printf ("\n---> FOR VERTEX %d:\n", i+1); fflush (stdout);
#endif

		process_vertex (i+1);

#ifndef DEBUG
		printProgressStar (i, End-Begin);
#endif
	}
	printf (" Done\n"); fflush (stdout);

// findFacesIntersection (2, 4);

	printf ("\n\t=== ABSOLUTE CURVATURE:\t\t%lf ===\n", TotalGaussMapArea);
	printf ("\t--- POSITIVE CURVATURE:\t\t%lf ---\n", PositiveGaussMapArea);
	printf ("\t--- NEGATIVE CURVATURE:\t\t%lf ---\n", NegativeGaussMapArea);
	printf ("\t--- GAUSSIAN CURVATURE:\t\t%lf ---\n", GaussianCurvature);
	printf ("\t----------------------------------------\n");
	printf ("\t--- MESH SURFACE AREA:\t%lf ---\n", ObjectData.surfaceArea);

#if (DEBUG >= 2)
	printf ("\t--- DIFFERENCE (K+ - K-):\t%.15lf ---\n", PositiveGaussMapArea - NegativeGaussMapArea);
	printf ("\t--- GAUSSIAN CURVATURE (k):\t%.15lf ---\n", GaussianCurvature);

	if (	( GaussianCurvature > (PositiveGaussMapArea - NegativeGaussMapArea) - 1E-6 ) &&
			( GaussianCurvature < (PositiveGaussMapArea - NegativeGaussMapArea) + 1E-6 ) )
		printf ("\tSIGNATURES MATCH\n");
	else
		printf ("\tWARNING! SIGNATURES DO NOT MATCH!\n");
#endif
}


/*
void BIG_PENTA_SADDLE_CHEAT (nodePtr polygon_list)
{
	int				i;
	nodePtr			bad_polygon = NULL;
	nodePtr			new_polygon = NULL;
	nodePtr			new_polygon_tail = NULL;
	nodePtr			polygon_list_tail = NULL;
	vectorPtr		vector = NULL;
	vectorPtr		vector_copy = NULL;

	printf ("\nBIG PENTA SADDLE CHEAT ------------ BEGIN\n");

	bad_polygon = (nodePtr) getNodeData (polygon_list, 2);
	printVectorList (bad_polygon);

	vector = (vectorPtr) getNodeData (bad_polygon, 1);
	vector_copy = (vectorPtr) xmalloc (sizeof (vectorStruct));
	memcpy (vector_copy, vector, sizeof (vectorStruct));
	vector_copy->vectorId += 100;

	new_polygon = addNode (new_polygon, vector_copy);
	new_polygon_tail = new_polygon;

	for (i=0; i<8; i++)
	{
		vector = (vectorPtr) removeNode (&bad_polygon, 2);
		new_polygon_tail = addNode (new_polygon_tail, vector);
	}

	vector = (vectorPtr) getNodeData (bad_polygon, 2);
	vector_copy = (vectorPtr) xmalloc (sizeof (vectorStruct));
	memcpy (vector_copy, vector, sizeof (vectorStruct));
	vector_copy->vectorId += 100;
	new_polygon_tail = addNode (new_polygon_tail, vector_copy);

	printf ("Bad polygon:\n");
	printVectorList (bad_polygon);
	printf ("New polygon:\n");
	printVectorList (new_polygon);

	polygon_list_tail = getListTail (polygon_list);
	polygon_list_tail = addNode (polygon_list_tail, new_polygon);

	printf ("BIG PENTA SADDLE CHEAT ------------ END\n\n");
}
*/


// Do all of the checks for the vertex indicated.
// Fills all the data structures for that vertex.
void process_vertex (int vertex_index)
{
	nodePtr			polygonNormals = NULL;
	nodePtr			normalList = NULL;
	nodePtr			extendedNormalList = NULL;
	nodePtr			vertexFacesList = NULL;
	nodePtr			loopPolygons = NULL;
	nodePtr			convex_hull_list = NULL;
	int				j;
	int				numLoopPolygons = 0;
	int				numPolygons = 0;
	int				numNormals = 0;
	int				positive_polys = 0;
	int				negative_polys = 0;
	int				flat_polys = 0;
	int				coneVertexArrayLength;
	boolean			arcsIntersect = FALSE;
	boolean			c_h_vertex = FALSE;
	double			localArea = 0.0;
	vertexPtr		coneVertexArray = NULL;			// Array of the vertices of the star around a vertex.
	vertexPtr		projectedVertexArray = NULL;	// Array of the vertices projected on a plane.
	vectorStruct	vertexNormal;
	vertexDataStruct	current_vertex_data;

// printf ("PROCESSING ==> ");
// print_faces_around_vertex (vertex_index);

	current_vertex_data = VertexDataArray[vertex_index];

	// Get the list of faces that corresponds to
	//  the current vertex.
	vertexFacesList = ObjectData.vertexFaceArray[vertex_index];

	// Skip over vertices that do not appear in any face.
	if (ObjectData.facesPerVertex[vertex_index] == 0)
	{
		// Mark the vertex as deleted, in case the
		//  mesh is stored into a new obj file.
		ObjectData.vertexArray[vertex_index].vertexId *= -1;
		return;
	}

	// Order the faces around the current vertex in
	//  CCW orientation.
	vertexFacesList = order_faces_around_vertex (vertexFacesList, ObjectData.faceArray, vertex_index);
	ObjectData.vertexFaceArray[vertex_index] = vertexFacesList;

	// Get the angle around the current vertex,
	//  and store it in an array.
	current_vertex_data.angle_deficit = getAngleAroundVertex (vertexFacesList, ObjectData.faceArray, ObjectData.vertexArray, vertex_index, &(current_vertex_data.border_vertex) );
	GaussianCurvature += current_vertex_data.angle_deficit;

	// Create a new vertexArray with only the vertices
	//  in the star of the current vertex.
	// This will be used to determine the C.H. of the cone
	//  and used to compute the orientation.
	coneVertexArray = getConeVertices (vertexFacesList, ObjectData.faceArray, ObjectData.vertexArray, &current_vertex_data);
	// The length of the array will be the number of faces,
	//  plus one for the current vertex; plus the empty
	//  vertex at the beginning, but that one is not counted.
	coneVertexArrayLength = current_vertex_data.points_around_vertex;

	// Determine if the vertex is in a saddle or a cone.
	convex_hull_list = compute_3D_Convex_Hull (coneVertexArray, coneVertexArrayLength);
	c_h_vertex = find_vertex_in_object (vertex_index, convex_hull_list, coneVertexArray);
	freeList (convex_hull_list);

	// Get the artificial normal
	// vertexNormal = get_average_normal (ObjectData.normalArray[vertex_index]);
	vertexNormal = getArtificialNormal (vertexFacesList, ObjectData.faceArray, ObjectData.vertexArray);
	// Add to the array of artificial vertex normals.
	current_vertex_data.artificial_normal = vertexNormal;

	// Get the projection of the vertices of the star
	//  on a plane defined by the artificial normal.
	project_cone_on_plane (coneVertexArray, &current_vertex_data);
	projectedVertexArray = current_vertex_data.projected_vertices_array;

	// Obtain a list of the normals corresponding to
	//  the current vertex.
	normalList = getFaceNormalList (vertexFacesList, ObjectData.faceArray, &numNormals, vertex_index, &(current_vertex_data.feature_edge_list));

/*
if (vertex_index == 8289)
{
printf ("\nCHECKPOINT 1\n");
printVectorList (normalList);
}
*/

	// Special case for two faces on the same plane.
	// Add extra vectors between the 2 opposite normals.
	check_opposite_vectors (normalList, ObjectData.faceArray, ObjectData.vertexArray, &numNormals);

/*
if (vertex_index == 8289)
{
printf ("\nCHECKPOINT 2\n");
printVectorList (normalList);
}
*/

	// Check if the indicatrix generates loops
	//  of equal spherical polygons.
	// (i.e.: Monkey Saddle, Penta Saddle)
	loopPolygons = findLoopNormals (normalList, numNormals, &numLoopPolygons);

	// For each of the looped polygons, do the
	//  tests for self intersections.
	for (j=0; j<numLoopPolygons; j++)
	{
		normalList = (nodePtr) getNodeData (loopPolygons, j);

		// Locate where the arcs intersect each other.
		extendedNormalList = findArcIntersections (normalList, &arcsIntersect);

/*
if (vertex_index == 8289)
{
printf ("\nCHECKPOINT 3\n");
printVectorList (normalList);
}
*/

		// Divide the map into individual areas.
		// current_vertex_data.spherical_polygon_list = NULL;
		// current_vertex_data.spherical_polygon_list = testPolygonSplit (current_vertex_data.spherical_polygon_list, extendedNormalList);
		current_vertex_data.spherical_polygon_list = splitSphericalPolygon (current_vertex_data.spherical_polygon_list, extendedNormalList);
		// Deallocate information no longer used.
		free_pairs_list ();

		// Add to the existing normals, specially for
		//  the case of two or more loops.
		joinLists ( &(current_vertex_data.normals_around_vertex), extendedNormalList);
	}

// printf ("\nCHECKPOINT 4\n");
// printVectorList (normalList);

	freeListOfLists (loopPolygons);

// Correct the areas for the file Data/Bugs/penta-saddlenew-inter1.obj
// BIG_PENTA_SADDLE_CHEAT (current_vertex_data.spherical_polygon_list);

	numPolygons = getListLength (current_vertex_data.spherical_polygon_list);

	// Allocate memory for an array which will hold
	//  the orientation of each of the areas found.
	current_vertex_data.spherical_polygon_orientation = (spinType*) xmalloc (sizeof (spinType) * numPolygons);
	memset (current_vertex_data.spherical_polygon_orientation, 0, sizeof (spinType) * numPolygons);

	positive_polys = 0;
	negative_polys = 0;
	flat_polys = 0;

#if (DEBUG >= 1)
	printf ("  ->POLYGON AREA:\n"); fflush (stdout);
#endif
	// Compute the total area of the Gauss Map.
	for (j=0; j<numPolygons; j++)
	{
		polygonNormals = (nodePtr) removeNode (&(current_vertex_data.spherical_polygon_list), j);
		remove_duplicate_normals (&polygonNormals);
		// Re-insert the new list into the
		//  vertex data structure.
		if (j == 0)
			current_vertex_data.spherical_polygon_list = addFrontNode (current_vertex_data.spherical_polygon_list, polygonNormals);
		else
			insertDataAfterIndex (current_vertex_data.spherical_polygon_list, polygonNormals, j-1);

		localArea = findSphericalPolygonArea (polygonNormals, &current_vertex_data.spherical_polygon_orientation[j], c_h_vertex, coneVertexArray, coneVertexArrayLength);
		current_vertex_data.curvature += localArea;
		TotalGaussMapArea += localArea;

		if (current_vertex_data.spherical_polygon_orientation[j] == CCW)
		{
			positive_polys++;
			PositiveGaussMapArea += localArea;
		}
		else if (current_vertex_data.spherical_polygon_orientation[j] == CW)
		{
			negative_polys++;
			NegativeGaussMapArea += localArea;
		}
		else if (current_vertex_data.spherical_polygon_orientation[j] == COL)
			flat_polys++;

// testTriangulation (current_vertex_data.spherical_polygon_list, current_vertex_data.spherical_polygon_orientation);
	}

	// Find the largest curvature of any vertex.
	if (current_vertex_data.curvature > MAX_TAC)
		MAX_TAC = current_vertex_data.curvature;

#if (DEBUG >= 1)
	printf ("  ->POLYGON AREA - DONE!\n\n"); fflush (stdout);
#endif

	classify_vertex (&current_vertex_data, numPolygons, positive_polys, flat_polys, negative_polys);

	// Only for mixed of positive vertices,
	//  check if they are convex or concave.
	if ( (current_vertex_data.type == MIXED) || (current_vertex_data.type == ALL_POS) )
	{
		find_concave_vertices (&current_vertex_data, vertex_index);
	}

	// Store all values again in the array.
	VertexDataArray[vertex_index] = current_vertex_data;

/*
/////////////////////////////////////////////////
// 
// NEW TEST FUNCTION TO SPLIT MIXED VERTICES
	if (current_vertex_data.type == MIXED)
		split_mixed_vertex (vertex_index, coneVertexArray, coneVertexArrayLength);
// 
/////////////////////////////////////////////////
*/

	free (coneVertexArray);
}


// Classify the type of vertex.
void classify_vertex (vertexDataStruct* current_vertex_data, int numPolygons, int positive_polys, int flat_polys, int negative_polys)
{
#if (DEBUG >= 2)
	printf ("  ->VERTEX CLASSIFICATION:\n");
#endif

	if ( numPolygons == (positive_polys + flat_polys) && (positive_polys > 0) )
		current_vertex_data->type = ALL_POS;
	else if ( numPolygons == (negative_polys + flat_polys) && (negative_polys > 0) )
		current_vertex_data->type = ALL_NEG;
	else if ( flat_polys == numPolygons )
		current_vertex_data->type = FLAT;
	else
		current_vertex_data->type = MIXED;

#if (DEBUG >= 2)
	printf ("\tPOLYS:\tPOSITIVE=%d\tNEGATIVE=%d\tFLAT=%d\n", positive_polys, negative_polys, flat_polys); 
	printf ("\tTYPE OF VERTEX: %d\n", current_vertex_data->type);
	printf ("  ->VERTEX CLASSIFICATION - DONE!\n");
#endif
}


// Identify when a vertex is concave.
// Done by checking that all of the
//  neighbours are on one side of the plane
//  defined by the artificial normal.
void find_concave_vertices (vertexDataStruct* current_vertex_data, int vertex_index)
{
	int			num_neighbours = current_vertex_data->points_around_vertex;
	int			neighbour_index;
	int			i;
	vectorPtr	test_vector = NULL;


	for (i=2; i<=num_neighbours; i++)
	{
		neighbour_index = current_vertex_data->vertex_neighbour_index[i];
		// Build a vector along the edges,
		//  and then compare it to the artificial
		//  vertex normal.
		test_vector = vectorFromVertices ( &(ObjectData.vertexArray[neighbour_index]), &(ObjectData.vertexArray[vertex_index]) );
		if ( dotProduct (test_vector, &(current_vertex_data->artificial_normal)) < 0 )
		{
			if (current_vertex_data->type == MIXED)
				current_vertex_data->type = MIXED_CONCAVE;
			if (current_vertex_data->type == ALL_POS)
				current_vertex_data->type = ALL_POS_CONCAVE;

			break;
		}
	}
}


// Add vertices to a cone that is not convex, in such
//  a way that the tip of the cone will become convex,
//  and the new vertices will be saddles.
void split_mixed_vertex (int vertex_index, vertexPtr coneVertexArray, int coneVertexArrayLength)
{
	int				i, j;
	int				list_length = 0;
	nodePtr			coneConvexHull = NULL;
	facePtr			face = NULL;
	vectorPtr		plane_normal;
	vertexStruct	current_vertex;
	vertexStruct	plane_vertex;
	vertexStruct	projected_vertex;
	vertexStruct	v1, v2, v3;
	boolean			valid_face = FALSE;
	vertexStruct		Projected_Vertex;

	coneConvexHull = compute_3D_Convex_Hull (coneVertexArray, coneVertexArrayLength);

	mark_convex_hull_vertices (coneConvexHull, coneVertexArray, coneVertexArrayLength);

	list_length = getListLength (coneConvexHull);
	plane_vertex = coneVertexArray[1];

// printf("\n");
// printVertexArray (coneVertexArray, coneVertexArrayLength+1);

	// Don't do anything for saddle vertices.
	if (plane_vertex.extra != 1)
		return;

	for (i=2; i<coneVertexArrayLength; i++)
	{
		current_vertex = coneVertexArray[i];

		// If the vertex is not in the C.H.
		if (current_vertex.extra != 1)
		{
			for (j=0; j<list_length; j++)
			{
				face = (facePtr) getNodeData (coneConvexHull, j);
				valid_face = reorderFace (face, vertex_index);
// printf ("\n\t\tf %d (%d, %d, %d)\n",  face->faceId, face->vertex[0], face->vertex[1], face->vertex[2]);
				if ( valid_face && (face->vertex[1] == coneVertexArray[i-1].vertexId) )
				{

printf ("\n******* Projecting:\tv %d (%lf, %lf, %lf)\n", current_vertex.vertexId, current_vertex.x, current_vertex.y, current_vertex.z);
printf ("\t\t\tf %d (%d, %d, %d)\n",  face->faceId, face->vertex[0], face->vertex[1], face->vertex[2]);

					v1 = coneVertexArray[ face->vertex[0] ];
					v2 = coneVertexArray[ face->vertex[1] ];
					v3 = coneVertexArray[ face->vertex[2] ];
					plane_normal = getTriangleNormal (v1, v2, v3);

					projected_vertex = project_vertex_on_plane (*plane_normal, plane_vertex, current_vertex);

Projected_Vertex = projected_vertex;
printf ("******* At new vertex: %lf, %lf, %lf\n", projected_vertex.x, projected_vertex.y, projected_vertex.z);
				}
			}
		}
	}
}


// Run through the list of faces and ensure that they are 
//  in CCW order.
// Returns the same list, with the elements ordered.
nodePtr order_faces_around_vertex (nodePtr tempList, facePtr faceArray, int vertex_index)
{
	int			list_length = getListLength(tempList);
	int			i;
	int*		index = NULL;
	int			nextVertex = 0;
	int			previousVertex = 0;
	nodePtr		newList = NULL;
	nodePtr		newListTail = NULL;
	faceStruct	face;

// if (vertex_index == 8289)
// printf ("Resorting list of faces around vertex %d:\n", vertex_index);

	// Make sure each individual face has the vertex
	//  of interest first.
	for (i=0; i<list_length; i++)
	{
		index = (int*) getNodeData (tempList, i);
		face = faceArray[*index];
		reorderFace (&face, vertex_index);
		faceArray[*index] = face;
// if (vertex_index == 8289)
// printf ("\tFace %d: %d, %d, %d\n", face.faceId, face.vertex[0], face.vertex[1], face.vertex[2]);
	}

	// Insert a face to begin with.
	index = (int*) removeNode (&tempList, 0);
	face = faceArray[*index];
	newListTail = addNode (newListTail, index);
	newList = newListTail;
	list_length--;

	// Set the variables for the faces that should
	//  be inserted before or after the current list.
	nextVertex = face.vertex[2];
	previousVertex = face.vertex[1];

	// Exit the for loop when all the faces have
	//  been inserted in their correct position.
	for (i=0; list_length > 0; i++)
	{
		index = (int*) getNodeData (tempList, i%list_length);
		face = faceArray[*index];

		// Insert a face at the end of the list.
		if (face.vertex[1] == nextVertex)
		{
			nextVertex = face.vertex[2];
			newListTail = addNode (newListTail, index);

			removeNode (&tempList, i%list_length);
			list_length--;
		}
		// Insert a face at the beginning of the list.
		else if (face.vertex[2] == previousVertex)
		{
			previousVertex = face.vertex[1];
			newList = addFrontNode (newList, index);

			removeNode (&tempList, i%list_length);
			list_length--;
		}
	}

	return (newList);
}


// Compute the sum of the angles of the faces around a vertex.
// Receives as parameters the list of the faces for a vertex
//  and the array that contains all the vertices.
double getAngleAroundVertex (nodePtr faceIndexList, facePtr faceArray, vertexPtr vertexArray, int vertexIndex, boolean *border)
{
	int				i;
	int				first_index = -1;
	int				list_length = getListLength (faceIndexList);
	int*			index;
	double			angle = 0.0;
	double			total = 0.0;
	faceStruct		face;
	vertexStruct	vertex1;
	vertexStruct	vertex2;
	vertexStruct	vertex3;
	vectorPtr       vector1 = NULL;
	vectorPtr       vector2 = NULL;

	for (i=0; i<list_length; i++)
	{
		index = (int*) getNodeData (faceIndexList, i);
		face = faceArray[*index];
		vertex1 = vertexArray[(face.vertex[0])];
		vertex2 = vertexArray[(face.vertex[1])];
		vertex3 = vertexArray[(face.vertex[2])];

		vector1 = vectorFromVertices (&vertex1, &vertex2);
		vector2 = vectorFromVertices (&vertex1, &vertex3);

		angle = angleBetweenVectors (vector1, vector2);

		total += angle;

		free (vector1);
		free (vector2);

		if (first_index == -1)
			first_index = face.vertex[1];
	}

	// Obtain the correct gaussian for vertices
	//  on the borders. Non-border vertices will have the
	//  same neighbour vertex at the begining and end.
	if (first_index == face.vertex[2])
		return (2 * PI - total);
	else
	{
		// Mark the vertex as being on the border
		*border = TRUE;
		return (PI - total);
	}
}


// Create an array of vertices from those indicated
//  in a list of faces.
// Returns an array, with the vertices corresponding
//  to a cone.
// Also creates an array with only the indices of 
//  the neighbours, and inserts it into the
//  VertexDataArray.
vertexPtr getConeVertices (nodePtr faceList, facePtr faceArray, vertexPtr vertexArray, vertexDataPtr current_vertex_data)
{
	vertexPtr		coneVertexArray = NULL;
	vertexPtr		empty_vertex = NULL;
	int				list_length = getListLength (faceList);
	int				num_neighbours = list_length;
	int				i;
	int*			index = NULL;
	int*			neighbourIndexArray = NULL;
	faceStruct		face;

	// In the case of vertices in a border, the
	//  number of neighbours will be the number
	//  of adjacent faces plus one.
	if (current_vertex_data->border_vertex == TRUE)
		num_neighbours++;

	// The array will be the length of the number
	//  of neighbours plus 2: 1 for the empty
	//  vertex and 1 for the current vertex.
	coneVertexArray = (vertexPtr) xmalloc (sizeof (vertexStruct) * (num_neighbours + 2));
	// The array of neighbours also has an empty
	//  vertex, and the current vertex.
	// This is only an array of integers, with
	//  the indices to the vertices in the 
	//  array.
	neighbourIndexArray = (int*) xmalloc (sizeof (int) * (num_neighbours + 2));

	// Fill in the first two spaces of the array
	//  of vertices. These are not used anyway.
	neighbourIndexArray[0] = 0;
	neighbourIndexArray[1] = 1;

	// The first vertex in the array must be empty.
	// Create a new vertex, copy it, and then
	//  delete it, all so that 'valgrind' won't
	//  complain.
	empty_vertex = createVertex (0, 0, 0, 0);
	memcpy (&coneVertexArray[0], empty_vertex, sizeof (vertexStruct));
	free (empty_vertex);
	coneVertexArray[0].extra = 0;

	// Get the first face.
	index = (int*) getNodeData (faceList, 0);
	face = faceArray[*index];

	// Copy the structure for the current vertex.
	memcpy (&coneVertexArray[1], &vertexArray[face.vertex[0]], sizeof (vertexStruct));
	// coneVertexArray[1].vertexId = 1;

	// Copy the structure for the first vertex in the cone.
	memcpy (&coneVertexArray[2], &vertexArray[face.vertex[1]], sizeof (vertexStruct));
	// coneVertexArray[2].vertexId = 2;
	neighbourIndexArray[2] = face.vertex[1];

	// For each of the remaining faces.
	for (i=1; i<list_length; i++)
	{
		index = (int*) getNodeData (faceList, i);
		face = faceArray[*index];
		// Copy the structure for the i'th vertex in the cone.
		memcpy (&coneVertexArray[i+2], &vertexArray[face.vertex[1]], sizeof (vertexStruct));
		// coneVertexArray[i+2].vertexId = i+2;
		neighbourIndexArray[i+2] = face.vertex[1];
	}

	// Add another neighbour for border vertices.
	if (current_vertex_data->border_vertex == TRUE)
	{
		index = (int*) getNodeData (faceList, list_length-1);
		face = faceArray[*index];
		// Copy the structure for the i'th vertex in the cone.
		memcpy (&coneVertexArray[i+2], &vertexArray[face.vertex[2]], sizeof (vertexStruct));
		// coneVertexArray[i+2].vertexId = i+2;
		neighbourIndexArray[i+2] = face.vertex[2];
	}

	current_vertex_data->points_around_vertex = num_neighbours + 1;
	current_vertex_data->vertex_neighbour_index = neighbourIndexArray;

	return (coneVertexArray);
}


// Convert the normal of a vertex into a vector struct.
// The normal is already stored in ObjectData.normalArray
vectorStruct get_average_normal (vertexStruct vertex_normal)
{
	vectorStruct		normal_sum;

	normal_sum.i = vertex_normal.x;
	normal_sum.j = vertex_normal.y;
	normal_sum.k = vertex_normal.z;

	return (normal_sum);
}


// Compute an artificial normal vector for a vertex in a cone.
// New algorithm based on Shroeder et al.
vectorStruct getArtificialNormal (nodePtr face_list, facePtr face_array, vertexPtr vertex_array)
{
	int				i;
	int				list_length = getListLength (face_list);
	int*			face_index = NULL;
	faceStruct		face;
	vectorStruct	sum_vector;
	vectorStruct	average_vector;
	double			cone_area;
	double			triangle_area;

	sum_vector.i = 0.0;
	sum_vector.j = 0.0;
	sum_vector.k = 0.0;

	cone_area = area3D_cone (face_list, vertex_array, face_array);

	// Sum the face vectors multiplied by
	//  the face areas.
	for (i=0; i<list_length; i++)
	{
		face_index = (int*) getNodeData (face_list, i);
		face = face_array[*face_index];
					   
		triangle_area = area3D_face (face, vertex_array);

		sum_vector.i += face.faceNormal->i * triangle_area;
		sum_vector.j += face.faceNormal->j * triangle_area;
		sum_vector.k += face.faceNormal->k * triangle_area;
	}

	// Divide by the area of the cone.
	average_vector.i = sum_vector.i / cone_area;
	average_vector.j = sum_vector.j / cone_area;
	average_vector.k = sum_vector.k / cone_area;

	normalizeVector (&average_vector);

	return (average_vector);
}

/*
vectorStruct getArtificialNormal (nodePtr faceList, facePtr faceArray, vertexPtr vertexArray)
{
	// vectorPtr		edgeVector = NULL;
	// vectorPtr		axisSum = NULL;
	// vectorPtr		previousSum = (vectorPtr) xmalloc (sizeof (vectorStruct));
	// vertexStruct	starVertex;
	// vertexStruct	currentVertex;

	// Get the first face.
	face_index = (int*) getNodeData (faceList, 0);
	face = faceArray[*face_index];

	currentVertex = vertexArray[face.vertex[0]];

	// Sum the vectors corresponding to each edge
	for (i=0; i<list_length; i++)
	{
		// Get another vertex from the face
		face_index = (int*) getNodeData (faceList, i);
		face = faceArray[*face_index];
		starVertex = vertexArray[face.vertex[1]];

		// Create a vector along the edge
		edgeVector = vectorFromVertices (&starVertex, &currentVertex);
		normalizeVector (edgeVector);

		// Compute a perpendicular normal to the vertex.
		// This will be equal to the sum of all the edges
		//  on the sides of the faces.
		if (axisSum == NULL)
		{
			axisSum = (vectorPtr) xmalloc (sizeof (vectorStruct));
			memcpy (axisSum, edgeVector, sizeof (vectorStruct));
		}
		else
		{
			memcpy (previousSum, axisSum, sizeof (vectorStruct));
			free (axisSum);
			axisSum = sumVectors (edgeVector, previousSum);
		}
		free (edgeVector);
	}

	free (previousSum);
	normalizeVector (axisSum);

	return (*axisSum);
}
*/


// Project all the vertices of a cone into a plane that is defined
//  by the artificial normal of the vertex at the tip of the cone.
// Stores in the appropriate variables of the structure the following data:
// - vertex_neighbour_index : The vertexId of the neighbours
// - projected_vertices_array : New vertices corresponding to the
//		projection of the neighbours on a plane defined by the
//		artificial normal.
void project_cone_on_plane (vertexPtr vertexArray, vertexDataPtr current_vertex_data)
{
	int				i;
	int				numVertices;
	vertexStruct	current_vertex;
	vertexStruct	plane_vertex;
	vertexPtr		projectedVertexArray = NULL;	// Array of the vertices projected on a plane.
	vectorStruct	plane_normal;

	plane_normal = current_vertex_data->artificial_normal;
	numVertices = current_vertex_data->points_around_vertex;

	projectedVertexArray = (vertexPtr) xmalloc (sizeof (vertexStruct) * (numVertices + 1));

	plane_vertex = vertexArray[1];

	// Copy the first, empty vertex,
	//  and the current vertex.
	memcpy (&projectedVertexArray[0], &vertexArray[0], sizeof (vertexStruct));
	memcpy (&projectedVertexArray[1], &vertexArray[1], sizeof (vertexStruct));
	projectedVertexArray[1].vertexId = 1;

	for (i=2; i<=numVertices; i++)
	{
		current_vertex = vertexArray[i];

		// Get the projected vertex in the array.
		projectedVertexArray[i] = project_vertex_on_plane (plane_normal, plane_vertex, current_vertex);

		projectedVertexArray[i].vertexId = i;
		// projectedVertexArray[i].vertexId = -i;
		projectedVertexArray[i].extra = -i;
	}

	current_vertex_data->projected_vertices_array = projectedVertexArray;
}


// Deallocate the memory used for the global variables.
void freeObjectMemory ()
{
	int			i;

	// Again, use the index plus one to
	//  get the correct element of the
	//  array, considering the empty
	//  at the beginning.
	for (i=Begin; i<End; i++)
		freeVertexDataStructure ( &(VertexDataArray[i+1]) );

	free (VertexDataArray);

	free (ObjectData.vertexArray);
	free (ObjectData.normalArray);
	freeList (ObjectData.edgeList);
	for (i=1; i<=ObjectData.numVertices; i++)
	{
		freeList (ObjectData.vertexFaceArray[i]);
	}
	for (i=1; i<=ObjectData.numFaces; i++)
	{
		free (ObjectData.faceArray[i].faceNormal);
	}
	free (ObjectData.faceArray);
	free (ObjectData.vertexFaceArray);
	free (ObjectData.facesPerVertex);
}


// Test the subdivision of the gauss map areas
//  into spherical triangles.
// This is normally done in glGaussMap.c
//  for the visualisation.
void testTriangulation (nodePtr polygonList, spinType* orientations)
{
	nodePtr			polygonVertices = NULL;
	nodePtr			leanPolygon = NULL;
	int				numPolygons = getListLength (polygonList);
	int				numTriangles;
	int				numVertices;
	int				i;

	for (i=0; i<numPolygons; i++)
	{
		polygonVertices = (nodePtr) getNodeData (polygonList, i);

		// Remove the collinear vertices.
		leanPolygon = remove_collinear_vertices (polygonVertices);
		numVertices = getListLength (leanPolygon);

		// Triangulate the polygon.
		triangulateSphericalPolygon (leanPolygon, orientations[i]);
		numTriangles = getListLength (PolygonTriangles);

		freeListOfLists (PolygonTriangles);
		PolygonTriangles = NULL;
	}
}


// Locate the vector of intersection of two faces
void findFacesIntersection (int faceIndex1, int faceIndex2)
{
	vectorPtr		vector1;
	vectorPtr		vector2;
	vectorPtr		interVector = NULL;

	printf ("============= Vector of Intersection ==================\n");

	vector1 = ObjectData.faceArray[faceIndex1].faceNormal;
	vector2 = ObjectData.faceArray[faceIndex2].faceNormal;
	printf ("Vector 1 (face %d): %lf %lf %lf\n", faceIndex1, vector1->i, vector1->j, vector1->k);
	printf ("Vector 2 (face %d): %lf %lf %lf\n", faceIndex2, vector2->i, vector2->j, vector2->k);

	interVector = crossProduct (vector1, vector2);
	normalizeVector (interVector);
	printf ("Intersection Vector: %lf %lf %lf\n", interVector->i, interVector->j, interVector->k);

	printf ("============= Vector of Intersection ==================\n");
}


// Invert the normals for the faces that have at least
//  one concave edge.
void herisson_swap (objectStruct ObjectData)
{
	int				i, j;
	int				inversions_count = 0;
	int*			index = NULL;
	nodePtr			invertedFaces = NULL;
	edgePtr			edge = NULL;
	boolean			repeated_face_1;
	boolean			repeated_face_2;
	vectorPtr		normalVector = NULL;

	for (i=0; i<ObjectData.numEdges; i++)
	{
		edge = (edgePtr) getNodeData (ObjectData.edgeList, i);

		if (edge->type == CONCAVE)
		{
			repeated_face_1 = FALSE;
			repeated_face_2 = FALSE;

			// Check that the normals for a face have
			//  not already been inverted.
			for (j=0; j<inversions_count; j++)
			{
				index = (int*) getNodeData (invertedFaces, j);
				
				if ( *index == edge->face1 )
					repeated_face_1 = TRUE;
				if ( *index == edge->face2 )
					repeated_face_2 = TRUE;
			}

			// Invert the vector of the first face.
			if (!repeated_face_1)
			{
				normalVector = ObjectData.faceArray[edge->face1].faceNormal;
				normalVector->i *= -1;
				normalVector->j *= -1;
				normalVector->k *= -1;

				// Insert the face id into the list of already inverted normals.
				index = (int*) xmalloc (sizeof (int));
				*index = edge->face1;
				invertedFaces = addFrontNode (invertedFaces, index);
				inversions_count++;
			}

			// Invert the vector of the second face.
			if (!repeated_face_2)
			{
				normalVector = ObjectData.faceArray[edge->face2].faceNormal;
				normalVector->i *= -1;
				normalVector->j *= -1;
				normalVector->k *= -1;

				// Insert the face id into the list of already inverted normals.
				index = (int*) xmalloc (sizeof (int));
				*index = edge->face2;
				invertedFaces = addFrontNode (invertedFaces, index);
				inversions_count++;
			}
		}
	}

	for (i=inversions_count-1; i>=0; i--)
	{
		index = (int*) removeNode (&invertedFaces, i);
		free (index);
	}
	free (invertedFaces);
}


// Show the faces around a certain vertex.
void print_faces_around_vertex (int vertex_id)
{
	int			i;
	int*		face_index;
	nodePtr		faces_list = NULL;

	faces_list = ObjectData.vertexFaceArray[vertex_id];

	printf ("FACES AROUND VERTEX %d:\t", vertex_id);
	for (i=0; i<ObjectData.facesPerVertex[vertex_id]; i++)
	{
		face_index = (int*) getNodeData (faces_list, i);
		printf (" %d", *face_index);
	}
	printf ("\n");
}
