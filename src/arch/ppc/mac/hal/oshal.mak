###############################################################################
# File: hal.mak
#
# Purpose:
#   Compile the OS Hardware Abstraction layer library.
#
# History:
#
###############################################################################

# Subsystem produced by this makefile.
TARGET = hal.o

#==============================================================================
# Object files required to build subsystem.

OBJS=osapiarch.o 

#==============================================================================
# Source files required to build subsystem; used to generate dependencies.

SOURCES = $(OBJS:.o=.c)

