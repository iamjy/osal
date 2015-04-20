/*
** File: osapi-core-hw.h
**
** Author:  Ezra Yeheksli -Code 582/Raytheon
**
** Purpose: Contains functions prototype definitions and variables declarations
**          the OS Abstraction Layer, Core Hardware library
**
** $Revision: 1.1 $ 
**
** $Date: 2007/10/16 16:14:51EDT $
**
** $Log: osapi-hw-core.h  $
** Revision 1.1 2007/10/16 16:14:51EDT apcudmore 
** Initial revision
** Member added to project d:/mksdata/MKS-OSAL-REPOSITORY/src/os/inc/project.pj
** Revision 1.1 2007/08/24 13:43:23EDT apcudmore 
** Initial revision
** Member added to project d:/mksdata/MKS-CFE-PROJECT/fsw/cfe-core/os/inc/project.pj
** Revision 1.5 2006/01/20 11:56:17EST njyanchik 
** Fixed header file information to match api document
** Revision 1.8  2006/01/20 13:12:53  nyanchik
** Fixed issues with osapiarch.c files, OS_TaskDelay, OS_open, MutSemGetInfo, BinSemGetInfo, IntAttachHandler. Also added the shared memory api,  and a test program to test it (test3).
**
** Revisions for 2.2 include:
** a new scheduler mapper for Linux and OS X
** addition of OS_printf function
** fixed issues that would cause warnings at compile time
**
** Revision 1.7  2005/07/08 16:09:17  acudmore
** Removed FSW specific enumerations and the comment block.
**
** Revision 1.6  2005/07/06 18:04:47  nyanchik
** I had to change all of the memory functions to return int32 from uint32 because I noticed they were returning OS return codes. I also updated the API document.
**
** Revision 1.5  2005/07/05 18:34:55  nyanchik
** fixed issues found in code walkthrogh. Also removed the OS_Info* functions that are going in the BSP
**
** Revision 1.4  2005/06/15 16:43:48  nyanchik
** added extra parenthesis for the .h file # defines
**
**
** Date Written:
**              Nov-24-2003
**    
*/

#ifndef _osapi_hw_core_
#define _osapi_hw_core_

typedef enum 
{
   OS_PROM,
   OS_EEPROM
} os_prom_type ;

int32 OS_PortRead8         (uint32 PortAddress, uint8 *ByteValue);
int32 OS_PortWrite8        (uint32 PortAddress, uint8 ByteValue);
int32 OS_PortRead16        (uint32 PortAddress, uint16 *uint16Value);
int32 OS_PortWrite16       (uint32 PortAddress, uint16 uint16Value);
int32 OS_PortRead32        (uint32 PortAddress, uint32 *uint32Value);
int32 OS_PortWrite32       (uint32 PortAddress, uint32 uint32Value);
int32 OS_PortSetAttributes (uint32 key, uint32 value);
int32 OS_PortGetAttributes (uint32 key, uint32 *value);

/*
** Memory API
*/
int32 OS_MemRead8          (uint32 MemoryAddress, uint8 *ByteValue);
int32 OS_MemWrite8         (uint32 MemoryAddress, uint8 ByteValue);
int32 OS_EepromWrite8      (uint32 MemoryAddress, uint8 ByteValue);
int32 OS_MemRead16         (uint32 MemoryAddress, uint16 *uint16Value);
int32 OS_MemWrite16        (uint32 MemoryAddress, uint16 uint16Value);
int32 OS_EepromWrite16     (uint32 MemoryAddress, uint16 uint16Value);
int32 OS_MemRead32         (uint32 MemoryAddress, uint32 *uint32Value);
int32 OS_MemWrite32        (uint32 MemoryAddress, uint32 uint32Value);
int32 OS_MemCpy            (void *dest, void *src, uint32 n);
int32 OS_MemSet            (void *dest, uint8 value, uint32 n);
int32 OS_MemCheckRange     (uint32 Address, uint32 Size);
int32 OS_MemSetAttributes  (uint32 key, uint32 value);
int32 OS_MemGetAttributes  (uint32 key, uint32 *value);

int32 OS_EepromWriteEnable (void);
int32 OS_EepromWriteDisable(void);
int32 OS_EepromPowerUp     (void);
int32 OS_EepromPowerDown   (void);
int32 OS_EepromWrite32     (uint32 MemoryAddress, uint32 uint32Value);

#endif

