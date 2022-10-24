#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "faces.h"
#include "object3D.h"
#include "offWriter.h"

// Output the contents of an objectStruct
//  into an off file (Geomview).
void write_off_file (objectStruct object_data, char* filename)
{
	FILE*           OFF_FD = NULL;
	int             num_vertices;
	int             num_faces;
	int             i;
	time_t          start;
	time_t          stop;
	vertexStruct    vertex;
	faceStruct      face;

	printf ("\n[EXTRA STAGE] -- WRITING OFF FILE: %s\n", filename); fflush (stdout);
	(void) time (&start);

	num_vertices = object_data.numVertices;
	num_faces = object_data.numFaces;

	OFF_FD = xfopen (filename, "w");

	// Write the OFF file header.
	fprintf (OFF_FD, "    OFF\n");
	fprintf (OFF_FD, "%d %d %d\n", num_vertices, num_faces, 0);

	//Write the coordinates for the vertices.
	for (i=1; i<=num_vertices; i++)
	{
		vertex = object_data.vertexArray[i];
		fprintf (OFF_FD, "%lf\t%lf\t%lf\n", vertex.x, vertex.y, vertex.z);
		printProgressStar (i, num_vertices + num_faces);

	}

	// Write the faces.
	for (i=1; i<=num_faces; i++)
	{
		face = object_data.faceArray[i];
		// Substract 1 from the indexes of the vertices,
		//  since Geomview indexes them from 0.
		fprintf (OFF_FD, "%d\t%d\t%d\t%d\n", face.faceVertices, face.vertex[0]-1, face.vertex[1]-1, face.vertex[2]-1);
		printProgressStar (num_vertices + i, num_vertices + num_faces);
	}

	fclose (OFF_FD);

	(void) time (&stop);
	printf ("[done] (%ld seconds)\n", (int) stop - start); fflush (stdout);
}
