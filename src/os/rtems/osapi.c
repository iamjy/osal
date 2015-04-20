/*
** File   :	osapi.c
**
** Author :	Ezra Yeheskeli
**
** Purpose: 
**	   This file  contains some of the OS APIs abstraction layer.It 
**     contains those APIs that call the  OS. In this case the OS is the Rtems OS.
**
**  $Date: 2007/10/16 16:14:58EDT $
**  $Revision: 1.1 $
**  $Log: osapi.c  $
**  Revision 1.1 2007/10/16 16:14:58EDT apcudmore 
**  Initial revision
**  Member added to project d:/mksdata/MKS-OSAL-REPOSITORY/src/os/rtems/project.pj
**  Revision 1.31 2007/09/25 10:32:09EDT apcudmore 
**  Added OS task ID field to task_prop structure in OS_TaskGetInfo call.
**  Revision 1.30 2007/09/24 10:35:57EDT apcudmore 
**  Removed duplicate function: OS_GetTimebase
**  Revision 1.29 2007/07/09 12:18:39EDT apcudmore 
**  Added FPU mask functions to the OSAPI.
**  vxWorks 6 version is functional, others are stubs.
**  Revision 1.28 2007/07/05 14:59:01EDT njyanchik 
**  I sem-protected a bigger block in each of the create functions, and set the free flag (or IsValid 
**  flag) to false before sem protection ends. That way another task can't get the same ID. This
**  was done for the OS_open, OS_creat, and all OS_*Create calls.
**  Revision 1.27 2007/05/08 09:14:52EDT njyanchik 
**  The previous implementation of semaphores in linux (sem_open/ sem_close) 
**  is not supported on Cygwin. 
**  The calls need to use the sem_init/sem_destroy api's
**  Revision 1.26 2007/04/30 11:01:39EDT njyanchik 
**  The checks in vxworks and rtems for making sure a semaphore (or mutex) was not taken before
**  deleting were still in the code. They have now been removed.
**  Revision 1.25 2007/04/24 11:36:39EDT njyanchik 
**  I Implemented the followiing fixes:
**  Items 1,2,3 are for vxworks 5.5 , so we don't have to change that at all
**  Item 4: fixed by adding a check for the length of the volume name (volname) on entry to the function
**  Items 5,6, fixed by making the final strcpy a strncpy in OS_NameChange to make sure the string returned is less than or equal to the maximum number of bytes.
**  Item 7: fixed by making the first strcpy in OS_NameChange a strncpy to prevent the input from being too long. This way the string length of LocalName won't be too long to use in line 704.
**  Item 9: Fixed by making the error number parameter an int32 instead of a uint32
**  Revision 1.24 2007/04/05 07:43:42EDT njyanchik 
**  The OS_TaskExit APIs were added to all OS's
**  Revision 1.23 2007/04/04 08:11:42EDT njyanchik 
**  This CP changes the names of the previous APIs from OS_IntEnableAll/ OS_IntDisableAll to the 
**  more acurate OS_IntUnlock/OS_IntLock.
**  
**  It also adds in 2 new API's: OS_IntEnable and OS_IntDisable for disabling specific interrupts
**  Revision 1.22 2007/03/29 13:42:37EST njyanchik 
**  I made a syntax error in OS_SetLocalTime. It has been fixed.
**  Revision 1.21 2007/03/29 07:58:24EST njyanchik 
**  A new API, OS_SetLocalTime, has been added to give the user the ability to set the local clock.
**  This function is the compliment of OS_GetLocalTime.
**  Revision 1.20 2007/03/20 09:28:09EST njyanchik 
**  I added a counting semaphore implementation to all OS's. This also included removing the #define
**  OS_MAX_SEMAPHORES and creating two new ones, OS_MAX_BIN_SEMAPHORES and
**  OS_MAX_COUNT_SEMAPHORES in osconfig.h. Also, cfe_es_shell was changed in order to
**  accommodate the chanes to the #defines.
**  Revision 1.19 2007/03/15 11:16:50EST njyanchik 
**  I changed the interrupt enable/disable pair to use a lock key that records the previous state
**  of the interrupts before disabling, and then use that key to re-enable the interrupts.
**  The CFE core applications that use this pair were also fixed for this API change.
**  Revision 1.18 2007/02/28 10:24:51EST njyanchik 
**  There was an issue with the type declaration of the retuyrn value of the function. It was resolved.
**  Revision 1.17 2007/02/27 15:22:08EST njyanchik 
**  This CP has the initial import of the new file descripor table mechanism
**  Revision 1.16 2006/10/16 09:29:10EDT njyanchik 
**  This CP adds the  OS_BinSemFlush API. This also necessitated changing the
**  semaphore implementation of RTEMS to allow for the semaphore flushing 
**  function.
**  
**  Also with this change is the change to the ES startup. It was using OS_BinSemGive
**  because that had technically been implementing the semaphore flushing 
**  functionality. Since there is now a specified SemFlush, it was changed to 
**  used that call instead.
**  Revision 1.15 2006/09/11 14:25:00GMT-05:00 njyanchik 
**  There is now a new option when calling OS_TaskCreate a value of 
**  OS_FP_ENABLED as the 'flags' parameter will enable floating point operations.
**  
**  The #define was created in os-osapi-core.h
**  Revision 1.14 2006/08/25 13:55:12EDT njyanchik 
**  To make sure we are only checking against used names, I added a check to see
**  if the entry we are looking at is being used.This affects all OS's in the cFE
**  Revision 1.13 2006/06/21 07:58:26EDT njyanchik 
**  Changes occured in every osapi.c file and in the WriteToSysLog function in ES.
**  Revision 1.12 2006/06/08 13:32:44EDT apcudmore 
**  Updated osapi.c  to fix a typo with the semaphore table, the OS_QueueGet and the Interrupt enable/disable functions
**  Revision 1.11 2006/02/27 16:53:03GMT njyanchik 
**  I removed references to the circular buffer, as well as changed the osx/ linux/ rtems versions 
**  to not have a utility task at all.
**  Revision 1.10 2006/02/03 09:06:55EST njyanchik 
**  even though they all say "no differences, I'm copy and pasting the files from the osal v2.3 into the cfe 
**  to make sure there is a consitant version
*/
/****************************************************************************************
                                    INCLUDE FILES
****************************************************************************************/
#define _USING_RTEMS_INCLUDES_

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h> /* checking ETIMEDOUT */
#include <rtems.h>
#include <semaphore.h> /*sem_t */

/*
** User defined include files
*/
#include "common_types.h"
#include "osapi.h"

#include "stdlib.h"
#include "string.h"

/****************************************************************************************
                                     DEFINES
****************************************************************************************/

#define RTEMS_INT_LEVEL_ENABLE_ALL 0
#define RTEMS_INT_LEVEL_DISABLE_ALL 7

#define MAX_PRIORITY 255
#define UNINITIALIZED 0 
uint32  OS_CompAbsDelayedTime( uint32 milli_second , struct timespec * tm);
uint32 OS_FindCreator(void);
/****************************************************************************************
                                   GLOBAL DATA
****************************************************************************************/



/*  tables for the properties of objects */

/*tasks */
typedef struct
{
    int free;
    rtems_id id;
    char name [OS_MAX_API_NAME];
    int creator;
    uint32 stack_size;
    uint32 priority;
}OS_task_record_t;
    
/* queues */
typedef struct
{
    int free;
    rtems_id id;
    char name [OS_MAX_API_NAME];
    int creator;
}OS_queue_record_t;

/* Binary Semaphores */
typedef struct
{
    int free;
    sem_t id;
    char name [OS_MAX_API_NAME];
    int creator;
}OS_bin_sem_record_t;

/* Counting Semaphores */
typedef struct
{
    int free;
    sem_t id;
    char name [OS_MAX_API_NAME];
    int creator;
}OS_count_sem_record_t;
/* Mutexes */
typedef struct
{
    int free;
    pthread_mutex_t id;
    char name [OS_MAX_API_NAME];
    int creator;
}OS_mut_sem_record_t;

void             *OS_task_key = 0;         /* this is what rtems wants! */

/* Tables where the OS object information is stored */
OS_task_record_t    OS_task_table          [OS_MAX_TASKS];
OS_queue_record_t   OS_queue_table         [OS_MAX_QUEUES];
OS_bin_sem_record_t OS_bin_sem_table       [OS_MAX_BIN_SEMAPHORES];
OS_count_sem_record_t OS_count_sem_table   [OS_MAX_COUNT_SEMAPHORES];
OS_mut_sem_record_t OS_mut_sem_table       [OS_MAX_MUTEXES];

sem_t OS_task_table_sem;
sem_t OS_queue_table_sem;
sem_t OS_bin_sem_table_sem;
sem_t OS_mut_sem_table_sem;
sem_t OS_count_sem_table_sem;


/****************************************************************************************
                                INITIALIZATION FUNCTION
****************************************************************************************/

/*---------------------------------------------------------------------------------------
   Name: OS_API_Init

   Purpose: Initialize the tables that the OS API uses to keep track of information
            about objects

   returns: nothing
---------------------------------------------------------------------------------------*/


void OS_API_Init(void)
{
    int i;

    /* Initialize Task Table */

   
    for(i = 0; i < OS_MAX_TASKS; i++)
    {
        OS_task_table[i].free        = TRUE;
        OS_task_table[i].id          = UNINITIALIZED;
        OS_task_table[i].creator     = UNINITIALIZED;
        strcpy(OS_task_table[i].name,"");    
    }

    /* Initialize Message Queue Table */

    for(i = 0; i < OS_MAX_QUEUES; i++)
    {
        OS_queue_table[i].free        = TRUE;
        OS_queue_table[i].id          = UNINITIALIZED;
        OS_queue_table[i].creator     = UNINITIALIZED;
        strcpy(OS_queue_table[i].name,""); 
    }

    /* Initialize Binary Semaphore Table */

    for(i = 0; i < OS_MAX_BIN_SEMAPHORES; i++)
    {
        OS_bin_sem_table[i].free        = TRUE;
        OS_bin_sem_table[i].id          = UNINITIALIZED;
        OS_bin_sem_table[i].creator     = UNINITIALIZED;
        strcpy(OS_bin_sem_table[i].name,"");
    }

    /* Initialize Counting Semaphore Table */

    for(i = 0; i < OS_MAX_COUNT_SEMAPHORES; i++)
    {
        OS_count_sem_table[i].free        = TRUE;
        OS_count_sem_table[i].id          = UNINITIALIZED;
        OS_count_sem_table[i].creator     = UNINITIALIZED;
        strcpy(OS_count_sem_table[i].name,"");
    }


    /* Initialize Mutex Semaphore Table */

    for(i = 0; i < OS_MAX_MUTEXES; i++)
    {
        OS_mut_sem_table[i].free        = TRUE;
        OS_mut_sem_table[i].id          = UNINITIALIZED;
        OS_mut_sem_table[i].creator     = UNINITIALIZED;
        strcpy(OS_mut_sem_table[i].name,"");
    }
    

	sem_init(&(OS_task_table_sem ),
		0 ,                   /* no process sharing */
		OS_SEM_FULL ) ; /* initial value of sem_initial value */

	sem_init(&(OS_queue_table_sem ),
		0 ,                   /* no process sharing */
		OS_SEM_FULL ) ; /* initial value of sem_initial value */

    sem_init(&(OS_bin_sem_table_sem ),
		0 ,                   /* no process sharing */
		OS_SEM_FULL ) ; /* initial value of sem_initial value */
    
    sem_init(&(OS_count_sem_table_sem ),
		0 ,                   /* no process sharing */
		OS_SEM_FULL ) ; /* initial value of sem_initial value */

    sem_init(&(OS_mut_sem_table_sem ),
		0 ,                   /* no process sharing */
		OS_SEM_FULL ) ; /* initial value of sem_initial value */

    OS_FS_Init();

       
       return;
} /* end OS_API_Init */


/****************************************************************************************
                                    TASK API
****************************************************************************************/

/*---------------------------------------------------------------------------------------
   Name: OS_TaskCreate

   Purpose: Creates a task and starts running it.

   returns: OS_INVALID_POINTER if any of the necessary pointers are NULL
            OS_ERR_NAME_TOO_LONG if the name of the task is too long to be copied
            OS_ERR_INVALID_PRIORITY if the priority is bad
            OS_ERR_NO_FREE_IDS if there can be no more tasks created
            OS_ERR_NAME_TAKEN if the name specified is already used by a task
            OS_ERROR if the operating system calls fail
            OS_SUCCESS if success
            
    NOTES: task_id is passed back to the user as the ID. stack_pointer is usually null.
           Flags are unused at this point.


---------------------------------------------------------------------------------------*/


int32 OS_TaskCreate (uint32 *task_id, const char *task_name,const void *function_pointer,
                      const uint32 *stack_pointer, uint32 stack_size, uint32 priority, 
                      uint32 flags)
{
    uint32 possible_taskid;
    uint32 i;
	rtems_status_code  status;
	rtems_name         r_name;
	rtems_mode         r_mode;
	rtems_attribute    r_attributes;


    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */
    
    /* Check for NULL pointers */
    
    if( (task_name == NULL) || (function_pointer == NULL) || (task_id == NULL) )
        return OS_INVALID_POINTER;
    
    if (strlen(task_name) >= OS_MAX_API_NAME)
            return OS_ERR_NAME_TOO_LONG;

    /* Check for bad priority */

    if (priority > MAX_PRIORITY)
        return OS_ERR_INVALID_PRIORITY;

    
    /* Check Parameters */
    sem_wait( &(OS_task_table_sem));
    for(possible_taskid = 0; possible_taskid < OS_MAX_TASKS; possible_taskid++)
    {
        if (OS_task_table[possible_taskid].free == TRUE)
        {
            break;
        }
    }

    /* Check to see if the id is out of bounds */
    
    if( possible_taskid >= OS_MAX_TASKS || OS_task_table[possible_taskid].free != TRUE)
    {    
        sem_post( &(OS_task_table_sem));
        return OS_ERR_NO_FREE_IDS;
    }

    /* Check to see if the name is already taken */

    for (i = 0; i < OS_MAX_TASKS; i++)
    {
        if ((OS_task_table[i].free == FALSE) &&
           ( strcmp((char*)task_name, OS_task_table[i].name) == 0)) 
        {        
            sem_post( &(OS_task_table_sem));
            return OS_ERR_NAME_TAKEN;
        }
    }
    /* Set the possible task Id to not free so that
     * no other task can try to use it */

    OS_task_table[possible_taskid].free  = FALSE;
    
    sem_post( &(OS_task_table_sem));
    
	r_name = rtems_build_name('O','S',' ',' ');
	r_mode = RTEMS_PREEMPT | RTEMS_NO_ASR | RTEMS_NO_TIMESLICE | RTEMS_INTERRUPT_LEVEL(0);

    /* see if the user wants floating point enabled. If 
     * so, then se the correct option.
     */
    if (flags == OS_FP_ENABLED)
    {
        r_attributes = RTEMS_FLOATING_POINT | RTEMS_LOCAL;
    }
    else
    {
        r_attributes = RTEMS_LOCAL;
    }
	
    status = rtems_task_create(
			     r_name,
				 priority,
				 stack_size,
				 r_mode,
				 r_attributes,
				 &OS_task_table[possible_taskid].id); 
    
    /* check if task_create failed */

	if (status != RTEMS_SUCCESSFUL )
    {       
        sem_wait( &(OS_task_table_sem));
        OS_task_table[possible_taskid].free  = TRUE;
        sem_post( &(OS_task_table_sem));
		return OS_ERROR;
    } 

	/* will place the task in 'ready for scheduling' state */
	status = rtems_task_start (OS_task_table[possible_taskid].id, /*rtems task id*/
			     function_pointer,                                /*task entry point */
				 0 );                                             /* passed argument  */
	
	if (status != RTEMS_SUCCESSFUL )
    {		
        sem_wait( &(OS_task_table_sem));
        OS_task_table[possible_taskid].free  = TRUE;
        sem_post( &(OS_task_table_sem));
		return OS_ERROR;		
    }
    
    /* Set the task_id to the id that was found available 
       Set the name of the task, the stack size, and priority */
    
    *task_id = possible_taskid;
    
    strcpy(OS_task_table[*task_id].name, (char*) task_name);
   
    
    /* this Id no longer free */
    sem_wait( &(OS_task_table_sem));
    OS_task_table[*task_id].creator = OS_FindCreator();
    OS_task_table[*task_id].stack_size = stack_size;
    OS_task_table[*task_id].priority = priority;
    sem_post( &(OS_task_table_sem));
    return OS_SUCCESS;
    
} /* end OS_TaskCreate */


/*--------------------------------------------------------------------------------------
     Name: OS_TaskDelete

    Purpose: Deletes the specified Task and removes it from the OS_task_table.

    returns: OS_ERR_INVALID_ID if the ID given to it is invalid
             OS_ERROR if the OS delete call fails
             OS_SUCCESS if success
---------------------------------------------------------------------------------------*/

int32 OS_TaskDelete (uint32 task_id)
{    
    /* Check to see if the task_id given is valid */

    if (task_id >= OS_MAX_TASKS || OS_task_table[task_id].free == TRUE)
            return OS_ERR_INVALID_ID;

    /* Try to delete the task */

    if (rtems_task_delete(OS_task_table[task_id].id) != RTEMS_SUCCESSFUL)
		{
			return OS_ERROR;
		}    
    
    /*
     * Now that the task is deleted, remove its 
     * "presence" in OS_task_table
    */

    sem_wait( &(OS_task_table_sem));
    OS_task_table[task_id].free = TRUE;
    OS_task_table[task_id].id = UNINITIALIZED;
    strcpy(OS_task_table[task_id].name, "");
    OS_task_table[task_id].creator = UNINITIALIZED;
    OS_task_table[task_id].stack_size = UNINITIALIZED;
    OS_task_table[task_id].priority = UNINITIALIZED;
    sem_post( &(OS_task_table_sem));
    
    return OS_SUCCESS;
    
}/* end OS_TaskDelete */
/*--------------------------------------------------------------------------------------
     Name: OS_TaskExit

    Purpose: Exits the calling task and removes it from the OS_task_table.

    returns: Nothing 
---------------------------------------------------------------------------------------*/

void OS_TaskExit()
{
    uint32 task_id;

    task_id = OS_TaskGetId();

    sem_wait( &(OS_task_table_sem));
    
    OS_task_table[task_id].free = TRUE;
    OS_task_table[task_id].id = UNINITIALIZED;
    strcpy(OS_task_table[task_id].name, "");
    OS_task_table[task_id].creator = UNINITIALIZED;
    OS_task_table[task_id].stack_size = UNINITIALIZED;
    OS_task_table[task_id].priority = UNINITIALIZED;
    
    sem_post( &(OS_task_table_sem));

    rtems_task_delete(RTEMS_SELF);

}/*end OS_TaskExit */

/*---------------------------------------------------------------------------------------
   Name: OS_TaskDelay

   Purpose: Delay a task for specified amount of milliseconds

   returns: OS_ERROR if sleep fails
            OS_SUCCESS if success
---------------------------------------------------------------------------------------*/

int32 OS_TaskDelay (uint32 milli_second)
{
	struct timespec tm ;
	
	/* compute the second part */
	tm.tv_sec  = (time_t) (milli_second / 1000) ;
	/* take the residue and and convert to nano seconds */
	tm.tv_nsec = (milli_second % 1000) * 1000000 ;
	
	if( nanosleep(&tm, NULL) == 0)
    {
		return(OS_SUCCESS) ;
    }
	else
    {
		return(OS_ERROR) ;
    }
}/* end OS_TaskDelay */


/*---------------------------------------------------------------------------------------
   Name: OS_TaskSetPriority

   Purpose: Sets the given task to a new priority

    returns: OS_ERR_INVALID_ID if the ID passed to it is invalid
             OS__ERR_INVALID_PRIORITY if the priority is greater than the max 
             allowed
             OS_ERROR if the OS call to change the priority fails
             OS_SUCCESS if success
---------------------------------------------------------------------------------------*/


int32 OS_TaskSetPriority (uint32 task_id, uint32 new_priority)
{
    rtems_task_priority old_pri;
    
    /* Check Parameters */

    if(task_id >= OS_MAX_TASKS || OS_task_table[task_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    if (new_priority > MAX_PRIORITY)
        return OS_ERR_INVALID_PRIORITY;
    
    /* Set RTEMS Task Priority */

    if (rtems_task_set_priority(OS_task_table[task_id].id, new_priority, &old_pri)
       != RTEMS_SUCCESSFUL )
            return OS_ERROR;

    return OS_SUCCESS;

}/* end OS_TaskSetPriority */


/*---------------------------------------------------------------------------------------
   Name: OS_TaskRegister
  
   Purpose: Registers the calling task id with the task by adding the var to the tcb
  			It searches the OS_task_table to find the task_id corresponding to the t            cb_id
            
   Returns: OS_ERR_INVALID_ID if there the specified ID could not be found
            OS_ERROR if the OS call fails
            OS_SUCCESS if success
---------------------------------------------------------------------------------------*/

int32 OS_TaskRegister (void)
{
	rtems_id          rtems_task_id;
	rtems_status_code rtems_status;
	int 	          i;
	uint32	          task_id;
	
	/* 
	** Get RTEMS Task Id
	*/
	rtems_status = rtems_task_ident(RTEMS_SELF, 0, &rtems_task_id);
	if ( rtems_status != RTEMS_SUCCESSFUL )
    {		
		return(OS_ERROR);
    }
	
    for(i = 0; i < OS_MAX_TASKS; i++)
    {
        if(OS_task_table[i].id == rtems_task_id)
            break;
    }

    task_id = i;

    if(task_id >= OS_MAX_TASKS)
    {
        return OS_ERR_INVALID_ID;
    }

    /* Add RTEMS Task Variable */

	rtems_status = rtems_task_variable_add(
		rtems_task_id,        /* rtems task id */
		(void *)&OS_task_key, /* the task variable of type :void ** task_variable */
		NULL);                /* no function destructure is specified */
	
	if ( rtems_status != RTEMS_SUCCESSFUL )
		return(OS_ERROR);
    
	 OS_task_key = task_id;
    
    return OS_SUCCESS;

}/* end OS_TaskRegister */

/*---------------------------------------------------------------------------------------
   Name: OS_TaskGetId

   Purpose: This function returns the #defined task id of the calling task

   Notes: The OS_task_key is initialized by the task switch if AND ONLY IF the 
          OS_task_key has been registered via OS_TaskRegister(..).  If this is not 
          called prior to this call, the value will be old and wrong.
---------------------------------------------------------------------------------------*/
uint32 OS_TaskGetId (void)
{
    return (uint32) OS_task_key;

}/* end OS_TaskGetId */

/*--------------------------------------------------------------------------------------
    Name: OS_TaskGetIdByName

    Purpose: This function tries to find a task Id given the name of a task

    Returns: OS_INVALID_POINTER if the pointers passed in are NULL
             OS_ERR_NAME_TOO_LONG if th ename to found is too long to begin with
             OS_ERR_NAME_NOT_FOUND if the name wasn't found in the table
             OS_SUCCESS if SUCCESS
---------------------------------------------------------------------------------------*/

int32 OS_TaskGetIdByName (uint32 *task_id, const char *task_name)
{
    uint32 i;

    if (task_id == NULL || task_name == NULL)
        return OS_INVALID_POINTER;
    
    /* we don't want to allow names too long because they won't be found at all */
    
    if (strlen(task_name) >= OS_MAX_API_NAME)
            return OS_ERR_NAME_TOO_LONG;

    for (i = 0; i < OS_MAX_TASKS; i++)
    {
        if( (OS_task_table[i].free != TRUE) &&
          (strcmp(OS_task_table[i].name,(char*) task_name) == 0 ))
        {
            *task_id = i;
            return OS_SUCCESS;
        }
    }
    
    /* The name was not found in the table,
     *  or it was, and the task_id isn't valid anymore */
    
    return OS_ERR_NAME_NOT_FOUND;

}/* end OS_TaskGetIdByName */         

/*---------------------------------------------------------------------------------------
    Name: OS_TaskGetInfo

    Purpose: This function will pass back a pointer to structure that contains 
             all of the relevant info (creator, stack size, priority, name) about the 
             specified task. 

    Returns: OS_ERR_INVALID_ID if the ID passed to it is invalid
             OS_INVALID_POINTER if the task_prop pointer is NULL
             OS_SUCCESS if it copied all of the relevant info over
 
---------------------------------------------------------------------------------------*/
int32 OS_TaskGetInfo (uint32 task_id, OS_task_prop_t *task_prop)  
{
    /* Check to see that the id given is valid */
    
    if (task_id >= OS_MAX_TASKS || OS_task_table[task_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    if( task_prop == NULL)
        return OS_INVALID_POINTER;

    /* put the info into the stucture */
    sem_wait( &(OS_task_table_sem));
    task_prop -> creator =    OS_task_table[task_id].creator;
    task_prop -> stack_size = OS_task_table[task_id].stack_size;
    task_prop -> priority =   OS_task_table[task_id].priority;
    task_prop -> OStask_id =  (uint32) OS_task_table[task_id].id;
    
    sem_post( &(OS_task_table_sem));
    
    strcpy(task_prop-> name, OS_task_table[task_id].name);
    
    return OS_SUCCESS;
    
} /* end OS_TaskGetInfo */

/****************************************************************************************
                                MESSAGE QUEUE API
****************************************************************************************/
/*---------------------------------------------------------------------------------------
   Name: OS_QueueCreate

   Purpose: Create a message queue which can be refered to by name or ID

   Returns: OS_INVALID_POINTER if a pointer passed in is NULL
            OS_ERR_NAME_TOO_LONG if the name passed in is too long
            OS_ERR_NO_FREE_IDS if there are already the max queues created
            OS_ERR_NAME_TAKEN if the name is already being used on another queue
            OS_ERROR if the OS create call fails
            OS_SUCCESS if success

   Notes: the flahs parameter is unused.
---------------------------------------------------------------------------------------*/

int32 OS_QueueCreate (uint32 *queue_id, const char *queue_name, uint32 queue_depth, 
                       uint32 data_size, uint32 flags)
{
	rtems_status_code  status;
	rtems_name         r_name;
    uint32 possible_qid;
    uint32 i;

    if ( queue_id == NULL || queue_name == NULL)
        return OS_INVALID_POINTER;

    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */

    if (strlen(queue_name) >= OS_MAX_API_NAME)
            return OS_ERR_NAME_TOO_LONG;

   /* Check Parameters */
    sem_wait( &(OS_queue_table_sem));
    for(possible_qid = 0; possible_qid < OS_MAX_QUEUES; possible_qid++)
    {
        if (OS_queue_table[possible_qid].free == TRUE)
            break;
    }
    
    if( possible_qid >= OS_MAX_QUEUES || OS_queue_table[possible_qid].free != TRUE)
    {
        sem_post( &(OS_queue_table_sem));

        return OS_ERR_NO_FREE_IDS;
    }

    /* Check to see if the name is already taken */

    for (i = 0; i < OS_MAX_QUEUES; i++)
    {
        if ((OS_queue_table[i].free == FALSE) &&
                strcmp ((char*) queue_name, OS_queue_table[i].name) == 0)
        {
            sem_post( &(OS_queue_table_sem));
            return OS_ERR_NAME_TAKEN;
        }
    }

    /* set the ID free to false to prevent other tasks from grabbing it */
    OS_queue_table[possible_qid].free = FALSE;   
    sem_post( &(OS_queue_table_sem));

	/*
	** Create the message queue.
	** The queue attributes are set to default values; the waiting order
	** (RTEMS_FIFO or RTEMS_PRIORITY) is irrelevant since only one task waits
	** on each queue.
	** The RTEMS object name is not used by SB; it was set this way on ST5.
	*/
	r_name = rtems_build_name('S','B',' ',' ');
	status = rtems_message_queue_create(
		r_name,                        /* 32-bit RTEMS object name; not used */
		queue_depth,                   /* maximum number of messages in queue (queue depth) */
		data_size,                     /* maximum size in bytes of a message */
		RTEMS_FIFO|RTEMS_LOCAL,        /* attributes (default) */
		&(OS_queue_table[possible_qid].id)  /* object ID returned for queue */
		);
	
		/*
		** If the operation failed, report the error */
	if (status != RTEMS_SUCCESSFUL) 
    {    sem_wait( &(OS_queue_table_sem));
         OS_queue_table[possible_qid].free = TRUE;   
         sem_post( &(OS_queue_table_sem));
    	return OS_ERROR;
    }
    
    /* Set the queue_id to the id that was found available*/
    /* Set the name of the queue, and the creator as well */
    
    *queue_id = possible_qid;
     
   sem_wait( &(OS_queue_table_sem));

   OS_queue_table[*queue_id].free = FALSE;
   strcpy( OS_queue_table[*queue_id].name, (char*) queue_name);
   OS_queue_table[*queue_id].creator = OS_FindCreator();
   sem_post( &(OS_queue_table_sem));

   return OS_SUCCESS;

} /* end OS_QueueCreate */


/*--------------------------------------------------------------------------------------
    Name: OS_QueueDelete

    Purpose: Deletes the specified message queue.

    Returns: OS_ERR_INVALID_ID if the id passed in does not exist
             OS_ERROR if the OS call to delete the queue fails 
             OS_SUCCESS if success

    Notes: If There are messages on the queue, they will be lost and any subsequent
           calls to QueueGet or QueuePut to this queue will result in errors
---------------------------------------------------------------------------------------*/

int32 OS_QueueDelete (uint32 queue_id)
{
    /* Check to see if the queue_id given is valid */
    
    if (queue_id >= OS_MAX_QUEUES || OS_queue_table[queue_id].free == TRUE)
            return OS_ERR_INVALID_ID;

    /* Try to delete the queue */
	if (rtems_message_queue_delete(OS_queue_table[queue_id].id) != RTEMS_SUCCESSFUL)
		{
			return OS_ERROR;
		}
	    
    /* 
     * Now that the queue is deleted, remove its "presence"
     * in OS_queue_table
    */
    sem_wait( &(OS_queue_table_sem));

    OS_queue_table[queue_id].free = TRUE;
    strcpy(OS_queue_table[queue_id].name, "");
    OS_queue_table[queue_id].creator = UNINITIALIZED;
    OS_queue_table[queue_id].id = UNINITIALIZED;
    sem_post( &(OS_queue_table_sem));

    return OS_SUCCESS;

} /* end OS_QueueDelete */


/*---------------------------------------------------------------------------------------
   Name: OS_QueueGet

   Purpose: Receive a message on a message queue.  Will pend or timeout on the receive.
   Returns: OS_ERR_INVALID_ID if the given ID does not exist
            OS_ERR_INVALID_POINTER if a pointer passed in is NULL
            OS_QUEUE_EMPTY if the Queue has no messages on it to be recieved
            OS_QUEUE_TIMEOUT if the timeout was OS_PEND and the time expired
            OS_QUEUE_INVALID_SIZE if the size copied from the queue was not correct
            OS_SUCCESS if success
---------------------------------------------------------------------------------------*/

int32 OS_QueueGet (uint32 queue_id, void *data, uint32 size, uint32 *size_copied, 
                    int32 timeout)
{
    /* msecs rounded to the closest system tick count */
	rtems_status_code  status;
	rtems_interval     ticks;  
    rtems_id           rtems_queue_id;
    
    /* Check Parameters */

    if(queue_id >= OS_MAX_QUEUES || OS_queue_table[queue_id].free == TRUE)
        return OS_ERR_INVALID_ID;
    else
    {
        if( (data == NULL) || (size_copied == NULL) )
            return OS_INVALID_POINTER;
    }

   rtems_queue_id = OS_queue_table[queue_id].id; 
    
    /* Get Message From Message Queue */
    if(timeout == OS_PEND)
    {
       /*
       ** Pend forever until a message arrives.
       */
		status = rtems_message_queue_receive(
			rtems_queue_id,            /* message queue descriptor */
			data,                                    /* pointer to message buffer */
			size_copied,                             /* returned size of message */
			RTEMS_WAIT,                           /* wait option */
			RTEMS_NO_TIMEOUT                         /* timeout */
			);
    }
    
    else if(timeout == OS_CHECK)
    {
	/*
	** Get a message without waiting.  If no message is present,
	** return with a failure indication.
		*/
		status = rtems_message_queue_receive(
			rtems_queue_id,            /* message queue descriptor */
			data,                                    /* pointer to message buffer */
			size_copied,                             /* returned size of message */
			RTEMS_NO_WAIT,                           /* wait option */
			RTEMS_NO_TIMEOUT                         /* timeout */
			);
		
		if (status == RTEMS_UNSATISFIED)
			return OS_QUEUE_EMPTY;
        
        
    }/* else if*/
        else
        {
	/*
	** Wait for up to a specified amount of time for a message to arrive.
	** If no message arrives within the timeout interval, return with a
	** failure indication.
	*/
		    ticks = OS_Milli2Ticks(timeout);
		    status = rtems_message_queue_receive(
			rtems_queue_id,                     /* message queue descriptor */
			data,                               /* pointer to message buffer */
			size_copied,                        /* returned size of message */
			RTEMS_WAIT,                         /* wait option */
			ticks                               /* timeout */
			);
		
		    if (status == RTEMS_TIMEOUT)
 			    return OS_QUEUE_TIMEOUT;       
        
        }/* else */
   
   
	  /*
	  ** Check the status of the read operation.  If a valid message was
	  ** obtained, indicate success.  If an error occurred, send an event
	  ** to indicate an unexpected queue read error.
	*/
	if (status == RTEMS_SUCCESSFUL && *size_copied == size)
    {
		/* Success. */
		return OS_SUCCESS;
	}
	else 
        {
		return OS_QUEUE_INVALID_SIZE;
        }
   
}/* end OS_QueueGet */

/*---------------------------------------------------------------------------------------
   Name: OS_QueuePut

   Purpose: Put a message on a message queue.

   Returns: OS_ERR_INVALID_ID if the queue id passed in is not a valid queue
            OS_INVALID_POINTER if the data pointer is NULL
            OS_QUEUE_FULL if the queue cannot accept another message
            OS_ERROR if the OS call returns an error
            OS_SUCCESS if SUCCESS            
   
   Notes: The flags parameter is not used.  The message put is always configured to
            immediately return an error if the receiving message queue is full.
---------------------------------------------------------------------------------------*/

int32 OS_QueuePut (uint32 queue_id, void *data, uint32 size, uint32 flags)
{
	rtems_status_code  status;
    rtems_id           rtems_queue_id;
    /* Check Parameters */

    if(queue_id >= OS_MAX_QUEUES || OS_queue_table[queue_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    if (data == NULL)
        return OS_INVALID_POINTER;
    
    rtems_queue_id = OS_queue_table[queue_id].id; 

    /* Get Message From RTEMS Message Queue */

	/** Write the buffer pointer to the queue.  If an error occurred, report it
	** with the corresponding SB status code.
	*/
	status = rtems_message_queue_send(
				    rtems_queue_id,     /* message queue descriptor */
					data,                             /* pointer to message */
					size                              /* length of message */
					);
   
	if (status == RTEMS_SUCCESSFUL) 
    {
		return OS_SUCCESS;
    }
	else if (status == RTEMS_TOO_MANY) 
    {
	/* 
	** Queue is full. 
	*/
		return OS_QUEUE_FULL;
    }
	else 
    {
	/* 
	** Unexpected error while writing to queue. 
	*/
		return OS_ERROR;
    }
    
}/* end OS_QueuePut */


/*--------------------------------------------------------------------------------------
    Name: OS_QueueGetIdByName

    Purpose: This function tries to find a queue Id given the name of the queue. The             id of the queue is passed back in queue_id

    Returns: OS_INVALID_POINTER if the name or id pointers are NULL
             OS_ERR_NAME_TOO_LONG the name passed in is too long
             OS_ERR_NAME_NOT_FOUND the name was not found in the table
             OS_SUCCESS if success
             
---------------------------------------------------------------------------------------*/

int32 OS_QueueGetIdByName (uint32 *queue_id, const char *queue_name)
{
    uint32 i;

    if(queue_id == NULL || queue_name == NULL)
        return OS_INVALID_POINTER;
    
    /* a name too long wouldn't have been allowed in the first place
     * so we definitely won't find a name too long*/
 
    if (strlen(queue_name) >= OS_MAX_API_NAME)
            return OS_ERR_NAME_TOO_LONG;
    
    for (i = 0; i < OS_MAX_QUEUES; i++)
    {
        if (OS_queue_table[i].free != TRUE &&
           (strcmp(OS_queue_table[i].name, (char*) queue_name) == 0 ))
        {
            *queue_id = i;
            return OS_SUCCESS;
        }
    }

    /* The name was not found in the table,
     *  or it was, and the queue_id isn't valid anymore */
    return OS_ERR_NAME_NOT_FOUND;

}/* end OS_QueueGetIdByName */


/*---------------------------------------------------------------------------------------
    Name: OS_QueueGetInfo

    Purpose: This function will pass back a pointer to structure that contains 
             all of the relevant info (name and creator) about the specified queue. 

    Returns: OS_INVALID_POINTER if queue_prop is NULL
             OS_ERR_INVALID_ID if the ID given is not  a valid queue
             OS_SUCCESS if the info was copied over correctly
---------------------------------------------------------------------------------------*/

int32 OS_QueueGetInfo (uint32 queue_id, OS_queue_prop_t *queue_prop)  
{
    /* Check to see that the id given is valid */
    
    if (queue_prop == NULL)
        return OS_INVALID_POINTER;
    
    if (queue_id >= OS_MAX_QUEUES || OS_queue_table[queue_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    /* put the info into the stucture */
    sem_wait( &(OS_queue_table_sem));
    queue_prop -> creator =   OS_queue_table[queue_id].creator;
    strcpy(queue_prop -> name, OS_queue_table[queue_id].name);
    sem_post( &(OS_queue_table_sem));


    return OS_SUCCESS;
    
} /* end OS_QueueGetInfo */
/****************************************************************************************
                                  SEMAPHORE API
****************************************************************************************/

/*---------------------------------------------------------------------------------------
   Name: OS_BinSemCreate

   Purpose: Creates a binary semaphore with initial value specified by
            sem_initial_value and name specified by sem_name. sem_id will be 
            returned to the caller
            
   Returns: OS_INVALID_POINTER if sen name or sem_id are NULL
            OS_ERR_NAME_TOO_LONG if the name given is too long
            OS_ERR_NO_FREE_IDS if all of the semaphore ids are taken
            OS_ERR_NAME_TAKEN if this is already the name of a binary semaphore
            OS_SEM_FAILURE if the OS call failed
            OS_SUCCESS if success
            

   Notes: options is an unused parameter 
---------------------------------------------------------------------------------------*/

int32 OS_BinSemCreate (uint32 *sem_id, const char *sem_name, uint32 sem_initial_value, 
                        uint32 options)
{
    /* the current candidate for the new sem id */
    rtems_status_code return_code = 0;
    uint32 possible_semid;
    uint32 i;

    if (sem_id == NULL || sem_name == NULL)
        return OS_INVALID_POINTER;
    
    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */
    
    if (strlen(sem_name) >= OS_MAX_API_NAME)
            return OS_ERR_NAME_TOO_LONG;
    
    /* Check Parameters */
    sem_wait( &(OS_bin_sem_table_sem));

    for (possible_semid = 0; possible_semid < OS_MAX_BIN_SEMAPHORES; possible_semid++)
    {
        if (OS_bin_sem_table[possible_semid].free == TRUE)    
            break;
    }
    
    if((possible_semid >= OS_MAX_BIN_SEMAPHORES) ||  
       (OS_bin_sem_table[possible_semid].free != TRUE))
    {
        sem_post( &(OS_bin_sem_table_sem));
        return OS_ERR_NO_FREE_IDS;
    }
    
    /* Check to see if the name is already taken */
    for (i = 0; i < OS_MAX_BIN_SEMAPHORES; i++)
    {
        if ((OS_bin_sem_table[i].free == FALSE) &&
                strcmp ((char*) sem_name, OS_bin_sem_table[i].name) == 0)
        {
            sem_post( &(OS_bin_sem_table_sem));
            return OS_ERR_NAME_TAKEN;
        }
    }
    OS_bin_sem_table[possible_semid].free = FALSE;
    sem_post( &(OS_bin_sem_table_sem));

    /* Create RTEMS Semaphore */

    return_code = rtems_semaphore_create( sem_name, sem_initial_value,0,
                                          RTEMS_NO_PRIORITY_CEILING,
                                          &(OS_bin_sem_table[possible_semid].id));
    
    /* check if Create failed */
	if ( return_code != RTEMS_SUCCESSFUL )
    {
        sem_wait( &(OS_bin_sem_table_sem));
        OS_bin_sem_table[possible_semid].free = TRUE;
        sem_post( &(OS_bin_sem_table_sem));
		return OS_SEM_FAILURE;
    }
    /* Set the sem_id to the one that we found available */
    /* Set the name of the semaphore,creator and free as well */

    *sem_id = possible_semid;
    
    sem_wait( &(OS_bin_sem_table_sem));
    OS_bin_sem_table[*sem_id].free = FALSE;
    strcpy(OS_bin_sem_table[*sem_id].name , (char*) sem_name);
    OS_bin_sem_table[*sem_id].creator = OS_FindCreator();
    sem_post( &(OS_bin_sem_table_sem));
    
    return OS_SUCCESS;
    
}/* end OS_BinSemCreate */


/*--------------------------------------------------------------------------------------
     Name: OS_BinSemDelete

    Purpose: Deletes the specified Binary Semaphore.

    Returns: OS_ERR_INVALID_ID if the id passed in is not a valid binary semaphore
             OS_ERR_SEM_NOT_FULL if the semahore is taken and cannot be deleted
             OS_SEM_FAILURE the OS call failed
             OS_SUCCESS if success
    
    Notes: Since we can't delete a semaphore which is currently locked by some task 
           (as it may ber crucial to completing the task), the semaphore must be full to
           allow deletion.
---------------------------------------------------------------------------------------*/

int32 OS_BinSemDelete (uint32 sem_id)
{
    /* Check to see if this sem_id is valid */
    if (sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

	/* we must make sure the semaphore is given  to delete it */
     rtems_semaphore_release(OS_bin_sem_table[sem_id].id);
    
	if (rtems_semaphore_delete( OS_bin_sem_table[sem_id].id) != RTEMS_SUCCESSFUL) 
	{
		return OS_SEM_FAILURE;
	}
    
    /* Remove the Id from the table, and its name, so that it cannot be found again */
    sem_wait( &(OS_bin_sem_table_sem));
    OS_bin_sem_table[sem_id].free = TRUE;
    strcpy(OS_bin_sem_table[sem_id].name , "");
    OS_bin_sem_table[sem_id].creator = UNINITIALIZED;
    OS_bin_sem_table[sem_id].id = UNINITIALIZED;
    sem_post( &(OS_bin_sem_table_sem));

    return OS_SUCCESS;

}/* end OS_BinSemDelete */

/*---------------------------------------------------------------------------------------
    Name: OS_BinSemGive

    Purpose: The function  unlocks the semaphore referenced by sem_id by performing
             a semaphore unlock operation on that semaphore.If the semaphore value 
             resulting from this operation is positive, then no threads were blocked             waiting for the semaphore to become unlocked; the semaphore value is
             simply incremented for this semaphore.

    
    Returns: OS_SEM_FAILURE the semaphore was not previously  initialized or is not
             in the array of semaphores defined by the system
             OS_ERR_INVALID_ID if the id passed in is not a binary semaphore
             OS_SUCCESS if success
                
---------------------------------------------------------------------------------------*/

int32 OS_BinSemGive (uint32 sem_id)
{
    /* Check Parameters */

    if(sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    /* Give Semaphore */
    
	if( rtems_semaphore_release(OS_bin_sem_table[sem_id].id) != RTEMS_SUCCESSFUL)
    {
		return OS_SEM_FAILURE ;
    }
	else
    {
		return  OS_SUCCESS ;
    }

}/* end OS_BinSemGive */
/*---------------------------------------------------------------------------------------
    Name: OS_BinSemFlush

    Purpose: The function  releases all the tasks pending on this semaphore. Note
             that the state of the semaphore is not changed by this operation.
    
    Returns: OS_SEM_FAILURE the semaphore was not previously  initialized or is not
             in the array of semaphores defined by the system
             OS_ERR_INVALID_ID if the id passed in is not a binary semaphore
             OS_SUCCESS if success
                
---------------------------------------------------------------------------------------*/

int32 OS_BinSemFlush (uint32 sem_id)
{
    /* Check Parameters */

    if(sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    /* Give Semaphore */
    
	if( rtems_semaphore_flush(OS_bin_sem_table[sem_id].id) != RTEMS_SUCCESSFUL)
    {
		return OS_SEM_FAILURE ;
    }
	else
    {
		return  OS_SUCCESS ;
    }

}/* end OS_BinSemFlush */

/*---------------------------------------------------------------------------------------
    Name:    OS_BinSemTake

    Purpose: The locks the semaphore referenced by sem_id by performing a 
             semaphore lock operation on that semaphore.If the semaphore value 
             is currently zero, then the calling thread shall not return from 
             the call until it either locks the semaphore or the call is 
             interrupted by a signal.

    Return:  OS_SEM_FAILURE : the semaphore was not previously initialized
             or is not in the array of semaphores defined by the system
             OS_ERR_INVALID_ID the Id passed in is not a valid binar semaphore
             OS_SEM_FAILURE if the OS call failed
             OS_SUCCESS if success
             
----------------------------------------------------------------------------------------*/

int32 OS_BinSemTake (uint32 sem_id)
{
    /* Check Parameters */

    if(sem_id >= OS_MAX_BIN_SEMAPHORES  || OS_bin_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    /* Note to self: Check out sem wait in the manual */
	if ( rtems_semaphore_obtain(OS_bin_sem_table[sem_id].id, RTEMS_WAIT, 
                                RTEMS_NO_TIMEOUT)!= RTEMS_SUCCESSFUL)
    {
	    return OS_SEM_FAILURE;
    }
    else
    {
        return OS_SUCCESS;
    }

}/* end OS_BinSemTake */


/*---------------------------------------------------------------------------------------
    Name: OS_BinSemTimedWait
    
    Purpose: The function locks the semaphore referenced by sem_id . However,
             if the semaphore cannot be locked without waiting for another process
             or thread to unlock the semaphore , this wait shall be terminated when 
             the specified timeout ,msecs, expires.

    Returns: OS_SEM_TIMEOUT if semaphore was not relinquished in time
             OS_SUCCESS if success
             OS_SEM_FAILURE the semaphore was not previously initialized or is not
             in the array of semaphores defined by the system
             OS_ERR_INVALID_ID if the ID passed in is not a valid semaphore ID
----------------------------------------------------------------------------------------*/


int32 OS_BinSemTimedWait (uint32 sem_id, uint32 msecs)
{
    /* msecs rounded to the closest system tick count */
  	rtems_status_code ret_val ;
	uint32 TimeInTicks;

    /* Check Parameters */

    if( (sem_id >= OS_MAX_BIN_SEMAPHORES) || (OS_bin_sem_table[sem_id].free == TRUE) )
        return OS_ERR_INVALID_ID;	
		
	TimeInTicks = OS_Milli2Ticks(msecs);
    
	ret_val  =  
		rtems_semaphore_obtain(OS_bin_sem_table[sem_id].id, RTEMS_WAIT,TimeInTicks ) ;
	
	switch (ret_val)
    {
    case RTEMS_TIMEOUT :
		ret_val = OS_SEM_TIMEOUT ;
		break ;
		
    case RTEMS_SUCCESSFUL :
		ret_val = OS_SUCCESS ;
		break ;
		
    default :
		ret_val = OS_SEM_FAILURE ;
		break ;
		
    }
	return ret_val;
}/* end OS_BinSemTimedWait */

/*--------------------------------------------------------------------------------------
    Name: OS_BinSemGetIdByName

    Purpose: This function tries to find a binary sem Id given the name of a bin_sem
             The id is returned through sem_id

    Returns: OS_INVALID_POINTER is semid or sem_name are NULL pointers
             OS_ERR_NAME_TOO_LONG if the name given is to long to have been stored
             OS_ERR_NAME_NOT_FOUND if the name was not found in the table
             OS_SUCCESS if success
             
---------------------------------------------------------------------------------------*/
int32 OS_BinSemGetIdByName (uint32 *sem_id, const char *sem_name)
{
    uint32 i;

    if (sem_id == NULL || sem_name == NULL)
        return OS_INVALID_POINTER;
    
    /* a name too long wouldn't have been allowed in the first place
     * so we definitely won't find a name too long*/
    
    if (strlen(sem_name) >= OS_MAX_API_NAME)
            return OS_ERR_NAME_TOO_LONG;

    for (i = 0; i < OS_MAX_BIN_SEMAPHORES; i++)
    {
        if (OS_bin_sem_table[i].free != TRUE &&
           (strcmp (OS_bin_sem_table[i].name , (char*) sem_name) == 0) )
        {
            *sem_id = i;
            return OS_SUCCESS;
        }
    }
    /* The name was not found in the table,
     *  or it was, and the sem_id isn't valid anymore */

    return OS_ERR_NAME_NOT_FOUND;
    
}/* end OS_BinSemGetIdByName */
/*---------------------------------------------------------------------------------------
    Name: OS_BinSemGetInfo

    Purpose: This function will pass back a pointer to structure that contains 
             all of the relevant info( name and creator) about the specified binary
             semaphore.
             
    Returns: OS_ERR_INVALID_ID if the id passed in is not a valid semaphore 
             OS_INVALID_POINTER if the bin_prop pointer is null
             OS_SUCCESS if success
---------------------------------------------------------------------------------------*/

int32 OS_BinSemGetInfo (uint32 sem_id, OS_bin_sem_prop_t *bin_prop)  
{
    /* Check to see that the id given is valid */
    
    if (sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    if (bin_prop == NULL)
        return OS_INVALID_POINTER;

    /* put the info into the stucture */
    sem_wait( &(OS_bin_sem_table_sem));

    bin_prop ->creator =    OS_bin_sem_table[sem_id].creator;
    strcpy(bin_prop-> name, OS_bin_sem_table[sem_id].name);
    sem_post( &(OS_bin_sem_table_sem));

    return OS_SUCCESS;
    
} /* end OS_BinSemGetInfo */
/*---------------------------------------------------------------------------------------
   Name: OS_CountSemCreate

   Purpose: Creates a countary semaphore with initial value specified by
            sem_initial_value and name specified by sem_name. sem_id will be 
            returned to the caller
            
   Returns: OS_INVALID_POINTER if sen name or sem_id are NULL
            OS_ERR_NAME_TOO_LONG if the name given is too long
            OS_ERR_NO_FREE_IDS if all of the semaphore ids are taken
            OS_ERR_NAME_TAKEN if this is already the name of a countary semaphore
            OS_SEM_FAILURE if the OS call failed
            OS_SUCCESS if success
            

   Notes: options is an unused parameter 
---------------------------------------------------------------------------------------*/

int32 OS_CountSemCreate (uint32 *sem_id, const char *sem_name, uint32 sem_initial_value, 
                        uint32 options)
{
    /* the current candidate for the new sem id */
    rtems_status_code return_code = 0;
    uint32 possible_semid;
    uint32 i;

    if (sem_id == NULL || sem_name == NULL)
        return OS_INVALID_POINTER;
    
    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */
    
    if (strlen(sem_name) >= OS_MAX_API_NAME)
            return OS_ERR_NAME_TOO_LONG;
    
    /* Check Parameters */
    sem_wait( &(OS_count_sem_table_sem));

    for (possible_semid = 0; possible_semid < OS_MAX_COUNT_SEMAPHORES; possible_semid++)
    {
        if (OS_count_sem_table[possible_semid].free == TRUE)    
            break;
    }
    
    if((possible_semid >= OS_MAX_COUNT_SEMAPHORES) ||  
       (OS_count_sem_table[possible_semid].free != TRUE))
    {
        sem_post( &(OS_count_sem_table_sem));
        return OS_ERR_NO_FREE_IDS;
    }
    
    /* Check to see if the name is already taken */
    for (i = 0; i < OS_MAX_COUNT_SEMAPHORES; i++)
    {
        if ((OS_count_sem_table[i].free == FALSE) &&
                strcmp ((char*) sem_name, OS_count_sem_table[i].name) == 0)
        {
            sem_post( &(OS_count_sem_table_sem));
            return OS_ERR_NAME_TAKEN;
        }
    }
    OS_count_sem_table[possible_semid].free = FALSE;
    sem_post( &(OS_count_sem_table_sem));

    /* Create RTEMS Semaphore */

    return_code = rtems_semaphore_create( sem_name, sem_initial_value,0,
                                          RTEMS_NO_PRIORITY_CEILING,
                                          &(OS_count_sem_table[possible_semid].id));
    
    /* check if Create failed */
	if ( return_code != RTEMS_SUCCESSFUL )
    {        
        sem_wait( &(OS_count_sem_table_sem));
        OS_count_sem_table[possible_semid].free = TRUE;
        sem_post( &(OS_count_sem_table_sem));

		return OS_SEM_FAILURE;
    }
    /* Set the sem_id to the one that we found available */
    /* Set the name of the semaphore,creator and free as well */

    *sem_id = possible_semid;
    
    sem_wait( &(OS_count_sem_table_sem));
    OS_count_sem_table[*sem_id].free = FALSE;
    strcpy(OS_count_sem_table[*sem_id].name , (char*) sem_name);
    OS_count_sem_table[*sem_id].creator = OS_FindCreator();
    sem_post( &(OS_count_sem_table_sem));
    
    return OS_SUCCESS;
    
}/* end OS_CountSemCreate */


/*--------------------------------------------------------------------------------------
     Name: OS_CountSemDelete

    Purpose: Deletes the specified Counting Semaphore.

    Returns: OS_ERR_INVALID_ID if the id passed in is not a valid countary semaphore
             OS_ERR_SEM_NOT_FULL if the semahore is taken and cannot be deleted
             OS_SEM_FAILURE the OS call failed
             OS_SUCCESS if success
    
    Notes: Since we can't delete a semaphore which is currently locked by some task 
           (as it may ber crucial to completing the task), the semaphore must be full to
           allow deletion.
---------------------------------------------------------------------------------------*/

int32 OS_CountSemDelete (uint32 sem_id)
{
    /* Check to see if this sem_id is valid */
    if (sem_id >= OS_MAX_COUNT_SEMAPHORES || OS_count_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;


	/* we must make sure the semaphore is given  to delete it */
     rtems_semaphore_release(OS_bin_sem_table[sem_id].id);
    
	if (rtems_semaphore_delete( OS_count_sem_table[sem_id].id) != RTEMS_SUCCESSFUL) 
	{
		return OS_SEM_FAILURE;
	}
    
    /* Remove the Id from the table, and its name, so that it cannot be found again */
    sem_wait( &(OS_count_sem_table_sem));
    OS_count_sem_table[sem_id].free = TRUE;
    strcpy(OS_count_sem_table[sem_id].name , "");
    OS_count_sem_table[sem_id].creator = UNINITIALIZED;
    OS_count_sem_table[sem_id].id = UNINITIALIZED;
    sem_post( &(OS_count_sem_table_sem));

    return OS_SUCCESS;

}/* end OS_CountSemDelete */

/*---------------------------------------------------------------------------------------
    Name: OS_CountSemGive

    Purpose: The function  unlocks the semaphore referenced by sem_id by performing
             a semaphore unlock operation on that semaphore.If the semaphore value 
             resulting from this operation is positive, then no threads were blocked             waiting for the semaphore to become unlocked; the semaphore value is
             simply incremented for this semaphore.

    
    Returns: OS_SEM_FAILURE the semaphore was not previously  initialized or is not
             in the array of semaphores defined by the system
             OS_ERR_INVALID_ID if the id passed in is not a countary semaphore
             OS_SUCCESS if success
                
---------------------------------------------------------------------------------------*/

int32 OS_CountSemGive (uint32 sem_id)
{
    /* Check Parameters */

    if(sem_id >= OS_MAX_COUNT_SEMAPHORES || OS_count_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    /* Give Semaphore */
    
	if( rtems_semaphore_release(OS_count_sem_table[sem_id].id) != RTEMS_SUCCESSFUL)
    {
		return OS_SEM_FAILURE ;
    }
	else
    {
		return  OS_SUCCESS ;
    }

}/* end OS_CountSemGive */

/*---------------------------------------------------------------------------------------
    Name:    OS_CountSemTake

    Purpose: The locks the semaphore referenced by sem_id by performing a 
             semaphore lock operation on that semaphore.If the semaphore value 
             is currently zero, then the calling thread shall not return from 
             the call until it either locks the semaphore or the call is 
             interrupted by a signal.

    Return:  OS_SEM_FAILURE : the semaphore was not previously initialized
             or is not in the array of semaphores defined by the system
             OS_ERR_INVALID_ID the Id passed in is not a valid countar semaphore
             OS_SEM_FAILURE if the OS call failed
             OS_SUCCESS if success
             
----------------------------------------------------------------------------------------*/

int32 OS_CountSemTake (uint32 sem_id)
{
    /* Check Parameters */

    if(sem_id >= OS_MAX_COUNT_SEMAPHORES  || OS_count_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    /* Note to self: Check out sem wait in the manual */
	if ( rtems_semaphore_obtain(OS_count_sem_table[sem_id].id, RTEMS_WAIT, 
                                RTEMS_NO_TIMEOUT)!= RTEMS_SUCCESSFUL)
    {
	    return OS_SEM_FAILURE;
    }
    else
    {
        return OS_SUCCESS;
    }

}/* end OS_CountSemTake */


/*---------------------------------------------------------------------------------------
    Name: OS_CountSemTimedWait
    
    Purpose: The function locks the semaphore referenced by sem_id . However,
             if the semaphore cannot be locked without waiting for another process
             or thread to unlock the semaphore , this wait shall be terminated when 
             the specified timeout ,msecs, expires.

    Returns: OS_SEM_TIMEOUT if semaphore was not relinquished in time
             OS_SUCCESS if success
             OS_SEM_FAILURE the semaphore was not previously initialized or is not
             in the array of semaphores defined by the system
             OS_ERR_INVALID_ID if the ID passed in is not a valid semaphore ID
----------------------------------------------------------------------------------------*/


int32 OS_CountSemTimedWait (uint32 sem_id, uint32 msecs)
{
    /* msecs rounded to the closest system tick count */
  	rtems_status_code ret_val ;
	uint32 TimeInTicks;

    /* Check Parameters */

    if( (sem_id >= OS_MAX_COUNT_SEMAPHORES) || (OS_count_sem_table[sem_id].free == TRUE) )
        return OS_ERR_INVALID_ID;	
		
	TimeInTicks = OS_Milli2Ticks(msecs);
    
	ret_val  =  
		rtems_semaphore_obtain(OS_count_sem_table[sem_id].id, RTEMS_WAIT,TimeInTicks ) ;
	
	switch (ret_val)
    {
    case RTEMS_TIMEOUT :
		ret_val = OS_SEM_TIMEOUT ;
		break ;
		
    case RTEMS_SUCCESSFUL :
		ret_val = OS_SUCCESS ;
		break ;
		
    default :
		ret_val = OS_SEM_FAILURE ;
		break ;
		
    }
	return ret_val;
}/* end OS_CountSemTimedWait */

/*--------------------------------------------------------------------------------------
    Name: OS_CountSemGetIdByName

    Purpose: This function tries to find a countary sem Id given the name of a count_sem
             The id is returned through sem_id

    Returns: OS_INVALID_POINTER is semid or sem_name are NULL pointers
             OS_ERR_NAME_TOO_LONG if the name given is to long to have been stored
             OS_ERR_NAME_NOT_FOUND if the name was not found in the table
             OS_SUCCESS if success
             
---------------------------------------------------------------------------------------*/
int32 OS_CountSemGetIdByName (uint32 *sem_id, const char *sem_name)
{
    uint32 i;

    if (sem_id == NULL || sem_name == NULL)
        return OS_INVALID_POINTER;
    
    /* a name too long wouldn't have been allowed in the first place
     * so we definitely won't find a name too long*/
    
    if (strlen(sem_name) >= OS_MAX_API_NAME)
            return OS_ERR_NAME_TOO_LONG;

    for (i = 0; i < OS_MAX_COUNT_SEMAPHORES; i++)
    {
        if (OS_count_sem_table[i].free != TRUE &&
           (strcmp (OS_count_sem_table[i].name , (char*) sem_name) == 0) )
        {
            *sem_id = i;
            return OS_SUCCESS;
        }
    }
    /* The name was not found in the table,
     *  or it was, and the sem_id isn't valid anymore */

    return OS_ERR_NAME_NOT_FOUND;
    
}/* end OS_CountSemGetIdByName */
/*---------------------------------------------------------------------------------------
    Name: OS_CountSemGetInfo

    Purpose: This function will pass back a pointer to structure that contains 
             all of the relevant info( name and creator) about the specified countary
             semaphore.
             
    Returns: OS_ERR_INVALID_ID if the id passed in is not a valid semaphore 
             OS_INVALID_POINTER if the count_prop pointer is null
             OS_SUCCESS if success
---------------------------------------------------------------------------------------*/

int32 OS_CountSemGetInfo (uint32 sem_id, OS_count_sem_prop_t *count_prop)  
{
    /* Check to see that the id given is valid */
    
    if (sem_id >= OS_MAX_COUNT_SEMAPHORES || OS_count_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    if (count_prop == NULL)
        return OS_INVALID_POINTER;

    /* put the info into the stucture */
    sem_wait( &(OS_count_sem_table_sem));

    count_prop ->creator =    OS_count_sem_table[sem_id].creator;
    strcpy(count_prop-> name, OS_count_sem_table[sem_id].name);
    sem_post( &(OS_count_sem_table_sem));

    return OS_SUCCESS;
    
} /* end OS_CountSemGetInfo */

/****************************************************************************************
                                  MUTEX API
****************************************************************************************/

/*---------------------------------------------------------------------------------------
    Name: OS_MutSemCreate

    Purpose: Creates a mutex semaphore initially full.

    Returns: OS_INVALID_POINTER if sem_id or sem_name are NULL
             OS_ERR_NAME_TOO_LONG if the sem_name is too long to be stored
             OS_ERR_NO_FREE_IDS if there are no more free mutex Ids
             OS_ERR_NAME_TAKEN if there is already a mutex with the same name
             OS_SEM_FAILURE if the OS call failed
             OS_SUCCESS if success
    
    Notes: the options parameter is not used in this implementation

---------------------------------------------------------------------------------------*/

int32 OS_MutSemCreate (uint32 *sem_id, const char *sem_name, uint32 options)
{
	int                 return_code;
	int                 mutex_init_attr_status;
	int                 mutex_setprotocol_status ;
	pthread_mutexattr_t mutex_attr ;    
	uint32				possible_semid;
	uint32				i;	    

    /* Check Parameters */

    if (sem_id == NULL || sem_name == NULL)
        return OS_INVALID_POINTER;
    
    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */
    
    if (strlen(sem_name) >= OS_MAX_API_NAME)
            return OS_ERR_NAME_TOO_LONG;

    sem_wait( &(OS_mut_sem_table_sem));

    for (possible_semid = 0; possible_semid < OS_MAX_MUTEXES; possible_semid++)
    {
        if (OS_mut_sem_table[possible_semid].free == TRUE)    
            break;
    }
    
    if( (possible_semid >= OS_MAX_MUTEXES) ||
        (OS_mut_sem_table[possible_semid].free != TRUE) )
    {
        sem_post( &(OS_mut_sem_table_sem));
        return OS_ERR_NO_FREE_IDS;
    }
    

    /* Check to see if the name is already taken */


    for (i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if ((OS_mut_sem_table[i].free == FALSE) &&
                strcmp ((char*)sem_name, OS_mut_sem_table[i].name) == 0)
        {
            sem_post( &(OS_mut_sem_table_sem));
            return OS_ERR_NAME_TAKEN;
        }
    }
    
    OS_mut_sem_table[possible_semid].free = FALSE;
    sem_post( &(OS_mut_sem_table_sem));

	/* 
	** init the attribute with default values 
	*/
	mutex_init_attr_status = pthread_mutexattr_init( &mutex_attr) ; 
	if (mutex_init_attr_status)
	{
        sem_wait( &(OS_mut_sem_table_sem));
        OS_mut_sem_table[possible_semid].free = TRUE;
        sem_post( &(OS_mut_sem_table_sem));
		return OS_SEM_FAILURE;
	}

	/* not set the attribute to our desire : priority inherence */
	mutex_setprotocol_status = pthread_mutexattr_setprotocol(&mutex_attr,PTHREAD_PRIO_INHERIT) ;
	if (mutex_setprotocol_status)
	{
        sem_wait( &(OS_mut_sem_table_sem));
        OS_mut_sem_table[possible_semid].free = TRUE;
        sem_post( &(OS_mut_sem_table_sem));
		return OS_SEM_FAILURE;
	}
	
	/* 
	** create the mutex 
	*/
	
	/* 
	** upon successful initialization, the state of the mutex becomes initialized and ulocked 
	*/
	return_code = pthread_mutex_init( ( &OS_mut_sem_table[possible_semid].id),&mutex_attr); 
	if ( return_code != 0 )
    { 
        sem_wait( &(OS_mut_sem_table_sem));
        OS_mut_sem_table[possible_semid].free = TRUE;
        sem_post( &(OS_mut_sem_table_sem));
		return OS_SEM_FAILURE;
	}    

    *sem_id = possible_semid;

    sem_wait( &(OS_mut_sem_table_sem));
    strcpy(OS_mut_sem_table[*sem_id].name, (char*)sem_name);
    OS_mut_sem_table[*sem_id].free = FALSE;
    OS_mut_sem_table[*sem_id].creator = OS_FindCreator();
    sem_post( &(OS_mut_sem_table_sem));

    
    return OS_SUCCESS;

}/* end OS_MutSemCreate */


/*--------------------------------------------------------------------------------------
     Name: OS_MutSemDelete

    Purpose: Deletes the specified Mutex Semaphore.
    
    Returns: OS_ERR_INVALID_ID if the id passed in is not a valid mutex
             OS_ERR_SEM_NOT_FULL if the mutex is empty 
             OS_SEM_FAILURE if the OS call failed
             OS_SUCCESS if success

    Notes: The mutex must be full to take it, so we have to check for fullness

---------------------------------------------------------------------------------------*/

int32 OS_MutSemDelete (uint32 sem_id)
{
    int status=-1;
    /* Check to see if this sem_id is valid   */
    if (sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;


	/* we must make sure the mutex is given  to delete it */
    pthread_mutex_unlock( &(OS_mut_sem_table[sem_id].id));

    
    status = pthread_mutex_destroy( &(OS_mut_sem_table[sem_id].id)); 
    
    /* 0 = success */   
    if( status != 0)
        return OS_SEM_FAILURE;
    /* Delete its presence in the table */
    
    sem_wait( &(OS_mut_sem_table_sem));

    OS_mut_sem_table[sem_id].free = TRUE;
    OS_mut_sem_table[sem_id].id = UNINITIALIZED;
    strcpy(OS_mut_sem_table[sem_id].name , "");
    OS_mut_sem_table[sem_id].creator = UNINITIALIZED;
    sem_post( &(OS_mut_sem_table_sem));

    
    return OS_SUCCESS;

}/* end OS_MutSemDelete */


/*---------------------------------------------------------------------------------------
    Name: OS_MutSemGive

    Purpose: The function releases the mutex object referenced by sem_id.The 
             manner in which a mutex is released is dependent upon the mutex's type 
             attribute.  If there are threads blocked on the mutex object referenced by 
             mutex when this function is called, resulting in the mutex becoming 
             available, the scheduling policy shall determine which thread shall 
             acquire the mutex.

    Returns: OS_SUCCESS if success
             OS_SEM_FAILURE if the semaphore was not previously  initialized 
             OS_ERR_INVALID_ID if the id passed in is not a valid mutex

---------------------------------------------------------------------------------------*/

int32 OS_MutSemGive (uint32 sem_id)
{
    /* Check Parameters */

    if(sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;
    
	if(pthread_mutex_unlock(&(OS_mut_sem_table[sem_id].id)) != 0)
    {
		return OS_SEM_FAILURE ;
    }
	else
    {
		return OS_SUCCESS ;
    }

}/* end OS_MutSemGive */


/*---------------------------------------------------------------------------------------
    Name: OS_MutSemTake

    Purpose: The mutex object referenced by sem_id shall be locked by calling this
             function. If the mutex is already locked, the calling thread shall
             block until the mutex becomes available. This operation shall return
             with the mutex object referenced by mutex in the locked state with the              calling thread as its owner.

    Returns: OS_SUCCESS if success
             OS_SEM_FAILURE if the semaphore was not previously initialized or is 
             not in the array of semaphores defined by the system
             OS_ERR_INVALID_ID the id passed in is not a valid mutex
---------------------------------------------------------------------------------------*/
int32 OS_MutSemTake (uint32 sem_id)
{
    /* Check Parameters */

   if(sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

	if( pthread_mutex_lock(&(OS_mut_sem_table[sem_id].id) ))
    {
		return OS_SEM_FAILURE ;
    }
	else
    {
		return OS_SUCCESS ;
    }

}/* end OS_MutSemGive */


/*--------------------------------------------------------------------------------------
    Name: OS_MutSemGetIdByName

    Purpose: This function tries to find a mutex sem Id given the name of a bin_sem
             The id is returned through sem_id

    Returns: OS_INVALID_POINTER is semid or sem_name are NULL pointers
             OS_ERR_NAME_TOO_LONG if the name given is to long to have been stored
             OS_ERR_NAME_NOT_FOUND if the name was not found in the table
             OS_SUCCESS if success
             
---------------------------------------------------------------------------------------*/

int32 OS_MutSemGetIdByName (uint32 *sem_id, const char *sem_name)
{
    uint32 i;

    if(sem_id == NULL || sem_name == NULL)
        return OS_INVALID_POINTER;

    /* a name too long wouldn't have been allowed in the first place
     * so we definitely won't find a name too long*/
    
    if (strlen(sem_name) >= OS_MAX_API_NAME)
            return OS_ERR_NAME_TOO_LONG;

    for (i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if ((OS_mut_sem_table[i].free != TRUE) &&
           (strcmp (OS_mut_sem_table[i].name, (char*) sem_name) == 0))
        {
            *sem_id = i;
            return OS_SUCCESS;
        }
    }
    
    /* The name was not found in the table,
     *  or it was, and the sem_id isn't valid anymore */
    return OS_ERR_NAME_NOT_FOUND;

}/* end OS_MutSemGetIdByName */
/*---------------------------------------------------------------------------------------
    Name: OS_MutnSemGetInfo

    Purpose: This function will pass back a pointer to structure that contains 
             all of the relevant info( name and creator) about the specified mutex
             semaphore.
             
    Returns: OS_ERR_INVALID_ID if the id passed in is not a valid semaphore 
             OS_INVALID_POINTER if the mut_prop pointer is null
             OS_SUCCESS if success
---------------------------------------------------------------------------------------*/

int32 OS_MutSemGetInfo (uint32 sem_id, OS_mut_sem_prop_t *mut_prop)  
{
    /* Check to see that the id given is valid */
    
    if (sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    if (mut_prop == NULL)
        return OS_INVALID_POINTER;
    
    /* put the info into the stucture */    
    sem_wait( &(OS_mut_sem_table_sem));
    mut_prop -> creator =   OS_mut_sem_table[sem_id].creator;
    strcpy(mut_prop-> name, OS_mut_sem_table[sem_id].name);
    sem_post( &(OS_mut_sem_table_sem));

    return OS_SUCCESS;
    
} /* end OS_BinSemGetInfo */


/****************************************************************************************
                                    INFO API
****************************************************************************************/


/*---------------------------------------------------------------------------------------
   Name: OS_Milli2Ticks

   Purpose: This function accepts a time interval in milli_seconds as input an
            returns the tick equivalent is o.s. system clock ticks. The tick
            value is rounded up.  This algorthim should change to use a integer divide.
---------------------------------------------------------------------------------------*/

int32 OS_Milli2Ticks (uint32 milli_seconds)
{
	uint32 num_of_ticks,tick_duration_usec ;
	
	tick_duration_usec = OS_Tick2Micros() ;
	
	num_of_ticks = 
		( (milli_seconds * 1000) + tick_duration_usec -1 ) / tick_duration_usec ;
	
	return(num_of_ticks) ; 
}/* end OS_Milli2Ticks */


/*---------------------------------------------------------------------------------------
   Name: OS_InfoGetTicks

   Purpose: This function returns the duration of a system tick in micro seconds.
---------------------------------------------------------------------------------------*/

int32 OS_Tick2Micros (void)
{
/* sysconf(_SC_CLK_TCK) returns  ticks/second.
** 1/sysconf(_SC_CLK_TCK) is the duration of a tick in seconds
** 1000000 * sysconf(_SC_CLK_TCK) is the duration of a tick in 
**	microsecond seconds 
*/
	return(	1000000/sysconf(_SC_CLK_TCK) );
	
	/*
    comment:
    last line could be replaced with:
    return(	1000000/TICKS_PER_SECOND );
    where TICKS_PER_SECOND is #defined .
    but then one could #define :
    TICKS_IN_USEC
    if so , don't call the api but use  TICKS_IN_USEC directly 
	*/
	    
}/* end OS_InfoGetTicks */

/*---------------------------------------------------------------------------------------
 * Name: OS_GetLocalTime
 * 
 * Purpose: This functions get the local time of the machine its on
 * ------------------------------------------------------------------------------------*/

int32 OS_GetLocalTime(OS_time_t *time_struct)
{
    
   int status;
   struct  timespec  time;

   if (time_struct == NULL)
      return OS_INVALID_POINTER;
   
    status = clock_gettime(CLOCK_REALTIME, &time);
    if (status != 0)
        return OS_ERROR;

   time_struct -> seconds = time.tv_sec;
   time_struct -> microsecs = time.tv_nsec / 1000;

   return OS_SUCCESS;
} /* end OS_GetLocalTime */


/*---------------------------------------------------------------------------------------
 * Name: OS_SetLocalTime
 * 
 * Purpose: This function sets the local time of the machine its on
 * ------------------------------------------------------------------------------------*/

int32 OS_SetLocalTime(OS_time_t *time_struct)
{
    
   int status;
   struct  timespec  time;

   if (time_struct == NULL)
      return OS_INVALID_POINTER;
   
   time.tv_sec = time_struct -> seconds;
   time.tv_nsec = (time_struct -> microsecs) * 1000;

    status = clock_settime(CLOCK_REALTIME, &time);
    if (status != 0)
        return OS_ERROR;


   return OS_SUCCESS;
} /* end OS_SetLocalTime */

/****************************************************************************************
                                 INFO API
****************************************************************************************/

/*---------------------------------------------------------------------------------------
   Name: OS_IntAttachHandler

   Purpose: The call associates a specified C routine to a specified interrupt
            number.Upon occurring of the InterruptNumber the InerruptHandler
            routine will be called and passed the parameter.

   Parameters:
        InterruptNumber : The Interrupt Number that will cause the start of the ISR
        InerruptHandler : The ISR associatd with this interrupt
        parameter :The parameter that is passed to the ISR

---------------------------------------------------------------------------------------*/

int32 OS_IntAttachHandler (uint32 InterruptNumber, void *InterruptHandler, 
                            int32 parameter)
{
	rtems_status_code ret_status;
	uint32 status ;
	rtems_isr_entry old_handler;
	
	ret_status = rtems_interrupt_catch( 
		(rtems_isr_entry)InterruptHandler,
		(rtems_vector_number)InterruptNumber,
		&old_handler);
	
	switch (ret_status) 
    {
    case RTEMS_SUCCESSFUL :
		status = OS_SUCCESS;
		break ;
		
    case RTEMS_INVALID_NUMBER :
		status = OS_INVALID_INT_NUM;
		break ;
		
    case RTEMS_INVALID_ADDRESS :
		status = OS_INVALID_POINTER;
		break ;
		
    default :
		status = OS_ERROR;
		break ;
		
    }
	return(status) ;
}/* end OS_IntAttachHandler */
/*---------------------------------------------------------------------------------------
   Name: OS_IntUnlock

   Purpose: Enable previous state of interrupts

   Parameters:
        IntLevel : The Interrupt Level to be reinstated 
---------------------------------------------------------------------------------------*/

int32 OS_IntUnlock (int32 IntLevel)
{
    int32 ReturnCode;
    
   rtems_interrupt_enable ( (rtems_interrupt_level) IntLevel); 

    ReturnCode = OS_SUCCESS;
    
    return ReturnCode;

}/* end OS_IntUnlock */


/*---------------------------------------------------------------------------------------
   Name: OS_IntLock

   Purpose: Disable interrupts.

   Parameters:
   
   Returns: Interrupt level before OS_IntDisableAll Call   
---------------------------------------------------------------------------------------*/

int32 OS_IntLock (void)
{
   rtems_interrupt_level rtems_int_level;

   rtems_interrupt_disable(rtems_int_level) ; 
   return ( (int32) rtems_int_level) ;


}/* end OS_IntLock */


/*---------------------------------------------------------------------------------------
   Name: OS_IntEnable

   Purpose: Enable previous state of interrupts

   Parameters:
        IntLevel : The Interrupt Level to be reinstated 
---------------------------------------------------------------------------------------*/

int32 OS_IntEnable (int32 Level)
{
    int32 ReturnCode;
    
   rtems_interrupt_enable ( (rtems_interrupt_level) Level); 

    ReturnCode = OS_SUCCESS;
    
    return ReturnCode;

}/* end OS_IntEnable */


/*---------------------------------------------------------------------------------------
   Name: OS_IntDisable

   Purpose: Disable the corresponding interrupt number.

   Parameters:
   
   Returns: Interrupt level before OS_IntDisable Call   
---------------------------------------------------------------------------------------*/

int32 OS_IntDisable (int32 Level)
{
    int32 ReturnCode;
    
    rtems_interrupt_disable ( (rtems_interrupt_level) Level); 

    ReturnCode = OS_SUCCESS;
    
    return ReturnCode;

}/* end OS_IntDisable */

/*---------------------------------------------------------------------------------------
 *  Name: OS_GetErrorName()
 *  purpose: A handy function to copy the name of the error to a buffer.
---------------------------------------------------------------------------------------*/
int32 OS_GetErrorName(int32 error_num, os_err_name_t * err_name)
{
    os_err_name_t local_name;
    uint32 return_code;

    return_code = OS_SUCCESS;
    
    switch (error_num)
    {
        case OS_SUCCESS: 
            strcpy(local_name,"OS_SUCCESS"); break;
        case OS_ERROR: 
            strcpy(local_name,"OS_ERROR"); break;
        case OS_INVALID_POINTER: 
            strcpy(local_name,"OS_INVALID_POINTER"); break;
        case OS_ERROR_ADDRESS_MISALIGNED: 
            strcpy(local_name,"OS_ADDRESS_MISALIGNED"); break;
        case OS_ERROR_TIMEOUT: 
            strcpy(local_name,"OS_ERROR_TIMEOUT"); break;
        case OS_INVALID_INT_NUM: 
            strcpy(local_name,"OS_INVALID_INT_NUM"); break;
        case OS_SEM_FAILURE:
            strcpy(local_name,"OS_SEM_FAILURE"); break;
        case OS_SEM_TIMEOUT:
            strcpy(local_name,"OS_SEM_TIMEOUT"); break;
        case OS_QUEUE_EMPTY:
            strcpy(local_name,"OS_QUEUE_EMPTY"); break;
        case OS_QUEUE_FULL:
            strcpy(local_name,"OS_QUEUE_FULL"); break;
        case OS_QUEUE_TIMEOUT:
            strcpy(local_name,"OS_QUEUE_TIMEOUT"); break;
        case OS_QUEUE_INVALID_SIZE:
            strcpy(local_name,"OS_QUEUE_INVALID_SIZE"); break;
        case OS_QUEUE_ID_ERROR:
            strcpy(local_name,"OS_QUEUE_ID_ERROR"); break;
        case OS_ERR_NAME_TOO_LONG:
            strcpy(local_name,"OS_ERR_NAME_TOO_LONG"); break;
        case OS_ERR_NO_FREE_IDS:
            strcpy(local_name,"OS_ERR_NO_FREE_IDS"); break;
        case OS_ERR_NAME_TAKEN:
            strcpy(local_name,"OS_ERR_NAME_TAKEN"); break;
        case OS_ERR_INVALID_ID:
            strcpy(local_name,"OS_ERR_INVALID_ID"); break;
        case OS_ERR_NAME_NOT_FOUND:
            strcpy(local_name,"OS_ERR_NAME_NOT_FOUND"); break;
        case OS_ERR_SEM_NOT_FULL:
            strcpy(local_name,"OS_ERR_SEM_NOT_FULL"); break;
        case OS_ERR_INVALID_PRIORITY:
            strcpy(local_name,"OS_ERR_INVALID_PRIORITY"); break;

        default: strcpy(local_name,"ERROR_UNKNOWN");
                 return_code = OS_ERROR;
    }

    strcpy((char*) err_name, local_name);


     return return_code;
}
/*---------------------------------------------------------------------------------------
 * Name: OS_FindCreator
 * Purpose: Finds the creator of a the current task  to store in the table for lookup 
 *          later 
---------------------------------------------------------------------------------------*/

uint32 OS_FindCreator(void)
{
	rtems_id          rtems_task_id;
    int i; 
    /* find the calling task ID */
    rtems_task_ident(RTEMS_SELF, 0, &rtems_task_id);

    for (i = 0; i < OS_MAX_TASKS; i++)
    {
        if (rtems_task_id == OS_task_table[i].id)
            break;
    }

    return i;
    
}
/*-----------------------------------------------------------------------------*/
uint32  OS_CompAbsDelayedTime( uint32 milli_second , struct timespec * tm)
{
	int return_status ;
	
	/* get the current time */
	return_status = clock_gettime( CLOCK_REALTIME,  tm );
		
	/* add the delay to the current time */
	tm->tv_sec  += (time_t) (milli_second / 1000) ;
	/* convert residue ( milli seconds)  to nano second */
	tm->tv_nsec +=  (milli_second % 1000) * 1000000 ;
	
	if(tm->tv_nsec > 999999999 )
    {
		tm->tv_nsec -= 1000000000 ;
		tm->tv_sec ++ ;
    }
	
	return(OS_SUCCESS) ;
}
/* ---------------------------------------------------------------------------
 * Name: OS_printf 
 * 
 * Purpose: This function abstracts out the printf type statements. This is 
 *          useful for using OS- specific thats that will allow non-polled
 *          print statements for the real time systems. 
 *

 ---------------------------------------------------------------------------*/
void OS_printf( const char *String, ...)
{
    va_list     ptr;
    char msg_buffer [OS_BUFFER_SIZE];
    
    va_start(ptr,String);
    vsnprintf(&msg_buffer[0], (size_t)OS_BUFFER_SIZE, String, ptr);
    va_end(ptr);
    
    msg_buffer[OS_BUFFER_SIZE -1] = '\0';
    printf("%s", &msg_buffer[0]);
    
}/* end OS_printf*/


/*
**
**   Name: OS_FPUExcSetMask
**
**   Purpose: This function sets the FPU exception mask
**
**   Notes: The exception environment is local to each task Therefore this must be
**          called for each task that that wants to do floating point and catch exceptions.
*/
int32 OS_FPUExcSetMask(uint32 mask)
{
    /*
    ** Not implemented in RTEMS.
    */
    return(OS_SUCCESS);
}

/*
**
**   Name: OS_FPUExcGetMask
**
**   Purpose: This function gets the FPU exception mask
**
**   Notes: The exception environment is local to each task Therefore this must be
**          called for each task that that wants to do floating point and catch exceptions.
*/
int32 OS_FPUExcGetMask(uint32 *mask)
{
    /*
    ** Not implemented in RTEMS.
    */
    return(OS_SUCCESS);
}

