#include <GL/gl.h>
#include <GL/glu.h>

#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "faces.h"
#include "object3D.h"
#include "sphericalGeometry.h"
#include "vertexGeometry.h"
#include "objWriter.h"
#include "offWriter.h"
#include "plyWriter.h"

#include "glTools.h"
#include "glObjectLists.h"
#include "glConvexHull.h"
#include "glRotation.h"

/* Libraries to display text.
   Taken from the surcecode
   of the game Chromium B.S.U. */
#include "Chromium/TexFont.h"

#define AXES_LIST			0
#define	SPHERE_LIST			1
#define POINT_LIST			2

#define	OBJECT_LIST			3
#define	ANGLE_OBJECT_LIST	4
#define	TAC_OBJECT_LIST	5
#define	OBJECT_NORMALS_LIST	6
#define	STAR_LIST			7
#define C_H_LIST			8
#define C_H_WIRE_LIST		9

#define	EXTENDED_STAR_LIST	10
#define	ARCS_MAP_LIST		11
#define	GAUSS_MAP_POS_LIST	12
#define	GAUSS_MAP_NEG_LIST	13

#define FIXED_LISTS			10
#define LISTS_PER_VERTEX	4



// External variables
extern objectStruct			ObjectData;
extern vertexDataStruct*	VertexDataArray;
extern int					ShowVertex;
extern int					Begin;
extern int					End;
extern nodePtr				ConvexHull;

// For writing decimated files
extern char*				File_Name;
extern int					Removal_count;

// 
extern GLUquadricObj*	quadratic;
extern GLuint			displayNormals;

// Variables for printing messages within OpenGL
extern int				width, height;
extern GLuint			font;

// Variable for the triangulation of the polygons
extern nodePtr			PolygonTriangles;

// Global variables
GLuint*			listIndex;


// Size of the sphere for the Gauss Map
double			Sphere_Radius = 2.0f;

// Maximum length of objects to be displayed.
#ifdef SINGLE_DISPLAY
double      	Max_Size = 3.0;
#else
double      	Max_Size = 2.0;
#endif

// Variable to select the colour code to display.
int				numPalettes = 3;
int				colourPalette = 0;

// Variables for decimation
// Storage for the new decimated vertices
//  and faces as lists of indexes.
nodePtr			decimated_vertex_list = NULL;
nodePtr			decimated_face_list = NULL;

// Variables to select which areas to show.
boolean			ShowPositive = TRUE;
boolean			ShowNegative = TRUE;

boolean			ShowArcs = FALSE;
boolean			ShowNormals = FALSE;
boolean			ShowExtendedNormals = FALSE;
boolean			ShowSphere = TRUE;
boolean			ShowBack = FALSE;
boolean			ShowConvexHull = FALSE;
boolean			ShowConvexHullWire = FALSE;
boolean			ShowObject = TRUE;
boolean			ShowVertexNormal = FALSE;
int				ShowNormalNames = FALSE;

// Variable to hold font information
TexFont*		texFont = 0;

// Function declarations
void gl_prepare_scene ();
void gl_cleanup ();
void gl_draw_scene (int object);
void gl_create_display_lists ();
void gl_draw_projection_plane ();
void gl_extended_normal_star_to_display_list (GLuint list, nodePtr normalList);
void gl_normal_star_to_display_list (GLuint list, objectStruct objectData);
void gl_draw_path (nodePtr path_list, objectStruct objectData);
void gl_show_single_normal (int vertexNumber, objectStruct objectData);
void gl_arcs_map (GLuint list, nodePtr normalList);
void gl_oriented_arcs_map (GLuint list, nodePtr	polygonList, spinType* orientations);
void gl_oriented_gauss_map (GLuint list, nodePtr polygonList, spinType* orientations, boolean convex, spinType direction);
void gl_setup_clip_plane (int plane, vectorPtr vertex1, vectorPtr vertex2, GLdouble* plane_equation);
void gl_draw_vertex_star (int vertexIndex);
void gl_draw_artificial_normal (vertexStruct currentVertex);
void gl_quadric_sphere (GLuint list);
void gl_point_list (GLuint list);
void gl_scene_event( int event, int data, int xpos, int ypos );

// External event handler function
extern void gl_event( int event, int data, int xpos,  int ypos );

// External function to print in OpenGL
extern void gl_printf( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha, GLint x, GLint y, GLuint font, const char *format, ... );

// External function for mesh decimation
extern void visual_vertex_decimation ();



// Call the functions to load the textures and to create the
//  display lists.
void gl_prepare_scene ()
{
	// BEGIN FONT SECTION
	GLuint  texobj;

	texFont = txfLoadFont("Chromium/fonts/space.txf");
	if(!texFont)
	{
		printf("\nERROR loading texture font. Check data path and try again.\n\n");
		exit(1);
	}
	glGenTextures(1, &texobj);
	txfEstablishTexture(texFont, texobj, GL_FALSE);
	// END FONT SECTION

	// Reduce the size of the object read, to fit in the display area.
	ObjectData.displayVertexArray = normalizeVertexArray (ObjectData.vertexArray, ObjectData.numVertices, Max_Size);

	gl_create_display_lists ();

	// Hide the sphere at first if there are too many vertices.
	if ( (ShowVertex == 0) && (ObjectData.numVertices > 10) )
		ShowSphere = FALSE;

#ifdef BnW
	glDisable (GL_LIGHTING);
#endif
}


void gl_cleanup ()
{
	// BEGIN FONT SECTION
    glDeleteTextures(1, &texFont->texobj);
	txfUnloadFont(texFont);
	texFont = 0;
	// END FONT SECTION


	free (listIndex);
	free (quadratic);
}


// Print a 3D text on screen, at the current coordinates.
void gl_draw_text (char* string)
{
	int			w, a, d;	// width, max ascent, max descent
	int			length;
	GLboolean	light_status;

	length = strlen (string);

	// gl_billboard_init ();

	glPushMatrix ();

		gl_no_lighting (&light_status);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.0, -3);

		glDisable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GEQUAL, 0.5);

		txfGetStringMetrics(texFont, string, length, &w, &a, &d);

		// Quick fix to standardize the size of the fonts.
		w = 30;

		glScalef (0.3, 0.3, 0.3);
		glScalef(1.0/w, 1.0/w, 1.0/w);
		glTranslatef(-w/2.0, d-(a+d)/2.0, 0.0);

		txfRenderFancyString(texFont, string, length);

		glEnable(GL_BLEND);

		glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_TEXTURE_2D);
		gl_restore_lighting (light_status);

	glPopMatrix ();

	// gl_billboard_end ();
}


// Show the name of each normal vector at the
//  endpoint of each vector.
void gl_draw_normal_names (nodePtr normalList)
{
	int			numNormals = getListLength (normalList);
	int			i;
	int			counter = 0;
	char		string[5];
	double		n_x, n_y, n_z;
	vectorPtr	vector = NULL;

	for (i=0; i<5; i++)
		string[i] = '\0';

	for (i=0; i<numNormals; i++)
	{
		vector = (vectorPtr) getNodeData (normalList, i);

		if ( (ShowNormalNames == 2) && (vector->vectorId < 0) )
			continue;

		sprintf (string, "%4d", vector->vectorId);

		/*
		n_x = vector->i * (Sphere_Radius * 1.1);
		n_y = vector->j * (Sphere_Radius * 1.1);
		n_z = vector->k * (Sphere_Radius * 1.1);
		*/
		// Place the vector names further away the more there are.
		n_x = vector->i * ( Sphere_Radius + (0.2 + counter * 0.1) );
		n_y = vector->j * ( Sphere_Radius + (0.2 + counter * 0.1) );
		n_z = vector->k * ( Sphere_Radius + (0.2 + counter * 0.1) );

		glPushMatrix ();

		// Extend the vectors to reach their names.
			glColor3f(0.0, 0.0, 0.0);
			glBegin (GL_LINES);
				glVertex3f (vector->i * Sphere_Radius, vector->j * Sphere_Radius, vector->k * Sphere_Radius);
				glVertex3f (n_x, n_y, n_z);
			glEnd ();

			glTranslated (n_x, n_y, n_z);

			gl_billboard_init ();

				/*
				// Separate the name printing for matching
				//  intersection pairs.
				if (vector->type == MATCH_1)
					glTranslated (0.0, 0.1, 0.0);
				else if (vector->type == MATCH_2)
					glTranslated (0.0, -0.1, 0.0);
				*/

				gl_draw_text (string);

			gl_billboard_end ();

		glPopMatrix ();

		counter++;
	}
}


// Draw the names of the faces around a vertex.
void gl_draw_face_names (int current_vertex, objectStruct objectData)
{
	int				i;
	int*			face_index = NULL;
	double			n_x, n_y, n_z;
	char			string[5];
	faceStruct		face;
	vertexStruct	vertex1;
	vertexStruct	vertex2;
	vectorPtr		edge_vector = NULL;
	vectorPtr		normal_vector = NULL;
	GLdouble		gl_rotation_matrix[16];

	for (i=0; i<objectData.facesPerVertex[current_vertex]; i++)
	{
		face_index = (int*) getNodeData (objectData.vertexFaceArray[current_vertex], i);
		face = objectData.faceArray[*face_index];
		reorderFace (&face, current_vertex);

		vertex1 = objectData.displayVertexArray[ face.vertex[1] ];
		vertex2 = objectData.displayVertexArray[ face.vertex[2] ];

		// Get the coordinates of the oposite edge
		//  of the current face.
		n_x = (vertex2.x + vertex1.x) / 2;
		n_y = (vertex2.y + vertex1.y) / 2;
		n_z = (vertex2.z + vertex1.z) / 2;

		// Prepare the rotation matrix, to align
		//  with the plane of the face.
		edge_vector = vectorFromVertices (&vertex2, &vertex1);
		normal_vector = face.faceNormal;
		rotateToXYplane (edge_vector, normal_vector, gl_rotation_matrix);

		sprintf (string, "%4d", face.faceId);

		glPushMatrix ();

			glColor3f(0.0, 0.0, 0.0);
			glTranslated (n_x, n_y, n_z);
			glMultMatrixd (gl_rotation_matrix);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			glRotatef(-90.0, 0.0, 0.0, 1.0);

			gl_billboard_init ();
				glScalef(1.5, 1.5, 1.5);
				gl_draw_text (string);
			gl_billboard_end ();

		glPopMatrix ();

		free (edge_vector);
	}
}


// Scene with the object and the arcs.
void gl_draw_scene_1 ()
{
	GLboolean		light_status;
	GLfloat			arc_offset_factor = 1.0;
	GLfloat			arc_offset_units = 1.0;
	int				i;
	float			pointsColor[4] = {1.0f,1.0f,0.0f,1.0f};
	vertexStruct	currentVertex;

	if (ShowVertex > 0)
	{
		currentVertex = ObjectData.displayVertexArray[ShowVertex];
	}

	// Turn lights off
	gl_no_lighting (&light_status);

	glEnable(GL_POLYGON_OFFSET_FILL);

	for (i=Begin; i<End; i++)
	{
		glPushMatrix ();

		// Translate the Gauss Map to the location of the selected vertex.
		if (ShowVertex > 0)
		{
			glTranslatef (currentVertex.x, currentVertex.y, currentVertex.z);
		}

		// Draw the Normal Star 
		if (ShowNormals)
			glCallList (listIndex[STAR_LIST]);

		// Draw the Extended Normal Star 
		if	(ShowExtendedNormals)
			glCallList (listIndex[EXTENDED_STAR_LIST + (LISTS_PER_VERTEX * i)]);

		// Draw the arcs
		if (ShowArcs)
		{
			// Enable Polygon Offset for the arcs.
			glPolygonOffset(arc_offset_factor, arc_offset_units);
			// Increase the parameters for the next arc.
			arc_offset_factor += 0.1;
			arc_offset_units += 0.1;

			glCallList (listIndex[ARCS_MAP_LIST + (LISTS_PER_VERTEX * i)]);
		}

		glPopMatrix ();
	}

	glDisable(GL_POLYGON_OFFSET_FILL);

	if (displayNormals)
	{
		glCallList (listIndex[OBJECT_NORMALS_LIST]);
	}

	// Draw the selected vertex as a point (if there is one).
	if ( (ShowVertex > 0) && (ObjectData.vertexArray[ShowVertex].vertexId > 0) )
	// if (ShowVertex > 0)
	{
		if (ShowObject)
		{
			gl_draw_vertex_star (ShowVertex);
			glColor4fv( pointsColor );
			glPushMatrix ();
				glTranslatef (currentVertex.x, currentVertex.y, currentVertex.z);
				glCallList (listIndex[POINT_LIST]);
			glPopMatrix ();
		}

		// Draw the artificial vertex normal
		if (ShowVertexNormal)
		{
			if (ShowObject)
				gl_draw_artificial_normal (currentVertex);
			gl_draw_projection_plane ();
		}
	}

/*
//////////////////////////////////////////////
//	TESTING THE NEW PROJECTED POINT

glPushMatrix ();
	glColor4f (0.0f, 0.0f, 0.0f, 1.0f);
	glTranslatef (Projected_Vertex.x, Projected_Vertex.y, Projected_Vertex.z);
	glCallList (listIndex[POINT_LIST]);
glPopMatrix ();

//
//////////////////////////////////////////////
*/

	gl_restore_lighting (light_status);

	// Draw the object.
	if (ShowBack == FALSE)
		glEnable (GL_CULL_FACE);

	// Display the names of the faces.
	if ( ShowNormalNames && (ShowVertex > 0) )
	{
		gl_draw_face_names (ShowVertex, ObjectData);
	}

	if (ShowObject)
	{
		if (colourPalette == 0)
		{
			glCallList (listIndex[OBJECT_LIST]);
		}
		else if (colourPalette == 1)
		{
			glCallList (listIndex[ANGLE_OBJECT_LIST]);
		}
		else if (colourPalette == 2)
		{
			glCallList (listIndex[TAC_OBJECT_LIST]);
		}
	}

	if (ShowConvexHull)
	{
		glCallList (listIndex[C_H_LIST]);
	}
	if (ShowConvexHullWire)
	{
		glCallList (listIndex[C_H_WIRE_LIST]);
	}

	glDisable (GL_CULL_FACE);

	if (ShowVertex > 0)
	{
#ifdef SINGLE_DISPLAY
		gl_printf( 0.0f, 0.0f, 0.0f, 1.0f, width - 440, height - 40, font, "Showing vertex %d", ShowVertex );
		if (colourPalette == 0)
			gl_printf( 0.0f, 0.0f, 0.0f, 1.0f, width - 440, height - 60, font, "Curvature (PGM): %lf", VertexDataArray[ShowVertex].curvature );
		if (colourPalette == 1)
			gl_printf( 0.0f, 0.0f, 0.0f, 1.0f, width - 440, height - 60, font, "Curvature (AD): %lf", VertexDataArray[ShowVertex].angle_deficit );
#else
		gl_printf( 0.0f, 0.0f, 0.0f, 1.0f, width - 220, height - 40, font, "Showing vertex %d", ShowVertex );
		if (colourPalette == 0)
			gl_printf( 0.0f, 0.0f, 0.0f, 1.0f, width - 220, height - 60, font, "Curvature (PGM): %lf", VertexDataArray[ShowVertex].curvature );
		if (colourPalette == 1)
			gl_printf( 0.0f, 0.0f, 0.0f, 1.0f, width - 220, height - 60, font, "Curvature (AD): %lf", VertexDataArray[ShowVertex].angle_deficit );
#endif
	}
}


// Scene with the Gauss Map on a sphere.
void gl_draw_scene_2 ()
{
	GLint			shade_status[2];
	GLfloat			arc_offset_factor = 1.0;
	GLfloat			arc_offset_units = 1.0;
	GLfloat			sphere_offset_factor = 1.0;
	GLfloat			sphere_offset_units = 1.0;
	float			faceColour[4] = {0.4f,0.4f,0.4f,0.5f};
	int				i;

	if (ShowSphere)
	{
		glEnable(GL_POLYGON_OFFSET_FILL);

		// Store the current polygon mode.
		glGetIntegerv (GL_SHADE_MODEL, shade_status);
		// Switch to smooth shading
		glShadeModel (GL_SMOOTH);

		if ( (ShowVertex > 0) && displayNormals )
			gl_show_single_normal (ShowVertex, ObjectData);

		for (i=Begin; i<End; i++)
		{
			glCallList (listIndex[EXTENDED_STAR_LIST + (LISTS_PER_VERTEX * i)]);

			// Enable Polygon Offset for the arcs.
			glPolygonOffset(arc_offset_factor, arc_offset_units);
			// Increase the parameters for the next arc.
			arc_offset_factor += 0.1;
			arc_offset_units += 0.1;

			// Draw the arcs.
			glCallList (listIndex[ARCS_MAP_LIST + (LISTS_PER_VERTEX * i)]);

			// Enable Polygon Offset for the arcs.
			glPolygonOffset(sphere_offset_factor, sphere_offset_units);
			// Increase the parameters for the next arc.
			sphere_offset_factor -= 0.1;
			sphere_offset_units -= 0.1;

			// Draw the spherical polygons.
			if (ShowPositive)
				glCallList (listIndex[GAUSS_MAP_POS_LIST + (LISTS_PER_VERTEX * i)]);
			if (ShowNegative)
				glCallList (listIndex[GAUSS_MAP_NEG_LIST + (LISTS_PER_VERTEX * i)]);
		}

		// Display the names of the vectors.
		if ( ShowNormalNames && (ShowVertex > 0) )
		{
			gl_draw_normal_names (VertexDataArray[ShowVertex].normals_around_vertex);
		}

		// Draw a transparent sphere.
		glEnable (GL_CULL_FACE); 
		glPushMatrix ();
			// Scale a bit so that the sphere will not
			//  overlap with the Gauss Map.
			glScalef (0.99, 0.99, 0.99);
			glColor4fv ( faceColour );
			// Set a normal offset for the base sphere.
			glPolygonOffset(1.0, 1.0);
			glCallList (listIndex[SPHERE_LIST]);
		glPopMatrix ();
		glDisable (GL_CULL_FACE);

		// Switch back to the previous state.
		glShadeModel (shade_status[0]);

		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}


// Main function to generate all display lists to be used.
void gl_create_display_lists ()
{
	int					i;
	int					numVertices;
	int					numLists;
	boolean				convex = FALSE;
	vertexDataStruct    current_vertex_data;

	numVertices = ObjectData.numVertices;
	numLists = FIXED_LISTS + (LISTS_PER_VERTEX * numVertices);

	listIndex = (GLuint*) xmalloc (sizeof(GLuint) * numLists);

	// Allocate space for the Display lists, and initialize the indexes.
	listIndex[0] = glGenLists (numLists);
	if (listIndex[0] == 0)
	{
		printf ("Unable to create Display Lists.\nExiting.\n");
		exit (1);
	}
	for (i=1; i<numLists; i++)
		listIndex[i] = listIndex[i-1] + 1;

	printf ("\tGenerating display lists:\t"); fflush (stdout);

	gl_faces_to_display_list (listIndex[OBJECT_LIST], ObjectData, 1);
	gl_faces_to_display_list (listIndex[ANGLE_OBJECT_LIST], ObjectData, 2);
	gl_faces_to_display_list (listIndex[TAC_OBJECT_LIST], ObjectData, 3);
	gl_normals_to_display_list (listIndex[OBJECT_NORMALS_LIST], ObjectData);
	gl_normal_star_to_display_list (listIndex[STAR_LIST], ObjectData);
	gl_convex_hull_to_display_list (listIndex[C_H_LIST], ConvexHull, ObjectData);
	gl_convex_hull_wireframe_to_display_list (listIndex[C_H_WIRE_LIST], ConvexHull, ObjectData);

	// Create the display lists.
	for (i=Begin; i<End; i++)
	// for (i=0; i<numVertices; i++)
	{
		current_vertex_data = VertexDataArray[i+1];

		// Skip over vertices that do not appear in any face.
		if (ObjectData.facesPerVertex[i+1] == 0)
			continue;

		convex = TRUE;

		gl_extended_normal_star_to_display_list (listIndex[EXTENDED_STAR_LIST + (LISTS_PER_VERTEX * i)], current_vertex_data.normals_around_vertex);
		gl_arcs_map (listIndex[ARCS_MAP_LIST + (LISTS_PER_VERTEX * i)], current_vertex_data.normals_around_vertex);
		// gl_oriented_arcs_map (listIndex[ARCS_MAP_LIST + (LISTS_PER_VERTEX * i)], current_vertex_data.spherical_polygon_list, current_vertex_data.spherical_polygon_orientation);
		gl_oriented_gauss_map (listIndex[GAUSS_MAP_POS_LIST + (LISTS_PER_VERTEX * i)], current_vertex_data.spherical_polygon_list, current_vertex_data.spherical_polygon_orientation, convex, CCW);
		gl_oriented_gauss_map (listIndex[GAUSS_MAP_NEG_LIST + (LISTS_PER_VERTEX * i)], current_vertex_data.spherical_polygon_list, current_vertex_data.spherical_polygon_orientation, convex, CW);

#ifndef DEBUG
		printProgressStar (i, End-Begin);
#endif
	}
    printf (" Done\n"); fflush (stdout);

	gl_point_list (listIndex[POINT_LIST]);
	gl_quadric_sphere (listIndex[SPHERE_LIST]);
}


// Clear the display lists corresponding
//  to a vertex deleted by decimation.
void gl_remove_lists (int removed_vertex_id)
{
	// gl_extended_normal_star_to_display_list (listIndex[EXTENDED_STAR_LIST + (LISTS_PER_VERTEX * (removed_vertex_id-1) )],
	glNewList (listIndex[EXTENDED_STAR_LIST + (LISTS_PER_VERTEX * (removed_vertex_id-1) )], GL_COMPILE);
	glEndList ();

	// gl_arcs_map (listIndex[ARCS_MAP_LIST + (LISTS_PER_VERTEX * (removed_vertex_id-1) )],
	glNewList (listIndex[ARCS_MAP_LIST + (LISTS_PER_VERTEX * (removed_vertex_id-1) )], GL_COMPILE);
	glEndList ();

	// gl_oriented_arcs_map (listIndex[ARCS_MAP_LIST + (LISTS_PER_VERTEX * (removed_vertex_id-1) )],
	glNewList (listIndex[ARCS_MAP_LIST + (LISTS_PER_VERTEX * (removed_vertex_id-1) )], GL_COMPILE);
	glEndList ();

	// gl_oriented_gauss_map (listIndex[GAUSS_MAP_POS_LIST + (LISTS_PER_VERTEX * (removed_vertex_id-1) )],
	glNewList (listIndex[GAUSS_MAP_POS_LIST + (LISTS_PER_VERTEX * (removed_vertex_id-1) )], GL_COMPILE);
	glEndList ();

	// gl_oriented_gauss_map (listIndex[GAUSS_MAP_NEG_LIST + (LISTS_PER_VERTEX * (removed_vertex_id-1) )],
	glNewList (listIndex[GAUSS_MAP_NEG_LIST + (LISTS_PER_VERTEX * (removed_vertex_id-1) )], GL_COMPILE);
	glEndList ();
}


// Recreate the lists related to modified vertices.
// Used for decimation of vertices.
void gl_update_lists (int removed_vertex_id)
{
	int					i;
	int					update_vertex_id;
	boolean				convex = FALSE;
	vertexDataStruct    current_vertex_data;
	vertexDataStruct    removed_vertex_data;

	gl_faces_to_display_list (listIndex[OBJECT_LIST], ObjectData, 1);
	gl_faces_to_display_list (listIndex[ANGLE_OBJECT_LIST], ObjectData, 2);
	gl_faces_to_display_list (listIndex[TAC_OBJECT_LIST], ObjectData, 3);
	gl_normals_to_display_list (listIndex[OBJECT_NORMALS_LIST], ObjectData);
	gl_normal_star_to_display_list (listIndex[STAR_LIST], ObjectData);

	removed_vertex_data = VertexDataArray[removed_vertex_id];

	// Update the vertices around the one deleted.
	for (i=2; i<=removed_vertex_data.points_around_vertex; i++)
	{
		update_vertex_id = removed_vertex_data.vertex_neighbour_index[i];

		current_vertex_data = VertexDataArray[update_vertex_id];

		convex = TRUE;

		gl_extended_normal_star_to_display_list (listIndex[EXTENDED_STAR_LIST + (LISTS_PER_VERTEX * (update_vertex_id-1) )], current_vertex_data.normals_around_vertex);
		// Draw the indicatrix again.
		gl_arcs_map (listIndex[ARCS_MAP_LIST + (LISTS_PER_VERTEX * (update_vertex_id-1) )], current_vertex_data.normals_around_vertex);

		// gl_oriented_arcs_map (listIndex[ARCS_MAP_LIST + (LISTS_PER_VERTEX * (update_vertex_id-1) )], current_vertex_data.spherical_polygon_list, current_vertex_data.spherical_polygon_orientation);
		gl_oriented_gauss_map (listIndex[GAUSS_MAP_POS_LIST + (LISTS_PER_VERTEX * (update_vertex_id-1) )], current_vertex_data.spherical_polygon_list, current_vertex_data.spherical_polygon_orientation, convex, CCW);
		gl_oriented_gauss_map (listIndex[GAUSS_MAP_NEG_LIST + (LISTS_PER_VERTEX * (update_vertex_id-1) )], current_vertex_data.spherical_polygon_list, current_vertex_data.spherical_polygon_orientation, convex, CW);
	}
}


// Draw a plane and the points of the star of
//  the vertex projected onto such a plane.
void gl_draw_projection_plane ()
{
	int				i;
	vectorPtr		vector1 = NULL;
	vectorPtr		vector2 = NULL;
	vertexPtr		projectedVertexArray = NULL;
	vertexPtr		original_array = NULL;
	vertexStruct	vertex0;
	vertexStruct	vertex1;
	vertexStruct	vertex2;
	GLdouble		gl_rotation_matrix[16];
	float			planeColour[4] = {0.8f,0.8f,0.0f,0.5f};
	float			pointColour[4] = {0.0f,0.0f,0.0f,1.0f};

	original_array = VertexDataArray[ShowVertex].projected_vertices_array;
	projectedVertexArray = normalizeVertexArray (original_array, VertexDataArray[ShowVertex].points_around_vertex, Max_Size);

	// Get the vertex at the tip of the cone.
	vertex0 = projectedVertexArray[1];
	// Get two more vertices to create 2 vectors.
	vertex1 = projectedVertexArray[2];
	vertex2 = projectedVertexArray[3];

	// Create two reference vectors from the vertices.
	vector1 = vectorFromVertices (&vertex0, &vertex1);
	vector2 = vectorFromVertices (&vertex0, &vertex2);

	// Get the rotation matrix to relocate the plane.
	rotateToXYplane (vector1, vector2, gl_rotation_matrix);

	glColor4fv ( pointColour );

	vertex2 = projectedVertexArray[VertexDataArray[ShowVertex].points_around_vertex];

	// Draw the projected points.
	for (i=2; i<=VertexDataArray[ShowVertex].points_around_vertex; i++)
	{
		glPushMatrix ();
			vertex1 = projectedVertexArray[i];
			glTranslatef (vertex1.x, vertex1.y, vertex1.z);
			glCallList (listIndex[POINT_LIST]);
		glPopMatrix ();
		glLineWidth( 3.0 );
		glBegin (GL_LINES);
			glVertex3f (vertex1.x, vertex1.y, vertex1.z);
			glVertex3f (vertex2.x, vertex2.y, vertex2.z);
		glEnd ();
		glLineWidth( 1.0 );
		vertex2 = vertex1;
	}

	glColor4fv ( planeColour );

	// Draw the projection plane.
	glPushMatrix ();
		// glTranslatef (vertex0.x, vertex0.y, vertex0.z);
		// Rotate the coordinate axis.
		glMultMatrixd (gl_rotation_matrix);
		// Draw a plane as a quad.
		glBegin (GL_QUADS);
			glVertex3f (-4.0,  0.0,  0.0);
			glVertex3f ( 0.0,  4.0,  0.0);
			glVertex3f ( 4.0,  0.0,  0.0);
			glVertex3f ( 0.0, -4.0,  0.0);
		glEnd ();
	glPopMatrix ();

	free (projectedVertexArray);
}


// Generate a star made up of the normals to the faces
//  of the object. This is done by making all the
//  normals begin at the origin.
void gl_extended_normal_star_to_display_list (GLuint list, nodePtr normalList)
{
	float		faceColour[4] = {0.0f,0.0f,0.0f,1.0f};
	double		n_x, n_y, n_z;
	int			i;
	int			numNormals = getListLength (normalList);
	vectorPtr	vector = NULL;

	// Initialise the list, in compile mode.
	glNewList (list, GL_COMPILE);

	glColor4fv ( faceColour );
	glLineWidth( 3.0 );

	for (i=0; i<numNormals; i++)
	{
		vector = (vectorPtr) getNodeData (normalList, i);

		n_x = vector->i * Sphere_Radius;
		n_y = vector->j * Sphere_Radius;
		n_z = vector->k * Sphere_Radius;

		glBegin (GL_LINES);
			glVertex3f (0.0f, 0.0f, 0.0f);
			glVertex3f (n_x, n_y, n_z);
		glEnd ();
	}

	glLineWidth( 1.0 );

	glEndList ();
}


// Generate a star made up of the normals to the faces
//  of the object. This is done by making all the
//  normals begin at the origin.
void gl_normal_star_to_display_list (GLuint list, objectStruct objectData)
{
	float		faceColour[4] = {0.8f,0.8f,0.0f,1.0f};
	double		n_x, n_y, n_z;
	int			i;
	int			numFaces = objectData.numFaces;
	faceStruct	face;

	// Initialise the list, in compile mode.
	glNewList (list, GL_COMPILE);

	glColor4fv ( faceColour );
	glLineWidth( 3.0 );

	for (i=1; i<=numFaces; i++)
	{
		face = objectData.faceArray[i];

		// Skip over faces removed by decimation. 
		if (face.faceId < 0)
			continue;

		n_x = face.faceNormal->i * Sphere_Radius;
		n_y = face.faceNormal->j * Sphere_Radius;
		n_z = face.faceNormal->k * Sphere_Radius;

		glBegin (GL_LINES);
			glVertex3f (0.0f, 0.0f, 0.0f);
			glVertex3f (n_x, n_y, n_z);
		glEnd ();
	}

	glLineWidth( 1.0 );

	glEndList ();
}


// Create a display list of a path between two points.
void gl_draw_path (nodePtr path_list, objectStruct objectData)
{
	float			path_colour[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	int				list_length = getListLength (path_list);
	int				i;
	int*			index = NULL;
	vertexStruct	vertex1;
	vertexStruct	vertex2;
	GLboolean		light_status;

	glColor4fv ( path_colour );
	glLineWidth( 3.0 );
	glTranslatef (0.0f, 0.0f, 0.0035f);
	gl_no_lighting (&light_status);

	for (i=0; i<list_length-1; i++)
	{
		index = (int*) getNodeData (path_list, i);
		vertex1 = objectData.displayVertexArray[*index];
		index = (int*) getNodeData (path_list, i+1);
		vertex2 = objectData.displayVertexArray[*index];

		glBegin (GL_LINES);
			glVertex3f (vertex1.x, vertex1.y, vertex1.z);
			glVertex3f (vertex2.x, vertex2.y, vertex2.z);
		glEnd ();
	}

	gl_restore_lighting (light_status);

	glLineWidth( 1.0 );
}


// Generate a star made up of the normals to the faces
//  of the object. This is done by making all the
//  normals begin at the origin.
void gl_show_single_normal (int vertexNumber, objectStruct objectData)
{
	float			faceColour[4] = {1.0f,0.0f,1.0f,1.0f};
	double			n_x, n_y, n_z;
	vertexStruct	normalVector;

	glColor4fv ( faceColour );
	glLineWidth( 3.0 );

 	normalVector = objectData.normalArray[vertexNumber];
	n_x = normalVector.x * 3;
	n_y = normalVector.y * 3;
	n_z = normalVector.z * 3;

	glBegin (GL_LINES);
		glVertex3f (0.0f, 0.0f, 0.0f);
		glVertex3f (n_x, n_y, n_z);
	glEnd ();

	glLineWidth( 1.0 );
}


// Draw arcs to join the normals of the faces
//  of the vertex.
// Receives a list of the normals for one vertex.
void gl_arcs_map (GLuint list, nodePtr normalList)
{
#ifdef BnW
	float		faceColour[4] = {0.0f,0.0f,0.0f,1.0f};
	double		width = 0.1;
#else
	float		faceColour[4] = {0.8f,0.8f,0.0f,1.0f};
	double		width = 0.2;
#endif
	double		angle;
	int			i;
	int			numFaces = getListLength (normalList);
	vectorPtr	vertex1 = NULL;
	vectorPtr	vertex2 = NULL;
	GLdouble	gl_rotation_matrix[16];

	// Initialise the list, in compile mode.
	glNewList (list, GL_COMPILE);

	for (i=0; i<numFaces; i++)
	{
		vertex1 = (vectorPtr) getNodeData (normalList, i);
		vertex2 = (vectorPtr) getNodeData (normalList, (i+1)%numFaces);

		angle = angleBetweenVectors (vertex1, vertex2);
		angle = angle * 180 / PI;

		// Obtain the rotation matrix to align
		//  the XY plane with the one for the vectors.
		rotateToXYplane (vertex1, vertex2, gl_rotation_matrix);

		glColor4fv ( faceColour );

		// Create the arcs to join the vectors.
		glPushMatrix ();
			// Rotate the coordinate axis.
			glMultMatrixd (gl_rotation_matrix);
			// Select a number of segments for the arcs,
			//  according to the angle between vectors.
			// NOTE: 180 / 16 = 11.25
			// Using 12 to avoid some problems !?!?!?
			//  when comparing against some numbers close
			//  to 11.05???
			if (angle <= 12)
				gluPartialDisk(quadratic, Sphere_Radius-width, Sphere_Radius, 2, 1, 0, angle);
			else
				gluPartialDisk(quadratic, Sphere_Radius-width, Sphere_Radius, (int) angle*32/180, 1, 0, angle);
		glPopMatrix ();

		faceColour[0] -= 0.8 / (float) numFaces;
		faceColour[1] -= 0.8 / (float) numFaces;
	}

	glEndList ();
}


// Draw arcs to join the normals of the faces
//  of the vertex.
// Each individual area is rendered in a different colour
//  according to its orientation.
// Receives a list of the normals for one vertex.
void gl_oriented_arcs_map (GLuint list, nodePtr polygonList, spinType* orientations)
{
	float		negativeColour[4] = {0.0f,0.0f,1.0f,0.8f};
	float		positiveColour[4] = {1.0f,0.0f,0.0f,0.8f};
	double		angle;
	int			i, j;
	int			numPolygons = getListLength (polygonList);
	int			numVertices;
	nodePtr		polygonVertices = NULL;
	vectorPtr	vertex1 = NULL;
	vectorPtr	vertex2 = NULL;
	GLdouble	gl_rotation_matrix[16];

	// Initialise the list, in compile mode.
	glNewList (list, GL_COMPILE);

	for (i=0; i<numPolygons; i++)
	{
		polygonVertices = (nodePtr) getNodeData (polygonList, i);
		numVertices = getListLength (polygonVertices);

		if (orientations[i] == CCW)
			glColor4fv ( positiveColour );
		else
			glColor4fv ( negativeColour );


		for (j=0; j<numVertices; j++)
		{
			vertex1 = (vectorPtr) getNodeData (polygonVertices, j);
			vertex2 = (vectorPtr) getNodeData (polygonVertices, (j+1)%numVertices);

			angle = angleBetweenVectors (vertex1, vertex2);
			angle = angle * 180 / PI;

			rotateToXYplane (vertex1, vertex2, gl_rotation_matrix);

			// Create the arcs to join the vectors.
			glPushMatrix ();
			glMultMatrixd (gl_rotation_matrix);
			// Select a number of segments for the arcs,
			//  according to the angle of between vectors.
			if (angle <= 180/16)
				gluPartialDisk(quadratic, Sphere_Radius-0.2, Sphere_Radius, 2, 1, 0, angle);
			else
				gluPartialDisk(quadratic, Sphere_Radius-0.2, Sphere_Radius, (int) angle*32/180, 1, 0, angle);
			glPopMatrix ();
		}
	}

	glEndList ();
}


// Generate the Gauss Map on the surface of a sphere.
// Each individual area is rendered in a different colour
//  according to its orientation.
void gl_oriented_gauss_map (GLuint list, nodePtr polygonList, spinType* orientations, boolean convex, spinType direction)
{
#ifdef BnW
	float		negativeColour[4] = {1.0f,1.0f,1.0f,0.8f};
	float		positiveColour[4] = {0.0f,0.0f,0.0f,0.8f};
#else
	float		negativeColour[4] = {0.0f,0.0f,1.0f,0.8f};
	float		positiveColour[4] = {1.0f,0.0f,0.0f,0.8f};
#endif
	float		positiveConcaveColour[4] = {1.0f,0.5f,0.0f,0.8f};
	/*
	// float		scale_factor = 0.05;
	float		scale_factor = 0.1;
	float		scale[3] = {1.0f, 1.0f, 1.0f};
	*/
	GLdouble	plane_equation[4] = {0.0, 0.0, 0.0, 0.0};
	int			i, j;
	int			numPolygons = getListLength (polygonList);
	int			numVertices = 0;
	int			numTriangles;
	nodePtr		polygonVertices = NULL;
	nodePtr		leanPolygon = NULL;
	nodePtr		triangleList = NULL;
	vectorPtr	vertex1 = NULL;
	vectorPtr	vertex2 = NULL;
	vectorPtr	vertex3 = NULL;

	// Positive areas.
	// Initialise the list, in compile mode.
	glNewList (list, GL_COMPILE);

	glPushMatrix ();

	// Define the variables used, corresponding to
	//  the direction of the polygon.
	if (direction == CCW)
	{
		if (convex)
			glColor4fv ( positiveColour );
		else
			glColor4fv ( positiveConcaveColour );

		plane_equation[2] = -1.0;
	}
	else if (direction == CW)
	{
		glColor4fv ( negativeColour );
		plane_equation[2] = 1.0;
	}

	for (i=0; i<numPolygons; i++)
	// for (i=0; i<1; i++)
	{
		// Draw only the polygons corresponding to the
		//  orientation selected by the user.
		// Either positive or negative. The rest are discarded.
		if (orientations[i] == direction)
		{
			// Get the list for the current polygon.
			polygonVertices = (nodePtr) getNodeData (polygonList, i);

// printf ("Polygon %d = \n", i);
// printVectorList (polygonVertices);

			// Remove the collinear vertices.
			leanPolygon = remove_collinear_vertices (polygonVertices);
			numVertices = getListLength (leanPolygon);

// printf ("After removal of collinear:\n");
// printVectorList (leanPolygon);

			// Triangulate the polygon.
			triangulateSphericalPolygon (leanPolygon, orientations[i]);
			numTriangles = getListLength (PolygonTriangles);

// printf ("Number of triangles in polygon: %d\n", numTriangles);	

			for (j=0; j<numTriangles; j++)
			{
				// Get the current triangle.
				triangleList = (nodePtr) getNodeData (PolygonTriangles, j);

				// Skip the drawing of the area if there are
				//  less than 3 sides to the polygon.
				if (getListLength(triangleList) == 3)
				{
					// Get the vertices of the triangle
					vertex1 = (vectorPtr) getNodeData (triangleList, 0);
					vertex2 = (vectorPtr) getNodeData (triangleList, 1);
					vertex3 = (vectorPtr) getNodeData (triangleList, 2);

					// Position the clipping planes
					gl_setup_clip_plane (0, vertex1, vertex2, plane_equation);
					gl_setup_clip_plane (1, vertex2, vertex3, plane_equation);
					gl_setup_clip_plane (2, vertex3, vertex1, plane_equation);

					// Draw the sphere.
					glCallList (listIndex[SPHERE_LIST]);

					// Remove the clipping planes.
					glDisable (GL_CLIP_PLANE0+0);
					glDisable (GL_CLIP_PLANE0+1);
					glDisable (GL_CLIP_PLANE0+2);
				}
			}

/*
// Scale the spherical polygons, to see them appart.
scale[0] += (scale_factor / scale[0]);
scale[1] += (scale_factor / scale[1]);
scale[2] += (scale_factor / scale[2]);
glScalef (scale[0], scale[1], scale[2]);
*/

			// Deallocate memory.
			freeListOfLists (PolygonTriangles);
			PolygonTriangles = NULL;
		}	// if (orientations[i] == direction)
	}	// for (i=0; i<numPolygons; i++)

	glPopMatrix ();

	glEndList ();
}


// Place a clipping plane on the plane where the vertices lie.
void gl_setup_clip_plane (int plane, vectorPtr vertex1, vectorPtr vertex2, GLdouble* plane_equation)
{
	GLdouble	gl_rotation_matrix[16];

	// Find a rotation matrix to match the Y plane
	//  with the plane where the vertices lie.
	rotateToXYplane (vertex1, vertex2, gl_rotation_matrix);

	glPushMatrix ();
	// Rotate according to the matrix found.
	glMultMatrixd (gl_rotation_matrix);

	// Enable the clipping plane.
	glClipPlane (GL_CLIP_PLANE0+plane, plane_equation);
	glEnable (GL_CLIP_PLANE0+plane);

	glPopMatrix ();
}


// Draw the star of a vertex as a wireframe of its triangles.
// Use polygon offset to avoid stitching (artifacts).
void gl_draw_vertex_star (int vertexIndex)
{
	nodePtr			vertexFaceList = NULL;
	faceStruct		face;
	GLint 			polygon_mode_status[2];
	GLboolean		light_status;
	int				listLength;
	int*			faceIndex;
	int				v1, v2, v3, v4;
	int				i;

	vertexFaceList = ObjectData.vertexFaceArray[vertexIndex];
	listLength = getListLength (vertexFaceList);

	// Switch off the lights.
	gl_no_lighting (&light_status);
	// Store the current polygon mode.
	glGetIntegerv (GL_POLYGON_MODE, polygon_mode_status);
	// Switch to line mode (wireframe).
	glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

	glEnable(GL_POLYGON_OFFSET_FILL);

	glColor3f(0.0f, 0.0f, 0.0f);
	glLineWidth( 3.0 );

	if (face.faceVertices == 4)
		glBegin ( GL_QUADS ) ;
	else
		glBegin ( GL_TRIANGLES ) ;

	for (i=0; i<listLength; i++)
	{
		faceIndex = (int*) getNodeData (vertexFaceList, i);
		face = ObjectData.faceArray[*faceIndex];
		v1 = face.vertex[0];
		v2 = face.vertex[1];
		v3 = face.vertex[2];
		v4 = face.vertex[3];

		glVertex3f (ObjectData.displayVertexArray[v1].x, ObjectData.displayVertexArray[v1].y, ObjectData.displayVertexArray[v1].z);
		glVertex3f (ObjectData.displayVertexArray[v2].x, ObjectData.displayVertexArray[v2].y, ObjectData.displayVertexArray[v2].z);
		glVertex3f (ObjectData.displayVertexArray[v3].x, ObjectData.displayVertexArray[v3].y, ObjectData.displayVertexArray[v3].z);
		if (face.faceVertices == 4)
			glVertex3f (ObjectData.displayVertexArray[v4].x, ObjectData.displayVertexArray[v4].y, ObjectData.displayVertexArray[v4].z);
	}

	glEnd ( ) ;

	glLineWidth( 1.0 );

	// Switch back to the previous state.
	glPolygonMode (GL_FRONT, polygon_mode_status[0]);
	glPolygonMode (GL_BACK, polygon_mode_status[1]);

	// Restore light to previous state.
	gl_restore_lighting (light_status);

	glDisable(GL_POLYGON_OFFSET_FILL);
}


void gl_draw_artificial_normal (vertexStruct currentVertex)
{
	float			vectorColour[4] = {0.2f,0.0f,0.5f,1.0f};
	double			n_x, n_y, n_z;
	vectorStruct	normalVector;

	glColor4fv ( vectorColour );
	glLineWidth ( 3.0 );

	// Array of vertex normals does not include a null
	//  element at the beginning (unlike the list of vertices)
	// For that reason, it is necessary to substract one from
	//  ShowVertex.
	normalVector = VertexDataArray[ShowVertex].artificial_normal;
	n_x = normalVector.i;
	n_y = normalVector.j;
	n_z = normalVector.k;

	glPushMatrix ();
		glTranslatef (currentVertex.x, currentVertex.y, currentVertex.z);
		glBegin (GL_LINES);
			glVertex3f (0.0f, 0.0f, 0.0f);
			glVertex3f (n_x, n_y, n_z);
		glEnd ();
	glPopMatrix ();

	glLineWidth ( 1.0 );
}


// Generate a quadric sphere, to show as the surface
//  of the Gauss Map.
void gl_quadric_sphere (GLuint list)
{
	// Initialise the list, in compile mode.
	glNewList (list, GL_COMPILE);
		gluSphere(quadratic, Sphere_Radius, 32, 32);
	glEndList ();
}


// Generate a display list for an octaedron,
//  to be used to represent a point.
void gl_point_list (GLuint list)
{
	glNewList (list, GL_COMPILE);
		glBegin (GL_TRIANGLES);
			glVertex3f ( 0.0,  0.1,  0.0);
			glVertex3f ( 0.0,  0.0,  0.1);
			glVertex3f ( 0.1,  0.0,  0.0);

			glVertex3f ( 0.0,  0.1,  0.0);
			glVertex3f ( 0.1,  0.0,  0.0);
			glVertex3f ( 0.0,  0.0, -0.1);

			glVertex3f ( 0.0,  0.1,  0.0);
			glVertex3f ( 0.0,  0.0, -0.1);
			glVertex3f (-0.1,  0.0,  0.0);

			glVertex3f ( 0.0,  0.1,  0.0);
			glVertex3f (-0.1,  0.0,  0.0);
			glVertex3f ( 0.0,  0.0,  0.1);

			glVertex3f ( 0.0, -0.1,  0.0);
			glVertex3f ( 0.0,  0.0, -0.1);
			glVertex3f (-0.1,  0.0,  0.0);

			glVertex3f ( 0.0, -0.1,  0.0);
			glVertex3f (-0.1,  0.0,  0.0);
			glVertex3f ( 0.0,  0.0,  0.1);

			glVertex3f ( 0.0, -0.1,  0.0);
			glVertex3f ( 0.0,  0.0,  0.1);
			glVertex3f ( 0.1,  0.0,  0.0);

			glVertex3f ( 0.0, -0.1,  0.0);
			glVertex3f ( 0.1,  0.0,  0.0);
			glVertex3f ( 0.0,  0.0, -0.1);
		glEnd ();
	glEndList ();
}


// Function to handle scene specific events.
void gl_scene_event( int event, int data, int xpos, int ypos )
{
	int		numVertices;

	numVertices = ObjectData.numVertices;

	switch (event)
	{
		case 4:
			switch (data)
			{
				// Toggle showing the positive areas.
				case '=': case '+':
					ShowPositive ^= 1;
					break;
				// Toggle showing the negative areas.
				case '-': case '_':
					ShowNegative ^= 1;
					break;

				// Toggle showing the negative areas.
				case ';': case ':':
					colourPalette = (colourPalette + 1) % numPalettes;
					break;

				// Toggle showing the Normal Star.
				case '1':
					ShowNormals ^= 1;
					break;

				// Toggle showing the Arcs between normals.
				case '2':
					ShowArcs ^= 1;
					break;

				// Toggle showing the Extended Normal Star.
				case '3':
					ShowExtendedNormals ^= 1;
					if (ShowNormals && ShowExtendedNormals)
						ShowNormals = FALSE;
					break;

				// Toggle showing the Gauss Map Sphere.
				case '4':
					ShowSphere ^= 1;
					break;

				// Toggle showing the Artificial normal of the vertex.
				case '5':
					ShowVertexNormal ^= 1;
					break;

				// Toggle showing the Object itself.
				case '6':
					ShowObject ^= 1;
					break;

				// Toggle showing the Object itself.
				case '7':
					ShowNormalNames = (ShowNormalNames + 1) % 3;
					break;


#ifdef DECIMATOR
				// Call the decimation algorithm once.
				case '9':
					visual_vertex_decimation ();
					break;

				// Write output file.
				case '0':
					{
						objectStruct	new_object;
						char			filename[100];

						new_object = get_decimated_object (ObjectData);

						// OBJ file format.
						sprintf (filename, "%s-%d.obj", File_Name, Removal_count);
						write_obj_file (new_object, filename);

						// PLY file format.
						sprintf (filename, "%s-%d.ply", File_Name, Removal_count);
						write_ply_file (new_object, filename);

						// OFF file format.
						// sprintf (filename, "%s-%d.off", File_Name, Removal_count);
						// write_off_file (new_object, filename);

						free (new_object.vertexArray);
						free (new_object.faceArray);
					}
					break;
#endif

				// Change to view different vertices.
				case ',':
					if (ShowVertex > 0)
						ShowVertex--;
					break;
				case '.':
					if (ShowVertex < numVertices)
						ShowVertex++;
					break;

				case '{':
					if (ShowVertex > 10)
						ShowVertex -= 10;
					break;
				case '}':
					if (ShowVertex < numVertices-10)
						ShowVertex += 10;
					break;

				case '[':
					if (ShowVertex > 100)
						ShowVertex -= 100;
					break;
				case ']':
					if (ShowVertex < numVertices-100)
						ShowVertex += 100;
					break;

				// Toggle back face culling of the object
				case 'b': case 'B':
					ShowBack ^= 1;
					break;

				// Toggle showing the Convex Hull
				case 'h': case 'H':
					ShowConvexHull ^= 1;
					break;

				// Toggle showing the Convex Hull wireframe
				case 'j': case 'J':
					ShowConvexHullWire ^= 1;
					break;

				// Call the standard event handler.
				default:
					gl_event (event, data, xpos, ypos);
					break;
			}

			if (ShowVertex == 0)
			{
				Begin = 0;
				End = numVertices;
			}
			else
			{
				Begin = ShowVertex - 1;
				End = ShowVertex;
			}

			break;

		// Call the standard event handler.
		default:
			gl_event (event, data, xpos, ypos);
			break;
	}
}
