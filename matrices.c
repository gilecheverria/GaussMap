#include <stdio.h>
#include <math.h>

#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "matrices.h"


// Transpose a matrix.
void transposeMatrix (double original[][3], double inverted[][3])
{
	int	i, j;

	for (i=0; i<3; i++)
	{
		for (j=0; j<3; j++)
		{
			inverted[j][i] = original[i][j];
		}
	}
}


// Multilpy two matrices.
void matrixMultiplication (double a[][3], double b[][3], double result[][3])
{
	int i, j;

	for(i=0; i<3; i++)
	{
		for(j=0; j<3; j++)
		{
			result[i][j] =	a[i][0] * b[0][j] +
					a[i][1] * b[1][j] +
					a[i][2] * b[2][j];
		}
	}
}


// Multilpy a matrix and a vector.
// Returns a new vector.
vectorPtr matrixVectorMultiplication (double matrix[][3], vectorPtr vector)
{
	int		i;
	double		tmpVector[3] = {vector->i, vector->j, vector->k};
	double		tmpResult[3];
	vectorPtr	result = NULL;

	for(i=0; i<3; i++)
	{
		tmpResult[i] =	matrix[i][0] * tmpVector[0] + 
				matrix[i][1] * tmpVector[1] + 
				matrix[i][2] * tmpVector[2];
	}

	result = (vectorPtr) xmalloc (sizeof (vectorStruct));

	result->i = tmpResult[X];
	result->j = tmpResult[Y];
	result->k = tmpResult[Z];

	return (result);
}


// Print a matrix on screen.
void printMatrix (double a[][3])
{
	int i, j;

	for (i=0; i<3; i++)
	{
		for (j=0; j<3; j++)
		{
			printf("%.14f\t", a[i][j]);
		}
		printf("\n");
	}
}


// Print a matrix on screen.
// Such matrix is stored as a single dimension array
//  of 16 elements, representing a 4X4 matrix.
void printLinearMatrix (double linearMatrix[])
{
	int i;

	for (i=0; i<4; i++)
		printf ("%.4f\t%.4f\t%.4f\t%.4f\n",	linearMatrix[i],
							linearMatrix[i+4],
							linearMatrix[i+8],
							linearMatrix[i+12]	);
}


// Generate a matrix to produce a rotation around 
//  a unit vector of the specified amount of angles.
// The angle must be in radians.
void rotationMatrixAroundVector (vectorPtr vector, double angle, double rotationMatrix[][3])
{
	double		x = vector->i;
	double		y = vector->j;
	double		z = vector->k;

	rotationMatrix[0][0] = x*x + (1 - x*x) * cos(angle);
	rotationMatrix[0][1] = -z*sin(angle) + (1-cos(angle))*x*y;
	rotationMatrix[0][2] =  y*sin(angle) + (1-cos(angle))*x*z;

	rotationMatrix[1][0] =  z*sin(angle) + (1-cos(angle))*x*y;
	rotationMatrix[1][1] = y*y + (1 - y*y) * cos(angle);
	rotationMatrix[1][2] = -x*sin(angle) + (1-cos(angle))*y*z;

	rotationMatrix[2][0] = -y*sin(angle) + (1-cos(angle))*x*z;
	rotationMatrix[2][1] =  x*sin(angle) + (1-cos(angle))*y*z;
	rotationMatrix[2][2] = z*z + (1 - z*z) * cos(angle);
}


// Convert a square matrix into a single dimension array.
// As used by OpenGL for the glMultMatrix function.
// The array will have mXn elements.
void matrixToLinear (double initialMatrix[][3], double finalMatrix[])
{
	int		i, j;
	int		k = 0;

	for (j=0; j<3; j++)
	{
		for (i=0; i<3; i++)
			finalMatrix[k++] = initialMatrix[i][j];
		finalMatrix[k++] = 0;
	}

	for (j=0; j<3; j++)
		finalMatrix[k++] = 0;

	finalMatrix[k++] = 1;
}
