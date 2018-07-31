

CC = mpicc

PROGRAMS = \
	 libCFG \
	 libCFG_noMCW \
	 libCFG_noMCW_ac \
	 libCFG_noMCW_multiport

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
libCFG_noMCW_multiport: libCFG_noMCW_multiport.c
	$(CC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
