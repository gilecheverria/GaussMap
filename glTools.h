//  so that a billboard will
//  always be facing in the same direction.
void gl_billboard_init ();


// Restore the modelview matrix.
void gl_billboard_end ();


// Turn off lighting if it is enabled.
void gl_no_lighting (GLboolean* light_status);


// Turn lighting on again if necesary
void gl_restore_lighting (GLboolean light_status);
