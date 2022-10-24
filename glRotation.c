#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "matrices.h"


// Find the rotation matrix needed to rotate the whole OpenGL scene
//  to match the XY plane with the plane defined by the 
//  two vectors given as parameters.
void rotateToXYplane (vectorPtr vector1, vectorPtr vector2, double gl_rotation_matrix[])
{
	static float	SMALL_EPSILON = 1E-3;
	vectorPtr		normalVector1 = NULL;
	vectorPtr		normalVector2 = NULL;
	vectorPtr		tmpVector = NULL;
	vectorPtr		yAxis = NULL;
	vectorPtr		zAxis = NULL;
	double			rotationMatrix1[3][3];
	double			rotationMatrix2[3][3];
	double			inverseMatrix[3][3];
	double			finalRotationMatrix[3][3];
	double			angle1;
	double			angle2;

	yAxis = (vectorPtr) xmalloc (sizeof (vectorStruct));
	yAxis->i = 0.0;
	yAxis->j = 1.0;
	yAxis->k = 0.0;

	zAxis = (vectorPtr) xmalloc (sizeof (vectorStruct));
	zAxis->i = 0.0;
	zAxis->j = 0.0;
	zAxis->k = 1.0;

// printf ("\n");
// printf ("Vector1:\t%f %f %f\t==> %f\n", vector1->i, vector1->j, vector1->k, vectorLength(vector1));
// printf ("Vector2:\t%f %f %f\t==> %f\n", vector2->i, vector2->j, vector2->k, vectorLength(vector2));

	// Find a vector perpendicular to both the Y-axis
	//  and one of the input vectors.
	normalVector1 = crossProduct (yAxis, vector1);
// printf ("normalVector1:\t%f %f %f\t==> %f\n", normalVector1->i, normalVector1->j, normalVector1->k, vectorLength(normalVector1));

	// Obtain the angle between these two, in radians.
	angle1 = angleBetweenVectors (yAxis, vector1);
// printf ("Angle1: %.14f\n", angle1 * 180 / PI);

	// Clear the memory for the rotation matrices.
	memset (rotationMatrix1, 0, sizeof (double) * 9);
	memset (rotationMatrix2, 0, sizeof (double) * 9);
	memset (inverseMatrix, 0, sizeof (double) * 9);
	memset (finalRotationMatrix, 0, sizeof (double) * 9);

	rotationMatrixAroundVector (normalVector1, angle1, rotationMatrix1);
	rotationMatrixAroundVector (normalVector1, -angle1, inverseMatrix);

	// Correct the orientation of the planes,
	//  in the case where the first vector is 
	//  equal to the negative Y-axis.
	// (Used for the clipping planes)
	if ( (angle1 < PI+SMALL_EPSILON) && (angle1 > PI-SMALL_EPSILON) )
	{
		// If there is no rotation around other axis,
		//  then cancell the whole rotation.
		if ( rotationMatrix1[0][3] == 0.0)
		{
			rotationMatrix1[0][0] = 1.0;
			inverseMatrix[0][0] = 1.0;
		}
	}

/*
printf ("RotationMatrix1:\n");
printMatrix (rotationMatrix1);
printf ("\n");
printf ("InverseMatrix:\n");
printMatrix (inverseMatrix);
printf ("\n");
*/

	// Adjust the second vector according to the rotation matrix
	//  obtained.
	tmpVector = matrixVectorMultiplication (inverseMatrix, vector2);
	normalizeVector (tmpVector);
// printf ("tmpVector:\t%f %f %f\t==> %f\n", tmpVector->i, tmpVector->j, tmpVector->k, vectorLength(tmpVector));

	// Find a vector perpendicular to both the Y axis
	//  and the second input vector.
	normalVector2 = crossProduct (tmpVector, yAxis);
// printf ("normalVector2:\t%f %f %f\t==> %f\n", normalVector2->i, normalVector2->j, normalVector2->k, vectorLength(normalVector2));

	// Obtain the angle between the new vector and
	//  the Z-axis.
	angle2 = angleBetweenVectors (zAxis, normalVector2);
	if (normalVector2->i <= 0)
		angle2 *= -1;
// printf ("Angle2: %.9f\n", angle2 * 180 / PI);

	rotationMatrixAroundVector (yAxis, angle2, rotationMatrix2);

// printf ("RotationMatrix2:\n");
// printMatrix (rotationMatrix2);

	// Multiply both rotation matrices to obtain a single one.
	matrixMultiplication (rotationMatrix1, rotationMatrix2, finalRotationMatrix);

	matrixToLinear (finalRotationMatrix, gl_rotation_matrix);

// printf ("\nThe FINAL rotation matrix:\n");
// printMatrix (finalRotationMatrix);
// printf ("OpenGL rotation Array:\n");
// printLinearMatrix (gl_rotation_matrix);

	free (normalVector1);
	free (normalVector2);
	free (tmpVector);

	free (yAxis);
	free (zAxis);
}
