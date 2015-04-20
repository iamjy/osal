## 
## osal-config.mak
##
## This file specifies the Hardware Architecture ( CPU ), 
## Platform ( Board ), OS, and BSP to build.
##
## NOTE: Make sure each selection does not have any spaces
##       after it!
##
##---------------------------------------------------------
## Hardware Architecture
## Supported Arhcitectures include:
##  x86
##  ppc
##  coldfire
##
##---------------------------------------------------------
HWARCH = x86

##---------------------------------------------------------
## Platform
##---------------------------------------------------------
PLATFORM = pc

##---------------------------------------------------------
## Operating System
## OS = The operating system selected for the Abstraction implementation
##---------------------------------------------------------
OS = linux

##---------------------------------------------------------
## BSP -- BSP/Operating system for the board
## BSP = vxworks, vxworks6, etc,
##       This is to allow different BSP/OS directories without
##       having to duplicate the OS API directory. For example
##       on the ppc/mcp750 board, we need a vxworks and vxworks6
##       BSP, but they can still share the vxworks OS abstraction.
##---------------------------------------------------------
BSP = linux
