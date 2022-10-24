#include <GL/gl.h>
#include <GL/glu.h>


// Reset the model view matrix
//  so that a billboard will
//  always be facing in the same direction.
void gl_billboard_init ()
{
	int		i,j;
	float	modelview[16];

	// save the current modelview matrix
	glPushMatrix();

	// get the current modelview matrix
	glGetFloatv(GL_MODELVIEW_MATRIX , modelview);

	// undo all rotations
	// beware all scaling is lost as well
	for( i=0; i<3; i++ )
		for( j=0; j<3; j++ )
		{
			if ( i==j )
				modelview[i*4+j] = 1.0;
			else
				modelview[i*4+j] = 0.0;
		}

	// set the modelview with no rotations and scaling
	glLoadMatrixf(modelview);
}


// Restore the modelview matrix.
void gl_billboard_end ()
{
	glPopMatrix();
}


// Turn off lighting if it is enabled.
void gl_no_lighting (GLboolean* light_status)
{
	// Store the light condition.
	glGetBooleanv (GL_LIGHTING, light_status);

	// Switch off the lights.
	glDisable (GL_LIGHTING);
}


// Turn lighting on again if necesary
void gl_restore_lighting (GLboolean light_status)
{
	if (light_status)
		glEnable (GL_LIGHTING);
}
