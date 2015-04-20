/*
** File: osapi.h
**
** Author:  Alan Cudmore - Code 582
**
** Purpose: Contains functions prototype definitions and variables declarations
**          for the OS Abstraction Layer, Core OS module
**
** $Revision: 1.1 $
**
** $Date: 2007/10/16 16:14:52EDT $
**
** $Log: osapi.h  $
** Revision 1.1 2007/10/16 16:14:52EDT apcudmore 
** Initial revision
** Member added to project d:/mksdata/MKS-OSAL-REPOSITORY/src/os/inc/project.pj
** Revision 1.2 2007/09/28 15:46:49EDT rjmcgraw 
** Updated version numbers to 5.0
** Revision 1.1 2007/08/24 13:43:25EDT apcudmore
** Initial revision
** Member added to project d:/mksdata/MKS-CFE-PROJECT/fsw/cfe-core/os/inc/project.pj
** Revision 1.9.1.1 2007/05/21 08:58:51EDT njyanchik
** The trunk version number has been updated to version 0.0
** Revision 1.9 2006/06/12 10:20:07EDT rjmcgraw
** Updated OS_MINOR_VERSION from 3 to 4
** Revision 1.8 2006/02/03 09:30:45EST njyanchik
** Changed version number to 2.3
** Revision 1.7 2006/01/20 11:56:16EST njyanchik
** Fixed header file information to match api document
** Revision 1.15  2005/11/09 13:35:49  nyanchik
** Revisions for 2.2 include:
** a new scheduler mapper for Linux and OS X
** addition of OS_printf function
** fixed issues that would cause warnings at compile time
**
**
*/

#ifndef _osapi_
#define _osapi_

#include "common_types.h"

#define OS_SUCCESS                     (0)
#define OS_ERROR                       (-1)
#define OS_INVALID_POINTER             (-2)
#define OS_ERROR_ADDRESS_MISALIGNED    (-3)
#define OS_ERROR_TIMEOUT               (-4)
#define OS_INVALID_INT_NUM             (-5)
#define OS_SEM_FAILURE                 (-6)
#define OS_SEM_TIMEOUT                 (-7)
#define OS_QUEUE_EMPTY                 (-8)
#define OS_QUEUE_FULL                  (-9)
#define OS_QUEUE_TIMEOUT               (-10)
#define OS_QUEUE_INVALID_SIZE          (-11)
#define OS_QUEUE_ID_ERROR              (-12)
#define OS_ERR_NAME_TOO_LONG           (-13)
#define OS_ERR_NO_FREE_IDS             (-14)
#define OS_ERR_NAME_TAKEN              (-15)
#define OS_ERR_INVALID_ID              (-16)
#define OS_ERR_NAME_NOT_FOUND          (-17)
#define OS_ERR_SEM_NOT_FULL            (-18)
#define OS_ERR_INVALID_PRIORITY        (-19)

#define OS_MAJOR_VERSION (2)
#define OS_MINOR_VERSION (10)

/*
** Defines for Queue Timeout parameters
*/
#define OS_PEND   (0)
#define OS_CHECK (-1)
/*
** Include the configuration file
*/
#include "osconfig.h"

/*
** Include the OS API modules
*/
#include "osapi-os-core.h"
#include "osapi-os-filesys.h"
#include "osapi-os-net.h"

/*
** Include the Hardware API modules
*/
#include "osapi-hw-core.h"
#include "osapi-hw-analog.h"
#include "osapi-hw-pci.h"


#endif

