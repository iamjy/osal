/*
** osprocessor.h
**
** Testing
**
**      This is governed by the NASA Open Source Agreement and may be used, 
**      distributed and modified only pursuant to the terms of that agreement. 
**
**      Copyright © 2004-2006, United States government as represented by the 
**      administrator of the National Aeronautics Space Administration.  
**      All rights reserved. 
**
**
*/

#ifndef _osprocessor_
#define _osprocessor_



#include <stdio.h>
#include <string.h>
#include "common_types.h"

/*
** Processor Context type. 
** This is needed to determine the size of the context entry in the ER log.
** Although this file is in a CPU directory, it really is OS dependant, so supporting
** multiple OSs on the same CPU architecture ( i.e. x86/linux, x86/windows, x86/osx ) 
** will require IFDEFS. 
*/
typedef struct 
{
   uint32   regs[32];
   
} OS_ExceptionContext_t;

#define OS_CPU_CONTEXT_SIZE (sizeof(OS_ExceptionContext_t))


#endif

