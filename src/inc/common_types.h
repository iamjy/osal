/*---------------------------------------------------------------------------
**
**  Filename:
**    $Id: common_types.h 1.1 2007/10/16 16:14:49EDT apcudmore Exp  $
**
**      Copyright © 2004-2006, United States government as represented by the 
**      administrator of the National Aeronautics Space Administration.  
**      All rights reserved. This software was created at NASA’s Goddard 
**      Space Flight Center pursuant to government contracts.
**
**      This is governed by the NASA Open Source Agreement and may be used, 
**      distributed and modified only pursuant to the terms of that agreement. 
**
**  Purpose:
**	    Unit specification for common types.
**
**  Design Notes:
**         Assumes make file has defined processor family
**
**  References:
**     Flight Software Branch C Coding Standard Version 1.0a
**
**
**	Notes:
**
**
**  $Date: 2007/10/16 16:14:49EDT $
**  $Revision: 1.1 $
**  $Log: common_types.h  $
**  Revision 1.1 2007/10/16 16:14:49EDT apcudmore 
**  Initial revision
**  Member added to project d:/mksdata/MKS-OSAL-REPOSITORY/src/inc/project.pj
**  Revision 1.2 2006/06/08 14:28:32EDT David Kobe (dlkobe) 
**  Added NASA Open Source Legal Statement
**  Revision 1.1 2005/06/09 09:57:51GMT-05:00 rperera 
**  Initial revision
**  Member added to project d:/mksdata/MKS-CFE-REPOSITORY/cfe-core/inc/project.pj
**  Revision 1.6  2005/03/24 19:20:52  rmcgraw
**  Wrapped the boolean defintion for all three processors with #ifndef _USING_RTEMS_INCLUDES_
**
**  Revision 1.5  2005/03/10 16:59:08  acudmore
**  removed boolean prefix to TRUE and FALSE defintion to avoid vxWorks conflict.
**
**  Revision 1.4  2005/03/07 20:23:34  acudmore
**  removed duplicate boolean definition
**
**  Revision 1.3  2005/03/07 20:05:17  acudmore
**  updated with __PPC__ macro that gnu compiler uses
**
**  Revision 1.2  2005/03/04 16:02:44  acudmore
**  added coldfire architecture
**
**  Revision 1.1  2005/03/04 15:58:45  acudmore
**  Added common_types.h
**
**
**
**-------------------------------------------------------------------------*/

#ifndef _common_types_
#define _common_types_


/*
** Includes
*/

/*
** Macro Definitions
*/


#if defined(_ix86_)
/* ----------------------- Intel x86 processor family -------------------------*/

  /* Little endian */
  #undef   _STRUCT_HIGH_BIT_FIRST_
  #define  _STRUCT_LOW_BIT_FIRST_

#ifndef _USING_RTEMS_INCLUDES_
  typedef unsigned char         boolean;
#endif

  typedef signed char           int8;
  typedef short int             int16;
  typedef long int              int32;
  typedef unsigned char         uint8;
  typedef unsigned short int    uint16;
  typedef unsigned long int     uint32;

#elif defined(__PPC__)
   /* ----------------------- Motorola Power PC family ---------------------------*/
   /* The PPC can be programmed to be big or little endian, we assume native */
   /* Big endian */
   #define _STRUCT_HIGH_BIT_FIRST_
   #undef  _STRUCT_LOW_BIT_FIRST_

#ifndef _USING_RTEMS_INCLUDES_
    typedef unsigned char         boolean;
#endif

   typedef signed char           int8;
   typedef short int             int16;
   typedef long int              int32;
   typedef unsigned char         uint8;
   typedef unsigned short int    uint16;
   typedef unsigned long int     uint32;

#elif defined(_m68k_)
   /* ----------------------- Motorola m68k/Coldfire family ---------------------------*/
   /* Big endian */
   #define _STRUCT_HIGH_BIT_FIRST_
   #undef  _STRUCT_LOW_BIT_FIRST_

#ifndef _USING_RTEMS_INCLUDES_
   typedef unsigned char         boolean;
#endif

   typedef signed char           int8;
   typedef short int             int16;
   typedef long int              int32;
   typedef unsigned char         uint8;
   typedef unsigned short int    uint16;
   typedef unsigned long int     uint32;

#else  /* not any of the above */
   #error undefined processor
#endif  /* processor types */


#ifndef NULL              /* pointer to nothing */
   #define NULL ((void *) 0)
#endif

#ifndef TRUE              /* Boolean true */
   #define TRUE (1)
#endif

#ifndef FALSE              /* Boolean false */
   #define FALSE (0)
#endif

/******************************************************************************
**  NOTE:  The RTEMS operating system defines a typedef named 'boolean' (which
**  is an unsigned int).  When compiling a source file with RTEMS header files,
**  one should insert:
**    #define _USING_RTEMS_INCLUDES_
**  before including this file so that the definition of 'boolean' in this file
**  will be skipped.  Otherwise, one will get a compiler warning about multiple
**  declarations.
******************************************************************************/
#define OS_PACK  __attribute__ ((packed))

#endif  /* _common_types_ */
