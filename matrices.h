// Transpose a matrix.
void transposeMatrix (double original[][3], double inverted[][3]);

// Multilpy two matrices.
void matrixMultiplication (double a[][3], double b[][3], double result[][3]);

// Multilpy a matrix and a vector.
// Returns a new vector.
vectorPtr matrixVectorMultiplication (double matrix[][3], vectorPtr vector);

// Print a matrix on screen.
void printMatrix (double a[][3]);

// Print a matrix on screen.
// Such matrix is stored as a single dimension array
//  of 16 elements, representing a 4X4 matrix.
void printLinearMatrix (double linearMatrix[]);

// Generate a matrix to produce a rotation around 
//  a unit vector of the specified amount of angles.
void rotationMatrixAroundVector (vectorPtr vector, double grads, double rotationMatrix[][3]);

// Convert a square matrix into a single dimension array.
// As used by OpenGL for the glMultMatrix function.
// The array will have mXn elements.
void matrixToLinear (double initialMatrix[][3], double finalMatrix[]);
