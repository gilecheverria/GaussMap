/*
 * Generic X11+GLX basecode,
 * by Christophe Devine <devine@cr0.net>
 * this program is licensed under the GPL.
 */

#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/gl.h>

#include <sys/time.h>
#include <stdio.h>
#include <time.h>


/* external gl data */
extern char	*AppTitle;
extern int	width, height, font;

/* external gl rountines */
extern int  gl_data( void );
extern int  gl_init( void );
extern void gl_draw( void );
extern void gl_resize( void );
extern void gl_event( int event, int data,
                      int xpos,  int ypos );
extern void gl_scene_event(	int event, int data,
							int xpos,  int ypos );

extern void gl_prepare_scene ();
extern void gl_cleanup ();

/* variables associated with the window */
Display		*dpy;
Window		win;
Atom		wmDelete;
GLXContext	ctx;
Cursor		null_cursor;

int	modeswitch;
int	active;
int	run;


/* declaration of the GLX initialization function */
int glx_init( int fullscreen );


/* main OpenGL method */
int glMain ( void )
{
    int		fullscreen;
    XEvent	event;

    if( gl_data() )
    {
        fprintf( stderr, "gl_data failed\n" );
        return( 1 );
    }

    fullscreen = 0;

    do
    {
        modeswitch  = 0;
        fullscreen ^= 1;

        if( glx_init( fullscreen ) )
        {
            fprintf( stderr, "glx_init failed\n" );
            return( 1 );
        }

        if( gl_init() )
        {
            fprintf( stderr, "gl_init failed\n" );
            return( 1 );
        }

        gl_resize();

        run = 1;

		gl_prepare_scene ();

        while( run )
        {
            if( active )
            {
                gl_draw();
                glXSwapBuffers( dpy, win );
            }
            else
            {
                XPeekEvent( dpy, &event );
            }

            while( XPending( dpy ) )
            {
                XNextEvent( dpy, &event );

                switch( event.type )
                {
                    case ButtonPress:
                    {
                        int x = event.xmotion.x,
                            y = event.xmotion.y;

                        switch( event.xbutton.button )
                        {
                            case Button1: gl_event( 0, 0, x, y ); break;
                            case Button2: gl_event( 0, 1, x, y ); break;
                            case Button3: gl_event( 0, 2, x, y ); break;
                        }

                        break;
                    }

                    case ButtonRelease:
                    {
                        int x = event.xmotion.x,
                            y = event.xmotion.y;

                        switch( event.xbutton.button )
                        {
                            case Button1: gl_event( 1, 0, x, y ); break;
                            case Button2: gl_event( 1, 1, x, y ); break;
                            case Button3: gl_event( 1, 2, x, y ); break;
                        }

                        break;
                    }

                    case MotionNotify:
                    {
                        int x = event.xmotion.x,
                            y = event.xmotion.y;

                        switch( event.xbutton.button )
                        {
                            case Button1: gl_event( 2,  0, x, y ); break;
                            case Button3: gl_event( 2,  1, x, y ); break;
                            default:      gl_event( 2, -1, x, y ); break;
                        }

                        break;
                    }

                    case KeyPress:
                    {
                        break;
                    }

                    case KeyRelease:
                    {
                        int key = XLookupKeysym( &event.xkey, 0 );

                        switch( key )
                        {
                            case XK_Tab:
                                modeswitch = 1;

                            case XK_Escape:
								gl_cleanup ();
                                run = 0;
                                break;

							case XK_Up:
								gl_event( 5, 0, -1, -1 );
								break;

							case XK_Down:
								gl_event( 5, 1, -1, -1 );
								break;

							case XK_Left:
								gl_event( 5, 2, -1, -1 );
								break;

							case XK_Right:
								gl_event( 5, 3, -1, -1 );
								break;

                            default:
                                gl_scene_event( 4, key, -1, -1 );
                                break;
                        }

                        break;
                    }

                    case UnmapNotify: active = 0; break;
                    case   MapNotify: active = 1; break;

                    case ConfigureNotify:
                    {
                        width  = event.xconfigure.width;
                        height = event.xconfigure.height;
                        gl_resize();
                        break;
                    }

                    case ClientMessage:    
                    {
                        if( event.xclient.data.l[0] == (int) wmDelete )
                        {
                            active = run = 0;
                        }
                        break;
                    }

                    case ReparentNotify: break;

                    default:
                    {
                        printf( "caught unknown event, type %d\n",
                                event.type );
                        break;
                    }
                }
            }
        }

        glXMakeCurrent( dpy, None, NULL );
        glXDestroyContext( dpy, ctx );
        XDestroyWindow( dpy, win );
        XCloseDisplay( dpy );
    }
    while( modeswitch );

    return( 0 );
}

int glx_init( int fullscreen )
{
    int vi_attr[] =
        {
            GLX_RGBA,
            GLX_DOUBLEBUFFER,
            GLX_RED_SIZE,       4, 
            GLX_GREEN_SIZE,     4, 
            GLX_BLUE_SIZE,      4, 
            GLX_DEPTH_SIZE,    16,
            None
        };

    XVisualInfo *vi;
    Window root_win;
    XWindowAttributes win_attr;
    XSetWindowAttributes set_attr;
    XFontStruct *fixed;
    XColor black =
    {
        0, 0, 0, 0, 0, 0
    };

    if( ( dpy = XOpenDisplay( NULL ) ) == NULL )
    {
        fprintf( stderr, "XOpenDisplay failed\n" );
        return( 1 );
    }

    if( ( vi = glXChooseVisual( dpy, DefaultScreen( dpy ),
                                vi_attr ) ) == NULL )
    {
        fprintf( stderr, "glXChooseVisual failed\n" );
        XCloseDisplay( dpy );
        return( 1 );
    }

    root_win = RootWindow( dpy, vi->screen );

    XGetWindowAttributes( dpy, root_win, &win_attr );

    width  = ( fullscreen ) ? win_attr.width  : 640;
    height = ( fullscreen ) ? win_attr.height : 480;
    // width  = ( fullscreen ) ? win_attr.width  : 1024;
    // height = ( fullscreen ) ? win_attr.height : 768;

    set_attr.border_pixel = 0;

    set_attr.colormap =
        XCreateColormap( dpy, root_win, vi->visual, AllocNone );

    set_attr.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask |
        ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;

    set_attr.override_redirect = ( ( fullscreen ) ? True : False );

    win = 
        XCreateWindow(
                dpy, root_win, 0, 0, width, height, 0, vi->depth,
                InputOutput, vi->visual, CWBorderPixel | CWColormap |
                CWEventMask | CWOverrideRedirect, &set_attr );

    XStoreName( dpy, win, AppTitle );
    XMapWindow( dpy, win );

    if( fullscreen )
    {
        XGrabKeyboard(  dpy, win, True, GrabModeAsync,
                        GrabModeAsync, CurrentTime );
    }
    else
    {
        wmDelete = XInternAtom( dpy, "WM_DELETE_WINDOW", True );
        XSetWMProtocols( dpy, win, &wmDelete, 1 );
    }

    if( ( ctx = glXCreateContext( dpy, vi, NULL, True ) ) == NULL )
    {
        fprintf( stderr, "glXCreateContext failed\n" );
        XDestroyWindow( dpy, win );
        XCloseDisplay( dpy );
        return( 1 );
    }

    if( glXMakeCurrent( dpy, win, ctx ) == False )
    {
        fprintf( stderr, "glXMakeCurrent failed\n" );
        glXDestroyContext( dpy, ctx );
        XDestroyWindow( dpy, win );
        XCloseDisplay( dpy );
        return( 1 );
    }

    font = glGenLists( 256 );

    fixed = XLoadQueryFont(
        dpy, "-misc-fixed-medium-r-*-*-20-*-*-*-*-*-*-*" );

    null_cursor = XCreateGlyphCursor(
        dpy, fixed->fid, fixed->fid, ' ', ' ', &black, &black );

    glXUseXFont( fixed->fid, 0, 256, font );

    XFreeFont( dpy, fixed );

    return( 0 );
}

/* timer structure */

struct timer
{
    struct timeval start;
};

/* timer query/reset routine */

float timer( struct timer *t, int reset )
{
    float delta;
    struct timeval offset;

    gettimeofday( &offset, NULL );

    delta = (float) ( offset.tv_sec  - t->start.tv_sec  ) +
            (float) ( offset.tv_usec - t->start.tv_usec ) / 1e6;

    if( reset )
    {
        t->start.tv_sec  = offset.tv_sec;
        t->start.tv_usec = offset.tv_usec;
    }

    return( delta );
}

/* mouse handling routines */

void hide_mouse( void )
{
    XDefineCursor( dpy, win, null_cursor );
}

void move_mouse( int x, int y )
{
    XWarpPointer( dpy, None, win, 0, 0, 0, 0, x, y );
}
