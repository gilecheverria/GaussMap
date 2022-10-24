#include <GL/gl.h>
#include <stdio.h>

#include "tools.h"

// Save a screenshot of the rendered scene.
void SaveScreenShot(unsigned int serial, int width, int height)
{
	unsigned char	*pixels;
	FILE			*SHOT_FD;
	char			name[50];
	char			number[6];

	if (serial < 10)
		sprintf (number, "0000%i", serial);
	else if (serial < 100)
		sprintf (number, "000%i", serial);
	else if (serial < 1000)
		sprintf (number, "00%i", serial);
	else if (serial < 10000)
		sprintf (number, "0%i", serial);

	sprintf (name, "Screenshots/screenshot-%s.tga", number);

	SHOT_FD = xfopen (name, "wb");

	pixels = (unsigned char*) xmalloc ( sizeof (unsigned char) * (width*height*3) );

	glReadPixels (0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels);

	unsigned char TGAheader[12]={0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char header[6] = {width%256, width/256, height%256, height/256, 24, 0};

	fwrite (TGAheader, sizeof(unsigned char), 12, SHOT_FD);
	fwrite (header, sizeof(unsigned char), 6, SHOT_FD);
	fwrite (pixels, sizeof(unsigned char), width*height*3, SHOT_FD);

	fclose (SHOT_FD);

	free (pixels);

	return;
}
