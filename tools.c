#include "tools.h"


// Memory allocation with validation.
void* xmalloc (size_t size)
{
	void*	pointer = (void*) malloc (size);

	if (pointer == NULL) {
		printf ("Unable to allocate memory. Exiting.\n");
		exit (1);
	}

	return (pointer);
}


// File opener with validation.
FILE* xfopen (char* fileName, char* mode)
{
	FILE*	fileDescriptor = fopen (fileName, mode);

	if (fileDescriptor == NULL) {
		printf ("Unable to open file: '%s'. Exiting.\n", fileName);
		exit (1);
	}

	return (fileDescriptor);
}


// Initialise the seed for the random numbers.
void initRandom ()
{
	srand (time(0));
}


// Random number generator.
int randomInt (int max)
{
	int	number;

	number = (int) ((float) max * rand() / (RAND_MAX + 1.0));
	// number = 1 + (int) ((float) max * rand() / (RAND_MAX + 1.0));
	return (number);
}


// Print a spinning progress star and the percent completed.
void printProgressStar (int counter, int total)
{
	static char	progress_star[] = {'|', '/', '-', '\\'};
	int			percent = counter * 100 / total;

	if (percent > 9) {
		printf ("\b");
		fflush (stdout);
	}

	printf ("\b\b\b\b\b%c %d %%", progress_star[counter%4], percent);
	fflush (stdout);
}


// Print the contents of an array of ints.
void print_integer_array (int* array, int length)
{
	int			i;

	printf ("Integer array: %d", array[0]);
	for (i=1; i<length; i++)
		printf (", %d", array[i]);
	printf ("\n");

}
