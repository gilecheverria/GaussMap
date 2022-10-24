#include <stdio.h>
#include <math.h>

#include "tools.h"
#include "lists.h"
#include "vertices.h"
#include "geometry.h"
#include "faces.h"
#include "sphericalGeometry.h"
#include "convexHull3D.h"


extern nodePtr		PolygonTriangles;

// List for storing the pairs of intersecting vectors.
// Each pair structure is an array of 6 integers.
// [0] vector Id of intersection 1
// [1] vector Id of intersection 2
// [2] endpoint of arc 1
// [3] endpoint of arc 1
// [4] endpoint of arc 2
// [5] endpoint of arc 2
nodePtr				IntersectionPairs = NULL;
nodePtr				IntersectionPairsCopy = NULL;


// Extract the normals from a list of faces.
// Return a list with only the normals of the faces.
nodePtr getFaceNormalList (nodePtr faceList, facePtr faceArray, int* numNormals, int vertex_index, nodePtr* feature_edge_list)
{
	nodePtr			normalList = NULL;
	nodePtr			normalListTail = NULL;
	vectorPtr		vector = NULL;
	vectorPtr		previousVector = NULL;
	vectorPtr		nextVector = NULL;
	vectorPtr		newVector = NULL;
	faceStruct		face;
	spinType		orientation;
	int				i;
	int				numFaces = getListLength (faceList);
	int				counter = 0;
	int*			index = NULL;
	int*			feature_edge = NULL;
	static double	feature_threshold = 1.2;
	double			angle_1;
	double			angle_2;

	// Get the last face, and set its normal as
	//  the 'previous' one.
	index = (int*) getNodeData (faceList, numFaces-1);
	face = faceArray[*index];
	previousVector = face.faceNormal;

	for (i=0; i<numFaces; i++)
	{
		// Get the normal vector of the current face.
		index = (int*) getNodeData (faceList, i);
		face = faceArray[*index];
		vector = face.faceNormal;

		// Skip vectors that are the same.
		if ( !relaxedEqualVectors (vector, previousVector) || (numFaces == 1) )
		{
			newVector = (vectorPtr) xmalloc (sizeof (vectorStruct));
			memcpy (newVector, vector, sizeof (vectorStruct));
			normalListTail = addNode (normalListTail, newVector);
			if (normalList == NULL)
				normalList = normalListTail;

			// (used for decimation/triangulation).
			if (angleBetweenVectors (previousVector, vector) > feature_threshold)
			{
				reorderFace (&face, vertex_index);
				// Store the vertexId of the endpoint
				//  of the edge.
				feature_edge = createIntPointer (face.vertex[1]);
				// Insert into the list.
				*feature_edge_list = addFrontNode (*feature_edge_list, feature_edge);
			}

			previousVector = vector;
			counter++;
		}
	}

	// Check for collinear arcs.
	// Only if there are more than two normals.
	if (counter > 2)
	{
		for (i=0; i<counter; i++)
		{
			previousVector = (vectorPtr) getNodeData (normalList, i);
			vector = (vectorPtr) getNodeData (normalList, (i+1)%counter);
			nextVector = (vectorPtr) getNodeData (normalList, (i+2)%counter);

			// Remove coplanar contiguous arcs. (Both lie in a flat face)
			orientation = sphereAngleOrientation (previousVector, vector, nextVector, 1);
			if (orientation == COL)
			{
				angle_1 = angleBetweenVectors (previousVector, vector);
				angle_2 = angleBetweenVectors (previousVector, nextVector);

				if (angle_1 > angle_2)
					newVector = removeNode (&normalList, (i+2)%counter);
				else
					newVector = removeNode (&normalList, (i+1)%counter);
				free (newVector);
				i--;
				counter--;
			}
		}
	}

	// Check the case when all normals were eliminated.
	// This is the case when all the normals are equal.
	// A vertex in the middle of a flat face. 
	if (counter == 0)
	{
		// Get the normal vector of the first face.
		index = (int*) getNodeData (faceList, 0);
		face = faceArray[*index];
		vector = face.faceNormal;

		// Add the first normal to the list.
		newVector = (vectorPtr) xmalloc (sizeof (vectorStruct));
		memcpy (newVector, vector, sizeof (vectorStruct));
		normalList = addNode (normalListTail, newVector);
		counter = 1;
	}

	*numNormals = counter;

	return (normalList);
}


// Add extra vectors between pairs of opposite vectors.
// When two faces lie on the same plane, but in contrary directions.
void check_opposite_vectors (nodePtr normalList, facePtr faceArray, vertexPtr vertexArray, int* numNormals)
{
	int				i;
	int				counter = *numNormals;
	vectorPtr		vector = NULL;
	vectorPtr		nextVector = NULL;
	vectorPtr		edgeVector = NULL;
	vectorPtr		newNormal = NULL;
	faceStruct		face;
	vertexStruct	vertex1;
	vertexStruct	vertex2;

	if (counter >=2)
	{
		for (i=0; i<counter; i++)
		{
			vector = (vectorPtr) getNodeData (normalList, i);
			nextVector = (vectorPtr) getNodeData (normalList, (i+1)%counter);

			// There will be no two vectors facing the
			//  same direction, so the next test will
			//  only be true for opposite vectors.
			if (colinearVectors (vector, nextVector))
			{
				face = faceArray[vector->vectorId];
				vertex1 = vertexArray[face.vertex[0]];
				vertex2 = vertexArray[face.vertex[2]];
				// Create a vector along the edge joining
				//  the two faces.
				edgeVector = vectorFromVertices (&vertex1, &vertex2);
				// Get the vector normal to the edge.
				newNormal = crossProduct (vector, edgeVector);
				newNormal->type = NORMAL;
				// Add the new vector to the list.
				insertDataAfterIndex (normalList, newNormal, i);
				counter++;
				// Increase 'i' to skip over the vector just inserted.
				i++;
				free (edgeVector);
			}
		}
	}

	// Update the number of vectors.
	*numNormals = counter;
}


// Delete normals from a list, whenever they are
//  equal to the previous item.
void remove_duplicate_normals (nodePtr* normal_list)
{
	vectorPtr	vector = NULL;
	vectorPtr	previousVector = NULL;
	vectorPtr	removed_vector = NULL;
	int			i;
	int			list_length = getListLength (*normal_list);

	// Get the last face, and set its normal as
	//  the 'previous' one.
	previousVector = (vectorPtr) getNodeData (*normal_list, list_length-1);

	for (i=0; i<list_length; i++)
	{
		// Get the normal vector of the current face.
		vector = (vectorPtr) getNodeData (*normal_list, i);

		// Skip vectors that are the same.
		if ( relaxedEqualVectors (vector, previousVector) && (list_length > 1) )
		{
			removed_vector = removeNode (normal_list, i);
			free (removed_vector);
			list_length--;
			i--;
		}
		else
			previousVector = vector;
	}
}


// Locate loops in the normal star of a vertex.
// This will be the case mostly of saddle type vertices.
// Test whether the pattern of vertices repeats itself
//  when traversing the star.
// Each repeated loop is stored as an individual list.
// Returns a list containing each of these individual polygons.
nodePtr findLoopNormals (nodePtr normalList, int numNormals, int* numBasicPolygons)
{
	nodePtr		newList = NULL;
	nodePtr		independentPolygonsList = NULL;
	vectorPtr	firstVector = NULL;
	vectorPtr	vector1 = NULL;
	vectorPtr	vector2 = NULL;
	int			i, j;
	int			counter = 0;
	int			loopSize = 0;
	int*		repeatLocations = NULL;
	boolean		loopedPolygon = FALSE;

	// Skip over cases with less than 6 normals.
	// This is to ensure that there is at least
	//  two loops of 3 vectors each, as the
	//  minimal case.
	if (numNormals < 6)
	{
		// Insert the vector list as the only loop.
		independentPolygonsList = addFrontNode (independentPolygonsList, normalList);
		*numBasicPolygons = 1;

		return (independentPolygonsList);
	}

	repeatLocations = (int*) xmalloc (sizeof (int) * (numNormals / 3));
	memset (repeatLocations, 0, sizeof (int) * (numNormals / 3));

	firstVector = (vectorPtr) getNodeData (normalList, 0);

	// Compare each of the vectors with the first one.
	for (i=1; i<numNormals; i++)
	{
		vector1 = (vectorPtr) getNodeData (normalList, i);

		// Any vectors that are similar are recorded, and 
		//  the flag is set to continue with the process.
		if (equalVectors (vector1, firstVector))
		{
			loopedPolygon = TRUE;
			repeatLocations[counter++] = i;
		}
	}

	if (loopedPolygon)
	{
		loopSize = repeatLocations[0];

		// Check that there are enough elements for
		//  entire loops.
		if (numNormals%loopSize != 0)
		{
			counter = 0;
			loopedPolygon = FALSE;
		}
		else
		{
			// Check that the distances between the loops are equal.
			for (i=2; i<counter; i++)
			{
				if ((repeatLocations[i] - repeatLocations[i-1]) != loopSize)
				{
					counter = 0;
					loopedPolygon = FALSE;
				}
			}
		}
	}

	// Check that all the elements in the loop are also repeated.
	for (i=0; i<repeatLocations[0] && loopedPolygon; i++)
	{
		vector1 = (vectorPtr) getNodeData (normalList, i);

		// Check against all other remaining vectors.
		for (j=1; j<=counter; j++)
		{
			vector2 = (vectorPtr) getNodeData (normalList, i + (loopSize * j));

			// If they are different, the loop is broken.
			if (!equalVectors (vector1, vector2))
			{
				loopedPolygon = FALSE;
				break;
			}
		}
		// Exit the for loop if the normal loop is broken.
		if (loopedPolygon == FALSE)
		{
			counter = 0;
			break;
		}
	}

	// Split the list into several polygons, at the repeatLocations.
	for (i=counter; i>0 && loopedPolygon; i--)
	{
		for (j=loopSize-1; j>=0; j--)
		{
			vector1 = (vectorPtr) removeNode (&normalList, (loopSize * i) + j);
			newList = addFrontNode (newList, vector1);

		}
		// Insert the new list into the list of lists.
		independentPolygonsList = addFrontNode (independentPolygonsList, newList);
	}

	// Insert the remaining items as another polygon.
	independentPolygonsList = addFrontNode (independentPolygonsList, normalList);

	*numBasicPolygons = 1 + counter;

	free (repeatLocations);

	return (independentPolygonsList);
}


// Compare a new intersection against those already
//  stored in a list.
void matchIntersectionPairs (int vectorId_1, int vectorId_2, int vectorId_3, int vectorId_4, int new_Id)
{
	boolean		matched = FALSE;
	int			num_pairs = 0;
	int			i;
	int*		currentPair = NULL;

	// Try to match the intersection just found with
	//  one of the previous ones.
	num_pairs = getListLength (IntersectionPairs);
	matched = FALSE;
	for (i=0; i<num_pairs; i++)
	{
		currentPair = (int*) getNodeData (IntersectionPairs, i);

		// If the intersection matches, then update the 
		//  list of pairs, and delete the item with the
		//  segment delimiters.
		if (	(vectorId_1 == currentPair[4]) &&
				(vectorId_2 == currentPair[5]) &&
				(vectorId_3 == currentPair[2]) &&
				(vectorId_4 == currentPair[3])	)
		{
			currentPair[1] = new_Id;
			matched = TRUE;
			break;
		}
	}

	if (!matched)
	{
		// Add a new node to the list of intersectionPairs.
		currentPair = (int*) xmalloc ( sizeof (int) * 6 );
		currentPair[0] = new_Id;
		currentPair[1] = 0;
		currentPair[2] = vectorId_1;
		currentPair[3] = vectorId_2;
		currentPair[4] = vectorId_3;
		currentPair[5] = vectorId_4;
		IntersectionPairs = addFrontNode (IntersectionPairs, currentPair);
	}

# if (DEBUG >= 2)
	printf ("\t+++ Intersection of arcs %2d-%2d and %2d-%2d at new point %3d\n"
		, vectorId_1
		, vectorId_2
		, vectorId_3
		, vectorId_4
		, new_Id);
# endif
}


// Set the list of intersections of a single segment
//  in the correct order. Add the new vector specified as
//  'intersection' in the correct place.
void orderIntersectionsList (nodePtr* tmpList, int tmpLength, vectorPtr vector1, vectorPtr intersection)
{
	vectorPtr	previousIntersection = NULL;
	double		angle = 0.0;
	double		previousAngle = 0.0;
	int			insertionPoint;
	int			i;

	// If there are more than one intersections
	//  in this segment, then make sure they are
	//  ordered correctly.

	// Starting at index 0 the initial vector is
	//  also checked. This allows the intersections
	//  to go before the vector.
	if (tmpLength > 0)
	{
		angle = angleBetweenVectors (vector1, intersection);
		insertionPoint = 0;

		for (i=0; i<tmpLength; i++)
		{
			previousIntersection = (vectorPtr) getNodeData (*tmpList, i);
			previousAngle = angleBetweenVectors (vector1, previousIntersection);
			// 13 / 01 / 2007
			// For vertices as in: triceratops.obj 225
			// Check that the angle is not the same.
			// This occurs if the intersection and the vector
			//  are nearly at the same point.
			if (angle >= previousAngle)
				insertionPoint++;
		}

		// If the angle is the smallest, insert at the front
		//  of the temporary list.
		if (insertionPoint == 0)
			*tmpList = addFrontNode (*tmpList, intersection);
		// Otherwise, insert at the correct location.
		// The index of insertion is reduced by 1, for the function
		//  'insertDataAfterIndex' to work correctly.
		else
			insertDataAfterIndex (*tmpList, intersection, insertionPoint-1);
	}
	// If this is the first intersection, just insert it directly.
	else
	{
		*tmpList = addNode (*tmpList, intersection);
	}
}


// Remove pairs which have not been matched.
// Also remove the intersection vectors that were not matched.
void prunePairsList (nodePtr* extendedNormalList)
{
	int			i, j;
	int			numPairs = getListLength (IntersectionPairs);
	int			badVector;
	int*		currentPair = NULL;
	vectorPtr	currentVector = NULL;

	for (i=0; i<numPairs && IntersectionPairs; i++)
	{
		currentPair = (int*) getNodeData (IntersectionPairs, i);

		if (currentPair[1] == 0)
		{
			currentPair = (int*) removeNode (&IntersectionPairs, i);
			i--;
			numPairs--;
			// Get the Id of the unmatched vector.
			badVector = currentPair[0];

			for (j=0; j<getListLength(*extendedNormalList); j++)
			{
				currentVector = (vectorPtr) getNodeData (*extendedNormalList, j);
				if (currentVector->vectorId == badVector)
				{
					// Remove the vector from the extended list.
					currentVector = removeNode (extendedNormalList, j);
					free (currentVector);
					printf ("\tPRUNING: Discarding intersection vector %d, between segments %d-%d and %d-%d\n",
							badVector, currentPair[2], currentPair[3], currentPair[4], currentPair[5]);
					break;
				}
			}
			free (currentPair);
		}
	}
}


// Free the memory used for the
//  matching of intersection pairs.
void free_pairs_list (void)
{
	freeList (IntersectionPairs);
	IntersectionPairs = NULL;
}


// Finds the vectors where the arcs that define a polygon
//  have self intersections.
// Returns a list of vectors, containing both the face normals
//  of the object, and the vectors where the arcs intersect.
nodePtr findArcIntersections (nodePtr normalList, boolean* arcsIntersect)
{
	nodePtr		extendedNormalList = NULL;
	nodePtr		extendedNormalListTail = NULL;
	nodePtr		tmpList = NULL;
	vectorPtr	vector1 = NULL;
	vectorPtr	vector2 = NULL;
	vectorPtr	vector3 = NULL;
	vectorPtr	vector4 = NULL;
	vectorPtr	vector1_1 = NULL;
	vectorPtr	vector3_1 = NULL;
	vectorPtr	tmpVector = NULL;
	vectorPtr	intersection = NULL;
	boolean		intersect = FALSE;
	int			numNormals = getListLength (normalList);
	int			endpoint = 0;
	int			tmpLength = 0;
	int			counter = 1;
	int			i, j;
#if (DEBUG >= 2)
	int*		currentPair = NULL;
#endif

#if (DEBUG >= 1)
	printf ("  ->ARC INTERSECTIONS:\n"); fflush (stdout); fflush (stdout);
#endif

// printf ("LIST RECEIVED BY 'findArcIntersections':\n");
// printVectorList (normalList);

	// Clear the arrays for the intersection pairs.
	freeList (IntersectionPairs);
	freeList (IntersectionPairsCopy);
	IntersectionPairs = NULL;
	IntersectionPairsCopy = NULL;

	// With 3 normals or less, there can be no intersections.
	// All possible cases must have been eliminated in
	//  "getFaceNormalList".
	// Simply return a copy of the original list.
	if (numNormals < 4)
		return ( copyList (normalList, sizeof (vectorStruct)) );

	// For each face around the vertex,
	//  test its normal vector.
	for (i=0; i<numNormals; i++)
	{
		vector1 = (vectorPtr) getNodeData (normalList, i);
		vector2 = (vectorPtr) getNodeData (normalList, (i+1)%numNormals);

		// Reset the values for the insertion of
		//  intersection points.
		tmpList = NULL;
		tmpLength = 0;

		// Insert the vector into the new list.
		tmpVector = (vectorPtr) xmalloc (sizeof (vectorStruct));
		memcpy (tmpVector, vector1, sizeof (vectorStruct));
		tmpList = addNode (tmpList, tmpVector);
		tmpLength++;

		// Test against all other segments.
		for (j=0; j<numNormals; j++)
		{
			// Check that the segments are not contiguous.
			// There can be no intersection of contiguous segments.
			if ( (i != (j+1)%numNormals) && ((i+1)%numNormals != j) && (i != j) )
			{
				vector3 = (vectorPtr) getNodeData (normalList, j);
				vector4 = (vectorPtr) getNodeData (normalList, (j+1)%numNormals);

				intersection = arcIntersection (vector1, vector2, vector3, vector4, &intersect, &endpoint);

				// If there was any intersection.
				if (intersect)
				{
#if (DEBUG >= 3)
	printf ("\tIntersection: segments %d-%d and %d-%d\tEndpoint = %d\n", vector1->vectorId, vector2->vectorId, vector3->vectorId, vector4->vectorId, endpoint);
#endif
					// Check for the case of intersection at the
					//  endpoint of the arcs.
					switch (endpoint)
					{
						case 0:		// Normal case, not at the endpoints.
									// Will match another intersection
									//  with case 0:
							// Mark the intersection vector as such.
							intersection->vectorId = -counter;
							intersection->type = INTER;
							intersection->num_intersections = 1;

							// Prepare the list of intersections for this segment.
							// Also inserts the new vector into the list.
							orderIntersectionsList (&tmpList, tmpLength, vector1, intersection);
							tmpLength++;

							// Check if the new intersection matches a previous one.
							matchIntersectionPairs (vector1->vectorId, vector2->vectorId, vector3->vectorId, vector4->vectorId, intersection->vectorId);
							counter++;
							break;

						case 1:		// Intersection at vector1
									// Will match an intersection with case 4:
							// Mark the first vector as an intersection vector.
							// The vector is already inserted as the first
							//  element in the temporary list.
							free (intersection);
							intersection = (vectorPtr) tmpList->data;
							intersection->type = INTER;
							intersection->num_intersections += 1;

							// Get the new segment endpoints for the matching.
							if (i != 0)
								vector1_1 = (vectorPtr) getNodeData (normalList, (i-1)%numNormals);
							else
								vector1_1 = (vectorPtr) getNodeData (normalList, numNormals-1);

							// Check if the new intersection matches a previous one.
							// Using the new endpoints to match the pair.
							matchIntersectionPairs (vector1_1->vectorId, vector2->vectorId, vector3->vectorId, vector4->vectorId, intersection->vectorId);
							counter++;
							break;

						case 4:		// Intersection at vector3
									// Will match an intersection with case 1:
							// Mark the intersection vector as such.
							intersection->vectorId = -counter;
							intersection->type = INTER;
							intersection->num_intersections = 1;

							// Get the new segment endpoints for the matching.
							if (j != 0)
								vector3_1 = (vectorPtr) getNodeData (normalList, (j-1)%numNormals);
							else
								vector3_1 = (vectorPtr) getNodeData (normalList, numNormals-1);

							// Prepare the list of intersections for this segment.
							// Also inserts the new vector into the list.
							orderIntersectionsList (&tmpList, tmpLength, vector1, intersection);
							tmpLength++;

							// Check if the new intersection matches a previous one.
							// Using the new endpoints to match the pair.
							matchIntersectionPairs (vector1->vectorId, vector2->vectorId, vector3_1->vectorId, vector4->vectorId, intersection->vectorId);
							counter++;
							break;

						// Intersection at the same location as two of
						//  the endpoints.
						case 5:		// Intersection at vector1 and vector3
									// Will match another intersection
									//  with case 5:
							// Mark the first vector as an intersection vector.
							// The vector is already inserted as the first
							//  element in the temporary list.
							free (intersection);
							intersection = (vectorPtr) tmpList->data;
							intersection->type = INTER;
							intersection->num_intersections += 1;

							// Get the new segment endpoints for testing and matching.
							if (i != 0)
								vector1_1 = (vectorPtr) getNodeData (normalList, (i-1)%numNormals);
							else
								vector1_1 = (vectorPtr) getNodeData (normalList, numNormals-1);

							if (j != 0)
								vector3_1 = (vectorPtr) getNodeData (normalList, (j-1)%numNormals);
							else
								vector3_1 = (vectorPtr) getNodeData (normalList, numNormals-1);

							// Test that there really is an intersection of the new
							//  segments in between the new endpoints,
							//  by checking the 4 segments involved.
							if ( !double_check_intersection (vector1_1, vector1, vector2, vector3_1, vector3, vector4) )
							{
								// Return the vector to it's previous state.
								intersection->type = NORMAL;
								intersection->num_intersections -= 1;
// printf ("DOUBLE CHECK FAILED: Vector %d not intersection\n", intersection->vectorId);
								break;
							}

							// Check if the new intersection matches a previous one.
							// Using the new endpoints to match the pair.
							matchIntersectionPairs (vector1_1->vectorId, vector2->vectorId, vector3_1->vectorId, vector4->vectorId, intersection->vectorId);
// printf ("MATCHING: %d, SEGMENTS %d-%d, %d-%d\n", intersection->vectorId, vector1_1->vectorId, vector2->vectorId, vector3_1->vectorId, vector4->vectorId);
							counter++;
							break;

						default:
							// Just free the memory allocated
							//  to the intersection vector.
							if (intersection != NULL)
								free (intersection);
							break;

					}	// switch (endpoint)
				}	// if (intersect)
			}	// if ( (i != (j+1)%numNormals) && ((i+1)%numNormals != j) )
		}	// for (j=0; j<numNormals; j++)

		// Insert the temporary list into the final list of normals.
		joinLists (&extendedNormalList, tmpList);
		// Update the tail of the list.
		extendedNormalListTail = getListTail (extendedNormalList);
	}	// for (i=0; i<numNormals; i++)

	prunePairsList (&extendedNormalList);

#if (DEBUG >= 2)
	printf ("\n\tNEW list of normals has %d elements\n", getListLength (extendedNormalList));
	for (j=0; j<getListLength(extendedNormalList); j++)
	{
		vector1 = (vectorPtr) getNodeData (extendedNormalList, j);
		printf ("\t%3d = %.4f\t%.4f\t%.4f\n", vector1->vectorId, vector1->i, vector1->j, vector1->k);
	}
	printf ("\n\tINTERSECTION PAIRS:\n");
	for (j=0; j<getListLength (IntersectionPairs); j++)
	{
		currentPair = getNodeData (IntersectionPairs, j);
		printf ("\tPair: %3d, %3d\n", currentPair[0], currentPair[1]);
	}

	printf ("\t%d INTERSECTIONS FOUND\n", getListLength (IntersectionPairs));
#endif

#if (DEBUG >= 1)
	printf ("  ->ARC INTERSECTIONS - DONE!\n\n"); fflush (stdout);
#endif

	return (extendedNormalList);
}


// Do an extra check, to see if the neighbours are really
//  separated by an intersection.
// In the case of intersections at the same location as
//  two face normals.
boolean double_check_intersection (vectorPtr vector1_1, vectorPtr vector1, vectorPtr vector2, vectorPtr vector3_1, vectorPtr vector3, vectorPtr vector4) 
{
	boolean		double_check_1 = FALSE;
	boolean		double_check_2 = FALSE;
	boolean		double_check_3 = FALSE;
	boolean		double_check_4 = FALSE;
	int			endpoint_1;
	int			endpoint_2;
	int			endpoint_3;
	int			endpoint_4;

// printf ("DOUBLE CHECKS: %d-%d-%d, AND %d-%d-%d\n", vector1_1->vectorId, vector1->vectorId, vector2->vectorId, vector3_1->vectorId, vector3->vectorId, vector4->vectorId);

	arcIntersection (vector1_1, vector2, vector3_1, vector3, &double_check_1, &endpoint_1);
// printf ("Check = %d\tEndpoint_1 = %d\n", double_check_1, endpoint_1);
	arcIntersection (vector1_1, vector2, vector3,   vector4, &double_check_2, &endpoint_2);
// printf ("Check = %d\tEndpoint_2 = %d\n", double_check_2, endpoint_2);
	arcIntersection (vector1_1, vector1, vector3_1, vector4, &double_check_3, &endpoint_3);
// printf ("Check = %d\tEndpoint_3 = %d\n", double_check_3, endpoint_3);
	arcIntersection (vector1,   vector2, vector3_1, vector4, &double_check_4, &endpoint_4);
// printf ("Check = %d\tEndpoint_4 = %d\n", double_check_4, endpoint_4);

	// Don't count the intersection if the arcs only touch
	//  at a single vertex but don't cross each other.
	if (	( double_check_1 && endpoint_1 != 6 && endpoint_1 != 9 ) ||
			( double_check_2 && endpoint_2 != 6 && endpoint_2 != 9 ) ||
			( double_check_3 && endpoint_3 != 6 && endpoint_3 != 9 ) ||
			( double_check_4 && endpoint_4 != 6 && endpoint_4 != 9 ) 	)
	// if (!double_check_1 && !double_check_2 && !double_check_3 && !double_check_4)
	{
		return (TRUE);
	}

	return (FALSE);
}


// Cycle through the list of normals and call
//  'splitSphericalPolygon' for each possibility
// Return the list of polygons that contains the
//  polygon with the longest perimeter.
nodePtr	testPolygonSplit (nodePtr polygonList, nodePtr normalList)
{
	vectorPtr	vector = NULL;
	nodePtr		results_list = NULL;
	nodePtr		test_polygon_list = NULL;
	nodePtr		normalListCopy = copyList (normalList, sizeof (vectorStruct) );
	int			list_length = getListLength (normalList);
	int			num_polygons;
	int			i, j;
	int			largest_index = 0;
	int			concave_angle_count = 0;
	int			concave_angle_total = 0;
	int			most_concaves = 0;
	// double		perimeter = 0.0;
	// double		largest_perimeter = 0.0;
	angleType	polygon_orientation;

	most_concaves = list_length;

	for (i=0; i<list_length; i++)
	{
		// Get the first element in the list.
		vector = getNodeData (normalListCopy, 0);
		// If it is a normal vector.
		if (vector->vectorId > 0)
		{
			results_list = splitSphericalPolygon (NULL, normalListCopy);
			num_polygons = getListLength (results_list);

			concave_angle_total = 0;

			for (j=0; j<num_polygons; j++)
			{
				test_polygon_list = (nodePtr) getNodeData (results_list, j);

				// Classify by direction of angles.
				polygon_orientation = find_spherical_polygon_orientation (test_polygon_list, &concave_angle_count);
// printf ("Returned %d concaves (%d orientation)\n", concave_angle_count, polygon_orientation);
				concave_angle_total += concave_angle_count;

				// Classify by length of perimeter.
				// perimeter = measure_spherical_polygon_perimeter (test_polygon_list);
				// if ( (perimeter > largest_perimeter) || (largest_perimeter == 0.0) )
				// {
					// largest_perimeter = perimeter;
					// largest_index = i;
				// }
			}

			// Free memory used for this temporary result.
			freeListOfLists (results_list);

// printf ("VECTOR %d, INDEX %d: GOT %d POLYGONS. %d CONCAVE ANGLES\n\n", vector->vectorId, i, num_polygons, concave_angle_total);

			// Mark the set of polygons with the
			//  least amount of direction changes.
			if (concave_angle_total <= most_concaves)
			{
				most_concaves = concave_angle_total;
				largest_index = i;
			}
		}

		rotateListNode (&normalListCopy);
	}

printf ("=== GOING BACK TO INDEX %d\n", largest_index);

	// Get back to the list that produced the
	//  longest polygons.
	for (i=0; i<largest_index; i++)
	{
		rotateListNode (&normalListCopy);
	}

	// Call the function to split the polygons
	//  one last time.
	polygonList = splitSphericalPolygon (NULL, normalListCopy);

	return (polygonList);
}


// Measure the length of the arcs that delimit a
//  spherical polygon.
double measure_spherical_polygon_perimeter (nodePtr polygon_list)
{
	int				i;
	int				list_length = getListLength (polygon_list);
	double			perimeter = 0.0;
	vectorPtr		vector1 = NULL;
	vectorPtr		vector2 = NULL;

	for (i=0; i<list_length; i++)
	{
		vector1 = (vectorPtr) getNodeData (polygon_list, i);
		vector2 = (vectorPtr) getNodeData (polygon_list, (i+1)%list_length);

		perimeter += angleBetweenVectors (vector1, vector2);
	}

	return (perimeter);
}


// Determine the possible orientation of a spherical
//  polygon, based on the number of angles in each
//  orientation, in the curve that delimits its perimeter.
angleType find_spherical_polygon_orientation (nodePtr polygon_list, int* concave_angle_total)
{
	int			i;
	int			list_length = getListLength (polygon_list);
	int			convex_angle_count = 0;
	int			concave_angle_count = 0;
	int			orientation_changes = 0;
	int			convex_change = 0;
	int			concave_change = 0;
	vectorPtr	vector1 = NULL;
	vectorPtr	vector2 = NULL;
	vectorPtr	vector3 = NULL;
	spinType	angle_orientation;
	spinType	previous_orientation = COL;

	for (i=0; i<list_length; i++)
	{
		vector1 = (vectorPtr) getNodeData (polygon_list, i);
		vector2 = (vectorPtr) getNodeData (polygon_list, (i+1)%list_length);
		vector3 = (vectorPtr) getNodeData (polygon_list, (i+2)%list_length);

		angle_orientation = sphereAngleOrientation (vector1, vector2, vector3, 2);
		
		if (angle_orientation == COL)
			continue;

		if ( (angle_orientation != previous_orientation) || (previous_orientation == COL) )
		{
			orientation_changes++;

			previous_orientation = angle_orientation;

			// Count the number of direction changes
			//  in the respective direction
			if (angle_orientation == CCW)
				convex_change++;
			else // if (angle_orientation == CW)
				concave_change++;
		}

		// Count the number of actual angles in
		//  each direction.
		if (angle_orientation == CCW)
			convex_angle_count++;
		else // if (angle_orientation == CW)
			concave_angle_count++;
	}

	printf ("CONVEX: %d, CONCAVE %d\n", convex_angle_count, concave_angle_count);

	if (convex_change > concave_change)
	{
		*concave_angle_total = concave_change;
		return (CONVEX);
	}
	else if (convex_change < concave_change)
	{
		*concave_angle_total = convex_change;
		return (CONCAVE);
	}
	else // if (convex_change == concave_change)
	{
		printf ("\n\tWARNING! UNCERTAIN ORIENTATION OF SPHERICAL POLYGON\n\n");
		// In case of a draw, count the real
		//  number of angles.
		if (convex_angle_count > concave_angle_count)
		{
			*concave_angle_total = convex_change;
			return (CONVEX);
		}
		else if (concave_angle_count > convex_angle_count)
		{
			*concave_angle_total = concave_change;
			return (CONCAVE);
		}
		return (COLLINEAR);
	}
}


// Separate a complex spherical polygon into several
//  simple ones, removing any self intersections.
// Return a list of lists of normals. Each list will
//  represent a simple polygon.
nodePtr splitSphericalPolygon (nodePtr polygonList, nodePtr normalList)
{
	nodePtr		tmpList = NULL;
	nodePtr		normalListCopy = copyList (normalList, sizeof (vectorStruct) );
	vectorPtr	vector = NULL;
	vectorPtr	tmpVector = NULL;
	vectorPtr	tmpVector2 = NULL;
	boolean		match = FALSE;
	boolean		skip = FALSE;
	int*		currentPair = NULL;
	int*		intersectionIndex = NULL;
	int*		visitedVertices = NULL;
	int			listLength = getListLength (normalList); 
	int			pairsLength = getListLength (IntersectionPairs); 
	int			remainingPairs = pairsLength;
	int			active_intersections = 0;
	int			match_end;
	int			match_begin;
	int			match_vector_id;
	int			matchIndex;
	int			numElements;
	int			nextElement;
	int			i, j, k, l;

// printf ("LIST RECEIVED BY 'splitSphericalPolygon':\n");
// printVectorList (normalList);

	// Make a copy of the list of pairs, to be used
	//  when relabelling the vectors.
	IntersectionPairsCopy = copyList (IntersectionPairs, sizeof (int) * 6);

#if (DEBUG >= 1)
	printf ("  ->POLYGON SPLITTING:\n"); fflush (stdout);
#endif

	// Allocate memory for an array that will store
	//  the indexes in the list of vectors where
	//  the intersection vectors are found.
	intersectionIndex = (int*) xmalloc ( sizeof (int) * pairsLength);
	memset (intersectionIndex, 0, sizeof (int) * pairsLength);
	// Memory for an array of the vertices visited,
	//  between each pair.
	visitedVertices = (int*) xmalloc ( sizeof (int) * pairsLength);
	memset (visitedVertices, 0, sizeof (int) * pairsLength);

	i = 0;

	while (IntersectionPairsCopy != NULL)
	{
		// Allow 'i' to loop around the list.
		i = i % listLength;

		vector = (vectorPtr) getNodeData (normalListCopy, i);

		// Find an unvisited intersection vector.
		if (vector->num_intersections > 0)
		// if (vector->type == INTER)
		{
			match = FALSE;
			skip = FALSE;

			// Compare against the previously visited intersections.
			for (j=0; j<active_intersections; j++)
			{
				tmpVector = (vectorPtr) getNodeData (normalListCopy, intersectionIndex[j]);

				// If the loop has closed, by visiting an
				//  intersection vector for the 2nd time.
				for (k=0; k<remainingPairs; k++)
				{
					currentPair = (int*) getNodeData (IntersectionPairsCopy, k);
					if (	( (vector->vectorId == currentPair[0]) && (tmpVector->vectorId == currentPair[1]) ) ||
							( (vector->vectorId == currentPair[1]) && (tmpVector->vectorId == currentPair[0]) )		)
					{
						// Mark the matching intersection.
						// Only if there have been any normal 
						//  vectors since the last intersection.
						if (visitedVertices[j] > 0)
						{
							match = TRUE;
							match_end = i;
							match_begin = intersectionIndex[j];
							matchIndex = j;
							match_vector_id = tmpVector->vectorId;
							// Remove the pair from the list.
							removeNode (&IntersectionPairsCopy, k);
							free (currentPair);
							remainingPairs--;
						}
						// Do not process this polygon, but note
						//  that the matching intersection has been found.
						else
						{
							skip = TRUE;
							intersectionIndex[j] = i;
						}

						break;
					}
				}

				if (match || skip)
					break;
			}	// for (j=0; j<active_intersections; j++)

			if (match)
			{
// printf ("New Intersection at vector %d\n", vector->vectorId);
				// Mark the vector as visited.
				vector->type = SPLIT;
				vector->num_intersections -= 1;

				// Mark as visited the second occurrence of
				//  all the intersection vectors
				//  that are included in the polygon found.

				// For each intersection vertex found.
				for (j=matchIndex; j<active_intersections; j++)
				{
					tmpVector = (vectorPtr) getNodeData (normalListCopy, intersectionIndex[j]);
					// For each of the remaining intersection
					//  vectors in the input list.
					for (k=match_end; k<listLength && (tmpVector->num_intersections > 0); k++)
					// for (k=match_end; k<listLength && (tmpVector->type == INTER); k++)
					{
						tmpVector2 = (vectorPtr) getNodeData (normalListCopy, k);

						// For each of the pairs of intersections.
						for (l=0; l<remainingPairs && (tmpVector2->num_intersections > 0); l++)
						// for (l=0; l<remainingPairs && (tmpVector2->type == INTER); l++)

						// Removed the check for equal vectors, to allow the removal of
						//  pairs for multiple intersection normals.
						//  (case 5:) in findArcIntersections.
						// for (l=0; l<remainingPairs && (tmpVector2->type == INTER) && ( !equalVectors (tmpVector2, vector) ); l++)
						{
							currentPair = (int*) getNodeData (IntersectionPairsCopy, l);

							if (	( (tmpVector->vectorId == currentPair[0]) && (tmpVector2->vectorId == currentPair[1]) ) ||
									( (tmpVector->vectorId == currentPair[1]) && (tmpVector2->vectorId == currentPair[0]) )		)
							{
								tmpVector2->num_intersections -= 1;
								if (tmpVector2->num_intersections == 0)
									tmpVector2->type = VISITED;
								// Remove the pair from the list.
// printf ("Removing intersection pair %d %d\n", currentPair[0], currentPair[1]);
								removeNode (&IntersectionPairsCopy, l);
								free (currentPair);
								remainingPairs--;
								break;
							}
						}
					}
				}

				// Remove the items from the copy of the
				//  normals list, and insert them into the
				//  temporary list.
				if (match_end > match_begin)
					numElements = match_end - 1 - match_begin;
				else
					numElements = match_end - 1 - (match_begin - listLength);
				k = match_end - 1;
				for (j=0; j<numElements; j++)
				{
					nextElement = (listLength + k) % listLength;

					// Skip vectors which are the same as the intersection vectors.
					tmpVector = (vectorPtr) getNodeData (normalListCopy, nextElement);
					if ( !equalVectors (tmpVector, vector) )
					{
						tmpVector = (vectorPtr) removeNode (&normalListCopy, nextElement);
						tmpList = addFrontNode (tmpList, tmpVector);
						// Set the vector as no longer having intersections
						tmpVector->num_intersections = 0;
						listLength--;
					}

					if (k > -1)
						k--;
				}
				// Insert the matching vector, the one at
				//  the beginning of the loop just found.
				//  k = match_begin
				nextElement = (listLength + k) % listLength;
				tmpVector = (vectorPtr) removeNode (&normalListCopy, nextElement);
				// Mark the vector as the match pair.
				tmpVector->type = SPLIT;
				tmpList = addFrontNode (tmpList, tmpVector);
				listLength--;

				// Clear the array of indexes to the intersection
				//  vectors in the list.
				memset (intersectionIndex, 0, sizeof (int) * pairsLength);
				memset (visitedVertices, 0, sizeof (int) * pairsLength);
				active_intersections = 0;

				// Reset the counters for the loop to start again.
				// i = -1;	// 'i' will be incremented at the end of the loop,
						//  making it 0 before the next loop.
				// Make 'i' go back to the position of the last intersection.
				i = i - getListLength (tmpList) - 1;

#if (DEBUG >= 2)
	printf ("\t>> ADDING A POLYGON OF %d VERTICES\n", getListLength (tmpList));
#endif
#if (DEBUG >= 3)
	printVectorList (tmpList);
#endif
				// Insert the list of the polygon found into
				//  the list of lists.
				polygonList = addFrontNode (polygonList, tmpList);
				// Clear the temporary list.
				tmpList = NULL;
			}	// if (match)
			// If the loop is not closed yet.
			// And the intersection was not skipped.
			else if (!skip)
			{
				// Register the location of an intersection
				//  vector in the list of normals.
				if (active_intersections < pairsLength)
					intersectionIndex[active_intersections++] = i;
			}
		}	// if (vector->type == INTER )
		else if ( vector->type == NORMAL )
		{
			for (j=0; j<active_intersections; j++)
			{
				visitedVertices[j] += 1;
			}
		}

		// Check that all the vectors mentioned in
		//  the pairs are still in 'normalListCopy'.
		// Otherwise, eliminate the pair.
		discard_redundant_pairs (&remainingPairs, normalListCopy, listLength);

		i++;
	}	// while (IntersectionPairsCopy != NULL)

#if (DEBUG >= 2)
	printf ("\t** ADDING FINAL POLYGON OF %d VERTICES\n", getListLength (normalListCopy));
#endif
#if (DEBUG >= 3)
	printVectorList (normalListCopy);
#endif
	// Insert the remaining vectors into the list of lists.
	// These will form another polygon.
	polygonList = addFrontNode (polygonList, normalListCopy);

#if (DEBUG >= 2)
	printf ("\t%d POLYGONS FOUND\n", getListLength (polygonList) );
#endif

	// Deallocate memory.
	free (intersectionIndex);
	free (visitedVertices);

#if (DEBUG >= 1)
	printf ("  ->POLYGON SPLITTING - DONE!\n\n"); fflush (stdout);
#endif

	return (polygonList);
}


// Delete from the list of pairs those for which the
//  vectors involved are no longer present in the list.
void discard_redundant_pairs (int* remainingPairs, nodePtr normalList, int listLength)
{
	boolean		still_inside_0 = FALSE;
	boolean		still_inside_1 = FALSE;
	vectorPtr	vector = NULL;
	int*		currentPair = NULL;
	int			i, j;

	for (i=0; i<*remainingPairs; i++)
	{
		still_inside_0 = FALSE;
		still_inside_1 = FALSE;

		currentPair = (int*) getNodeData (IntersectionPairsCopy, i);

		for (j=0; j<listLength; j++)
		{
			vector = (vectorPtr) getNodeData (normalList, j);

			if (vector->vectorId == currentPair[0])
				still_inside_0 = TRUE;
			if (vector->vectorId == currentPair[1])
				still_inside_1 = TRUE;
		}

		// If one or both of the vectors are missing.
		if (!still_inside_0 || !still_inside_1)
		{
#if (DEBUG >= 2)
printf ("\tDiscarding pair of intersections %d and %d\n", currentPair[0], currentPair[1]);
#endif
			// Remove the pair from the list.
			removeNode (&IntersectionPairsCopy, i);
			free (currentPair);
			*remainingPairs -= 1;
		}
	}
}


// Compute the area of a polygon that lies on the surface of
//  a sphere.
// The polygon is defined by the normals of a 3D object.
// The normals are unit vectors, all beginning at the origin.
double findSphericalPolygonArea (nodePtr normalList, spinType* orientation, boolean c_h_vertex, vertexPtr coneVertexArray, int coneVertexArrayLength)
{
	vectorPtr	vector1 = NULL;
	vectorPtr	vector2 = NULL;
	vectorPtr	vector3 = NULL;
	double		area;
	double		arcLength = 0.0;
	double		sumAngles = 0.0;
	// double		radius = 1.0;	// Using a unit sphere.
	int			numNormals = getListLength (normalList);
	int			i;
	int			concave_count = 0;
	boolean		invertArea = FALSE;
	angleType	angle_orientation;

#if (DEBUG >= 2)
	printf ("\tPolygon with %d vertices:\n", numNormals);
#endif

/*
////////	NEW TEST FOR ORIENTATION	////////
////////	BASED ON CHANGES OF ORIENTATION		////////
*orientation = find_spherical_polygon_orientation (normalList, &concave_count);

sumAngles = 0.0;

// Recompute possible area.
for (i=0; i<numNormals; i++)
{
	vector1 = (vectorPtr) getNodeData (normalList, i);
	vector2 = (vectorPtr) getNodeData (normalList, (i+1)%numNormals);
	vector3 = (vectorPtr) getNodeData (normalList, (i+2)%numNormals);

	sumAngles += findSphericalAngle (vector1, vector2, vector3, *orientation, &angle_orientation);
}

area = ( sumAngles - (numNormals-2) * PI ) ;// * radius * radius;		// Skip multiplications since radius = 1.0
*/

	// Consider the case of Flat vertices. These do not
	//  really have an orientation.
	if (numNormals == 1)
	{
		*orientation = COL;
		return (0.0);
	}

	// Find the area of the polygon, considering
	//  a positive orientation.
	*orientation = CCW;
	for (i=0; i<numNormals; i++)
	{
		vector1 = (vectorPtr) getNodeData (normalList, i);
		vector2 = (vectorPtr) getNodeData (normalList, (i+1)%numNormals);
		vector3 = (vectorPtr) getNodeData (normalList, (i+2)%numNormals);

		arcLength += angleBetweenVectors (vector1, vector2);

		sumAngles += findSphericalAngle (vector1, vector2, vector3, *orientation, &angle_orientation);

		if (angle_orientation == CONCAVE)
			concave_count++;
	}
	area = ( sumAngles - (numNormals-2) * PI ) ;// * radius * radius;		// Skip multiplications since radius = 1.0

	// Get rid of the cases where several vectors are in the same position.
	// This is the case when several arcs intersect at the same place.
	if ( arcLength < (0 + EPSILON) )
	{
		*orientation = COL;
		return (0.0);
	}

	// If the area is greater than 2 PI,
	//  and the arc length is less than 2 PI,
	//  then the orientation is negative.
	// The area must be recomputed.
	// if ( ( area > (2 * PI) ) && ( !c_h_vertex || arcLength < (2 * PI) ) )
	// if ( ( area > (2 * PI) ) && ( arcLength < (2 * PI) ) )
	if ( area > (2 * PI) )
	{
		if (arcLength < (2 * PI))
			invertArea = TRUE;
		// else if ( concave_count != 2 )
			// invertArea = TRUE;
		else if (c_h_vertex && concave_count != 2)
			invertArea = TRUE;
		else if (!c_h_vertex && concave_count != 3)
			invertArea = TRUE;
	}

// printf ("\nAREA: %lf\tARC_LENGTH: %lf\n", area, arcLength);
// printf ("NUMBER OF CONCAVES: %d\tINVERT = %d\n", concave_count, invertArea);

	// Recompute the area, and switch to negative orientation.
	if (invertArea)
	{
		sumAngles = 0.0;
		*orientation = CW;

		// Recompute possible area.
		for (i=0; i<numNormals; i++)
		{
			vector1 = (vectorPtr) getNodeData (normalList, i);
			vector2 = (vectorPtr) getNodeData (normalList, (i+1)%numNormals);
			vector3 = (vectorPtr) getNodeData (normalList, (i+2)%numNormals);

			sumAngles += findSphericalAngle (vector1, vector2, vector3, *orientation, &angle_orientation);
		}
		
		area = ( sumAngles - (numNormals-2) * PI ) ;// * radius * radius;		// Skip multiplications since radius = 1.0
	}

#if (DEBUG >= 2)
	printf ("\tSum of angles: %f (%.2f PI) // Total Area: ", sumAngles, sumAngles/PI);

	if (*orientation == CCW)
		printf ("+ ");
	if (*orientation == CW)
		printf ("- ");

	printf ("%.4f\n", area);
#endif

	return (area);
}


// Find out the orientation of an angle formed by three points on
//  the surface of a sphere.
// Returns COL, CW or CCW.
spinType sphereAngleOrientation (vectorPtr vector1, vectorPtr vector2, vectorPtr vector3, int type)
{
	vectorPtr	tangent1 = NULL;
	vectorPtr	tangent2 = NULL;
	vectorPtr	orientationVector = NULL;
	double		product = 0;
	spinType	spinOrientation;

	/*
	// Make a first check that the vectors are not equal.
	if (	equalVectors (vector1, vector2) ||
			equalVectors (vector1, vector3) ||
			equalVectors (vector2, vector3)		)
		return (COL);
		*/

	// Get the tangents to the curve at the point of the angle.
	tangent1 = findSphereTangent (vector2, vector1);
	tangent2 = findSphereTangent (vector2, vector3);

	if (type == 2)
	{
		// Switch the direction of the first tangent, to preserve the
		//  direction of the curve.
		tangent1->i *= -1;
		tangent1->j *= -1;
		tangent1->k *= -1;
	}

	if ( equalVectors (tangent1, tangent2) )
		spinOrientation = COL;
	else
	{
		// Get their cross product and normalise it.
		orientationVector = crossProduct (tangent1, tangent2);

		product = dotProduct (vector2, orientationVector);

		// If the dot product is less than zero,
		//  the orientation of the angle is Clockwise.
		if (product < 0.0)
			spinOrientation = CW;
		else
			spinOrientation = CCW;
	}

	// Deallocate the memory used.
	free (tangent1);
	free (tangent2);
	free (orientationVector);

	return (spinOrientation);
}


// Compute the angle between two great circles in a sphere.
// The 2 arcs are defined by 3 vectors, and the angle is computed
//  at the middle vector. (vector2)
double findSphericalAngle (vectorPtr vector1, vectorPtr vector2, vectorPtr vector3, spinType polygonOrientation, angleType* angleOrientation)
{
	vectorPtr	tangent1 = NULL;
	vectorPtr	tangent2 = NULL;
	double		angle;
	double		finalAngle;
	spinType	orientation;

	// The angle between the normals of the planes is the same
	//  as the angle between the tangent vectors to those planes.
	tangent1 = crossProduct (vector1, vector2);
	tangent2 = crossProduct (vector2, vector3);
	// tangent1 = findSphereTangent (vector2, vector1);
	// tangent2 = findSphereTangent (vector2, vector3);

	angle = angleBetweenVectors (tangent1, tangent2);

	// Find the orientation of the tangent vectors.
	orientation = sphereAngleOrientation (vector1, vector2, vector3, 2);

	// Change the orientation, according to the direction of the
	//  polygon.
	if ( polygonOrientation == CW )
	{
		if ( orientation == CCW )		// Concave angle
		{
			finalAngle = PI + angle;
			*angleOrientation = CONCAVE;
		}
		else if ( orientation == CW )	// Convex angle
		{
			finalAngle = PI - angle;
			*angleOrientation = CONVEX;
		}
		else
		{
			finalAngle = PI;
			*angleOrientation = COLLINEAR;
		}
	}
	else	// if ( polygonOrientation == CCW )
	{
		if ( orientation == CW )		// Concave angle
		{
			finalAngle = PI + angle;
			*angleOrientation = CONCAVE;
		}
		else if ( orientation == CCW)	// Convex angle
		{
			finalAngle = PI - angle;
			*angleOrientation = CONVEX;
		}
		else
		{
			finalAngle = PI;
			*angleOrientation = COLLINEAR;
		}
	}

	// printf ("VECTOR %3d: ANGLE = %lf\tFINAL = %lf\tORIENT = %d\n", vector2->vectorId, angle, finalAngle, *angleOrientation);

	free (tangent1);
	free (tangent2);

	return (finalAngle);
}


// Find a vector tangent to a sphere, that lies on the same plane
//  as two radius vectors of the unit sphere.
vectorPtr findSphereTangent (vectorPtr mainVector, vectorPtr referenceVector)
{
	vectorPtr	normalVector = NULL;
	vectorPtr	tangentVector = NULL;

	normalVector = crossProduct (mainVector, referenceVector);
	tangentVector = crossProduct (normalVector, mainVector);

	free (normalVector);

	return (tangentVector);
}


// Determine the vector where two great circles of a sphere intersect.
// The great circles are defined by arcs, which in turn are given by
//  by two vectors each.
// Returns the vector where the two arcs intersect, or NULL if the
//  arcs do not intersect.
vectorPtr arcIntersection (vectorPtr arc1vector1, vectorPtr arc1vector2, vectorPtr arc2vector1, vectorPtr arc2vector2, boolean* intersect, int* endpoint)
{
	vectorPtr		arc1_normal = NULL;
	vectorPtr		arc2_normal = NULL;
	vectorPtr		cross1_i = NULL;
	vectorPtr		crossi_2 = NULL;
	vectorPtr		cross3_i = NULL;
	vectorPtr		crossi_4 = NULL;
	vectorPtr		intersectionArc1 = NULL;
	vectorPtr		intersectionArc2 = NULL;
	double			dotArc1;
	double			dotArc2;

	*intersect = FALSE;
	*endpoint = 0;

	// Get the vectors perpendicular to the arcs.
	arc1_normal = crossProduct (arc1vector1, arc1vector2);
	arc2_normal = crossProduct (arc2vector1, arc2vector2);

	// The intersection points are given by the cross product of the
	//  "normals" to the arcs.
	intersectionArc1 = crossProduct (arc1_normal, arc2_normal);
	intersectionArc2 = crossProduct (arc1_normal, arc2_normal);

	// If the vector for the intersection is equal to zero,
	//  don't do anything else.
	// Happens if the two arcs lie on the same plane.
	if ( (intersectionArc1->i == 0) && (intersectionArc1->j == 0) && (intersectionArc1->k == 0) )
	{
		free (arc1_normal);
		free (arc2_normal);
		free (intersectionArc1);
		free (intersectionArc2);
		return (NULL);
	}

	// Trial solution for PROGRAM_ERROR: 016 (01 / 08 / 2006)
	// Two arcs that are coplanar should not have
	//  an intersection reported. This should only happen
	//  when there are two or more independent areas
	//  in the gauss map.
	if ( colinearVectors (arc1_normal, arc2_normal) )
	{
		free (arc1_normal);
		free (arc2_normal);
		free (intersectionArc1);
		free (intersectionArc2);
		return (NULL);
	}


	// Get the cross products of the intersections and the
	//  endpoints of the arcs.
	cross1_i = crossProduct (arc1vector1, intersectionArc1);
	crossi_2 = crossProduct (intersectionArc1, arc1vector2);
	cross3_i = crossProduct (arc2vector1, intersectionArc2);
	crossi_4 = crossProduct (intersectionArc2, arc2vector2);

	// Check for special cases when the intersection may
	//  occur at the endpoint of one of the segments.
	if ( colinearVectors (arc1vector1, intersectionArc1) )
	{
		free (cross1_i);
		cross1_i = (vectorPtr) xmalloc (sizeof (vectorStruct));
		memcpy (cross1_i, crossi_2, sizeof (vectorStruct));
		*endpoint |= 1;		// Intersection at vector1
	}
	if ( colinearVectors (arc1vector2, intersectionArc1) )
	{
		free (crossi_2);
		crossi_2 = (vectorPtr) xmalloc (sizeof (vectorStruct));
		memcpy (crossi_2, cross1_i, sizeof (vectorStruct));
		*endpoint |= 2;		// Intersection at vector2
	}
	if ( colinearVectors (arc2vector1, intersectionArc2) )
	{
		free (cross3_i);
		cross3_i = (vectorPtr) xmalloc (sizeof (vectorStruct));
		memcpy (cross3_i, crossi_4, sizeof (vectorStruct));
		*endpoint |= 4;		// Intersection at vector3
	}
	if ( colinearVectors (arc2vector2, intersectionArc1) )
	{
		free (crossi_4);
		crossi_4 = (vectorPtr) xmalloc (sizeof (vectorStruct));
		memcpy (crossi_4, cross3_i, sizeof (vectorStruct));
		*endpoint |= 8;		// Intersection at vector4
	}

	// The intersection lies within the arc, only if the cross
	//  products are equal.
	if ( equalVectors(cross1_i, crossi_2) )
	{
		// If the intersection vector is opposite of the arc.
		dotArc1  = dotProduct (intersectionArc1, arc1vector1);
		dotArc1 += dotProduct (intersectionArc1, arc1vector2);

		if (dotArc1 < 0)
		{
			// Invert the direction of the vector.
			intersectionArc1->i *= -1;
			intersectionArc1->j *= -1;
			intersectionArc1->k *= -1;
		}
	}
	else
	{
		free (intersectionArc1);
		intersectionArc1 = NULL;
	}

	// The intersection lies within the arc, only if the cross
	//  products are equal.
	if ( equalVectors(cross3_i, crossi_4) )
	{
		// If the intersection vector is opposite of the arc.
		dotArc2  = dotProduct (intersectionArc2, arc2vector1);
		dotArc2 += dotProduct (intersectionArc2, arc2vector2);

		if (dotArc2 < 0)
		{
			// Invert the direction of the vector.
			intersectionArc2->i *= -1;
			intersectionArc2->j *= -1;
			intersectionArc2->k *= -1;
		}
	}
	else
	{
		free (intersectionArc2);
		intersectionArc2 = NULL;
	}

	// Free the memory allocated for the temporary vectors.
	free (arc1_normal);
	free (arc2_normal);
	free (cross1_i);
	free (crossi_2);
	free (cross3_i);
	free (crossi_4);

	// Check that the two intersection vectors exist and are equal.
	if ( intersectionArc1 && intersectionArc2 && equalVectors(intersectionArc1, intersectionArc2) )
	{
		*intersect = TRUE;
		free (intersectionArc2);
		return (intersectionArc1);
	}
	else
	{
		free (intersectionArc1);
		free (intersectionArc2);
		return (NULL);
	}
}


// Remove the vertices from a polygon which are
//  collinear with the adjacent vertices.
// Returns a copy of the parameter list, minus
//  the useless vertices.
nodePtr remove_collinear_vertices (nodePtr polygonList)
{
	nodePtr		newList = NULL;
	nodePtr		newListTail = NULL;
	vectorPtr	vertex1 = NULL;
	vectorPtr	vertex2 = NULL;
	vectorPtr	vertex3 = NULL;
	vectorPtr	vertex4 = NULL;
	vectorPtr	leanVertex = NULL;
	spinType	orientation;
	int			listLength = getListLength (polygonList);
	int			i;

	// Insert the first element of a list.
	vertex2 = (vectorPtr) getNodeData (polygonList, 0);
	leanVertex = (vectorPtr) xmalloc (sizeof (vectorStruct));
	memcpy (leanVertex, vertex2, sizeof (vectorStruct));
	newListTail = addNode (newListTail, leanVertex);
	newList = newListTail;

	for (i=1; i<listLength; i++)
	{
		vertex1 = (vectorPtr) getNodeData (polygonList, (listLength-1+i)%listLength);
		vertex2 = (vectorPtr) getNodeData (polygonList, i);
		vertex3 = (vectorPtr) getNodeData (polygonList, (i+1)%listLength);
		vertex4 = (vectorPtr) getNodeData (polygonList, (i+2)%listLength);

		orientation = sphereAngleOrientation (vertex1, vertex2, vertex3, 2);

		if ( (orientation != COL) || (!colinearVectors (vertex2, vertex4)) )
		{
			leanVertex = (vectorPtr) xmalloc (sizeof (vectorStruct));
			memcpy (leanVertex, vertex2, sizeof (vectorStruct));
			newListTail = addNode (newListTail, leanVertex);
		}
	}

	return (newList);
}


// Divide a spherical polygon into triangles,
//  to facilitate its rendering.
void triangulateSphericalPolygon (nodePtr polygonList, spinType polygonOrientation)
{
	nodePtr		part1List = NULL;
	nodePtr		part2List = NULL;
	int			listLength = getListLength (polygonList);

	if (listLength <= 3)
	{
		PolygonTriangles = addFrontNode (PolygonTriangles, polygonList);
	}
	else
	{
		divideSphericalPolygon (polygonList, &part1List, &part2List, polygonOrientation);
		triangulateSphericalPolygon (part1List, polygonOrientation);
		triangulateSphericalPolygon (part2List, polygonOrientation);
	}
}


// Divide a spherical polygon into two simpler polygons.
// The division is done by drawing a diagonal between two
//  vertices of the polygon. These two vertices will now
//  belong to both of the polygons.
void divideSphericalPolygon (nodePtr polygonList, nodePtr* part1List, nodePtr* part2List, spinType polygonOrientation)
{
	vectorPtr		vertex1 = NULL;
	vectorPtr		vertex2 = NULL;
	vectorPtr		vertex3 = NULL;
	vectorPtr		vertex4 = NULL;
	vectorPtr		tmpVertex = NULL;
	vertexStruct	point1;
	vertexStruct	point2;
	int				listLength = getListLength (polygonList);
	int				i, j;
	int				closest = 0;
	spinType		orientation;
	boolean			inside = FALSE;
	double			distance;
	double			smallDistance = 0;

	for (i=0; i<listLength; i++)
	{
		vertex1 = (vectorPtr) getNodeData (polygonList, (listLength-1+i)%listLength);
		vertex2 = (vectorPtr) getNodeData (polygonList, i);
		vertex3 = (vectorPtr) getNodeData (polygonList, (i+1)%listLength);

		orientation = sphereAngleOrientation (vertex1, vertex2, vertex3, 2);

		// If vertex2 is a convex angle.
		if ( (orientation == polygonOrientation) && (listLength > 3) )
		{
			// Check that no other vertices lie inside of the triangle
			//  formed by the 3 vertices.
			for (j=0; j<listLength; j++)
			{
				vertex4 = (vectorPtr) getNodeData (polygonList, j);

				if ( !(	equalVectors(vertex1, vertex4) ||
						equalVectors(vertex2, vertex4) ||
						equalVectors(vertex3, vertex4) )	)
				{
					inside = pointInsideSphericalTriangle (vertex1, vertex2, vertex3, vertex4, polygonOrientation);

					// If a vertex lies inside, check its distance.
					if (inside)
					{
						point1.x = vertex2->i;
						point1.y = vertex2->j;
						point1.z = vertex2->k;
						point2.x = vertex4->i;
						point2.y = vertex4->j;
						point2.z = vertex4->k;
						distance = squareDistance (point1, point2);
						
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
				vertex4 = (vectorPtr) getNodeData (polygonList, closest);
				tmpVertex = (vectorPtr) xmalloc (sizeof (vectorStruct));
				memcpy (tmpVertex, vertex4, sizeof (vectorStruct));
				*part2List = addFrontNode (*part2List, tmpVertex);

				if (closest < i)
					closest += listLength;

				for (j=closest-1; j>i; j--)
				{
					vertex4 = (vectorPtr) removeNode (part1List, (j%listLength));
					*part2List = addFrontNode (*part2List, vertex4);
				}

				// Copy the second of the vertices that will belong to
				//  both new polygons.
				tmpVertex = (vectorPtr) xmalloc (sizeof (vectorStruct));
				memcpy (tmpVertex, vertex2, sizeof (vectorStruct));
				*part2List = addFrontNode (*part2List, tmpVertex);
			}
			else		// Split the polygon with a diagonal between: vertex1, vertex3.
			{
				// Copy one of the vertices that will belong to
				//  both new polygons.
				tmpVertex = (vectorPtr) xmalloc (sizeof (vectorStruct));
				memcpy (tmpVertex, vertex1, sizeof (vectorStruct));
				*part1List = addFrontNode (*part1List, tmpVertex);

				// Copy the second of the vertices that will belong to
				//  both new polygons.
				tmpVertex = (vectorPtr) xmalloc (sizeof (vectorStruct));
				memcpy (tmpVertex, vertex3, sizeof (vectorStruct));
				*part1List = addFrontNode (*part1List, tmpVertex);

				*part1List = addFrontNode (*part1List, vertex2);

				*part2List = polygonList;
				// Remove the one vertex that will belong to the other polygon.
				removeNode (part2List, i);
			}

			// Stop the for loop.
			break;
		}	// if ( (orientation == polygonOrientation) && (listLength > 3) )
	}	// for (i=0; i<listLength; i++)
}


// Determine if a vertex is inside of a spherical triangle.
// Returns true if the point is inside.
boolean pointInsideSphericalTriangle (vectorPtr vertex1, vectorPtr vertex2, vectorPtr vertex3, vectorPtr vertex4, spinType polygonOrientation)
{
	spinType	orientation1;
	spinType	orientation2;
	spinType	orientation3;

	orientation1 = sphereAngleOrientation (vertex4, vertex1, vertex2, 2);
	orientation2 = sphereAngleOrientation (vertex4, vertex2, vertex3, 2);
	orientation3 = sphereAngleOrientation (vertex4, vertex3, vertex1, 2);

	if (	(orientation1 == polygonOrientation) &&
			(orientation2 == polygonOrientation) &&
			(orientation3 == polygonOrientation)	)
		return (TRUE);
	else
		return (FALSE);
}
