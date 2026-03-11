# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0
##################################  MAKEFILE  #################################
#                                                                             #
#   File:    Makefile.linux                                                   #
#   Author:  Andy Foulks                                                      #
#   Created: 6.30.00                                                          #
#                                                                             #
#   To add a file:                                                            #
#       Do Nothing.                                                           #
#       If there is a C++ file in the directory that you DON'T want           #
#       to be automatically compiled, name it <filename>.C or                 #
#       <filename>.cpp.bak or something like that.                            #
#                                                                             #
#   To add a new module (i.e. subdirectory):                                  #
#       Define your module as a real directory name,                          #
#       then add it to the end of the "MODULES" variable.                     #
#                                                                             #
###############################################################################

OS := $(shell uname -s)

ifeq (${OS}, Linux)
PLATFORM        =       linux
endif
# FIX: watch out for x86 vrs ppc mac issues
ifeq (${OS}, Darwin)
PLATFORM        =       Darwin
endif

export PLATFORM


###########################  PROJECT ROOT DIRECTORY  ##########################

ROOT               = .

##################################  MODULES  ##################################

ENGINE              = $(ROOT)/engine
POSABLESTATERELATIONSHIPS = $(ENGINE)/PosableStateRelationships
TEXTUREMAPS         = $(ENGINE)/TextureMaps
INTERACTORS         = $(ROOT)/interactors
VIS_OBJECTS         = $(ROOT)/objects
S57                 = $(VIS_OBJECTS)/S57
GEO_DATA_OBJECTS    = $(VIS_OBJECTS)/GeoData
NODE_TYPES          = $(VIS_OBJECTS)/nodeTypes
DXF                 = $(VIS_OBJECTS)/dxf
WIDGETS             = $(ROOT)/widgets
UTILITIES           = $(ROOT)/utilities
IMAGE               = $(UTILITIES)/Image
MISSION_PLAN        = $(VIS_OBJECTS)/MissionPlan

MODULES             = $(ROOT) $(ENGINE) $(S57)\
                      $(POSABLESTATERELATIONSHIPS) $(TEXTUREMAPS) \
                        $(INTERACTORS) $(VIS_OBJECTS) $(GEO_DATA_OBJECTS)\
						$(WIDGETS) $(UTILITIES) $(NODE_TYPES) $(DXF)\
						$(MISSION_PLAN) $(IMAGE)


#########################  MAKE DOES ALL OF THE REST!  ########################

SOURCES            = $(foreach dir, $(MODULES), $(wildcard $(dir)/*.cpp))
CSOURCES           = $(foreach dir, $(MODULES), $(wildcard $(dir)/*.c))
HEADERS            = $(foreach dir, $(MODULES), $(wildcard $(dir)/*.h))
OBJECTS            = $(SOURCES:.cpp=.o)
COBJECTS           = $(CSOURCES:.c=.o)
EXECUTABLE         = GeoZui4D 
#SYMLINKS          := utilities/slogcxx.h utilities/slogcxx.cpp

################################  DEPENDENCIES  ###############################

DEPENDS_DIR        = $(ROOT)/dependencies
DEPENDENCIES       = $(SOURCES:%.cpp=$(DEPENDS_DIR)/%.d)

#######  Platform Independent Variables:

    INCLUDE_DIRS       := $(MODULES) ../external-libs/include ../external-libs/include/libxml2 \
			/usr/X11R6/include ../external-libs/include/SDL ../external-libs/include/smpeg

    LIB_DIRS           :=  ../external-libs/lib ../external-libs/lib/Tktable2.9 /usr/X11R6/lib

    LIBS               := glut GLU Xmu Xext X11 jpeg tiff smpeg SDL_mixer SDL SDL_sound xml2 netcdf_c++ netcdf  ccom gdal SDL_net togl avformat avcodec avutil GLEW

# asound - Linux sound library

#######  Platform Dependent Variables:

HOST := $(shell hostname)

ifeq ($(PLATFORM),linux)
	CC             = g++
	ifndef RELEASE
		DEBUGFLAGS := -ggdb
		#DEBUGFLAGS += -H # outputs header info
		OPTIMIZE_FLAGS := -O0
		TCLLIBS := tk8.4g tcl8.4g
	else
		DEBUGFLAGS := -DNDEBUG
		OPTIMIZE_FLAGS := -O3
		TCLLIBS := tk8.4 tcl8.4
	endif
	CPPFLAGS       += -DDEV -DLINUX $(OPTIMIZE_FLAGS) $(DEBUGFLAGS) -Wall -Werror 

	ifdef PROFILE
		CPPFLAGS    += -pg # Add this to enable profiling
	endif

	LIBS           += GL Xi $(TCLLIBS) SM ICE Xt Xxf86vm asound Tktable2.9
endif

# /sw is fink!
ifeq ($(PLATFORM),Darwin)
  #FIX: is life better with g++-3.3 since fink transitional uses 3.3?
  CC             := g++
  CXX             := g++
  #CPPFLAGS_OPT := -O3 -fastmath -fexpensive-optimizations 
  # Mac Only flags first
  CPPFLAGS_OPT := -fast 
  CPPFLAGS_OPT += -fexpensive-optimizations -ffast-math -DNDEBUG
  #PROFILE := -pg
  CPPFLAGS_DBG       += -ggdb -O0   # debugging
  CPPFLAGS_DBG       += -DDEV  $(OPTIMIZE_FLAGS) 
  CPPFLAGS_DBG       += -Wall -Wno-deprecated ${PROFILE}
ifdef OPTIMIZE
  CPPFLAGS := ${CPPFLAGS_OPT}
else
  CPPFLAGS := ${CPPFLAGS_DBG}
endif

#  CPPFLAGS       += -DMPATROL
#  CPPFLAGS       += -include /sw/include/mpatrol.h
  #CPPFLAGS       += -Werror  # netcdf is lame
  INCLUDE_DIRS   += /sw/include /sw/include/SDL /sw/include/gdal1
  INCLUDE_DIRS   += /sw/include/libxml2
  #SDL_LIBS := ${shell sdl-config --libs}
  # Not needed from sdl-config: -lSDL -framework OpenGL
  SDL_LIBS := -lSDLmain -lGL -framework Cocoa 
  LIBS           += tk tcl vorbis z ogg mp3lame vorbisenc
  LIB_DIRS       += ./lib.${PLATFORM} 
  ADDITIONAL_OBJECTS := togl/togl.o ../external-libs/lib/Tktable2.9.a
  LIB_DIRS       += /sw/lib 
  LDFLAGS        := -bind_at_load ${SDL_LIBS}
endif


###### Additional object files can be added here:

    ADDITIONAL_OBJECTS = 

##################################  TARGETS  ##################################
#
#  Why two layers of recursion?  ("all" calls "make project" recursively, and
#  "project" calls "make visualizer" recursively).  Its all for the auto-
#  generated dependency files.  These ".d" files only get included if there
#  is one or more levels of recursion.  This insures that "make clean" will
#  not rebuild the ".d" files (take away the recursion and "make clean" will
#  do this -- its really annoying.)  The first level of recursion includes
#  the automatically generated files, EVEN IF THEY ARE OUT OF DATE.  Then,
#  of course, they are properly updated if any ".h" files changed.  The
#  SECOND level of recursion includes all of these newly generated ".d"
#  files before going on to make the visualizer executable, which now
#  has all of the proper dependencies included.  Believe it or not, 
#  this really works!
#

ifdef MAKING_PROJECT
    include $(DEPENDENCIES)
endif

all:  GZincludes.h.gch # $(SYMLINKS)
	$(MAKE) -C ../external-libs
	$(MAKE) -C gzmod compile
	@$(MAKE) MAKING_PROJECT=true project 

release:
	@$(MAKE) RELEASE=true all

project: $(DEPENDENCIES)
	@$(MAKE) $(EXECUTABLE)

docs: .FORCE_REMAKE
	doxygen Doxyfile
	$(MAKE) -C gzmod docs

$(EXECUTABLE): $(OBJECTS) $(COBJECTS)
	$(CXX) $(CPPFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(COBJECTS) $(ADDITIONAL_OBJECTS) \
			$(LIB_DIRS:%=-L%) $(LIBS:%=-l%) $(LDFLAGS)

%.o: %.cpp                      # $< == dependency, $@ == target
	$(CXX) $(CPPFLAGS) -o $@ $(INCLUDE_DIRS:%=-I%) -c $<

%.o: %.cc                      # $< == dependency, $@ == target
	$(CXX) $(CPPFLAGS) -o $@ $(INCLUDE_DIRS:%=-I%) -c $<

%.o: %.c                      # $< == dependency, $@ == target
	$(CC) $(CPPFLAGS) -o $@ $(INCLUDE_DIRS:%=-I%) -c $<

GZincludes.h.gch: GZincludes.h
	$(CXX) -DMAKING_PCH -x c++-header $(CPPFLAGS) $(INCLUDE_DIRS:%=-I%) GZincludes.h

# utilities/slogcxx.h: ../tools/slogcxx/src/slogcxx.h
# 	ln -s ../../tools/slogcxx/src/slogcxx.h utilities/
# 
# utilities/slogcxx.cpp: ../tools/slogcxx/src/slogcxx.cpp
# 	ln -s ../../tools/slogcxx/src/slogcxx.cpp utilities/


clean:
	@echo "Cleaning up..."
	rm -rf $(OBJECTS)  $(COBJECTS) $(EXECUTABLE) core $(MODULES:%=%/ii_files) default GZincludes.h.gch
	$(MAKE) -C gzmod clean

real-clean: realclean
realclean:
	@$(MAKE) clean
	rm -rf $(DEPENDS_DIR)
	$(MAKE) -C gzmod real-clean

#########################  DEPENDENCY FILE GENERATION  ########################
#  Note:
#    If you need to turn off the automatic dependency file generation,
#    comment out this entire section (from here to the end of this file)
#    and replace it with the following "safe mode" line:
#        $(OBJECTS): $(HEADERS)
#    which makes all of the .o files depend on ALL of the .h files.
#    Also change the "all" target to be:
#        all: $(EXECUTABLE)
#    and comment out the "project" target.

$(DEPENDENCIES): $(DEPENDS_DIR)

$(DEPENDS_DIR):
	@echo; echo "Generating dependency files..."; echo
	mkdir -p $(DEPENDS_DIR) $(MODULES:%=$(DEPENDS_DIR)/%)

$(DEPENDS_DIR)/%.d: %.cpp $(DEPENDS_DIR)
	@echo "Updating dependency file '$@'..."
	@echo "$@ `echo $< | sed 's/\.cpp/\.o/g'`: \
            `g++ $(INCLUDE_DIRS:%=-I%) -MM $< | cut -d ' ' -f 2-`" > $@

###############################################################################


.FORCE_REMAKE:

.PHONY: tags etags
tags: etags
etags:
	find . -name '*.[chCH]' -o -name '*.cpp' | egrep -v 'Tktable|Windows_Support' | etags -

######################################################################
# TESTING... area just for ideas ... do not count on the stuff below
######################################################################
LIB_SUBDIRS := $(ROOT)/engine
LIB_SUBDIRS += $(ENGINE)/PosableStateRelationships
LIB_SUBDIRS += $(ENGINE)/TextureMaps
LIB_SUBDIRS += $(ROOT)/interactors
LIB_SUBDIRS += $(ROOT)/objects
LIB_SUBDIRS += $(VIS_OBJECTS)/S57
LIB_SUBDIRS += $(VIS_OBJECTS)/GeoData
LIB_SUBDIRS += $(VIS_OBJECTS)/nodeTypes
LIB_SUBDIRS += $(VIS_OBJECTS)/dxf
LIB_SUBDIRS += $(ROOT)/widgets
LIB_SUBDIRS += $(ROOT)/utilities
LIB_SUBDIRS += $(VIS_OBJECTS)/MissionPlanDisplayTypes

build-libs:
	@for dir in ${LIB_SUBDIRS}; do \
	  echo $$dir; \
	  (cd $$dir && make libs); \
	done
