# root is relative from inside the created subdirectories in this folder:
export ROOT = ../..
export SYSTEM = ubuntu
export LIBDIR = $(ROOT)/lib/$(SYSTEM)
export INCLUDEDIR = -I$(ROOT)/include -I/X11
export LIBS32 = -lsig32
export LIBS64 = -lsigogl64 -lsigos64 -lsig64 -lglfw -lX11 -lGL
 #-lglfw -lrt -lm -lGL -lGLU 

# note: not all the libs listed above are needed to all examples

# names of the modules to be compiled:

target = libsig64 libsigogl64 libsigos64 libsigkin64 gstests64 shapes64
DIRS = $(target)

# to be included later: libsigogl64

# compilation options:
export CC = g++
export OPT32 = -O2 -m32 -std=c++11
export OPT64 = -O2 -m64 -std=c++11
export WARN = -Wall -Wno-switch -Wno-maybe-uninitialized

export CFLAGS32 = $(OPT32) $(WARN) $(INCLUDEDIR)
export CFLAGS64 = $(OPT64) $(WARN) $(INCLUDEDIR)
export LFLAGS32 = -m32 -L$(LIBDIR) $(LIBS32)
export LFLAGS64 = -m64 -L$(LIBDIR) $(LIBS64)

# Be quiet when building:
.SILENT:

# call makefiles of modules:
all: 
	@for dir in $(DIRS); do\
		echo "=== making $$dir ===";\
		mkdir -p $$dir; \
		cd $$dir; \
		$(MAKE) -f ../makefile.$$dir.mf;\
		cd ..; \
	done
	echo "=== done ===";

clean:
	$(RM) core *.o *~ *.x
	@for dir in $(DIRS); do \
		$(RM) -f -r $$dir; \
	done

cleanx:
	$(RM) core *.o *~ *.x

cleanogl:
	$(RM) core *.o *~ *.x
	$(RM) -f -r libsigogl64;

cleanos:
	$(RM) core *.o *~ *.x
	$(RM) -f -r libsigos64;
