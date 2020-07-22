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
 
ifeq ($(SUPERLU), 1)
#Super LU
SuperLUroot     = /home/mchow009/superlu/build
SUPERLULIB      = $(SuperLUroot)/lib64/libsuperlu
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

ACCELNUM = 0

ifdef INCDIR
INCDIRFLAG = -I$(INCDIR)
endif

ifdef LIBDIR
LIBDIRFLAG = -L$(LIBDIR)
endif
VERBOSE = 0
DEBUG3D = 0

ifeq ($(DEBUG),1)
	CXXFLAGS = -Wall -DDEBUG
else
	CXXFLAGS = -Wall
endif

ifeq ($(TRACE),1)
	CXXFLAGS += -DTRACING_ON=1
endif

include ../../../version_detection.mk

ifeq ($(GNUC_CPP0X), 1)
    CXXFLAGS += -std=c++0x
endif

ifneq ($(DEBUG),1)
	OPTFLAGS += -O3
else
	CXXFLAGS += 
endif

CXXFLAGS += -I$(CUDA_INSTALL_PATH)/include $(INCDIRFLAG) $(LIBDIRFLAG) -DVERBOSE=$(VERBOSE) -DMATHACCEL=$(ACCELNUM) -DDEBUG3D=$(DEBUG3D) -DSUPERLU=$(SUPERLU)

OPTFLAGS += -g3 -fPIC
OPTFLAGS += -DCUDART_VERSION=$(CUDART_VERSION)
CPP = g++
OEXT = o

OUTPUT_DIR=$(SIM_OBJ_FILES_DIR)/gpucalorie/hotspot

CSRCS = $(shell ls *.cc)
# all objects
OBJS = $(CSRCS:%.cc=$(OUTPUT_DIR)/%.$(OEXT))

all: $(OBJS)

$(OUTPUT_DIR)/%.o: %.cc
	$(CPP) $(OPTFLAGS) $(CXXFLAGS) -o $(OUTPUT_DIR)/$*.$(OEXT) -c $*.cc

$(OUTPUT_DIR)/Makefile.makedepend: depend

depend:
	touch $(OUTPUT_DIR)/Makefile.makedepend
	makedepend -f$(OUTPUT_DIR)/Makefile.makedepend -p$(OUTPUT_DIR)/ $(CSRCS) 2> /dev/null

clean:
	rm -f *.o core *~ *.a 
	rm -f Makefile.makedepend Makefile.makedepend.bak

include $(OUTPUT_DIR)/Makefile.makedepend
