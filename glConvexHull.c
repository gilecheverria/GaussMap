#include <GL/gl.h>
#include <GL/glu.h>

#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "faces.h"
#include "object3D.h"
#include "glTools.h"

extern vertexPtr C_H_NormalArray;

// Create a display list for the convex hull of an object.
void gl_convex_hull_to_display_list (GLuint list, nodePtr convexHull, objectStruct objectData)
{
	vertexPtr	vertexArray = objectData.displayVertexArray;
	facePtr		face;
	int			numFaces = getListLength (convexHull);
	int			i;
	int			v1, v2, v3;

	GLfloat c_h_material[] = { 0.5, 0.5, 0.0, 0.4 };

    // Initialize the list, in compile mode.
	glNewList (list, GL_COMPILE);

	glBegin (GL_TRIANGLES);

	glColor4fv (c_h_material);

	for (i=0; i<numFaces; i++)
	{
		face = (facePtr) getNodeData (convexHull, i);

		v1 = face->vertex[0];
		v2 = face->vertex[1];
		v3 = face->vertex[2];

		glNormal3f (C_H_NormalArray[v1].x, C_H_NormalArray[v1].y, C_H_NormalArray[v1].z);
		glVertex3f (vertexArray[v1].x, vertexArray[v1].y, vertexArray[v1].z);

		glNormal3f (C_H_NormalArray[v2].x, C_H_NormalArray[v2].y, C_H_NormalArray[v2].z);
		glVertex3f (vertexArray[v2].x, vertexArray[v2].y, vertexArray[v2].z);

		glNormal3f (C_H_NormalArray[v3].x, C_H_NormalArray[v3].y, C_H_NormalArray[v3].z);
		glVertex3f (vertexArray[v3].x, vertexArray[v3].y, vertexArray[v3].z);
	}

	glEnd ();

	glEndList ();
}


// Create a list for the wireframe of convex hull of an object.
void gl_convex_hull_wireframe_to_display_list (GLuint list, nodePtr convexHull, objectStruct objectData)
{
	vertexPtr	vertexArray = objectData.displayVertexArray;
	GLint		polygon_mode_status[2];
	GLboolean	light_status;
	facePtr		face;
	int			numFaces = getListLength (convexHull);
	float		edgeColour[4] = {0.0f,1.0f,1.0f,1.0f};
	int			i;
	int			v1, v2, v3;

	// Initialize the list, in compile mode.
	glNewList (list, GL_COMPILE);

	// Switch off the lights.
	gl_no_lighting (&light_status);
	// Store the current polygon mode.
	glGetIntegerv (GL_POLYGON_MODE, polygon_mode_status);
	// Switch to line mode (wireframe).
	glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

	glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, edgeColour);
	glColor4fv ( edgeColour );

	for (i=0; i<numFaces; i++)
	{
		face = (facePtr) getNodeData (convexHull, i);

		v1 = face->vertex[0];
		v2 = face->vertex[1];
		v3 = face->vertex[2];

		glBegin (GL_TRIANGLES);
			glVertex3f (vertexArray[v1].x, vertexArray[v1].y, vertexArray[v1].z);
			glVertex3f (vertexArray[v2].x, vertexArray[v2].y, vertexArray[v2].z);
			glVertex3f (vertexArray[v3].x, vertexArray[v3].y, vertexArray[v3].z);
		glEnd ();
	}

	// Switch back to the previous state.
	glPolygonMode (GL_FRONT, polygon_mode_status[0]);
	glPolygonMode (GL_BACK, polygon_mode_status[1]);

	// Restore light to previous state.
	gl_restore_lighting (light_status);

	glDisable(GL_POLYGON_OFFSET_FILL);

	glEndList ();
}
