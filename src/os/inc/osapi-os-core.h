/*
** File: osapi-os-core.h
**
** Author:  Ezra Yeheksli -Code 582/Raytheon
**
** Purpose: Contains functions prototype definitions and variables declarations
**          for the OS Abstraction Layer, Core OS module
**
** $Revision: 1.1 $ 
**
** $Date: 2007/10/16 16:14:52EDT $
**
** $Log: osapi-os-core.h  $
** Revision 1.1 2007/10/16 16:14:52EDT apcudmore 
** Initial revision
** Member added to project d:/mksdata/MKS-OSAL-REPOSITORY/src/os/inc/project.pj
** Revision 1.2 2007/09/25 10:32:11EDT apcudmore 
** Added OS task ID field to task_prop structure in OS_TaskGetInfo call.
** Revision 1.1 2007/08/24 13:43:23EDT apcudmore 
** Initial revision
** Member added to project d:/mksdata/MKS-CFE-PROJECT/fsw/cfe-core/os/inc/project.pj
** Revision 1.21 2007/07/09 12:18:42EDT apcudmore 
** Added FPU mask functions to the OSAPI.
** vxWorks 6 version is functional, others are stubs.
** Revision 1.20 2007/04/24 11:36:41EDT njyanchik 
** I Implemented the followiing fixes:
** Items 1,2,3 are for vxworks 5.5 , so we don't have to change that at all
** Item 4: fixed by adding a check for the length of the volume name (volname) on entry to the function
** Items 5,6, fixed by making the final strcpy a strncpy in OS_NameChange to make sure the string returned is less than or equal to the maximum number of bytes.
** Item 7: fixed by making the first strcpy in OS_NameChange a strncpy to prevent the input from being too long. This way the string length of LocalName won't be too long to use in line 704.
** Item 9: Fixed by making the error number parameter an int32 instead of a uint32
** Revision 1.19 2007/04/19 11:43:25EDT njyanchik 
** I mistyped a prototype in a header file
** Revision 1.18 2007/04/05 07:43:40EDT njyanchik 
** The OS_TaskExit APIs were added to all OS's
** Revision 1.17 2007/04/04 08:11:40EDT njyanchik 
** This CP changes the names of the previous APIs from OS_IntEnableAll/ OS_IntDisableAll to the 
** more acurate OS_IntUnlock/OS_IntLock.
** 
** It also adds in 2 new API's: OS_IntEnable and OS_IntDisable for disabling specific interrupts
** Revision 1.16 2007/03/29 07:58:22EST njyanchik 
** A new API, OS_SetLocalTime, has been added to give the user the ability to set the local clock.
** This function is the compliment of OS_GetLocalTime.
** Revision 1.15 2007/03/20 09:31:25EST njyanchik 
** I added a counting semaphore implementation to all OS's. This also included removing the #define
** OS_MAX_SEMAPHORES and creating two new ones, OS_MAX_BIN_SEMAPHORES and
** OS_MAX_COUNT_SEMAPHORES in osconfig.h. Also, cfe_es_shell was changed in order to
** accommodate the chanes to the #defines.
** Revision 1.14 2007/03/15 11:16:53EST njyanchik 
** I changed the interrupt enable/disable pair to use a lock key that records the previous state
** of the interrupts before disabling, and then use that key to re-enable the interrupts.
** The CFE core applications that use this pair were also fixed for this API change.
** Revision 1.13 2006/10/16 09:29:07EDT njyanchik 
** This CP adds the  OS_BinSemFlush API. This also necessitated changing the
** semaphore implementation of RTEMS to allow for the semaphore flushing 
** function.
** 
**  
*/

#ifndef _osapi_core_
#define _osapi_core_

#include "osapi.h"
#include "stdarg.h"   /* for va_list */

/*difines constants for OS_BinSemCreate for state of semaphore  */
#define OS_SEM_FULL     1
#define OS_SEM_EMPTY    0

/* #define for enabling floating point operations on a task*/
#define OS_FP_ENABLED 1

/*  tables for the properties of objects */

/*tasks */
typedef struct
{
    char name [OS_MAX_API_NAME];
    uint32 creator;
    uint32 stack_size;
    uint32 priority;
    uint32 OStask_id;
}OS_task_prop_t;
    
/* queues */
typedef struct
{
    char name [OS_MAX_API_NAME];
    uint32 creator;
}OS_queue_prop_t;

/* Binary Semaphores */
typedef struct
{                     
    char name [OS_MAX_API_NAME];
    uint32 creator;
}OS_bin_sem_prop_t;

/* Counting Semaphores */
typedef struct
{                     
    char name [OS_MAX_API_NAME];
    uint32 creator;
}OS_count_sem_prop_t;

/* Mutexes */
typedef struct
{
    char name [OS_MAX_API_NAME];
    uint32 creator;
}OS_mut_sem_prop_t;


/* struct for OS_GetLocalTime() */

typedef struct 
{ 
    uint32 seconds; 
    uint32 microsecs;
}OS_time_t; 


/* This typedef is for the OS_GetErrorName function, to ensure
 * everyone is making an array of the same length */

typedef char os_err_name_t[35];

/*
** Exported Functions
*/

/*
** Initialization of API
*/

void OS_API_Init (void);


/*
** Task API
*/

int32 OS_TaskCreate            (uint32 *task_id, const char *task_name, 
                                const void *function_pointer,
                                const uint32 *stack_pointer, 
                                uint32 stack_size,
                                uint32 priority, uint32 flags);

int32 OS_TaskDelete            (uint32 task_id); 
void OS_TaskExit               (void);
int32 OS_TaskDelay             (uint32 millisecond);
int32 OS_TaskSetPriority       (uint32 task_id, uint32 new_priority);
int32 OS_TaskRegister          (void);
uint32 OS_TaskGetId            (void);
int32 OS_TaskGetIdByName       (uint32 *task_id, const char *task_name);
int32 OS_TaskGetInfo           (uint32 task_id, OS_task_prop_t *task_prop);          

/* 
** Release 3 note:
** Task Loader function
** This function is used to load a module into RAM from a file system. It returns the 
start address which should be passed into the OS_TaskCreate function. The auto_start 
parameter will allow the modules "startup code" to run automatically. This will help keep
compatability with Posix process based systems.
*/
int32 OS_TaskLoad              (char *path, uint32 *start_address, uint32 auto_start, 
                                uint32 flags);

/*
** Message Queue API
*/

/*
** Queue Create now has the Queue ID returned to the caller.
*/
int32 OS_QueueCreate           (uint32 *queue_id, const char *queue_name,
                                uint32 queue_depth, uint32 data_size, uint32 flags);
int32 OS_QueueDelete           (uint32 queue_id);
int32 OS_QueueGet              (uint32 queue_id, void *data, uint32 size, 
                                uint32 *size_copied, int32 timeout);
int32 OS_QueuePut              (uint32 queue_id, void *data, uint32 size, 
                                uint32 flags);
int32 OS_QueueGetIdByName      (uint32 *queue_id, const char *queue_name);
int32 OS_QueueGetInfo          (uint32 queue_id, OS_queue_prop_t *queue_prop);

/*
** Semaphore API
*/

int32 OS_BinSemCreate          (uint32 *sem_id, const char *sem_name, 
                                uint32 sem_initial_value, uint32 options);
int32 OS_BinSemFlush            (uint32 sem_id);
int32 OS_BinSemGive            (uint32 sem_id);
int32 OS_BinSemTake            (uint32 sem_id);
int32 OS_BinSemTimedWait       (uint32 sem_id, uint32 msecs);
int32 OS_BinSemDelete          (uint32 sem_id);
int32 OS_BinSemGetIdByName     (uint32 *sem_id, const char *sem_name);
int32 OS_BinSemGetInfo         (uint32 sem_id, OS_bin_sem_prop_t *bin_prop);

int32 OS_CountSemCreate          (uint32 *sem_id, const char *sem_name, 
                                uint32 sem_initial_value, uint32 options);
int32 OS_CountSemGive            (uint32 sem_id);
int32 OS_CountSemTake            (uint32 sem_id);
int32 OS_CountSemTimedWait       (uint32 sem_id, uint32 msecs);
int32 OS_CountSemDelete          (uint32 sem_id);
int32 OS_CountSemGetIdByName     (uint32 *sem_id, const char *sem_name);
int32 OS_CountSemGetInfo         (uint32 sem_id, OS_count_sem_prop_t *count_prop);

/*
** Mutex API
*/

int32 OS_MutSemCreate           (uint32 *sem_id, const char *sem_name, uint32 options);
int32 OS_MutSemGive             (uint32 sem_id);
int32 OS_MutSemTake             (uint32 sem_id);
int32 OS_MutSemDelete           (uint32 sem_id);  
int32 OS_MutSemGetIdByName      (uint32 *sem_id, const char *sem_name); 
int32 OS_MutSemGetInfo          (uint32 sem_id, OS_mut_sem_prop_t *mut_prop);

/*
** OS Time/Tick related API
*/

int32 OS_Milli2Ticks           (uint32 milli_seconds);
int32 OS_Tick2Micros           (void);
int32  OS_GetLocalTime         (OS_time_t *time_struct);
int32  OS_SetLocalTime         (OS_time_t *time_struct);  

/*
** Exception API
*/

int32 OS_ExcAttachHandler      (uint32 ExceptionNumber, 
                                void (*ExceptionHandler)(uint32, uint32 *,uint32), 
                                int32 parameter);
int32 OS_ExcEnable             (int32 ExceptionNumber);
int32 OS_ExcDisable            (int32 ExceptionNumber);

/*
** Floating Point Unit API
*/

int32 OS_FPUExcAttachHandler   (uint32 ExceptionNumber, void * ExceptionHandler ,
                                 int32 parameter);
int32 OS_FPUExcEnable          (int32 ExceptionNumber);
int32 OS_FPUExcDisable         (int32 ExceptionNumber);
int32 OS_FPUExcSetMask         (uint32 mask);
int32 OS_FPUExcGetMask         (uint32 *mask);

/*
** Interrupt API
*/

int32 OS_IntAttachHandler  (uint32 InterruptNumber, void * InerruptHandler , int32 parameter);
int32 OS_IntUnlock      (int32 IntLevel);
int32 OS_IntLock        (void);

int32 OS_IntEnable      (int32 Level);
int32 OS_IntDisable     (int32 Level);

int32 OS_IntSetMask     (uint32 mask);
int32 OS_IntGetMask     (uint32 *mask);
int32 OS_IntAck         (int32 InterruptNumber);

/*
** Shared memory API 
*/
int32 OS_ShMemInit          (void);
int32 OS_ShMemCreate        (uint32 *Id, uint32 NBytes, char* SegName);
int32 OS_ShMemSemTake       (uint32 Id);
int32 OS_ShMemSemGive       (uint32 Id);
int32 OS_ShMemAttach        (uint32 * Address, uint32 Id);
int32 OS_ShMemGetIdByName   (uint32 *ShMemId, const char *SegName );

/*
** API for a useful debugging function
*/

int32 OS_GetErrorName      (int32 error_num, os_err_name_t* err_name);


/* Abstraction for printf statements */
void OS_printf( const char *string, ...);

#endif
