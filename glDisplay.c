/*
 * Gauss Map Visualization
 * by Gilberto Echeverria <gilecheverria@yahoo.com>
 * based on the grid code by Christophe Devine <devine@cr0.net>
 * this program is licensed under the GPL.
 */

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "tools.h"
#include "glScreenCapture.h"
#include "glTools.h"

#ifndef M_PI
#define M_PI 3.14159265358979f
#endif


GLUquadricObj *quadratic;

/* application window title */
char *AppTitle = "Gauss Map Viewer";

/* external OS-dependant functions */
struct htime
{
    unsigned char blob[16];
};

extern float timer( struct htime *t, int reset );

extern void move_mouse( int x, int y );
extern void hide_mouse( void );

// External variable for the fonts
extern Display*	dpy;

/* external OpenGL functions */
extern void gl_draw_scene_1 ();
extern void gl_draw_scene_2 ();

/* external global variables */
extern GLuint listIndex[];
extern GLuint filter;

/* global data */
int	width, height;

int	info;
int	frames;
GLuint	font;
GLfloat	fps;
GLfloat	rx, ry;
GLfloat	tx, ty;
GLfloat	mx, my;
GLfloat	x, y, z;
GLfloat	vx, vy, vz;
GLfloat	scale = 1.0f;
struct	htime tv, mt;
int     resetX, resetY;
int		resetTranslation;
int     activateRotation, activateScaling, activateTranslation;
int		screenshotCounter;

GLuint	displayLight, displayTextures, mode, displayAxes, shade, displayNormals;
GLuint	video_capture;

GLuint fogMode[] = { GL_EXP, GL_EXP2, GL_LINEAR };  /* three types of fog */
GLuint fogFilter = 0;   /* which fog to use */
GLfloat fogColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };    /* fog color */

/* text drawing routine declaration */
void gl_printf( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha,
                GLint x, GLint y, GLuint font, const char *format, ... );

/* new functions for rotation and scaling from the MRI viewer */
void gl_scale_scene ();
void gl_rotate_scene ();
void gl_translate_scene ();

// Font management functions
GLvoid buildFont(GLvoid);
GLvoid deleteFont(GLvoid);

/* data initialization function */
int gl_data( void )
{
    info = 0;

    x = rx = tx = vx = 0.0f;
    y = ry = ty = vy = 0.0f;
    z =           vz = 0.0f;

    resetX = 0;
    resetY = 0;

    resetTranslation = 0;

    activateRotation = 0;
    activateScaling = 0;
    activateTranslation = 0;

    mode = 1;
    shade = 1;
    displayLight = 1;
    displayTextures = 0;
    displayNormals = 0;
    displayAxes = 1;

	video_capture = 0;
    screenshotCounter = 1;

    return( 0 );
}

// define the properties of the first light source.
void gl_init_light ()
{
	float       lightPosition[4] = {-5.0f,5.0f,5.0f,0.0f};
	float       difuseLight[4] = {0.8f,0.8f,0.8f,1.0f};
	float       ambientLight[4] = {0.4f,0.4f,0.4f,1.0f};

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, difuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, difuseLight);
}


// define properties for the fog
void gl_init_fog ()
{
	glFogi(GL_FOG_MODE, fogMode[fogFilter]);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, 0.05f);
	glHint(GL_FOG_HINT, GL_DONT_CARE);
	// glHint(GL_FOG_HINT, GL_NICEST);
	// glFogf(GL_FOG_START, 1.0f);
	// glFogf(GL_FOG_END, 5.0f);
}


/* gl initialization function */
int gl_init( void )
{
    float       ambient[4] = {0.2f,0.2f,0.2f,1.0f};

    if (shade)
        glShadeModel(GL_SMOOTH);
    else
        glShadeModel(GL_FLAT);

    glEnable( GL_LINE_SMOOTH );
    glLineWidth( 1.0 );

	glEnable(GL_COLOR_MATERIAL);			// Enable Color Material

    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    // glPolygonMode (GL_FRONT, GL_FILL);
    // glPolygonMode (GL_BACK, GL_LINE);

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable(GL_DEPTH_TEST);				// Enables Depth Testing
    glDepthFunc(GL_LEQUAL);					// The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);		// Really Nice Perspective Calculations

    // Do not render the back face of polygons.
    // glEnable (GL_CULL_FACE);

    // Initialize the light properties
    glEnable (GL_LIGHTING);
    glLightModelfv (GL_LIGHT_MODEL_AMBIENT, ambient);
    glEnable (GL_LIGHT0);
    gl_init_light ();

    // Initialize the fog properties
    // glEnable(GL_FOG);
    gl_init_fog ();

    // Disable Texture Mapping
    glDisable(GL_TEXTURE_2D);

    // glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glClearColor( 0.7f, 0.7f, 0.7f, 1.0f );

    // Create a new font
    buildFont ();

    quadratic = gluNewQuadric();
    gluQuadricNormals(quadratic, GLU_SMOOTH);
    gluQuadricTexture(quadratic, GL_TRUE);

    glFlush ();

    hide_mouse();
    move_mouse( width / 2, height / 2 );
    timer( &mt, 1 );

    return( 0 );
}


void gl_scene( int object )
{
    glClear( GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glPushMatrix ();

		gl_translate_scene ();
		gl_scale_scene ();
		gl_rotate_scene ();

		// Draw the reference axis.
		if (displayAxes)
			glCallList (listIndex[0]);

		if (object == 0)
			gl_draw_scene_1 ();
		else
			gl_draw_scene_2 ();

    glPopMatrix ();
}


/* window drawing function */
void gl_draw( )
{
    glClear( GL_COLOR_BUFFER_BIT );

#ifdef SINGLE_DISPLAY
	// SINGLE VIEWPORT
    glViewport( 0, 0, width, height );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0, (GLdouble) width / height, 0.1, 500.0 );
    gluLookAt( 0.0, 1.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 );
    gl_init_light ();

	gl_scene (0);

#else

	// FIRST VIEWPORT
    glViewport( 0, 0, width/2, height );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0, (GLdouble) width / (height*2), 0.1, 500.0 );
    gluLookAt( 0.0, 1.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 );
    gl_init_light ();

	gl_scene (0);


	// SECOND VIEWPORT
	glViewport( width/2, 0, width/2, height );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0, (GLdouble) width / (height*2), 0.1, 500.0 );
    gluLookAt( 0.0, 1.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 );
    gl_init_light ();

	gl_scene (1);
#endif

	/*
	// FIRST VIEWPORT
    glViewport( 0, height/2, width/2, height/2 );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0, (GLdouble) width / height, 0.1, 500.0 );
    gluLookAt( 0.0, 1.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 );
    gl_init_light ();

	gl_scene (0);


	// SECOND VIEWPORT
	glViewport( width/2, height/2, width/2, height/2 );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0, (GLdouble) width / height, 0.1, 500.0 );
    gluLookAt( 0.0, 1.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 );
    gl_init_light ();

	gl_scene (1);

	// THIRD VIEWPORT
	glViewport( width/2, 0, width/2, height/2 );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0, (GLdouble) width / height, 0.1, 500.0 );
    gluLookAt( 0.0, 1.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 );
    gl_init_light ();

	gl_scene (2);

	// FOURTH VIEWPORT
	glViewport( 0, 0, width/2, height/2 );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0, (GLdouble) width / height, 0.1, 500.0 );
    gluLookAt( 0.0, 1.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 );
    gl_init_light ();

	gl_scene (3);
	*/


    // Print the framerate.
    if( fps && info )
    {
        gl_printf( 0.0f, 0.0f, 0.0f, 1.0f, width - 220, height - 40,
        font, "%5.1f fps", fps );
    }

    glFinish();

    frames++;
    if( timer( &tv, 0 ) >= 0.2f )
    {
        fps = (GLfloat) frames / timer( &tv, 1 );
        frames = 0;

		if (video_capture)
			SaveScreenShot(screenshotCounter++, width, height);
    }
}


/* window resizing function */
void gl_resize( void )
{
    glViewport( 0, 0, width, height );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 50.0, (GLdouble) width / height, 0.1, 500.0 );

    gluLookAt( 0.0, 1.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 );

    fps = 0.0f;
    frames = 0;
    timer( &tv, 1 );
}


/* event handling function */
void gl_event( int event, int data, int xpos, int ypos )
{
    GLfloat	tmpScale;

    if( event == 0 )    /* mouse button down */
    {
        if( data == 0 ) // left button
            activateRotation = 1;

        if( data == 1 ) // middle button
            activateTranslation = 1;

        if( data == 2 ) // right button
            activateScaling = 1;
    }

    if( event == 1 )    /* mouse button up */
    {
        if( data == 0 ) // left button
            activateRotation = 0;

        if( data == 1 ) // middle button
            activateTranslation = 0;

        if( data == 2 ) // right button
            activateScaling = 0;
    }

    if( event == 2 )    /* mouse move */
    {
	if (activateRotation)
	{
	    // Change the speed of rotation by the movement of the mouse.
            vy += 256.0f * (GLfloat) ( xpos - width  / 2 ) / width;
            vx += 256.0f * (GLfloat) ( ypos - height  / 2 ) / height;
	}

	if (activateTranslation)
	{
	    // Change the speed of rotation by the movement of the mouse.
	    mx += 256.0f * (GLfloat) ( xpos - width  / 2 ) / width;
	    my -= 256.0f * (GLfloat) ( ypos - height  / 2 ) / height;
	}

	if (activateScaling)
	{
	    tmpScale = scale;

	    scale -= scale * (GLfloat) ( ypos - height  / 2 ) / height;

	    if (scale < 0.001)
		    scale = tmpScale;
	}

        if( timer( &mt, 0 ) > 0.05 )
        {
            timer( &mt, 1 );
            move_mouse( width / 2, height / 2 );
        }
    }

    if( event == 3 )    /* key down */
    {
    }

    if( event == 4 )    /* key up */
    {
		switch (data)
		{
			// Space toggles the framerate display.
			case ' ':
				info ^= 1;
				break;

	/*
			// Exit selection mode.
			case 'p': case 'P':
					selectMode ^= 1;
			if (selectMode)
			{
				show_mouse ();
				activateRotation = 0;
				activateScaling = 0;
			}
			else
				hide_mouse ();
			break;
	*/

			// Save a single screenshot of the rendered scene
			case 'p': case 'P':
				SaveScreenShot(screenshotCounter++, width, height);
				break;

			// Toggle video capture
			case '\\':
				video_capture ^= 1;
				printf ("VIDEO CAPTURE IS: %d\n", video_capture);
				break;

			// Toggle drawing of the face normals.
			case 'n': case 'N':
				displayNormals ^= 1;
				break;

			// Toggle the drawing mode, FILL or LINE
			case 'm': case 'M':
				mode ^= 1;
				if (mode)
				{
					glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
					// glPolygonMode (GL_FRONT, GL_FILL);
					// glPolygonMode (GL_BACK, GL_LINE);
				}
				else
				{
					glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
				}
				break;

			// Toggle the drawing mode, FILL or LINE
			case 'k': case 'K':
				shade ^= 1;
				if (shade)
					glShadeModel (GL_SMOOTH);
				else
					glShadeModel (GL_FLAT);
				break;

			// Toggle lighting
			case 'l': case 'L':
				displayLight ^= 1;
				if (displayLight)
					glEnable (GL_LIGHTING);
				else
					glDisable (GL_LIGHTING);
				break;

			// Toggle displaying the reference axes
			case 'j': case 'J':
				displayAxes ^= 1;
				break;

			// Rotation around the X axis.
			case 'd': case 'D':
				vx += 20.0f;
				break;
			case 'e': case 'E':
				vx -= 20.0f;
				break;

			// Rotation around the Y axis.
			case 'f': case 'F':
				vy += 20.0f;
				break;
			case 's': case 'S':
				vy -= 20.0f;
				break;

			// Reset rotation around X axis.
			case 'z': case 'Z':
				resetX = 1;
				vx = 0.0f;
				break;

			// Reset rotation around the Y axis.
			case 'x': case 'X':
				resetY = 1;
				vy = 0.0f;
				break;

			// Reset rotation around the Y axis.
			case 'v': case 'V':
				resetTranslation = 1;
				mx = my = 0.0f;
				break;

			// Scaling
			case 'q': case 'Q':
				scale *= 1.1;
				break;
			case 'a': case 'A':
				tmpScale = scale;

				scale *= 0.9;

				if (scale < 0.001)
					scale = tmpScale;
				break;

			// Reset scaling
			case 'c': case 'C':
					scale = 1;
				break;
		}

    }

    if( event == 5 )    /* arrow keys */
    {
		switch (data)
		{
			case 0:	// UP Arrow key
					my += 20.0f;
			break;
			case 1:	// DOWN Arrow key
					my -= 20.0f;
			break;
			case 2:	// LEFT Arrow key
					mx -= 20.0f;
			break;
			case 3:	// RIGHT Arrow key
					mx += 20.0f;
			break;
		}
    }
}


// Scale the whole scene, according to the user control.
void gl_scale_scene ()
{
    glScalef( scale, scale, scale );
    glFlush();
}


// Update the variables for translation, and multiply the matrices.
void gl_translate_scene ()
{
    // Update the angles for rotation.
    if( fps )
    {
	if (resetTranslation)
	{
	    if (fps > 4.0f)
	    {
	    	tx *= 1.0f - 4.0f / fps;
	    	ty *= 1.0f - 4.0f / fps;
	    }
	    else
	    {
		tx = 0.0f;
		ty = 0.0f;
	    }
	    if ( (abs(tx) <= 0) && (abs(ty) <= 0) )
	    {
		resetTranslation = 0;
		tx = 0.0f;
		ty = 0.0f;
	    }
	}
	else
	{
            tx += 0.1 * (mx / fps);
            ty += 0.1 * (my / fps);
	}

        if( fps > 4.0f )
        {
            my *= 1.0f - 4.0f / fps;
            mx *= 1.0f - 4.0f / fps;
        }
        else
        {
            my = 0.0f;
            mx = 0.0f;
        }
    }

    glTranslatef( tx, ty, 0.0f );
    glFlush();
}


// Update the variables for rotation, and multiply the matrices.
void gl_rotate_scene ()
{
    GLfloat		M[16];

    // Update the angles for rotation.
    if( fps )
    {
	if (resetX)
	{
	    if (fps > 4.0f)
	    	rx *= 1.0f - 4.0f / fps;
	    else
		rx = 0.0f;
	    if (abs(rx) <= 0)
	    {
		resetX = 0;
		rx = 0.0f;
	    }
	}
	else
            rx += vx / fps;

	if (resetY)
	{
	    if (fps > 4.0f)
	    	ry *= 1.0f - 4.0f / fps;
	    else
		ry = 0.0f;
	    if (abs(ry) <= 0)
	    {
		resetY = 0;
		ry = 0.0f;
	    }
	}
	else
            ry += vy / fps;

        if( fps > 4.0f )
        {
            vy *= 1.0f - 4.0f / fps;
            vx *= 1.0f - 4.0f / fps;
        }
        else
        {
            vy = 0.0f;
            vx = 0.0f;
        }
    }

    // if( ry >  180.0f ) ry += -360.0f;
    // if( ry < -180.0f ) ry +=  360.0f;

    glRotatef( ry, 0.0f, 1.0f, 0.0f );
    glGetFloatv( GL_MODELVIEW_MATRIX, M );
    glRotatef( rx, M[0], M[4], M[8] );
    glGetFloatv( GL_MODELVIEW_MATRIX, M );
    glFlush();
}


/* text drawing routine */

void gl_printf( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha,
                GLint x, GLint y, GLuint font, const char *format, ... )
{
    va_list		argp;
    char		text[256];
    GLboolean	light_status;

    va_start( argp, format );
    vsprintf( text, format, argp );
    va_end( argp );

    // Disable lighting to get the correct
    //  color for the fonts.
	gl_no_lighting (&light_status);

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();

        glLoadIdentity();
        gluOrtho2D( 0.0, (GLdouble) width,
                    0.0, (GLdouble) height );

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        glColor4f( red, green, blue, alpha );
        glRasterPos2i( x, y );

		glPushAttrib(GL_LIST_BIT);				// Pushes The Display List Bits		( NEW )
			glListBase( font );
			glCallLists( strlen( text ), GL_UNSIGNED_BYTE, text );
		glPopAttrib();							// Pops The Display List Bits	( NEW )
    glPopMatrix();

    glMatrixMode( GL_PROJECTION );

    // Reenable lighting
	gl_restore_lighting (light_status);
}


// Create the display lists for the fonts
GLvoid buildFont(GLvoid)
{
    XFontStruct *fontData;
    
    font = glGenLists(96);      /* storage for 96 characters */
    /* load a font with a specific name in "Host Portable Character Encoding" */
    fontData = XLoadQueryFont(dpy,
        "-*-helvetica-bold-r-normal--24-*-*-*-p-*-iso8859-1");
    if (fontData == NULL)
    {
        /* this really *should* be available on every X Window System...*/
        fontData = XLoadQueryFont(dpy, "fixed");
        if (fontData == NULL)
        {
            printf("Problems loading fonts :-(\n");
            exit(1);
        }
    }
    /* build 96 display lists out of our font starting at char 32 */
    glXUseXFont(fontData->fid, 32, 96, font);
    /* free our XFontStruct since we have our display lists */
    XFreeFont(dpy, fontData);

    font -= 32;
}


// Free the lists for the fonts.
GLvoid deleteFont(GLvoid)
{
    glDeleteLists(font+32, 96);
}
