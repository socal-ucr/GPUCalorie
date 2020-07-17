#
# Thanks to Greg Link from Penn State University 
# for his math acceleration engine.
#

# Uncomment the following math acceleration flags 
# relevant to your target and set the appropriate
# path and flag options

#SUPERLU: [0-1]
ifndef SUPERLU
SUPERLU = 0
endif

SUPERLU = 1
 
ifeq ($(SUPERLU), 1)
#Super LU
SuperLUroot     = /home/mchow009/superlu/build
SUPERLULIB      = $(SuperLUroot)/lib64/libsuperlu.a
BLASLIB         = -L $(SuperLUroot)/lib64 -lblas
SLU_HEADER  = $(SuperLUroot)/include

MATHACCEL	= none
INCDIR		= $(SLU_HEADER)
LIBDIR		= 
LIBS  		= -lm $(SUPERLULIB) $(BLASLIB)
EXTRAFLAGS	= 
else
# default - no math acceleration
MATHACCEL	= none
INCDIR		= 
LIBDIR		= 
LIBS		= -lm
EXTRAFLAGS	= 
endif

# Intel Machines - acceleration with the Intel
# Math Kernel Library (MKL)
#MATHACCEL	= intel
#INCDIR		= /bigdisk/ks4kk/mkl/10.1.0.015/include
#LIBDIR		= /bigdisk/ks4kk/mkl/10.1.0.015/lib/em64t
#LIBS		= -lmkl_lapack -lmkl -lguide -lm -lpthread
#EXTRAFLAGS	= 

# AMD Machines - acceleration with the AMD
# Core Math Library (ACML)
#MATHACCEL	= amd
#INCDIR		= /uf1/ks4kk/lib/acml3.6.0/gfortran32/include
#LIBDIR		= /uf1/ks4kk/lib/acml3.6.0/gfortran32/lib
#LIBS		= -lacml -lgfortran -lm
#EXTRAFLAGS	= 

# Apple Machines - acceleration with the Apple
# Velocity Engine (AltiVec)
#MATHACCEL	= apple
#INCDIR		= 
#LIBDIR		= 
#LIBS		= -framework vecLib -lm
#EXTRAFLAGS	= 

# Sun Machines - acceleration with the SUN
# performance library (sunperf)
#MATHACCEL	= sun
#INCDIR		= 
#LIBDIR		= 
#LIBS		= -library=sunperf
#EXTRAFLAGS	= -dalign 

# basic compiler flags - special case for sun
ifeq ($(MATHACCEL), sun)
CC 			= CC
ifeq ($(DEBUG), 1)
OFLAGS		= -g -erroff=badargtypel2w
else
ifeq ($(DEBUG), 2)
OFLAGS		= -xpg -g -erroff=badargtypel2w
else
OFLAGS		= -xO4 -erroff=badargtypel2w
endif	# DEBUG = 2
endif	# DEBUG = 1
else	# MATHACCEL != sun	
CC 			= gcc
ifeq ($(DEBUG), 1)
OFLAGS		= -O0 -ggdb -Wall
else
ifeq ($(DEBUG), 2)
OFLAGS		= -O3 -pg -ggdb -Wall
else
OFLAGS		= -O3
endif	# DEBUG = 2
endif	# DEBUG = 1
endif	# end MATHACCEL
RM			= rm -f
AR			= ar qcv
RANLIB		= ranlib
OEXT		= o
LEXT		= a
# Verbosity level [0-3]
ifndef VERBOSE
VERBOSE	= 1
endif

#BU_3D: Debugging 3D [0-1]
ifndef DEBUG3D
DEBUG3D = 0
endif

# Numerical ID for each acceleration engine
ifeq ($(MATHACCEL), none)
ACCELNUM = 0
endif
ifeq ($(MATHACCEL), intel)
ACCELNUM = 1
endif
ifeq ($(MATHACCEL), amd)
ACCELNUM = 2
endif
ifeq ($(MATHACCEL), apple)
ACCELNUM = 3
endif
ifeq ($(MATHACCEL), sun)
ACCELNUM = 4
endif

ifdef INCDIR
INCDIRFLAG = -I$(INCDIR)
endif

ifdef LIBDIR
LIBDIRFLAG = -L$(LIBDIR)
endif

CFLAGS	= -fPIC $(OFLAGS) $(EXTRAFLAGS) $(INCDIRFLAG) $(LIBDIRFLAG) -DVERBOSE=$(VERBOSE) -DMATHACCEL=$(ACCELNUM) -DDEBUG3D=$(DEBUG3D) -DSUPERLU=$(SUPERLU) -g

# sources, objects, headers and inputs

# HotFloorplan
FLPSRC	= flp.c flp_desc.c npe.c shape.c 
FLPOBJ	= flp.$(OEXT) flp_desc.$(OEXT) npe.$(OEXT) shape.$(OEXT) 
FLPHDR	= flp.h npe.h shape.h
FLPIN = ev6.desc avg.p

# HotSpot
TEMPSRC	= temperature.c RCutil.c
TEMPOBJ	= temperature.$(OEXT) RCutil.$(OEXT)
TEMPHDR = temperature.h
TEMPIN	= 

#	Package model
PACKSRC	=	package.c
PACKOBJ	=	package.$(OEXT)
PACKHDR	=	package.h
PACKIN	=	package.config

# HotSpot block model
BLKSRC = temperature_block.c 
BLKOBJ = temperature_block.$(OEXT) 
BLKHDR	= temperature_block.h
BLKIN	= ev6.flp gcc.ptrace

# HotSpot grid model
GRIDSRC = temperature_grid.c
GRIDOBJ = temperature_grid.$(OEXT)
GRIDHDR	= temperature_grid.h
GRIDIN	= layer.lcf example.lcf example.flp example.ptrace

# Miscellaneous
MISCSRC = util.c wire.c
MISCOBJ = util.$(OEXT) wire.$(OEXT)
MISCHDR = util.h wire.h
MISCIN	= hotspot.config

OUTPUT_DIR=$(SIM_OBJ_FILES_DIR)/gpucalorie/hotspot

CSRCS = $(shell ls *.c)
# all objects
OBJ	= $(TEMPOBJ) $(PACKOBJ) $(BLKOBJ) $(GRIDOBJ) $(FLPOBJ) $(MISCOBJ)
OBJS = $(CSRCS:%.c=$(OUTPUT_DIR)/%.$(OEXT))

all: $(OBJS)

$(OUTPUT_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $(OUTPUT_DIR)/$*.$(OEXT) -c $*.c

$(OUTPUT_DIR)/Makefile.makedepend: depend

depend:
	touch $(OUTPUT_DIR)/Makefile.makedepend
	makedepend -f$(OUTPUT_DIR)/Makefile.makedepend -p$(OUTPUT_DIR)/ $(CSRCS) 2> /dev/null

filelist:
	@echo $(FLPSRC) $(TEMPSRC) $(PACKSRC) $(BLKSRC) $(GRIDSRC) $(MISCSRC) \
		  $(FLPHDR) $(TEMPHDR) $(PACKHDR) $(BLKHDR) $(GRIDHDR) $(MISCHDR) \
		  $(FLPIN) $(TEMPIN) $(PACKIN) $(BLKIN) $(GRIDIN) $(MISCIN) \
		  hotspot.h hotspot.c hotfloorplan.h hotfloorplan.c \
		  sim-template_block.c \
		  tofig.pl grid_thermal_map.pl \
		  Makefile
clean:
	$(RM) *.$(OEXT) *.obj *.d core *~ Makefile.bak hotspot hotfloorplan libhotspot.$(LEXT)

cleano:
	$(RM) *.$(OEXT) *.obj

include $(OUTPUT_DIR)/Makefile.makedepend
