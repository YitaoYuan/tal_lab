LIB     := libtal_mpi.so
CC      := mpic++ 
CFLAGS  := -O3 -DHAVE_MPI
LDFLAGS := -fPIC -shared 

SRCS-y := tal_mpi.cc ring.cc

all:main

main: $(SRCS-y) 
	mkdir -p build
	$(CC) $(CFLAGS) $(LDFLAGS) -o build/$(LIB) $(SRCS-y)

.PHONY: clean
clean:
	rm -f build/$(LIB)
	test -d build && rmdir -p build || true
