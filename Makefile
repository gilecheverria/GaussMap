# To compile and use valgrind, using the Nvidia drivers,
# set the environment variable:
#       export LD_ASSUME_KERNEL=2.3.98

CC = gcc

# CFLAGS = -O2 -Wall
CFLAGS = -g -Wall
# CFLAGS = -g -O2 -Wall -D BnW -D SINGLE_DISPLAY
# CFLAGS = -g -O2 -Wall -D BnW
# CFLAGS = -g -Wall -D DEBUG=3
# CFLAGS = -g -Wall

LDFLAGS =  -lGLU -lGL -lm -lX11
#LDFLAGS =  -I/usr/include -L/usr/lib/x86_64-linux-gnu -lGLU -lGL -lX11 -lm
# LDFLAGS =  -lm
OBJECTS = matrices.o vertices.o lists.o faces.o edges.o geometry.o objParser.o normals3D.o tools.o sphericalGeometry.o convexHull3D.o pathFinder.o vertexGeometry.o objWriter.o offWriter.o plyWriter.o
GLOBJECTS = glMain.o glTools.o glDisplay.o glObjectLists.o glGaussMap.o glConvexHull.o glRotation.o glScreenCapture.o Chromium/TexFont.o

GAUSSMAP = gaussMap
DECIMATOR = decimator
MAIN = $(DECIMATOR)

# all: $(GAUSSMAP)
all: $(DECIMATOR)

#$(GAUSSMAP): CFLAGS += -D SINGLE_DISPLAY
$(GAUSSMAP): $(GAUSSMAP).o $(OBJECTS) $(GLOBJECTS)
	$(CC) $(CFLAGS) \
           -o $@ $^ $(LDFLAGS)
#	rm -f *.o

$(DECIMATOR): CFLAGS += -D DECIMATOR -D SINGLE_DISPLAY
$(DECIMATOR): $(DECIMATOR).o $(GAUSSMAP).o $(OBJECTS) $(GLOBJECTS)
	$(CC) $(CFLAGS) -D DECIMATOR \
           -o $@ $^ $(LDFLAGS)
#	rm -f *.o

tags:
	ctags *.c *.h

clean:
	-rm -f *.o


# Debugging using Valgrind

# SAMPLEFILE = Data/flat.obj 1
# SAMPLEFILE = Data/Cones/cone04-1.obj 1
# SAMPLEFILE = Data/Cones/cone05-1.obj 1
# SAMPLEFILE = Data/Saddles/monkey-saddle.obj 1
# SAMPLEFILE = Data/Q-be/cube-04.obj
# SAMPLEFILE = Data/Q-be/cube-05.obj
# SAMPLEFILE = Data/Objects/object-08-3.obj 1
# SAMPLEFILE = Data/Tori/torus_smooth.obj
# SAMPLEFILE = Data/Tori/torus_final.obj
# SAMPLEFILE = Data/Tori/torus_orig.obj
# SAMPLEFILE = Data/weirdTriangle.obj
# SAMPLEFILE = Data/venus.obj
SAMPLEFILE = Data/hawaii.obj

EXECUTABLE = $(MAIN) $(SAMPLEFILE)
VALGRINDFLAGS = --suppressions=suppresedErrors.supp --leak-check=yes --show-reachable=yes --num-callers=6 --tool=memcheck
GDBATTACH = --db-attach=yes

check:
	valgrind $(VALGRINDFLAGS) ./$(EXECUTABLE)

suppressions:
	valgrind $(VALGRINDFLAGS) --gen-suppressions=yes ./$(EXECUTABLE)

debug:
	valgrind $(VALGRINDFLAGS) $(GDBATTACH) ./$(EXECUTABLE)

.PHONY: all clean tags check
