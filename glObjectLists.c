#include <GL/gl.h>
#include <GL/glu.h>

#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "faces.h"
#include "object3D.h"
#include "vertexGeometry.h"

extern vertexDataStruct*	VertexDataArray;
extern double				MAX_TAC;

static float    SMALL_EPSILON = 1E-6;

// Return the type of vertex according to angle deficit.
int get_angle_deficit_type(double angle_deficit)
{
	if (angle_deficit > SMALL_EPSILON)		 	// if (angle_deficit > 0)
		return (1);
	else if (angle_deficit < -SMALL_EPSILON)	// if (angle_deficit > 0)
		return (2);
	else										// if (angle_deficit == 0)
		return (0);
}


// Generate the display list for the model described
//  by the vertices and faces.
// NOTE: cotourType table:
//		1 = total absolute curvature (positive, negative, mixed, flat)
//		2 = gaussian curvature
//		3 = total absolute curvature (numerical)
void gl_faces_to_display_list (GLuint list, objectStruct objectData, int colourType)
{
	vertexPtr			vertexArray = objectData.displayVertexArray;
	vertexPtr			normalArray = objectData.normalArray;
	vertexDataStruct	vertex_data;
	faceStruct			face;
	double				tac;
	int					i;

#ifdef BnW
	float		vertexTypeColour[6][4] = {
										{0.7f,0.7f,0.7f,0.7f},		// Flat
										{0.0f,0.0f,0.0f,0.7f},		// All positive
										{1.0f,1.0f,1.0f,0.7f},		// All negative
										{0.3f,0.3f,0.3f,0.7f},		// Mixed
										{0.0f,0.0f,0.0f,0.7f},		// All positive concave
										{0.3f,0.3f,0.3f,0.7f}		// Mixed concave
										};
#else
	float		vertexTypeColour[6][4] = {
										{1.0f,1.0f,1.0f,0.7f},		// Flat
										{1.0f,0.0f,0.0f,0.7f},		// All positive
										{0.0f,0.0f,1.0f,0.7f},		// All negative
										{0.0f,1.0f,0.0f,0.7f},		// Mixed
										{1.0f,0.0f,1.0f,0.7f},		// All positive concave
										{0.6f,0.4f,0.0f,0.7f}		// Mixed concave
										};
#endif


/* ORIGINAL COLORS
										{0.2f,0.7f,0.2f,0.7f},		// All positive
										{0.8f,0.4f,0.2f,0.7f},		// All negative
										{0.2f,0.2f,0.2f,0.7f},		// Mixed
										{1.0f,1.0f,1.0f,0.7f}	};	// Flat
*/

/*
	float		angleTypeColour[2][4] = {
										{0.9f,0.9f,0.9f,0.7f},
										{0.1f,0.1f,0.1f,0.7f}	};
*/


	int			counter = 1;
	int			v1, v2, v3, v4;

	MAX_TAC /= 4;

	GLfloat mat_specular[] = { 0.5, 0.5, 0.5, 1.0 };
	// GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat low_shininess[] = { 5.0 };

	// Initialize the list, in compile mode.
	glNewList (list, GL_COMPILE);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, low_shininess);

	for (i=1; i<=objectData.numFaces; i++)
	{
		face = objectData.faceArray[i];

		// Skip over faces removed by decimation.
		if (face.faceId < 0)
			continue;

		v1 = face.vertex[0];
		v2 = face.vertex[1];
		v3 = face.vertex[2];
		v4 = face.vertex[3];

		glLoadName (counter++);

		if (face.faceVertices == 4)
		    glBegin (GL_QUADS);
		else
		    glBegin (GL_TRIANGLES);

			// glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, faceColour);
			// glColor4fv ( faceColour );

			vertex_data = VertexDataArray[v1];
			if (colourType == 1)
				glColor4fv (vertexTypeColour[vertex_data.type]);
			else if (colourType == 2)
				glColor4fv (vertexTypeColour[get_angle_deficit_type(vertex_data.angle_deficit)]);
			else if (colourType == 3)
			{
				tac = vertex_data.curvature;
				glColor4f (1-(tac/MAX_TAC), 1-(tac/MAX_TAC), 0.0f, 0.7f);
			}

			glNormal3f (normalArray[v1].x, normalArray[v1].y, normalArray[v1].z);
			glVertex3f (vertexArray[v1].x, vertexArray[v1].y, vertexArray[v1].z);

			vertex_data = VertexDataArray[v2];
			if (colourType == 1)
				glColor4fv (vertexTypeColour[vertex_data.type]);
			else if (colourType == 2)
				glColor4fv (vertexTypeColour[get_angle_deficit_type(vertex_data.angle_deficit)]);
			else if (colourType == 3)
			{
				tac = vertex_data.curvature;
				glColor4f (1-(tac/MAX_TAC), 1-(tac/MAX_TAC), 0.0f, 0.7f);
			}
			
			glNormal3f (normalArray[v2].x, normalArray[v2].y, normalArray[v2].z);
			glVertex3f (vertexArray[v2].x, vertexArray[v2].y, vertexArray[v2].z);

			vertex_data = VertexDataArray[v3];
			if (colourType == 1)
				glColor4fv (vertexTypeColour[vertex_data.type]);
			else if (colourType == 2)
				glColor4fv (vertexTypeColour[get_angle_deficit_type(vertex_data.angle_deficit)]);
			else if (colourType == 3)
			{
				tac = vertex_data.curvature;
				glColor4f (1-(tac/MAX_TAC), 1-(tac/MAX_TAC), 0.0f, 0.7f);
			}
			
			glNormal3f (normalArray[v3].x, normalArray[v3].y, normalArray[v3].z);
			glVertex3f (vertexArray[v3].x, vertexArray[v3].y, vertexArray[v3].z);

			if (face.faceVertices == 4)
			{
				vertex_data = VertexDataArray[v4];
				if (colourType == 1)
					glColor4fv (vertexTypeColour[vertex_data.type]);
				else if (colourType == 2)
					glColor4fv (vertexTypeColour[get_angle_deficit_type(vertex_data.angle_deficit)]);
				else if (colourType == 3)
				{
					tac = vertex_data.curvature;
					glColor4f (1-(tac/MAX_TAC), 1-(tac/MAX_TAC), 0.0f, 0.7f);
				}
				
				glNormal3f (normalArray[v4].x, normalArray[v4].y, normalArray[v4].z);
				glVertex3f (vertexArray[v4].x, vertexArray[v4].y, vertexArray[v4].z);
			}
		glEnd ();
	}

	glEndList ();
}


// Generate the display list for the normals
//  to the vertices.
void gl_normals_to_display_list (GLuint list, objectStruct objectData)
{
	vertexPtr	vertexArray = objectData.displayVertexArray;
	vertexPtr	normalArray = objectData.normalArray;
	float		faceColour[4] = {1.0f,0.0f,1.0f,1.0f};
	double		v_x, v_y, v_z;
	double		n_x, n_y, n_z;
	int			i;
	int			numVertices = objectData.numVertices;

	// Initialize the list, in compile mode.
	glNewList (list, GL_COMPILE);

	glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, faceColour);
	glColor4fv ( faceColour );

	for (i=1; i<=numVertices; i++)
	{
		// Skip over vertices removed by decimation.
		if (vertexArray[i].vertexId < 0)
			continue;

		v_x = vertexArray[i].x;
		v_y = vertexArray[i].y;
		v_z = vertexArray[i].z;

		n_x = normalArray[i].x / 3.0f;
		n_y = normalArray[i].y / 3.0f;
		n_z = normalArray[i].z / 3.0f;

		glBegin (GL_LINES);
			glVertex3f (v_x, v_y, v_z);
			glVertex3f (v_x+n_x, v_y+n_y, v_z+n_z);
		glEnd ();
	}

	glEndList ();
}
