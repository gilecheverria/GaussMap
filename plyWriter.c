#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "faces.h"
#include "object3D.h"
#include "plyWriter.h"

// Output the contents of an objectStruct
//  into a ply file.
void write_ply_file (objectStruct object_data, char* filename)
{
	FILE*			PLY_FD = NULL;
	int				num_vertices;
	int				num_faces;
	int				i;
	time_t			start;
	time_t			stop;
	vertexStruct	vertex;
	faceStruct		face;

    printf ("\n[EXTRA STAGE] -- WRITING PLY FILE: %s\n", filename); fflush (stdout);
	(void) time (&start);

	num_vertices = object_data.numVertices;
	num_faces = object_data.numFaces;

	PLY_FD = xfopen (filename, "w");

	// Write the header.
	fprintf (PLY_FD, "ply\n");
	fprintf (PLY_FD, "format ascii 1.0\n");
	fprintf (PLY_FD, "comment created by 'decimator'\n");
	fprintf (PLY_FD, "element vertex %d\n", num_vertices);
	fprintf (PLY_FD, "property float x\n");
	fprintf (PLY_FD, "property float y\n");
	fprintf (PLY_FD, "property float z\n");
	fprintf (PLY_FD, "element face %d\n", num_faces);
	fprintf (PLY_FD, "property list uchar int vertex_indices\n");
	fprintf (PLY_FD, "end_header\n");

	//Write the coordinates for the vertices.
	for (i=1; i<=num_vertices; i++)
	{
		vertex = object_data.vertexArray[i];
		fprintf (PLY_FD, "%lf %lf %lf\n", vertex.x, vertex.y, vertex.z);
		printProgressStar (i, num_vertices + num_faces);
	}

	// Write the faces.
	for (i=1; i<=num_faces; i++)
	{
		face = object_data.faceArray[i];
		fprintf (PLY_FD, "3 %d %d %d\n", face.vertex[0]-1, face.vertex[1]-1, face.vertex[2]-1);
		printProgressStar (num_vertices + i, num_vertices + num_faces);
	}

	fclose (PLY_FD);

	(void) time (&stop);
	printf ("[done] (%ld seconds)\n", (int) stop - start); fflush (stdout);
}
