/******************************************************************************
** File:  os_bspstart.c
**
**
**      This is governed by the NASA Open Source Agreement and may be used, 
**      distributed and modified only pursuant to the terms of that agreement. 
**
**      Copyright � 2004-2006, United States government as represented by the 
**      administrator of the National Aeronautics Space Administration.  
**      All rights reserved. 
**
**
** Purpose:
**   OSAL BSP main entry point.
**
** History:
**   2004/09/23  J.P. Swinski    | Initial version,
**   2004/10/01  P.Kutt          | Replaced OS API task delay with VxWorks functions
**                                 since OS API is initialized later.
**
******************************************************************************/

#define _USING_RTEMS_INCLUDES_


/*
**  Include Files
*/
#include <stdio.h>
#include <stdlib.h>
#include <bsp.h>

/*
** OSAL includes 
*/
#include "common_types.h"
#include "osapi.h"

/*
**  External Declarations
*/
void OS_Application_Startup(void);

/******************************************************************************
**  Function:  OS_BSPMain()
**
**  Purpose:
**    vxWorks/BSP Application entry point.
**
**  Arguments:
**    (none)
**
**  Return:
**    (none)
*/

void OS_BSPMain( void )
{

   /*
   ** Initialize the OS API
   */
   OS_API_Init();

   /*
   ** Call OSAL entry point.    
   */
   OS_Application_Startup();

   
   /*
   ** Just sleep while OSAL code runs
   */
   for ( ;; )
      sleep(1);

}


rtems_task Init(
  rtems_task_argument ignored
)
{

   int status;

   printf("\n\n");
   printf( "\n\n*** RTEMS Info ***\n" );
   printf("%s", _Copyright_Notice );
   printf("%s\n\n", _RTEMS_version );
   printf(" Workspace base %08X\n", (unsigned int)BSP_Configuration.work_space_start );
   printf(" Workspace size %d\n",   BSP_Configuration.work_space_size );
   printf("  Workspace top %08X\n", (unsigned int)BSP_Configuration.work_space_start 
                                    + BSP_Configuration.work_space_size );
   printf("\n");
   printf( "*** End RTEMS info ***\n\n" );

   /*
   ** Create the RTEMS Root file system
   */
   printf("Creating Root file system\n");
   status = rtems_create_root_fs();

   /*
   ** Call OSAL entry point
   */
   OS_BSPMain();

   printf( "*** END OF OSAL TEST ***\n" );

  exit( 0 );
}

/* configuration information */

/*
** RTEMS OS Configuration defintions
*/
#define TASK_INTLEVEL 0
#define CONFIGURE_INIT
#define CONFIGURE_INIT_TASK_ATTRIBUTES	(RTEMS_FLOATING_POINT | RTEMS_PREEMPT | RTEMS_NO_TIMESLICE | RTEMS_ASR | RTEMS_INTERRUPT_LEVEL(TASK_INTLEVEL))
#define CONFIGURE_INIT_TASK_STACK_SIZE	(20*1024)
#define CONFIGURE_INIT_TASK_PRIORITY	120

#define CONFIGURE_MAXIMUM_TASKS                      30 
#define CONFIGURE_MAXIMUM_TIMERS                      3
#define CONFIGURE_MAXIMUM_SEMAPHORES                  8 
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES             30

#define CONFIGURE_MAXIMUM_POSIX_THREADS              CONFIGURE_MAXIMUM_TASKS
#define CONFIGURE_MAXIMUM_POSIX_TIMERS               CONFIGURE_MAXIMUM_TIMERS
#define CONFIGURE_MAXIMUM_POSIX_SEMAPHORES           CONFIGURE_MAXIMUM_SEMAPHORES
#define CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES       CONFIGURE_MAXIMUM_MESSAGE_QUEUES
#define CONFIGURE_MAXIMUM_POSIX_MUTEXES              5 
#define CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES  5 
#define CONFIGURE_MAXIMUM_POSIX_KEYS                 5 
#define CONFIGURE_MAXIMUM_POSIX_QUEUED_SIGNALS       10

#define CONFIGURE_EXECUTIVE_RAM_SIZE	(1024*1024)

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS     100

#define CONFIGURE_MICROSECONDS_PER_TICK              10000

#include <rtems/confdefs.h>


