#include <math.h>
#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"


// Find the smallest of two numbers
double getMin (double first, double second)
{
	if (first < second)
		return (first);
	else
		return (second);
}


// Find the largest of two numbers
double getMax (double first, double second)
{
	if (first > second)
		return (first);
	else
		return (second);
}


double distance2Vertices (vertexStruct vertex1, vertexStruct vertex2)
{
	return ( sqrtf (squareDistance (vertex1, vertex2)) );
}


// Returns the square of the distance between 2 vertices.
double squareDistance (vertexStruct vertex1, vertexStruct vertex2)
{
	double	xDifference;
	double	yDifference;
	double	zDifference;

	xDifference = vertex2.x - vertex1.x;
	yDifference = vertex2.y - vertex1.y;
	zDifference = vertex2.z - vertex1.z;

	return ( (xDifference * xDifference) + (yDifference * yDifference) + (zDifference * zDifference) );
}


// Allocate memory and fill in the data for a segment structure.
segmentPtr createSegment (int vertex1, int vertex2, int info)
{
	segmentPtr	newSegment = NULL;

	newSegment = (segmentPtr) xmalloc (sizeof (segmentStruct));
	newSegment->vertex1 = vertex1;
	newSegment->vertex2 = vertex2;
	newSegment->info = info;

	return (newSegment);
}


// Find the components of a vector, enclosed between two vertices.
vectorPtr vectorFromVertices (vertexPtr vertex1, vertexPtr vertex2)
{
	vectorPtr	newVector;

	newVector = (vectorPtr) xmalloc (sizeof (vectorStruct));

	newVector->i = vertex2->x - vertex1->x;
	newVector->j = vertex2->y - vertex1->y;
	newVector->k = vertex2->z - vertex1->z;

	return (newVector);
}


// Find the angle formed by three vertices, by creating 2 vectors.
// Returns the angle in radians.
double angleBetweenVertices (vertexStruct vertex1, vertexStruct vertex2, vertexStruct vertex3)
{
	vectorPtr	vector1 = NULL;
	vectorPtr	vector2 = NULL;
	double		angle;

	vector1 = vectorFromVertices (&vertex1, &vertex2);
	vector2 = vectorFromVertices (&vertex2, &vertex3);

	angle = angleBetweenVectors (vector1, vector2);

	free (vector1);
	free (vector2);

	return (angle);
}


// Compute the angle between two vectors.
double angleBetweenVectors (vectorPtr vector1, vectorPtr vector2)
{
	double	length1;
	double	length2;
	double	lengthProduct;
	double	cosine;

	length1 = vectorLength (vector1);
	length2 = vectorLength (vector2);
	lengthProduct = length1 * length2;

	if (lengthProduct == 0)
	{
		// printf ("WARNING! Division by zero in 'angleBetweenVectors'. Returning 0.\n");
		return (0.0);
	}
	else
	{
		cosine = dotProduct (vector1, vector2) / lengthProduct;

		// Check that the cosine is within logical boundaries.
		if (cosine > 1)
			return ( acos (1.0) );
		else if (cosine < -1)
			return ( acos (-1.0) );
		else
			return ( acos (cosine) );
	}
}


// Test if the coordinates of two vertices are the same.
// Done based on the distance between both, given some tolerance.
boolean equalVertices (vertexStruct vertex1, vertexStruct vertex2)
{
	static float    SMALL_EPSILON = 1E-3;
	double          distance;

	distance = distance2Vertices (vertex1, vertex2);

	if ( (SMALL_EPSILON > distance) && (distance > -SMALL_EPSILON) )
		return (TRUE);
	else
		return (FALSE);
}


// Test whether a vector is equal to zero.
// In most cases, this is incorrect.
boolean vector_equal_to_zero (vectorPtr vector)
{
	return (	(vector->i == 0.0) &&
				(vector->j == 0.0) &&
				(vector->k == 0.0)		);
}


// Determine if two vectors have the same direction,
//  within a certain tolerance.
boolean equalVectors (vectorPtr vector1, vectorPtr vector2)
{
	static float		SMALL_EPSILON = 1E-4;
	double				x_difference;
	double				y_difference;
	double				z_difference;

	x_difference = vector2->i - vector1->i;
	y_difference = vector2->j - vector1->j;
	z_difference = vector2->k - vector1->k;

	if (	( (SMALL_EPSILON > x_difference) && (x_difference > -SMALL_EPSILON) ) &&
			( (SMALL_EPSILON > y_difference) && (y_difference > -SMALL_EPSILON) ) &&
			( (SMALL_EPSILON > z_difference) && (z_difference > -SMALL_EPSILON) )	)
		return (TRUE);
	else
		return (FALSE);
}


// Determine if two vectors have the same direction,
//  within a certain larger tolerance.
boolean relaxedEqualVectors (vectorPtr vector1, vectorPtr vector2)
{
	static float		SMALL_EPSILON = 1E-2;
	// static float		SMALL_EPSILON = 1E-3;
	double				x_difference;
	double				y_difference;
	double				z_difference;

	x_difference = vector2->i - vector1->i;
	y_difference = vector2->j - vector1->j;
	z_difference = vector2->k - vector1->k;

	if (	( (SMALL_EPSILON > x_difference) && (x_difference > -SMALL_EPSILON) ) &&
			( (SMALL_EPSILON > y_difference) && (y_difference > -SMALL_EPSILON) ) &&
			( (SMALL_EPSILON > z_difference) && (z_difference > -SMALL_EPSILON) )	)
		return (TRUE);
	else
		return (FALSE);
}



// Determine if two vectors are equal or opposite,
//  based on the angle between them.
boolean colinearVectors (vectorPtr vector1, vectorPtr vector2)
{
	static float		SMALL_EPSILON = 1E-4;
	// double				angle;
	double				x_difference;
	double				y_difference;
	double				z_difference;

	x_difference = vector2->i - vector1->i;
	y_difference = vector2->j - vector1->j;
	z_difference = vector2->k - vector1->k;

	if (	( (SMALL_EPSILON > x_difference) && (x_difference > -SMALL_EPSILON) ) &&
			( (SMALL_EPSILON > y_difference) && (y_difference > -SMALL_EPSILON) ) &&
			( (SMALL_EPSILON > z_difference) && (z_difference > -SMALL_EPSILON) )	)
		return (TRUE);

	// Test for vector2 equal to the inverted direction of vector1
	x_difference = vector2->i + vector1->i;
	y_difference = vector2->j + vector1->j;
	z_difference = vector2->k + vector1->k;

	if (	( (SMALL_EPSILON > x_difference) && (x_difference > -SMALL_EPSILON) ) &&
			( (SMALL_EPSILON > y_difference) && (y_difference > -SMALL_EPSILON) ) &&
			( (SMALL_EPSILON > z_difference) && (z_difference > -SMALL_EPSILON) )	)
		return (TRUE);

	return (FALSE);

	/*
	angle = angleBetweenVectors (vector1, vector2);

	// Return true if they are equal.
	if ( (angle > -SMALL_EPSILON) && (angle < SMALL_EPSILON) )
		return (TRUE);

	// Return true if they are opposite.
	if ( (angle > PI-SMALL_EPSILON) && (angle < PI+SMALL_EPSILON) )
		return (TRUE);

	return (FALSE);
	*/
}


// Obtain the dot product of two vectors.
double dotProduct (vectorPtr vector1, vectorPtr vector2)
{
	return ( (vector2->i * vector1->i) + (vector2->j * vector1->j) + (vector2->k * vector1->k) );
}


// Compute the cross product of two vectors.
vectorPtr crossProduct (vectorPtr vector1, vectorPtr vector2)
{
	vectorPtr	newVector = NULL;

	newVector = (vectorPtr) xmalloc (sizeof (vectorStruct) );

	newVector->i = vector1->j * vector2->k - vector1->k * vector2->j;
	newVector->j = vector1->k * vector2->i - vector1->i * vector2->k;
	newVector->k = vector1->i * vector2->j - vector1->j * vector2->i;

	normalizeVector (newVector);

	return (newVector);
}


// Sum two vectors.
vectorPtr sumVectors (vectorPtr vector1, vectorPtr vector2)
{
	vectorPtr	newVector = NULL;

	newVector = (vectorPtr) xmalloc (sizeof (vectorStruct) );

	newVector->i = vector1->i + vector2->i;
	newVector->j = vector1->j + vector2->j;
	newVector->k = vector1->k + vector2->k;

	return (newVector);
}


// Compute the length of a vector.
double vectorLength (vectorPtr vector)
{
	double	iSquared;
	double	jSquared;
	double	kSquared;

	iSquared = vector->i * vector->i;
	jSquared = vector->j * vector->j;
	kSquared = vector->k * vector->k;

	return ( sqrtf (iSquared + jSquared + kSquared) );
}


// Alter the size of a normal vector, to make it equal to 1.
void normalizeVector (vectorPtr vector)
{
	double		length;

	length = vectorLength (vector);

	if (length == 0.0f)
	{
		// printf ("WARNING! Vector '%d' of zero length in 'normalizeVector'. Returning 1.\n", vector->vectorId);
		length = 1.0f;
	}

	vector->i /= length;
	vector->j /= length;
	vector->k /= length;
}


// Get the orientation of the triangle formed by three vertices.
// This is done by obtaining the slopes of the two segments generated by the vertices.
spinType orientationFromVertices_2D (vertexStruct vertex1, vertexStruct vertex2, vertexStruct vertex3)
{
	double	orientation;

	orientation = (vertex2.y - vertex1.y) * (vertex3.x - vertex2.x) - (vertex3.y - vertex2.y) * (vertex2.x - vertex1.x);

	// Clock Wise
	if ( orientation > (0 + EPSILON) )
		return (CW);
	// Counter Clock Wise
	else if ( orientation < (0 - EPSILON) )
		return (CCW);
	// Collinear
	else
		return (COL);
}


// Get the orientation of the triangle formed by three vertices.
// Since the vertices can lie in any plane, the normal vector
//  to the plane is used to determine their orientation.
spinType orientationFromVertices_3D (vertexStruct vertex1, vertexStruct vertex2, vertexStruct vertex3, vectorStruct normal_vector)
{
	double		orientation;
	vectorPtr	vector1;
	vectorPtr	vector2;
	vectorPtr	cross_vector;

	vector1 = vectorFromVertices (&vertex1, &vertex2);
	vector2 = vectorFromVertices (&vertex2, &vertex3);

	cross_vector = crossProduct (vector1, vector2);
	orientation = dotProduct (cross_vector, &normal_vector);

	free (vector1);
	free (vector2);
	free (cross_vector);

	// Clock Wise
	if ( orientation > (0 + EPSILON) )
		return (CCW);
	// Counter Clock Wise
	else if ( orientation < (0 - EPSILON) )
		return (CW);
	// Collinear
	else
		return (COL);
}


// Check whether two segments intersect each other.
// Each segment is defined by its two endpoints.
boolean testSegmentsIntersect_2D (vertexStruct vertexA1, vertexStruct vertexA2, vertexStruct vertexB1, vertexStruct vertexB2)
{
	int		farVertex;
	spinType	orientation1;
	spinType	orientation2;
	spinType	orientation3;
	spinType	orientation4;

	orientation1 = orientationFromVertices_2D (vertexA1, vertexA2, vertexB1);
	orientation2 = orientationFromVertices_2D (vertexA1, vertexA2, vertexB2);
	orientation3 = orientationFromVertices_2D (vertexB1, vertexB2, vertexA1);
	orientation4 = orientationFromVertices_2D (vertexB1, vertexB2, vertexA2);

	// General case
	if ( (orientation1 != orientation2) && (orientation3 != orientation4) )
		return (TRUE);

	// Special case where the segments lie in the same line
	if ( (orientation1 ==  COL) && (orientation1 == orientation2) &&
	     (orientation2 == orientation3) && (orientation3 == orientation4) )
	{
		farVertex = findFarCollinearVertex (vertexA1, vertexA2, vertexB1);
		if (farVertex == vertexA2.vertexId)
			return (TRUE);
		else
		{
			farVertex = findFarCollinearVertex (vertexA1, vertexA2, vertexB2);
			if (farVertex == vertexA2.vertexId)
				return (TRUE);
		}
	}

	return (FALSE);
}


// Check whether two segments intersect each other.
boolean testSegmentsIntersect_NotInclusive_2D (vertexStruct vertexA1, vertexStruct vertexA2, vertexStruct vertexB1, vertexStruct vertexB2)
{
	int		farVertex;
	spinType	orientation1;
	spinType	orientation2;
	spinType	orientation3;
	spinType	orientation4;

	orientation1 = orientationFromVertices_2D (vertexA1, vertexA2, vertexB1);
	orientation2 = orientationFromVertices_2D (vertexA1, vertexA2, vertexB2);
	orientation3 = orientationFromVertices_2D (vertexB1, vertexB2, vertexA1);
	orientation4 = orientationFromVertices_2D (vertexB1, vertexB2, vertexA2);

	if ( !(	(vertexA1.vertexId == vertexB1.vertexId) ||
		(vertexA1.vertexId == vertexB2.vertexId) ||
		(vertexA2.vertexId == vertexB1.vertexId) ||
		(vertexA2.vertexId == vertexB2.vertexId)	) )
	{
		// Special case where one of the vertices in a segment
		//  lies on the other segment
		if (	( (orientation1 == COL) && (orientation2 != COL) ) ||
			( (orientation2 == COL) && (orientation1 != COL) ) ||
			( (orientation3 == COL) && (orientation4 != COL) ) ||
			( (orientation4 == COL) && (orientation3 != COL) )	)
				return (FALSE);
		// General case
		if ( (orientation1 != orientation2) && (orientation3 != orientation4) )
			return (TRUE);
	}

	// Special case where the segments lie in the same line
	if ( (orientation1 ==  COL) && (orientation1 == orientation2) &&
	     (orientation2 == orientation3) && (orientation3 == orientation4) )
	{
		farVertex = findFarCollinearVertex (vertexA1, vertexA2, vertexB1);
		if (	(vertexA1.vertexId != vertexB1.vertexId) &&
			(vertexA2.vertexId != vertexB1.vertexId) &&
			(farVertex == vertexA2.vertexId)		)
				return (TRUE);
		else
		{
			farVertex = findFarCollinearVertex (vertexA1, vertexA2, vertexB2);
			if (	(vertexA1.vertexId != vertexB2.vertexId) &&
				(vertexA2.vertexId != vertexB2.vertexId) &&
				(farVertex == vertexA2.vertexId)		)
					return (TRUE);
		}
	}

	return (FALSE);
}


// Test whether a segment intersects any of the segments in a curve.
boolean testSegmentCurveIntersect (vertexPtr vertexArray, nodePtr curve, vertexStruct vertex1, vertexStruct vertex2)
{
	nodePtr		curvePointer = curve;

	vertexStruct	curveVertex1;
	vertexStruct	curveVertex2;

	double		segmentMinY;
	double		segmentMaxY;
	double		curveMinY;
	double		curveMaxY;

	int*		vertexIndex = NULL;

	boolean		intersection = FALSE;

	vertexIndex = (int*) curvePointer->data;
	curveVertex1 = vertexArray[*vertexIndex];
	curvePointer = curvePointer->next;

	// Find the maximum and minimum values on the segment of interest.
	segmentMaxY = getMax (vertex1.y, vertex2.y);
	segmentMinY = getMin (vertex1.y, vertex2.y);

	while (curvePointer != NULL)
	{
		vertexIndex = (int*) curvePointer->data;
		curveVertex2 = vertexArray[*vertexIndex];

		// Find the maximum and minimum values on the current segment of the curve.
		curveMaxY = getMax (curveVertex1.y, curveVertex2.y);
		curveMinY = getMin (curveVertex1.y, curveVertex2.y);

		// Test for intersections only if any of the vertices of one segment falls
		//  within the range of the other segment.
		if (	( (curveVertex1.y <= segmentMaxY) && (curveVertex1.y >= segmentMinY) ) ||
			( (curveVertex2.y <= segmentMaxY) && (curveVertex2.y >= segmentMinY) ) ||
			( (vertex1.y <= curveMaxY) && (vertex1.y >= curveMinY) ) ||
			( (vertex2.y <= curveMaxY) && (vertex2.y >= curveMinY) )		)
		{
			if (	(vertex1.vertexId != curveVertex1.vertexId) &&
				(vertex1.vertexId != curveVertex2.vertexId) &&
				(vertex2.vertexId != curveVertex1.vertexId) &&
				(vertex2.vertexId != curveVertex2.vertexId)		)
				intersection = testSegmentsIntersect_2D (curveVertex1, curveVertex2, vertex1, vertex2);
			else
				intersection = testSegmentsIntersect_NotInclusive_2D (curveVertex1, curveVertex2, vertex1, vertex2);
			if (intersection)
				break;
		}

		curveVertex1 = curveVertex2;
		curvePointer = curvePointer->next;
	}

	return (intersection);
}


// Check which of two collinear vertices is farther from a reference vertex
// Returns the vertexId of the vertex that is farther away.
int findFarCollinearVertex (vertexStruct vertex1, vertexStruct vertex2, vertexStruct vertex3)
{
	vectorPtr	vector1_2 = NULL;
	vectorPtr	vector1_3 = NULL;
	double		length1_2;
	double		length1_3;
	double		cosine;
	int		farVertex;

	vector1_2 = vectorFromVertices (&vertex1, &vertex2);
	vector1_3 = vectorFromVertices (&vertex1, &vertex3);
	length1_2 = vectorLength (vector1_2);
	length1_3 = vectorLength (vector1_3);
	cosine = dotProduct (vector1_2, vector1_3) / (length1_2 * length1_3);

	// if ( (cosine > 0) && (length1_3 > length1_2) )

	// If the vertices are collinear in different directions,
	//  with respect to vertex1, then none is farther away.
	if (cosine < 0)
		farVertex = -1;
	else
	{
		if (length1_3 > length1_2)
			farVertex = vertex3.vertexId;
		else
			farVertex = vertex2.vertexId;
	}

	free (vector1_2);
	free (vector1_3);

	return (farVertex);
}


// Find the distance from a vertex to the segment defined within two other vertices.
// It returns the distance if the vertex is in front of the segment. If not, returns -1.
double distanceVertexToSegment (vertexStruct vertex, vertexStruct segmentVertex1, vertexStruct segmentVertex2)
{
	vectorPtr	segmentVector = NULL;
	vectorPtr	vectorToVertex = NULL;
	vertexPtr	newVertex = NULL;

	double		product1;
	double		product2;
	double		result = -1;
	double		b;

	segmentVector = vectorFromVertices (&segmentVertex2, &segmentVertex1);
	vectorToVertex = vectorFromVertices (&vertex, &segmentVertex1);

	// Distance to a segment from any vertex.
	product1 = dotProduct (vectorToVertex, segmentVector);
	if (product1 <= 0)
		result = distance2Vertices (vertex, segmentVertex1);
	else
	{
		product2 = dotProduct (segmentVector, segmentVector);
		if (product2 <= product1)
			result = distance2Vertices (vertex, segmentVertex2);

		else
		{
			b = product1 / product2;
			newVertex = createVertex (-1,	segmentVertex1.x - b * segmentVector->i,
							segmentVertex1.y - b * segmentVector->j,
							segmentVertex1.z - b * segmentVector->k	);
			result = distance2Vertices (vertex, *newVertex);
		}
	}

	/*
	// Distance to a segment from a vertex which can be projected
	//  inside of the segment.
	product1 = dotProduct (vectorToVertex, segmentVector);
	if (product1 == 0)
		result = distance2Vertices (vertex, segmentVertex1);
	else if (product1 > 0)
	{
		product2 = dotProduct (segmentVector, segmentVector);
		if (product2 == product1)
			result = distance2Vertices (vertex, segmentVertex2);

		else if (product1 < product2)
		{
			b = product1 / product2;
			newVertex = createVertex (-1,	segmentVertex1->x - b * segmentVector->i,
							segmentVertex1->y - b * segmentVector->j,
							segmentVertex1->z - b * segmentVector->k	);
			result = distance2Vertices (vertex, *newVertex);
		}
	}
	*/

	free (segmentVector);
	free (vectorToVertex);
	free (newVertex);

	return (result);
}


// Print the contents of a list of vectors.
void printVectorList (nodePtr list)
{
	nodePtr		pointer = list;
	vectorPtr	vector = NULL;

	printf ("Vector list:\t");

	while (pointer->next != NULL)
	{
		vector = (vectorPtr) pointer->data;
		printf ("%d, ", vector->vectorId);
		pointer = pointer->next;
	}

	vector = (vectorPtr) pointer->data;
	printf ("%d\n", vector->vectorId);
}


// Project a single vertex on a plane, along the normal of the plane.
// Takes as parameters the normal of the plane and a vertex in its surface,
//  and the vertex to be projected.
// Returns a new vertex in the variable 'projected_vertex'.
vertexStruct project_vertex_on_plane (vectorStruct plane_normal, vertexStruct plane_vertex, vertexStruct original_vertex)
{
	double          t;
	double          new_x;
	double          new_y;
	double          new_z;
	double          i_0 = plane_normal.i;
	double          j_0 = plane_normal.j;
	double          k_0 = plane_normal.k;
	double          x_1 = plane_vertex.x;
	double          y_1 = plane_vertex.y;
	double          z_1 = plane_vertex.z;
	double          x_i = original_vertex.x;
	double          y_i = original_vertex.y;
	double          z_i = original_vertex.z;
	vertexStruct	projected_vertex;

	///////////////////////////////////////////////////////////
	// NOTE: INCLUDE PROOF OF THIS FORMULA IN DOCUMENTATION. //
	///////////////////////////////////////////////////////////
	t = ( ( i_0 * (x_1 - x_i) ) + (j_0 * (y_1 - y_i) ) + (k_0 * (z_1 - z_i)) ) /
	( (i_0 * i_0) + (j_0 * j_0) + (k_0 * k_0) );

	new_x = x_i + i_0 * t;
	new_y = y_i + j_0 * t;
	new_z = z_i + k_0 * t;

	projected_vertex.x = new_x;
	projected_vertex.y = new_y;
	projected_vertex.z = new_z;

	return (projected_vertex);
}


// Recursively divide a polygon in a plane into triangles.
nodePtr triangulateFlatPolygon (vertexPtr vertexArray, nodePtr polygonList, vectorStruct plane_normal, nodePtr triangulation)
{
	nodePtr		part1List = NULL;
	nodePtr		part2List = NULL;
	int			listLength = getListLength (polygonList);

	if (listLength <= 3)
	{
		triangulation = addFrontNode (triangulation, polygonList);
	}
	else
	{
		divideFlatPolygon (vertexArray, polygonList, &part1List, &part2List, plane_normal);
		triangulation = triangulateFlatPolygon (vertexArray, part1List, plane_normal, triangulation);
		triangulation = triangulateFlatPolygon (vertexArray, part2List, plane_normal, triangulation);
	}

	return (triangulation);
}


// Divide a polygon into two simpler polygons.
// The division is done by drawing a diagonal between two
//  vertices of the polygon. These two vertices will now
//  belong to both of the polygons.
void divideFlatPolygon (vertexPtr vertexArray, nodePtr polygonList, nodePtr* part1List, nodePtr* part2List, vectorStruct plane_normal)
{
	vertexStruct	vertex1;
	vertexStruct	vertex2;
	vertexStruct	vertex3;
	vertexStruct	vertex4;
	int				listLength = getListLength (polygonList);
	int				i, j;
	int				closest = 0;
	int*			index = NULL;
	int*			tmpIndex = NULL;
	spinType		orientation;
	boolean			inside = FALSE;
	double			distance;
	double			smallDistance = 0;

	for (i=0; i<listLength; i++)
	{
		index = (int*) getNodeData (polygonList, (listLength-1+i)%listLength);
		vertex1 = vertexArray[*index];
		index = (int*) getNodeData (polygonList, i);
		vertex2 = vertexArray[*index];
		index = (int*) getNodeData (polygonList, (i+1)%listLength);
		vertex3 = vertexArray[*index];

		orientation = orientationFromVertices_3D (vertex1, vertex2, vertex3, plane_normal);

		// If vertex2 is a convex angle.
		if ( (orientation == CCW) && (listLength > 3) )
		{
			// Check that no other vertices lie inside of the triangle
			//  formed by the 3 vertices.
			for (j=0; j<listLength; j++)
			{
				index = (int*) getNodeData (polygonList, j);
				vertex4 = vertexArray[*index];

				if ( !(	equalVertices(vertex1, vertex4) ||
						equalVertices(vertex2, vertex4) ||
						equalVertices(vertex3, vertex4) )	)
				{
					inside = pointInsideFlatTriangle (vertex1, vertex2, vertex3, vertex4, plane_normal);

					// If a vertex lies inside, check its distance.
					if (inside)
					{
						distance = squareDistance (vertex2, vertex4);
						
						// Store the smallest distance and the index number
						//  for the closest vertex.
						if ( (distance < smallDistance) || (smallDistance == 0) )
						{
							smallDistance = distance;
							closest = j;
						}
					}

				}
			}	// for (j=0; j<listLength; j++)

			// If there was any point inside of the triangle.
			// Then the distance will be different from 0.
			if (smallDistance > 0)	// Split the polygon with a diagonal between: vertex2, vertex4.
			{
				*part1List = polygonList;

				// Copy one of the vertices that will belong to
				//  both new polygons.
				index = (int*) getNodeData (polygonList, closest);
				vertex4 = vertexArray[*index];
				tmpIndex = (int*) xmalloc (sizeof (int));
				memcpy (tmpIndex, &vertex4.vertexId, sizeof (int));
				*part2List = addFrontNode (*part2List, tmpIndex);

				if (closest < i)
					closest += listLength;

				// Put the rest of the vertices in the other list.
				for (j=closest-1; j>i; j--)
				{
					tmpIndex = (int*) removeNode (part1List, (j%listLength));
					*part2List = addFrontNode (*part2List, tmpIndex);
				}

				// Copy the second of the vertices that will belong
				//  to both new polygons.
				tmpIndex = (int*) xmalloc (sizeof (int));
				memcpy (tmpIndex, &vertex2.vertexId, sizeof (int));
				*part2List = addFrontNode (*part2List, tmpIndex);
			}
			else		// Split the polygon with a diagonal between: vertex1, vertex3.
			{
				// Copy one of the vertices that will belong to
				//  both new polygons.
				tmpIndex = (int*) xmalloc (sizeof (int));
				memcpy (tmpIndex, &vertex1.vertexId, sizeof (int));
				*part1List = addFrontNode (*part1List, tmpIndex);

				// Copy the second of the vertices that will belong to
				//  both new polygons.
				tmpIndex = (int*) xmalloc (sizeof (int));
				memcpy (tmpIndex, &vertex3.vertexId, sizeof (int));
				*part1List = addFrontNode (*part1List, tmpIndex);

				tmpIndex = (int*) xmalloc (sizeof (int));
				memcpy (tmpIndex, &vertex2.vertexId, sizeof (int));
				*part1List = addFrontNode (*part1List, tmpIndex);

				*part2List = polygonList;
				// Remove the one vertex that will belong to the other polygon.
				index = (int*) removeNode (part2List, i);
				free (index);
			}

			// Stop the for loop.
			break;
		}	// if ( (orientation == CCW) && (listLength > 3) )
	}	// for (i=0; i<listLength; i++)
}


// Determine if a vertex is inside of a triangle.
// Returns true if the point is inside.
boolean pointInsideFlatTriangle (vertexStruct vertex1, vertexStruct vertex2, vertexStruct vertex3, vertexStruct vertex4, vectorStruct plane_normal)
{
	spinType	orientation1;
	spinType	orientation2;
	spinType	orientation3;

	orientation1 = orientationFromVertices_3D (vertex4, vertex1, vertex2, plane_normal);
	orientation2 = orientationFromVertices_3D (vertex4, vertex2, vertex3, plane_normal);
	orientation3 = orientationFromVertices_3D (vertex4, vertex3, vertex1, plane_normal);

	if (	(orientation1 == CCW || orientation1 == COL) &&
			(orientation2 == CCW || orientation2 == COL) &&
			(orientation3 == CCW || orientation3 == COL)	)
		return (TRUE);
	else
		return (FALSE);
}


// Function taken from:
// Copyright 2000, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.
//									by Dan Sunday

// area3D_Polygon(): computes the area of a 3D planar polygon
//    Input:  int num_vertices = the number of vertices in the polygon
//            vertexPtr vertex_array = an array of n+2 vertices in a plane
//                       with vertex_array[n]=vertex_array[0] and vertex_array[n+1]=vertex_array[1]
//            vectorStruct normal_vector = unit normal vector of the polygon's plane
//    Return: the (double) area of the polygon
double area3D_Polygon( int num_vertices, vertexPtr vertex_array, vertexStruct normal_vector )
{
    double	 area = 0;
    double	 an, ax, ay, az;  // abs value of normal and its coords
    int  	 coord;           // coord to ignore: 1=x, 2=y, 3=z
    int  	 i, j, k;         // loop indices

    // select largest abs coordinate to ignore for projection
    ax = (normal_vector.x>0 ? normal_vector.x : -normal_vector.x);     // abs x-coord
    ay = (normal_vector.y>0 ? normal_vector.y : -normal_vector.y);     // abs y-coord
    az = (normal_vector.z>0 ? normal_vector.z : -normal_vector.z);     // abs z-coord

    coord = 3;                     // ignore z-coord
    if (ax > ay) {
        if (ax > az) coord = 1;    // ignore x-coord
    }
    else if (ay > az) coord = 2;   // ignore y-coord

    // compute area of the 2D projection
    for (i=1, j=2, k=0; i<=num_vertices; i++, j++, k++)
        switch (coord) {
        case 1:
            area += (vertex_array[i].y * (vertex_array[j].z - vertex_array[k].z));
            continue;
        case 2:
            area += (vertex_array[i].x * (vertex_array[j].z - vertex_array[k].z));
            continue;
        case 3:
            area += (vertex_array[i].x * (vertex_array[j].y - vertex_array[k].y));
            continue;
        }

    // scale to get area before projection
    an = sqrt( ax*ax + ay*ay + az*az);  // length of normal vector
    switch (coord) {
    case 1:
        area *= (an / (2*ax));
        break;
    case 2:
        area *= (an / (2*ay));
        break;
    case 3:
        area *= (an / (2*az));
    }
    return (area);
}
