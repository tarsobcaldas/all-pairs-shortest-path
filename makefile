# *  Define the name of the makefile.                                        *

MAKNAM = makefile

# *  Define the directories in which to search for library files.            *

LIBDRS =

# *  Define the directories in which to search for include files.            *

INCDRS =

# *  Define the library files.                                               *

LIBFLS =

# *  Define the source files.                                                *

SRCFLS = main.c\
				 mtrxop.c\

# *  Define the object files.                                                *

OBJFLS = main.o\
				 mtrxop.o\

# *  Define the executable.                                                  *

EXE    = shortest-path

# *  Define the compile and link options.                                    *

CC     = mpicc
LL     = mpicc
CFLAGS = -Wall -g
LFLAGS = -lm

# *  Define the rules.                                                       *

$(EXE): $(OBJFLS)
	$(LL) $(LFLAGS) -o $@ $(OBJFLS) $(LIBDRS) $(LIBFLS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $(INCDRS) $<

all:
	make -f $(MAKNAM) clean
	make -f $(MAKNAM) $(EXE)

clean:
	-rm $(EXE)
	-rm $(OBJFLS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

main.o: mtrxop.h mtrxop.c
mtrxop.o: mtrxop.h
