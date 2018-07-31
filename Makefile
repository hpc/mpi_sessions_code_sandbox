

CC = mpicc

PROGRAMS = \
	 libCFG \
	 libCFG_noMCW \
	 libCFG_noMCW_ac

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS) *~ *.o

# Don't rely on default rules for the Fortran and Java examples

libCFG: libCFG.c
	$(CC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
libCFG_noMCW: libCFG_noMCW.c
	$(CC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
libCFG_noMCW_ac: libCFG_noMCW_ac.c
	$(CC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
