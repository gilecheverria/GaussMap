#include <stdio.h>

#include "gaussMap.h"

#define CURVATURE	curvature
// #define CURVATURE	angle_deficit

// External functions
extern int glMain ();

void gl_remove_lists (int removed_vertex_id);
void gl_update_lists (int removed_vertex_id);
void gl_create_display_lists ();

// External variables
extern objectStruct			ObjectData;
extern vertexDataStruct*	VertexDataArray;
extern int					ShowVertex;

// Function declarations
void get_file_name (char* input_file);
void usage (char* programName);
void sort_vertices_by_decimation_parameter (void);
void update_sorted_decimation_array (int vertex_index);
double get_vertex_decimation_parameter (int vertex_index);
int get_vertex_with_minimum_curvature (void);
void visual_vertex_decimation (void);
int remove_minimal_curvature_vertex (void);

// New functions for half edge collapse.
int find_closest_vertex (int removed_vertex_id, vertexDataStruct current_vertex_data);
nodePtr update_neighbour_triangles (int merge_vertex_id, int removed_vertex_id, vertexDataStruct current_vertex_data);

// Functions to check the initial triangulation.
void validate_edges (nodePtr* triangle_list, int num_triangles);
boolean validate_edge_flip (edgePtr edge, facePtr face_1, facePtr face_2);

// Functions to do edge flipping.
void minimise_triangulation_curvature (nodePtr triangle_list, int num_triangles, int vertexId, vertexDataStruct current_vertex_data, double old_curvature);
// void minimise_triangulation_curvature (nodePtr triangle_list, int num_triangles);
double get_edge_curvature (edgePtr edge, faceStruct face_1, faceStruct face_2);
void execute_edge_flip (edgePtr edge, facePtr face_1, facePtr face_2, boolean inverted);
boolean check_existing_faces (int test_vertex, edgePtr edge);
void update_face_normal_vector (facePtr face);
void update_flip_vertex_data (edgePtr edge);
void recompute_edge_vertex_curvature (edgePtr edge);
void register_face_changes (facePtr face);

double get_link_curvature (int vertexId, vertexDataStruct current_vertex_data);


// Data structure to hold decimation parameter
typedef struct decimationDataStruct
{
	int		vertexId;
	double	decimation_parameter;
} decimationDataStruct;

typedef decimationDataStruct* decimationDataPtr;


// Global variables
char*				File_Name = NULL;
int					Removal_count = 0;
int					Removal_index = 1;
int*				Sorted_vertex_index_array = NULL;
double				Max_parameter = 0.0;
decimationDataPtr	Curvature_sorted_array = NULL;


// Program entry point
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

	get_file_name (inputFile);

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

	if (ShowVertex == 0)
	{
		////////	SORT VERTICES BY CURVATURE	////////
		printf ("\n[STAGE 3] -- SORTING VERTICES BY CURVATURE\n"); fflush (stdout);
		(void) time (&start);
		sort_vertices_by_decimation_parameter ();
		(void) time (&stop);
		printf ("[done] (%ld seconds)\n", (int) stop - start); fflush (stdout);
	}

// remove_minimal_curvature_vertex ();

	////////	RENDERING WITH OpenGL	////////
	printf ("\n[STAGE 4] -- GRAPHICAL DISPLAY\n"); fflush (stdout);
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
	printf ("Usage: %s [input file name]\n", programName);
	exit (1);
}

void get_file_name (char* input_file)
{
	size_t			length;

	length = strlen (input_file);
	File_Name = (char*) xmalloc (sizeof (char) * (length + 1));

	// Copy the filename minus the extension
	// (Assuming a 3 letter extension).
	strncpy (File_Name, input_file, length-4);
}

// Make a list sorting all of the vertices by increasing curvature.
void sort_vertices_by_decimation_parameter (void)
{
	int						i;
	int						sort_start = 1;
	int						sort_end = ObjectData.numVertices;
	int						max_index;
	int						min_index;
	double					max_param;
	double					min_param;
	decimationDataStruct	temp_data;

	Curvature_sorted_array = (decimationDataPtr) xmalloc (sizeof (decimationDataStruct) * (ObjectData.numVertices + 1));
	Sorted_vertex_index_array = (int*) xmalloc (sizeof (int) * (ObjectData.numVertices + 1));

	// Fill the array of decimation parameters
	for (i=1; i<=ObjectData.numVertices; i++)
	{
		// Get new parameter and store in data structure.
		Curvature_sorted_array[i].vertexId = i;
		Curvature_sorted_array[i].decimation_parameter = get_vertex_decimation_parameter (i);
	}

	// Initialise values for the limit parameters.
	max_index = 1;
	max_param = Curvature_sorted_array[max_index].decimation_parameter;
	min_index = ObjectData.numVertices;
	min_param = Curvature_sorted_array[min_index].decimation_parameter;

	// Do the sorting of the array, from both sides.
	while (sort_start < sort_end)
	{
		// Locate the smallest and larger values in the
		//  area yet unsorted.
		for (i=sort_start; i<=sort_end; i++)
		{
			if (Curvature_sorted_array[i].decimation_parameter > max_param)
			{
				max_param = Curvature_sorted_array[i].decimation_parameter;
				max_index = i;
			}
			if (Curvature_sorted_array[i].decimation_parameter < min_param)
			{
				min_param = Curvature_sorted_array[i].decimation_parameter;
				min_index = i;
			}
		}

		// Swap the locations of the min value and the
		//  first item in the array.
		if (min_index > sort_start)
		{
			// printf ("Swapping min to %d = %lf\n", min_index, min_param);
			memcpy (&temp_data, &Curvature_sorted_array[min_index], sizeof(decimationDataStruct));
			memcpy (&Curvature_sorted_array[min_index], &Curvature_sorted_array[sort_start], sizeof(decimationDataStruct));
			memcpy (&Curvature_sorted_array[sort_start], &temp_data, sizeof(decimationDataStruct));
		}
		// Swap the locations of the max value and the
		//  last item in the array.
		if (max_index == sort_start)
			max_index = min_index;
		if (max_index < sort_end)
		{
			// printf ("Swapping max to %d = %lf\n", max_index, max_param);
			memcpy (&temp_data, &Curvature_sorted_array[max_index], sizeof(decimationDataStruct));
			memcpy (&Curvature_sorted_array[max_index], &Curvature_sorted_array[sort_end], sizeof(decimationDataStruct));
			memcpy (&Curvature_sorted_array[sort_end], &temp_data, sizeof(decimationDataStruct));
		}

		// Prepare for the next loop.
		// Update the sorting boundaries.
		sort_start++;
		sort_end--;
		// Swap the values of the limit parameters.
		max_index = sort_start;
		max_param = Curvature_sorted_array[max_index].decimation_parameter;
		min_index = sort_end;
		min_param = Curvature_sorted_array[min_index].decimation_parameter;

#ifndef DEBUG
	printProgressStar (sort_start*2, ObjectData.numVertices);
#endif
	}

	// Update the locations of the vertices in the
	//  index array.
	for (i=1; i<=ObjectData.numVertices; i++)
	{
		Sorted_vertex_index_array[Curvature_sorted_array[i].vertexId] = i;
	}

#if (DEBUG >= 1)
	printf ("--------\tFINAL SORTED PARAMETER ARRAY\t--------\n");
	printf ("Num\tIndex\tValue\t\t||\tVertex\tPossition\n");
	for (i=1; i<=ObjectData.numVertices; i++)
	{
		printf ("%d\t%d\t%lf\t||\t", i, Curvature_sorted_array[i].vertexId, Curvature_sorted_array[i].decimation_parameter);
		printf ("%d\t%d\n", i, Sorted_vertex_index_array[i]);
	}
#endif

}


// Update the entry for a modified vertex in the
//  list sorted by decimation parameter.
void update_sorted_decimation_array (int vertex_index)
{
	int						array_index;
	int						test_index;
	int						direction = 0;
	int						counter = -1;
	int						i;
	double					old_parameter;
	double					new_parameter;
	double					test_parameter;
	decimationDataStruct	temp_data;

	array_index = Sorted_vertex_index_array[vertex_index];

	old_parameter = Curvature_sorted_array[array_index].decimation_parameter;
	new_parameter = get_vertex_decimation_parameter (vertex_index);

/*
printf ("======================================================\n");
printf ("=====  VERTEX %d: OLD = %lf  NEW = %lf  =====\n", vertex_index, old_parameter, new_parameter);
*/

	// Check if the new parameter produces a change
	//  of the position of the vertex in the sorted table.
	if (old_parameter != new_parameter)
	{
		// Determine whether to go front or back in the array.
		if (old_parameter < new_parameter)
			direction = 1;
		if (old_parameter > new_parameter)
			direction = -1;

		test_index = array_index;

/*
printf ("=====  NEW DIRECTION: %d  =====\n", direction);

printf ("--------\tSORTED ARRAY BEFORE\t--------\n");
printf ("Num\tIndex\tValue\t\t||\tVertex\tPossition\n");
for (i=1; i<=ObjectData.numVertices; i++)
{
printf ("%d\t%d\t%lf\t||\t", i, Curvature_sorted_array[i].vertexId, Curvature_sorted_array[i].decimation_parameter);
printf ("%d\t%d\n", i, Sorted_vertex_index_array[i]);
}
*/

		// Get the location where the new parameter should be.
		do
		{
			test_index += direction;
			test_parameter = Curvature_sorted_array[test_index].decimation_parameter;
			counter++;
		} while (	(	(direction == 1) && (test_parameter < new_parameter) &&
						(test_index <= ObjectData.numVertices) ) ||
					(	(direction == -1) && (test_parameter > new_parameter) &&
						(test_index >= Removal_index ) )	);
	}

	if (counter > 0)
	{
// printf ("======  SHIFT %d places  =====\n", counter);

		Curvature_sorted_array[array_index].decimation_parameter = new_parameter;
		memcpy (&temp_data, &Curvature_sorted_array[array_index], sizeof (decimationDataStruct));

		if (direction == 1)
			memmove (	&Curvature_sorted_array[array_index],
						&Curvature_sorted_array[array_index + 1],
						sizeof (decimationDataStruct) * counter	);
		else // if (direction == -1)
		{
			memmove (	&Curvature_sorted_array[array_index - (counter - 1)],
						&Curvature_sorted_array[array_index - counter],
						sizeof (decimationDataStruct) * counter	);
		}

		memcpy (&Curvature_sorted_array[array_index + (counter * direction)], &temp_data, sizeof (decimationDataStruct));

		// Update the index table.
		for (i=0; i<=counter; i++)
		{
			temp_data = Curvature_sorted_array[array_index + (i * direction)];
			Sorted_vertex_index_array[temp_data.vertexId] = array_index + (i * direction);
		}

/*
printf ("--------\tSORTED ARRAY AFTER\t--------\n");
printf ("Num\tIndex\tValue\t\t||\tVertex\tPossition\n");
for (i=1; i<=ObjectData.numVertices; i++)
{
printf ("%d\t%d\t%lf\t||\t", i, Curvature_sorted_array[i].vertexId, Curvature_sorted_array[i].decimation_parameter);
printf ("%d\t%d\n", i, Sorted_vertex_index_array[i]);
}
*/
	}
	else
	{
		Curvature_sorted_array[array_index].decimation_parameter = new_parameter;
	}
}


double get_vertex_decimation_parameter (int vertex_index)
{
	int					num_neighbours;
	double				test_parameter;
	double				polygon_area;
	double				cone_area;
	double				area_ratio;
	double				curvature_parameter;
	double				link_length;
	vertexDataStruct	current_vertex_data;
	vertexPtr			projected_array = NULL;

	current_vertex_data = VertexDataArray[vertex_index];

	projected_array = current_vertex_data.projected_vertices_array;
	num_neighbours = current_vertex_data.points_around_vertex;

	// No further analysis for vertices without
	//  any incident faces.
	if (num_neighbours == 0)
		return (0.0);

	// Copy the contents of the last two
	//  elements in the array of neighbours
	//  into the first two places.
	// There should be an empty vertex, and
	//  the current vertex in the spaces
	//  that will be overwriten.
	memcpy (&projected_array[0], &projected_array[num_neighbours-1], sizeof (vertexStruct));
	memcpy (&projected_array[1], &projected_array[num_neighbours], sizeof (vertexStruct));

	// Get the length of the link around the vertex
	// (the border of the vertex star).
	link_length = get_link_length (ObjectData.vertexFaceArray[vertex_index], ObjectData.vertexArray, ObjectData.faceArray, vertex_index);

	// Get the area of the polygon defined
	//  by the projected neighbours.
	// WTAC
	// polygon_area = area3D_Polygon (num_neighbours-1, projected_array, ObjectData.normalArray[vertex_index]);
	// LTAC
	polygon_area = (link_length * link_length) / (4 * PI);

	// Get the absolute value of the area.
	if (polygon_area < 0)
		polygon_area *= -1;
	// Get the area of the triangles
	//  that form the cone.
	cone_area = area3D_cone (ObjectData.vertexFaceArray[vertex_index], ObjectData.vertexArray, ObjectData.faceArray);
	area_ratio = polygon_area / cone_area;

	curvature_parameter = fabs (current_vertex_data.CURVATURE);

	// TAC
	// test_parameter = curvature_parameter;
	// ATAC
	test_parameter = curvature_parameter * cone_area;
	// WTAC & LTAC
	// test_parameter = curvature_parameter * area_ratio;

// printf ("CHECKING VERTEX %d: curv = %lf, link = %lf, poly = %lf, cone = %lf, ratio = %lf, param = %lf\n", vertex_index, current_vertex_data.curvature, link_length/(4*PI), polygon_area, cone_area, area_ratio, test_parameter);

	return (test_parameter);
}


int get_vertex_with_minimum_curvature (void)
{
	static double			border_tolerance = 0.5;
	decimationDataStruct	decimation_data;
	vertexDataStruct		current_vertex_data;

	decimation_data = Curvature_sorted_array[Removal_index++];
	Sorted_vertex_index_array[decimation_data.vertexId] *= -1;

	current_vertex_data = VertexDataArray[decimation_data.vertexId];

	/*
	// Do not check again vertices that have
	//  already been deleted before.
	if (ObjectData.vertexArray[vertex_index].vertexId < 0)
		continue;
	*/

	// Skip over vertices at the corners of a
	//  border in an object.
	// This is only for open objects.
	if (current_vertex_data.border_vertex && (current_vertex_data.angle_deficit > border_tolerance) )
	{
		// printf (" -- ARE WE EVER DOING THIS??? --\n");

		// Recursively call the same function to get
		//  the next element in the list.
		return (get_vertex_with_minimum_curvature ());
	}

	return (decimation_data.vertexId);
}


// Delete a face selected for removal from the list
//  of adjacent faces of a vertex.
void remove_vertex_face (int vertex_index, int removed_face_id)
{
	int					i;
	int					list_length;
	int*				face_index;
	nodePtr				vertex_faces_list = NULL;

	vertex_faces_list = ObjectData.vertexFaceArray[vertex_index];
	list_length = getListLength (vertex_faces_list);

	for (i=list_length-1; i>=0; i--)
	{
		face_index = (int*) getNodeData (vertex_faces_list, i);
		if (*face_index == removed_face_id)
		{
			face_index = removeNode (&vertex_faces_list, i);
			free (face_index);
		}
	}

	ObjectData.vertexFaceArray[vertex_index] = vertex_faces_list;
	ObjectData.facesPerVertex[vertex_index]--;
}


// Add a new face to the list of adjacent faces
//  of a vertex.
void add_vertex_face (int vertex_index, int new_face_id)
{
	int*				new_face_index = (int*) xmalloc (sizeof (int));
	nodePtr				vertex_faces_list = NULL;

	vertex_faces_list = ObjectData.vertexFaceArray[vertex_index];

	memcpy (new_face_index, &new_face_id, sizeof (int));
	vertex_faces_list = addFrontNode (vertex_faces_list, new_face_index);

	ObjectData.vertexFaceArray[vertex_index] = vertex_faces_list;
	ObjectData.facesPerVertex[vertex_index]++;
}


// Compute the normal to a vertex.
// Used only for the 3D display.
// NOTE: normals to vertices are stored
//  as another vertex, not as a vector.
void compute_vertex_normal (int update_vertex_id)
{
	int				i;
	int				list_length;
	int*			face_index;
	double			xSquared;
	double			ySquared;
	double			zSquared;
	double			vectorLength;
	faceStruct		face;
	nodePtr			vertex_faces_list;
	vertexStruct	vertex_normal;

	vertex_normal.x = 0.0;
	vertex_normal.y = 0.0;
	vertex_normal.z = 0.0;

	vertex_faces_list = ObjectData.vertexFaceArray[update_vertex_id];
	list_length = getListLength (vertex_faces_list);

	for (i=0; i<list_length; i++)
	{
		face_index = (int*) getNodeData (vertex_faces_list, i);
		face = ObjectData.faceArray[*face_index];

		vertex_normal.x += face.faceNormal->i;
		vertex_normal.y += face.faceNormal->j;
		vertex_normal.z += face.faceNormal->k;
	}

	vertex_normal.vertexId = update_vertex_id;
	vertex_normal.x /= list_length;
	vertex_normal.y /= list_length;
	vertex_normal.z /= list_length;

	xSquared = vertex_normal.x * vertex_normal.x;
	ySquared = vertex_normal.y * vertex_normal.y;
	zSquared = vertex_normal.z * vertex_normal.z;

	vectorLength = sqrtf (xSquared + ySquared + zSquared);

	vertex_normal.x /= vectorLength;
	vertex_normal.y /= vectorLength;
	vertex_normal.z /= vectorLength;

	ObjectData.normalArray[update_vertex_id] = vertex_normal;
}


// Recompute curvature data for changed vertices.
void update_neighbours_data (int removed_vertex_id, boolean border_vertex)
{
	int					i;
	int					update_vertex_id;

	for (i=2; i<=VertexDataArray[removed_vertex_id].points_around_vertex; i++)
	{
		update_vertex_id = VertexDataArray[removed_vertex_id].vertex_neighbour_index[i];

		ObjectData.facesPerVertex[update_vertex_id] = getListLength (ObjectData.vertexFaceArray[update_vertex_id]);

		freeVertexDataStructure ( &VertexDataArray[update_vertex_id] );
		initializeVertexDataStructure ( &VertexDataArray[update_vertex_id] );
#if (DEBUG >= 1)
	printf ("\n---> FOR VERTEX %d:\n", update_vertex_id); fflush (stdout);
#endif
		process_vertex (update_vertex_id);
		// compute_vertex_normal (update_vertex_id);
	}

}


// Remove the next vertex with the smallest curvature.
int remove_minimal_curvature_vertex (void)
{
	int					i;
	int					removed_vertex_id;
	int					merge_vertex_id;
	int					num_triangles;
	int					num_edges;
	double				old_curvature = 0.0;
	nodePtr				triangle_list = NULL;
	nodePtr				edge_list = NULL;
	vertexDataStruct	current_vertex_data;

	if (	(ObjectData.numVertices - Removal_count) <= 4 ||
			(ObjectData.numVertices - Removal_count) <= (ObjectData.numVertices * 0.02) )
	{
		printf ("\nWARNING! UNABLE TO DO DECIMATE THE MESH FURTHER\n");
		return (-1);
	}

	removed_vertex_id = get_vertex_with_minimum_curvature ();

	current_vertex_data = VertexDataArray[removed_vertex_id];

	Removal_count++;
// printf ("\n=================================================\n");
// printf ("REMOVING # %5d: VERTEX: %7d  CURVATURE: %lf  PARAMETER: %lf\n", Removal_count, removed_vertex_id, current_vertex_data.CURVATURE, Curvature_sorted_array[-Sorted_vertex_index_array[removed_vertex_id]].decimation_parameter);
printf ("REMOVING # %5d: VERTEX: %7d\tCURVATURE: %lf\n", Removal_count, removed_vertex_id, current_vertex_data.CURVATURE);

	// Only do the retriangulation if there are
	//  enough vertices for that.
	if ( (current_vertex_data.points_around_vertex - 1) > 2)
	{
		// Get the curvature of the vertex star before the removal.
		old_curvature = get_link_curvature (removed_vertex_id, current_vertex_data);
		old_curvature += current_vertex_data.CURVATURE;

		// Find the appropriate merge vertex that
		//  matches the removed vertex.
		merge_vertex_id = find_closest_vertex (removed_vertex_id, current_vertex_data);

		// Remove two triangles and update the rest
		//  of the faces around the removed vertex.
		triangle_list = update_neighbour_triangles (merge_vertex_id, removed_vertex_id, current_vertex_data);
		num_triangles = getListLength (triangle_list);

		// Get the list of edges within the polygon.
		edge_list = get_edge_list (triangle_list, num_triangles);
		num_edges = getListLength (edge_list);
		// Remove edges on the borders.
		remove_border_edges (&edge_list, &num_edges);

		// Check for duplicate faces that may be produced
		//  from the initial triangulation or future edge flips.
		validate_edges (&edge_list, num_edges);

		// Recalculate all the information about the
		//  vertices which were just modified.
		update_neighbours_data (removed_vertex_id, current_vertex_data.border_vertex);

		// Optimise the triangulation via edge flips.
		if (num_triangles > 1)
			minimise_triangulation_curvature (edge_list, num_edges, removed_vertex_id, current_vertex_data, old_curvature);
			// minimise_triangulation_curvature (edge_list, num_edges);
	}

	else	// No new faces to add.
	{
		// Recalculate all the information about the
		//  vertices which were just modified.
		update_neighbours_data (removed_vertex_id, current_vertex_data.border_vertex);
	}

	// For each of the neighbours.
	for (i=2; i<=current_vertex_data.points_around_vertex; i++)
		// Modify the entry in the list of vertices sorted
		//  by decimation parameter.
		update_sorted_decimation_array (current_vertex_data.vertex_neighbour_index[i]);

	// Change the Id of the vertex so that it is
	//  not considered for computations or display.
	ObjectData.vertexArray[removed_vertex_id].vertexId *= -1;

	return (removed_vertex_id);
}


// Locate in the closest of a vertices neighbours.
// Return the index of the closest vertex.
int find_closest_vertex (int removed_vertex_id, vertexDataStruct current_vertex_data)
{
	int				i;
	int				vertex_index;
	int				closest_index = 2;
	double			distance;
	double			min_distance = 0.0;
	vertexStruct	removed_vertex;
	vertexStruct	test_vertex;

	removed_vertex = ObjectData.vertexArray[removed_vertex_id];

	for (i=2; i<=current_vertex_data.points_around_vertex; i++)
	{
		vertex_index = current_vertex_data.vertex_neighbour_index[i];
		test_vertex = ObjectData.vertexArray[vertex_index];

		distance = squareDistance (removed_vertex, test_vertex);
		
		if ( (distance < min_distance) || (min_distance == 0) )
		{
			min_distance = distance;
			closest_index = i;
		}
	}

	vertex_index = current_vertex_data.vertex_neighbour_index[closest_index]; 

	return (vertex_index);
}

// Remove the two triangles adjacent to the edge being collapsed
//  and update the rest of the triangles incident on the removed
//  vertex.
// Returns a list of faces that will be used later for edge flipping.
nodePtr update_neighbour_triangles (int merge_vertex_id, int removed_vertex_id, vertexDataStruct current_vertex_data)
{
	int					i;
	int					list_length;
	int*				face_index = NULL;
	nodePtr				face_list = NULL;
	nodePtr				triangle_list = NULL;
	facePtr				temp_face = NULL;
	faceStruct			face;

	face_list = ObjectData.vertexFaceArray[removed_vertex_id];
	list_length = getListLength (face_list);
	for (i=0; i<list_length; i++)
	{
		face_index = (int*) getNodeData (face_list, i);
		face = ObjectData.faceArray[*face_index];

		reorderFace (&face, removed_vertex_id);

		// Identify the faces to be removed.
		if ( (face.vertex[1] == merge_vertex_id) || (face.vertex[2] == merge_vertex_id) )
		{
			// Change the faceId to ensure the faces are no
			// longer considered for computations or display.
			ObjectData.faceArray[*face_index].faceId *= -1;

			// Remove the faces from the lists of the
			//  merge and adjacent vertices.
			remove_vertex_face (face.vertex[1], face.faceId);
			remove_vertex_face (face.vertex[2], face.faceId);
		}
		// If the face is being kept, update it
		//  with the 'merge_vertex_id'.
		else
		{
			// Update the face.
			face.vertex[0] = merge_vertex_id;
			update_face_normal_vector (&face);

			// Register it with the merge_vertex
			//  and the global array of faces.
			add_vertex_face (merge_vertex_id, face.faceId);
			ObjectData.faceArray[face.faceId] = face;

			// Add a copy of the face to a list of
			//  triangles.
			temp_face = (facePtr) xmalloc (sizeof (faceStruct));
			memcpy (temp_face, &face, sizeof (faceStruct));
			triangle_list = addFrontNode (triangle_list, temp_face);
		}
	}

	return (triangle_list);
}


// Check that the initial triangulation presented
//  does not produce coplanar triangles with those
//  outside of the retriangulated are.
// If such a case happens, then use edge flipping
//  to correct the triangulation.
void validate_edges (nodePtr* edge_list, int num_edges)
{
	int					i;
	edgePtr				edge;
	faceStruct			face_1;
	faceStruct			face_2;
	boolean				valid_edge;

	for (i=0; i<num_edges; i++)
	{
		edge = (edgePtr) getNodeData (*edge_list, i);

		face_1 = ObjectData.faceArray[edge->face1];
		face_2 = ObjectData.faceArray[edge->face2];

		valid_edge = validate_edge_flip (edge, &face_1, &face_2);

		// Flip the edge if it is not good.
		if (!valid_edge)
		{
			// Edge rotation in normal direction, hence the FALSE value.
			execute_edge_flip (edge, &face_1, &face_2, FALSE);
			recompute_edge_vertex_curvature (edge);
			*edge_list = update_neighbour_edges (face_1, face_2, *edge_list, num_edges);
		}
	}
}


// Perform edge flipping to improve on the
//  initial triangulation.
void minimise_triangulation_curvature (nodePtr edge_list, int num_edges, int vertexId, vertexDataStruct current_vertex_data, double old_curvature)
// void minimise_triangulation_curvature (nodePtr edge_list, int num_edges)
{
#define NEW
	int				i;
	int				max_flips;
	int				flip_counter = 0;
	double			original_curvature;
	double			flip_curvature;
	edgePtr			edge = NULL;
	faceStruct		face_1;
	faceStruct		face_2;
	boolean			valid_flip;

	// Set a maximum number of flips to try.
	// max_flips = num_edges;
	// max_flips = num_edges * 2;
	max_flips = num_edges * num_edges * num_edges;
	i = 0;

// printf ("\n");
// printEdgeList (edge_list, num_edges);

	while (max_flips > 0)
	{
// printf ("\n====== EDGE %d ======\n", i);
		edge = (edgePtr) getNodeData (edge_list, (i%num_edges));
		face_1 = ObjectData.faceArray[edge->face1];
		face_2 = ObjectData.faceArray[edge->face2];

#ifdef NEW
		original_curvature = get_link_curvature (vertexId, current_vertex_data);
#else
		original_curvature = get_edge_curvature (edge, face_1, face_2);
#endif

		// Edge rotation in normal direction, hence the FALSE value.
		execute_edge_flip (edge, &face_1, &face_2, FALSE);
		valid_flip = validate_edge_flip (edge, &face_1, &face_2);

		// Flip back the edge if the result is invalid.
		if (!valid_flip)
		{
			// Rollback case 1. Edge flip in inverted direction.
			execute_edge_flip (edge, &face_1, &face_2, TRUE);
		}
		else
		{
			recompute_edge_vertex_curvature (edge);
#ifdef NEW
			flip_curvature = get_link_curvature (vertexId, current_vertex_data);
#else
			flip_curvature = get_edge_curvature (edge, face_1, face_2);
#endif

// printf ("CURVATURE INITIAL: %lf\t||\tBEFORE: %lf\t||\tAFTER: %lf\n", old_curvature, original_curvature, flip_curvature);

			// If the resulting curvature is smaller,
			//  then accept the flip and update the
			//  data of the related vertices.
#ifdef NEW
			if ( fabs(old_curvature - flip_curvature) < fabs(old_curvature - original_curvature) )
#else
			if ( flip_curvature < original_curvature )
#endif
			{
				flip_counter++;
				edge_list = update_neighbour_edges (face_1, face_2, edge_list, num_edges);
			}
			// Otherwise flip back to the original state.
			else
			{
// printf ("### INVALID FLIP: curvature increases\n");
				// Rollback case 2. Edge flip in inverted direction.
				execute_edge_flip (edge, &face_1, &face_2, TRUE);
				recompute_edge_vertex_curvature (edge);
			}
		}

// printf ("======= Edge %d, flip = %d\n", edge->edgeId, valid_flip);

		// Stop trying if none of the edges is
		//  flipped in the first round.
		if ( (i%num_edges) == num_edges-1 )
		{
			if (flip_counter == 0)
				break;
			else
				flip_counter = 0;
		}

		max_flips--;
		i++;
	}
}


// Perform an edge flip and update
//  all the faces and vertices involved.
// The variable inverted specifies in which
//  direction the faces will rotate.
void execute_edge_flip (edgePtr edge, facePtr face_1, facePtr face_2, boolean inverted)
{
// printf ("DATA BEFORE:\n");
// printEdgeData (edge, *face_1, *face_2);

	flip_edge (edge, face_1, face_2, inverted);

	register_face_changes (face_1);
	register_face_changes (face_2);

	update_flip_vertex_data (edge);

// printf ("DATA AFTER:\n");
// printEdgeData (edge, *face_1, *face_2);
}



// Check if there is an existing triangle that will
//  be equal and opposite to any of the two new triangles
//  generated from an edge flip.
// Returns false if such a triangle exists.
boolean validate_edge_flip (edgePtr edge, facePtr face_1, facePtr face_2)
{
	boolean		valid;

	valid = check_existing_faces (edge->vertex1, edge);
	if (!valid)
		return (valid);
	valid = check_existing_faces (edge->vertex2, edge);
	if (!valid)
		return (valid);

	// Reject the flip if a face has collapsed to three collinear vertices.
	// Such a face will have a normal vector equal to zero.
	if (vector_equal_to_zero (face_1->faceNormal) || vector_equal_to_zero (face_2->faceNormal))
	{
		printf ("### INVALID FLIP: new normal vector equal to zero\n");
		return (FALSE);
	}


	return (TRUE);
}


// Test if a new face is equal to any of the faces incident
//  on the received parameter 'test_vertex'.
boolean check_existing_faces (int test_vertex, edgePtr edge)
{
	int					i;
	int*				face_index = NULL;
	faceStruct			test_face;

	// Test for duplicated triangles in vertex 1.
	for (i=0; i<ObjectData.facesPerVertex[test_vertex]; i++)
	{
		face_index = getNodeData (ObjectData.vertexFaceArray[test_vertex], i);
		test_face = ObjectData.faceArray[*face_index];

		if (	(edge->face1 != test_face.faceId) &&
				(edge->face2 != test_face.faceId) &&
				find_edge_in_triangle (edge, test_face) )
		{
printf ("Found DUPLICATE EDGES: edge %d = %d, %d and face %d = %d, %d, %d\n", edge->edgeId, edge->vertex1, edge->vertex2, test_face.faceId, test_face.vertex[0], test_face.vertex[1], test_face.vertex[2]);
			return (FALSE);
		}
	}

	return (TRUE);
}


// Get the sum of the curvatures of all the
//  vertices in the triangles on both sides
//  of an edge.
double get_edge_curvature (edgePtr edge, faceStruct face_1, faceStruct face_2)
{
	double			curvature_sum = 0.0;

	// Curvature of vertices on the edge.
	curvature_sum  = fabs (VertexDataArray[edge->vertex1].CURVATURE);
	curvature_sum += fabs (VertexDataArray[edge->vertex2].CURVATURE);

	// Curvature of vertices opposite the edge.
	curvature_sum += fabs (VertexDataArray[edge->opposite_vertex_1].CURVATURE);
	curvature_sum += fabs (VertexDataArray[edge->opposite_vertex_2].CURVATURE);

	return (curvature_sum);
}

// Get the sum of the curvatures of the vertices in the link of
//  a particular vertex.
double get_link_curvature (int vertexId, vertexDataStruct current_vertex_data)
{
	int			i;
	double		curvature_sum = 0.0;

	// The vertices in the link are stored starting from the index 2.
	for (i=2; i<=current_vertex_data.points_around_vertex; i++)
		curvature_sum += VertexDataArray[current_vertex_data.vertex_neighbour_index[i]].CURVATURE;

	return (curvature_sum);
}


// Recompute the normal of a face.
void update_face_normal_vector (facePtr face)
{
	vertexStruct		update_vertex[3];
	vectorPtr			normal_vector;

	// Get new normals for the triangles.
	update_vertex[0] = ObjectData.vertexArray[face->vertex[0]];
	update_vertex[1] = ObjectData.vertexArray[face->vertex[1]];
	update_vertex[2] = ObjectData.vertexArray[face->vertex[2]];

	normal_vector = getTriangleNormal (update_vertex[0], update_vertex[1], update_vertex[2]);
	normal_vector->vectorId = face->faceId;
	normal_vector->type = NORMAL;
	normal_vector->num_intersections = 0;

	face->faceNormal = normal_vector;
}


// Recompute the curvature information for
//  the 4 vertices involved in an edge flip.
void update_flip_vertex_data (edgePtr edge)
{
	int					i;
	int					update_vertex[4];
	int*				face_index = NULL;
	nodePtr				vertex_faces_list = NULL;
	boolean				found;

	// Get the 4 vertices from the edge.
	update_vertex[0] = ObjectData.vertexArray[edge->vertex1].vertexId;
	update_vertex[1] = ObjectData.vertexArray[edge->vertex2].vertexId;
	update_vertex[2] = ObjectData.vertexArray[edge->opposite_vertex_1].vertexId;
	update_vertex[3] = ObjectData.vertexArray[edge->opposite_vertex_2].vertexId;

	// First edge vertex.
	// Find which of the faces is not in the vertices list.
	vertex_faces_list = ObjectData.vertexFaceArray[update_vertex[0]];
	found = FALSE;
	for (i=0; i<getListLength(vertex_faces_list); i++)
	{
		face_index = (int*) getNodeData (vertex_faces_list, i);
		if (*face_index == edge->face1)
		{
			found = TRUE;
			break;
		}
	}
	// Insert the appropriate face in the list.
	if (found)
		add_vertex_face (update_vertex[0], edge->face2);
	else
		add_vertex_face (update_vertex[0], edge->face1);

	// Second edge vertex.
	// Find which of the faces is not in the vertices list.
	vertex_faces_list = ObjectData.vertexFaceArray[update_vertex[1]];
	found = FALSE;
	for (i=0; i<getListLength(vertex_faces_list); i++)
	{
		face_index = (int*) getNodeData (vertex_faces_list, i);
		if (*face_index == edge->face1)
		{
			found = TRUE;
			break;
		}
	}
	// Insert the appropriate face in the list.
	if (found)
		add_vertex_face (update_vertex[1], edge->face2);
	else
		add_vertex_face (update_vertex[1], edge->face1);

	// Remove one of the faces from the new
	//  opposite vertices.
	remove_vertex_face (update_vertex[2], edge->face2);
	remove_vertex_face (update_vertex[3], edge->face1);
}


void recompute_edge_vertex_curvature (edgePtr edge)
{
	int					i;
	int					update_vertex[4];

	// Get the 4 vertices from the edge.
	update_vertex[0] = ObjectData.vertexArray[edge->vertex1].vertexId;
	update_vertex[1] = ObjectData.vertexArray[edge->vertex2].vertexId;
	update_vertex[2] = ObjectData.vertexArray[edge->opposite_vertex_1].vertexId;
	update_vertex[3] = ObjectData.vertexArray[edge->opposite_vertex_2].vertexId;

	// Update the information of the 4 vertices involved
	// in the flip.
	for (i=0; i<4; i++)
	{
		freeVertexDataStructure ( &VertexDataArray[update_vertex[i]] );
		initializeVertexDataStructure ( &VertexDataArray[update_vertex[i]] );
// printf (" -*- RECOMPUTING DATA FOR VERTEX %d\n", update_vertex[i]);
		process_vertex (update_vertex[i]);
	}
}


// Save the changes made to a face because of flips.
// The face is stored in ObjectData.faceArray
//  and its normal is recomputed.
void register_face_changes (facePtr face)
{
	update_face_normal_vector (face);
	memcpy (&(ObjectData.faceArray[face->faceId]), face, sizeof (faceStruct));
}


// Function called from OpenGL to execute
//  the decimation. It deletes a set
//  number of vertices.
void visual_vertex_decimation (void)
{
	int					i;
	int					removed_vertex_id;
	int					num_removals = 2000;
	int					remaining_vertices;
	int					total_vertices;
	double				decimation_percent;

	// Does not work when viewing only one vertex.
	if (ShowVertex == 0)
	{
		for (i=0; i<num_removals; i++)
		{
			removed_vertex_id = remove_minimal_curvature_vertex ();
			// Stop the loop if there are no more vertices to decimate.
			if (removed_vertex_id < 0)
			{
				removed_vertex_id = 0;
				break;
			}
		}

		gl_remove_lists (removed_vertex_id);
		gl_update_lists (removed_vertex_id);
		// gl_create_display_lists ();
	}
	else
		printf ("\nWARNING! UNABLE TO DO DECIMATION WHEN VIEWING ONLY ONE VERTEX\n");

	total_vertices = ObjectData.numVertices;
	remaining_vertices = total_vertices - Removal_count;
	decimation_percent = (double)remaining_vertices / (double)total_vertices * 100.0;
	printf ("MESH NOW DECIMATED TO %d VERTICES (%.2lf%%)\n", remaining_vertices, decimation_percent);
}


// Create a new objectStruct to contain the
//  decimated model.
// This one will not include the vertices
//  and faces removed during the process.
objectStruct get_decimated_object (objectStruct complex_object)
{
	objectStruct		new_object;
	int					new_num_vertices = 0;
	int					new_num_faces = 0;
	int					old_num_vertices;
	int					old_num_faces;
	int					i;
	int					removed_vertex_count = 0;
	int*				new_index_array = NULL;
	time_t				start;
	time_t				stop;
	nodePtr				new_vertex_list = NULL;
	nodePtr				new_vertex_list_tail = NULL;
	nodePtr				new_face_list = NULL;
	nodePtr				new_face_list_tail = NULL;
	vertexPtr			vertex_copy = NULL;
	vertexPtr			new_vertex_array = NULL;
	facePtr				face_copy = NULL;
	facePtr				new_face_array = NULL;
	vertexStruct		vertex;
	faceStruct			face;
 
	printf ("\n[EXTRA STAGE] -- PREPARING DECIMATED OBJECT\n"); fflush (stdout);
	(void) time (&start);

	old_num_vertices = complex_object.numVertices;
	old_num_faces = complex_object.numFaces;

	// Array for the corrected vertex id's.
	new_index_array = (int*) xmalloc (sizeof (int) * (old_num_vertices + 1));
	memset (new_index_array, 0, sizeof (int) * (old_num_vertices + 1));

	// Add an empty vertex at the beginning of the vertex list.
	// It will be disregarded, and thus the vertices will be
	//  indexed beginning with 1, when they are transferred to
	//  an array.
	vertex_copy = createVertex (0, 0, 0, 0);
	new_vertex_list = addNode (NULL, vertex_copy);
	new_vertex_list_tail = new_vertex_list;

	// Look for the vertices that were deleted.
	for (i=1; i<=old_num_vertices; i++)
	{
		vertex = complex_object.vertexArray[i];
		// If a vertex was removed, then add
		//  its index to a list.
		if (vertex.vertexId < 0)
		{
			// Increment the number of removed
			//  vertices.
			removed_vertex_count++;
		}
		// If the vertex is ok, add it to the
		//  list of final vertices.
		else
		{
			// Copy the vertex data.
			vertex_copy = (vertexPtr) xmalloc (sizeof (vertexStruct));
			memcpy (vertex_copy, &vertex, sizeof (vertexStruct));

			// Update the vertex id, discounting
			//  the removed vertices so far.
			vertex_copy->vertexId = vertex.vertexId - removed_vertex_count;

			// Insert into the list.
			new_vertex_list_tail = addNode (new_vertex_list_tail, vertex_copy);

			// Store the corresponding new vertexId
			//  in the array of new indices.
			new_index_array[i] = vertex_copy->vertexId;
			
			// Increment the number of valid vertices.
			new_num_vertices++;
		}

		printProgressStar (i, old_num_vertices + old_num_faces);
	}

	// Convert the list of vertices into an array.
	new_vertex_array = listToVertexArray (new_vertex_list, new_num_vertices+1);

	// Add an empty face at the beginning of the face list.
	// It will be disregarded, and thus the faces will be
	//  indexed beginning with 1, when they are transferred to
	//  an array.
	face_copy = createEmptyFace ();
	new_face_list = addNode (NULL, face_copy);
	new_face_list_tail = new_face_list;

	for (i=1; i<=old_num_faces; i++)
	{
		face = complex_object.faceArray[i];
		// If the face was removed, just skip it.
		if (face.faceId < 0)
		{
		}
		// For valid faces, correct the vertex id's
		//  which may have changed.
		else
		{
			// Copy the face data.
			face_copy = (facePtr) xmalloc (sizeof (faceStruct));
			memcpy (face_copy, &face, sizeof (faceStruct));

			// Correct the vertex id's.
			face_copy->vertex[0] = new_index_array[face.vertex[0]];
			face_copy->vertex[1] = new_index_array[face.vertex[1]];
			face_copy->vertex[2] = new_index_array[face.vertex[2]];

			// Insert into the list.
			new_face_list_tail = addNode (new_face_list_tail, face_copy);

			new_num_faces++;
		}

		printProgressStar (old_num_vertices + i, old_num_vertices + old_num_faces);
	}

	free (new_index_array);

	// Convert the list of faces into an array.
	new_face_array = listToFaceArray (new_face_list, new_num_faces+1);

	new_object.numVertices = new_num_vertices;
	new_object.numFaces = new_num_faces;
	new_object.vertexArray = new_vertex_array;
	new_object.faceArray = new_face_array;

	(void) time (&stop);
	printf ("[done] (%ld seconds)\n", (int) stop - start); fflush (stdout);

	return (new_object);
}
