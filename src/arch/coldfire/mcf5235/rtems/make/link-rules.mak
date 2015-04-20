###############################################################################
# File: link-rules.mak
#
# Purpose:
#   Makefile for linking code and producing the OSAL Core executable image.
#
# History:
#
###############################################################################

CORE_INSTALL_FILES = $(EXE_TARGET)

##
## Linker flags that are needed
##
LDFLAGS = 

##
## Libraries to link in
##
LIBS = 


##
## Application Link Rule
##
$(APPTARGET).$(APP_EXT): $(OBJS)
	$(COMPILER) --pipe -B/opt/rtems-4.7/m68k-rtems4.7/mcf5235/lib/ -specs bsp_specs -qrtems   \
        -g -Wall  -O2 -g -fomit-frame-pointer -g    -m528x \
        -m5200  -o $(APPTARGET).$(APP_EXT) \
        $(OBJS) $(CORE_OBJS) 
	m68k-rtems4.7-objcopy -O binary --strip-all $(APPTARGET).$(APP_EXT) $(APPTARGET).exe

