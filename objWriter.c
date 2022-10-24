#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "faces.h"
#include "object3D.h"
#include "objWriter.h"

// Output the contents of an objectStruct
//  into an obj file.
void write_obj_file (objectStruct object_data, char* filename)
{
	FILE*			OBJ_FD = NULL;
	int				num_vertices;
	int				num_faces;
	int				i;
	time_t			start;
	time_t			stop;
	vertexStruct	vertex;
	faceStruct		face;

	printf ("\n[EXTRA STAGE] -- WRITING OBJ FILE: %s\n", filename); fflush (stdout);
	(void) time (&start);

	num_vertices = object_data.numVertices;
	num_faces = object_data.numFaces;

	OBJ_FD = xfopen (filename, "w");

	//Write the coordinates for the vertices.
	fprintf (OBJ_FD, "# %d vertices\n", num_vertices);
	for (i=1; i<=num_vertices; i++)
	{
		vertex = object_data.vertexArray[i];
		fprintf (OBJ_FD, "v %lf %lf %lf\n", vertex.x, vertex.y, vertex.z);
		printProgressStar (i, num_vertices + num_faces);
	}

	// Write the faces.
	fprintf (OBJ_FD, "\n# %d faces\n", num_faces);
	for (i=1; i<=num_faces; i++)
	{
		face = object_data.faceArray[i];
		fprintf (OBJ_FD, "f %d %d %d\n", face.vertex[0], face.vertex[1], face.vertex[2]);
		printProgressStar (num_vertices + i, num_vertices + num_faces);
	}

	fclose (OBJ_FD);

	(void) time (&stop);
	printf ("[done] (%ld seconds)\n", (int) stop - start); fflush (stdout);
}
