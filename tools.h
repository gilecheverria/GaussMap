#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define X	0
#define Y	1
#define Z	2

typedef enum {FALSE, TRUE}	boolean;


FILE* xfopen (char* fileName, char* mode);
void* xmalloc (size_t size);
int randomInt (int max);
void initRandom ();
void printProgressStar (int counter, int total);
void print_integer_array (int* array, int length);
