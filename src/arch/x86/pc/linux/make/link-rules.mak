###############################################################################
# File: link-rules.mak
#
# Purpose:
#   Makefile for linking code and producing an executable image.
#
# History:
#
###############################################################################


##
## Linker flags that are needed
##
LDFLAGS = 

##
## Libraries to link in
##
LIBS = -lpthread

##
## OSAL Core Link Rule
## 
$(APPTARGET).$(APP_EXT): $(OBJS)
	$(COMPILER) $(DEBUG_FLAGS) -o $(APPTARGET).$(APP_EXT) $(OBJS) $(CORE_OBJS) $(LDFLAGS) $(LIBS)
	

