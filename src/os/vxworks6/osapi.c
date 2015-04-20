/*
** File   : osapi.c
** Author : Joe-Paul Swinski
*/


/*  Revisions: 2/7/2005 by Nicholas J Yanchik
 *  added global variables for name functionality:
 *
 *  char* OS_task_name_table[OS_MAX_TASKS]
 *  char* OS_mesage_q_name_table[OS_MAX_QUEUES]
 *  char* OS_binary_sem_name_table[OS_MAX_BIN_SEMAPHORES]
 *  char* OS_mutex_sem_name_table[OS_MAX_MUTEXES]
 *  Also added initializations for the above variables is OS_API_Init()
 *
 *  The Create() functions for the above were modified to return an Id to
 *  the caller, not have the Id passed into it from the caller.
 *
 *  Added functions:
 *  uint32 OS_TaskGetIdByName (uint32 *task_id, char *task_name)
 *  uint32 OS_QueueGetIdByName (uint32 *queue_id, char *queue_name)
 *  uint32 OS_BinSemGetIdByName (uint32 *sem_id, char *sem_name)
 *  uint32 OS_MutSemGetIdByName (uint32 *sem_id, char *sem_name)
 *
 *  InfoGetTaskId was changed to  OS_TaskGetId and moved to the Tasks section
 *
 *  2/8/05 - 2/9/05:
 *  Added functions:
 *  OS_TaskDelete (uint32 task_id)
 *  OS_QueueDelete (uint32 queue_id)
 *  OS_BinSemDelete (uint32 sem_id)
 *  OS_MutSemDelete (uint32 sem_id)
 *
*/


/****************************************************************************************
                                    INCLUDE FILES
****************************************************************************************/

#include "vxWorks.h"
#include "version.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "semLib.h"
#include "taskLib.h"
#include "sysLib.h"
#include "msgQLib.h"
#include "time.h"
#include "intLib.h"
#include "taskVarLib.h"
#include "logLib.h"

#include "common_types.h"
#include "osapi.h"

#ifdef _PPC_
  #include "arch/ppc/vxPpcLib.h"
  #include "arch/ppc/ivPpc.h"
#endif

/****************************************************************************************
                                     DEFINES
****************************************************************************************/
#define MAX_PRIORITY 255
#define UNINITIALIZED 0
#define OK	0
#define EOS	'\0'

/****************************************************************************************
                                   GLOBAL DATA
****************************************************************************************/


uint32 OS_FindCreator(void);


/*  tables for the properties of objects */

/*tasks */
typedef struct
{
    int free;
    int id;
    char name [OS_MAX_API_NAME];
    int creator;
    uint32 stack_size;
    uint32 priority;
}OS_task_record_t;

/* queues */
typedef struct
{
    int free;
    MSG_Q_ID id;                       /* a pointer to the id */
    char name [OS_MAX_API_NAME];
    int creator;
}OS_queue_record_t;

/* Binary Semaphores */
typedef struct
{
    int free;
    SEM_ID id;                       /* a pointer to the id */
    char name [OS_MAX_API_NAME];
    int creator;
}OS_bin_sem_record_t;

/* Counting Semaphores */
typedef struct
{
    int free;
    SEM_ID id;                       /* a pointer to the id */
    char name [OS_MAX_API_NAME];
    int creator;
}OS_count_sem_record_t;


/* Mutexes */
typedef struct
{
    int free;
    SEM_ID id;
    char name [OS_MAX_API_NAME];
    int creator;
}OS_mut_sem_record_t;

uint32      OS_task_key;


/* Tables where the OS object information is stored */
OS_task_record_t    OS_task_table          [OS_MAX_TASKS];
OS_queue_record_t   OS_queue_table         [OS_MAX_QUEUES];
OS_bin_sem_record_t OS_bin_sem_table       [OS_MAX_BIN_SEMAPHORES];
OS_count_sem_record_t OS_count_sem_table   [OS_MAX_COUNT_SEMAPHORES];
OS_mut_sem_record_t OS_mut_sem_table       [OS_MAX_MUTEXES];

SEM_ID OS_task_table_sem;
SEM_ID OS_queue_table_sem;
SEM_ID OS_bin_sem_table_sem;
SEM_ID OS_count_sem_table_sem; 
SEM_ID OS_mut_sem_table_sem;


/* for the utility task printing */
#ifdef OS_UTILITY_TASK_ON
    void UtilityTask(void);
    MSG_Q_ID buffQ;
    int32 OS_DroppedMessages;
#endif


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
        OS_task_table[i].creator     = UNINITIALIZED ;
        strcpy(OS_task_table[i].name,"");
    }

    /* Initialize Message Queue Table */

    for(i = 0; i < OS_MAX_QUEUES; i++)
    {
        OS_queue_table[i].free        = TRUE;
        OS_queue_table[i].id       = NULL;
        OS_queue_table[i].creator     = UNINITIALIZED ;
        strcpy(OS_queue_table[i].name,"");
    }

    /* Initialize Binary Semaphore Table */

    for(i = 0; i < OS_MAX_BIN_SEMAPHORES; i++)
    {
        OS_bin_sem_table[i].free        = TRUE;
        OS_bin_sem_table[i].id       = NULL;
        OS_bin_sem_table[i].creator     = UNINITIALIZED ;
        strcpy(OS_bin_sem_table[i].name,"");
    }

    /* Initialize Counting Semaphore Table */

    for(i = 0; i < OS_MAX_COUNT_SEMAPHORES; i++)
    {
        OS_count_sem_table[i].free        = TRUE;
        OS_count_sem_table[i].id       = NULL;
        OS_count_sem_table[i].creator     = UNINITIALIZED ;
        strcpy(OS_count_sem_table[i].name,"");
    }


    /* Initialize Mutex Semaphore Table */

    for(i = 0; i < OS_MAX_MUTEXES; i++)
    {
        OS_mut_sem_table[i].free        = TRUE;
        OS_mut_sem_table[i].id       = NULL;
        OS_mut_sem_table[i].creator     = UNINITIALIZED ;
        strcpy(OS_mut_sem_table[i].name,"");
    }

    OS_task_table_sem = semBCreate(SEM_Q_PRIORITY, OS_SEM_FULL);
    OS_queue_table_sem = semBCreate(SEM_Q_PRIORITY, OS_SEM_FULL);
    OS_bin_sem_table_sem = semBCreate(SEM_Q_PRIORITY, OS_SEM_FULL);
    OS_count_sem_table_sem = semBCreate(SEM_Q_PRIORITY, OS_SEM_FULL);
    OS_mut_sem_table_sem = semBCreate(SEM_Q_PRIORITY, OS_SEM_FULL);

    OS_FS_Init();
    
#ifdef OS_UTILITY_TASK_ON 

    taskSpawn("UTILITY_TASK", OS_UTILITYTASK_PRIORITY, 0, OS_UTILITYTASK_STACK_SIZE ,(FUNCPTR) (void *)UtilityTask,0,0,0,0,0,0,0,0,0,0);
   
    OS_DroppedMessages = 0;

    buffQ = msgQCreate(OS_BUFFER_MSG_DEPTH, OS_BUFFER_SIZE, MSG_Q_FIFO);

    #endif
    
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

int32 OS_TaskCreate (uint32 *task_id,  const char *task_name,
                      const void *function_pointer, const uint32 *stack_pointer,
                      uint32 stack_size, uint32 priority, uint32 flags)
{
    uint32 possible_taskid;
    uint32 i;
    int32 LocalFlags;

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

    semTake(OS_task_table_sem,WAIT_FOREVER);
    for(possible_taskid = 0; possible_taskid < OS_MAX_TASKS; possible_taskid++)
    {
        if (OS_task_table[possible_taskid].free  == 1)
        {
            break;
        }
    }


    /* Check to see if the id is out of bounds */

    if( possible_taskid >= OS_MAX_TASKS || OS_task_table[possible_taskid].free != TRUE)
    {
        semGive(OS_task_table_sem);
        return OS_ERR_NO_FREE_IDS;
    }

    /* Check to see if the name is already taken */
    for (i = 0; i < OS_MAX_TASKS; i++)
    {
        if ((OS_task_table[i].free == FALSE) &&
           ( strcmp(task_name, OS_task_table[i].name) == 0))
        {
            semGive(OS_task_table_sem);
            return OS_ERR_NAME_TAKEN;
        }
    }


    OS_task_table[possible_taskid].free = FALSE;
   
    semGive(OS_task_table_sem);

    /* Create VxWorks Task */

    
    /* see if the user wants floating point enabled. If 
     * so, then se the correct option.
     */
    if (flags == OS_FP_ENABLED)
    {
        LocalFlags = VX_FP_TASK;
    }
    else
    {
        LocalFlags = 0;
    }
    
    OS_task_table[possible_taskid].id = taskSpawn((char*)task_name, priority, LocalFlags, stack_size,
            (FUNCPTR)function_pointer,0,0,0,0,0,0,0,0,0,0);
    /* check if taskSpawn failed */

    if(OS_task_table[possible_taskid].id == ERROR)
    {
        semTake(OS_task_table_sem,WAIT_FOREVER);
        OS_task_table[possible_taskid].free = TRUE;
        semGive(OS_task_table_sem);
        return OS_ERROR;
    }

    /* Set the task_id to the id that was found available
       Set the name of the task, the stack size, and priority */

    *task_id = possible_taskid;

    strcpy(OS_task_table[*task_id].name, task_name);

    /* this Id no longer free */
    semTake(OS_task_table_sem,WAIT_FOREVER);
    OS_task_table[*task_id].free = FALSE;
    OS_task_table[*task_id].creator = OS_FindCreator();
    OS_task_table[*task_id].stack_size = stack_size;
    OS_task_table[*task_id].priority = priority;
    semGive(OS_task_table_sem);

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
    int status;
    /* Check to see if the task_id given is valid */

    if (task_id >= OS_MAX_TASKS || OS_task_table[task_id].free == TRUE)
            return OS_ERR_INVALID_ID;

    /* Try to delete the task */
    status = taskDelete(OS_task_table[task_id].id);
    if (status == ERROR)
        {
            /* These statements are here for debugging purposes only */
            if (errno == S_objLib_OBJ_ID_ERROR)
                printf("Error # %d S_objLib_OBJ_ID_ERROR\n\n", errno);
            else
                printf("I dont know this error number yet\n");

            return OS_ERROR;
        }

    /*
     * Now that the task is deleted, remove its
     * "presence" in OS_task_table
    */

    semTake(OS_task_table_sem,WAIT_FOREVER);
    OS_task_table[task_id].free = TRUE;
    OS_task_table[task_id].id = UNINITIALIZED;
    strcpy(OS_task_table[task_id].name, "");
    OS_task_table[task_id].creator = UNINITIALIZED;
    OS_task_table[task_id].stack_size = UNINITIALIZED;
    OS_task_table[task_id].priority = UNINITIALIZED;
    semGive(OS_task_table_sem);


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

    semTake(OS_task_table_sem,WAIT_FOREVER);

    OS_task_table[task_id].free = TRUE;
    OS_task_table[task_id].id = UNINITIALIZED;
    strcpy(OS_task_table[task_id].name, "");
    OS_task_table[task_id].creator = UNINITIALIZED;
    OS_task_table[task_id].stack_size = UNINITIALIZED;
    OS_task_table[task_id].priority = UNINITIALIZED;
    
    semGive(OS_task_table_sem);

    taskExit(OS_SUCCESS);

}/*end OS_TaskExit */

/*---------------------------------------------------------------------------------------
   Name: OS_TaskDelay

   Purpose: Delay a task for specified amount of milliseconds

   returns: OS_ERROR if sleep fails
            OS_SUCCESS if success

   Notes: VxWorks uses the system clock to handle task delays.  The system clock usually
            runs at 60Hz. This means that the resolution of the delay will be course.
            It rounds up.
---------------------------------------------------------------------------------------*/

int32 OS_TaskDelay (uint32 milli_second)
{
    /* msecs rounded to the closest system tick count */
    uint32 sys_ticks;

    sys_ticks = OS_Milli2Ticks(milli_second);

    /* if successful, the execution of task will pend here until delay finishes */
    if(taskDelay(sys_ticks) != OK)
        return OS_ERROR;

    return OS_SUCCESS;

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
    /* Check Parameters */

    if(task_id >= OS_MAX_TASKS || OS_task_table[task_id].free == TRUE)
        return OS_ERR_INVALID_ID;


    if (new_priority > MAX_PRIORITY)
        return OS_ERR_INVALID_PRIORITY;

    /* Set VxWorks Task Priority */

    if(taskPrioritySet(OS_task_table[task_id].id, new_priority) != OK)
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
    int     vxworks_task_id;
    int     i;
    uint32  task_id;

    /* Find Defined Task Id */

    vxworks_task_id = taskIdSelf();

    for(i = 0; i < OS_MAX_TASKS; i++)
    {
        if(OS_task_table[i].id == vxworks_task_id)
            break;
    }

    task_id = i;

    if(task_id >= OS_MAX_TASKS)
    {
        return OS_ERR_INVALID_ID;
    }

    /* Add VxWorks Task Variable */


    if(taskVarAdd(OS_task_table[task_id].id, (int*)(char *)&OS_task_key) != OK)
        return OS_ERROR;

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
    return OS_task_key;

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
        if((OS_task_table[i].free != TRUE) &&
          (strcmp(OS_task_table[i].name,(char *)task_name) == 0 ))
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
    semTake(OS_task_table_sem,WAIT_FOREVER);
    task_prop -> creator =    OS_task_table[task_id].creator;
    task_prop -> stack_size = OS_task_table[task_id].stack_size;
    task_prop -> priority =   OS_task_table[task_id].priority;
    task_prop -> OStask_id =  (uint32)OS_task_table[task_id].id;

    strcpy(task_prop-> name, OS_task_table[task_id].name);
    semGive(OS_task_table_sem);


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
    uint32 possible_qid;
    uint32 i;

    if ( queue_id == NULL || queue_name == NULL)
        return OS_INVALID_POINTER;

    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */

    if (strlen(queue_name) >= OS_MAX_API_NAME)
            return OS_ERR_NAME_TOO_LONG;

   /* Check Parameters */

    semTake(OS_queue_table_sem,WAIT_FOREVER);
    for(possible_qid = 0; possible_qid < OS_MAX_QUEUES; possible_qid++)
    {
        if (OS_queue_table[possible_qid].free == TRUE)
            break;
    }

    if( possible_qid >= OS_MAX_QUEUES || OS_queue_table[possible_qid].free != TRUE)
    {
        semGive(OS_queue_table_sem);
        return OS_ERR_NO_FREE_IDS;
    }

    /* Check to see if the name is already taken */

    for (i = 0; i < OS_MAX_QUEUES; i++)
    {
        if ((OS_queue_table[i].free == FALSE) &&
                strcmp ((char*)queue_name, OS_queue_table[i].name) == 0)
        {
            semGive(OS_queue_table_sem);
            return OS_ERR_NAME_TAKEN;
        }
    }

    OS_queue_table[possible_qid].free = FALSE;
    semGive(OS_queue_table_sem);

    /* Create VxWorks Message Queue */

    OS_queue_table[possible_qid].id = msgQCreate(queue_depth, data_size, MSG_Q_FIFO);

    /* check if message Q create failed */

    if(OS_queue_table[possible_qid].id == NULL)
    {
        semTake(OS_queue_table_sem,WAIT_FOREVER);
        OS_queue_table[possible_qid].free = TRUE;
        semGive(OS_queue_table_sem);
        return OS_ERROR;
    }

    /* Set the queue_id to the id that was found available*/
    /* Set the name of the queue, and the creator as well */

    *queue_id = possible_qid;

    semTake(OS_queue_table_sem,WAIT_FOREVER);

   OS_queue_table[*queue_id].free = FALSE;
   strcpy( OS_queue_table[*queue_id].name, (char*) queue_name);
   OS_queue_table[*queue_id].creator = OS_FindCreator();
   semGive(OS_queue_table_sem);

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
    if (msgQDelete(OS_queue_table[queue_id].id) != OK)
            return OS_ERROR;

    /*
     * Now that the queue is deleted, remove its "presence"
     * in OS_message_q_table and OS_message_q_name_table
    */

    semTake(OS_queue_table_sem,WAIT_FOREVER);

    OS_queue_table[queue_id].free = TRUE;
    strcpy(OS_queue_table[queue_id].name, "");
    OS_queue_table[queue_id].creator = UNINITIALIZED;
    OS_queue_table[queue_id].id = NULL;
    semGive(OS_queue_table_sem);


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
    int status = -1;
    uint32 sys_ticks;

    /* Check Parameters */

    if(queue_id >= OS_MAX_QUEUES || OS_queue_table[queue_id].free == TRUE)
    {
        return OS_ERR_INVALID_ID;
    }
    else
    {
        if( (data == NULL) || (size_copied == NULL) )
            return OS_INVALID_POINTER;
    }

    /* Get Message From VxWorks Message Queue */

    if(timeout == OS_PEND)
    {
        status = msgQReceive(OS_queue_table[queue_id].id, data, size, WAIT_FOREVER);
    }
    else
    {
        if (timeout == OS_CHECK)
        {
            status = msgQReceive(OS_queue_table[queue_id].id, data, size, NO_WAIT);

            if( (status == ERROR) && (errno == S_objLib_OBJ_UNAVAILABLE) )
                return OS_QUEUE_EMPTY;
        }
        else
        {
            sys_ticks = OS_Milli2Ticks(timeout);
            status = msgQReceive(OS_queue_table[queue_id].id, data, size, sys_ticks);

            if( (status == ERROR) && (errno == S_objLib_OBJ_TIMEOUT) )
                return OS_QUEUE_TIMEOUT;
        }
    }

    if(status == ERROR)
    {
        *size_copied = 0;
        return OS_ERROR;
    }

    else
    {
        if((uint32)status != size)
        {
            return OS_QUEUE_INVALID_SIZE;
        }
        else
        {
            *size_copied = (uint32)status;
            return OS_SUCCESS;
        }
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
    /* Check Parameters */

    if(queue_id >= OS_MAX_QUEUES || OS_queue_table[queue_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    if (data == NULL)
        return OS_INVALID_POINTER;

    /* Get Message From VxWorks Message Queue */

    if(msgQSend(OS_queue_table[queue_id].id, data, size, NO_WAIT, MSG_PRI_NORMAL) != OK)
    {
        if(errno == S_objLib_OBJ_UNAVAILABLE)
            return OS_QUEUE_FULL;
        else
            return OS_ERROR;
    }

    return OS_SUCCESS;

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
        if ( OS_queue_table[i].free != TRUE &&
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
    semTake(OS_queue_table_sem,WAIT_FOREVER);

    queue_prop -> creator =   OS_queue_table[queue_id].creator;
    strcpy(queue_prop -> name, OS_queue_table[queue_id].name);
    semGive(OS_queue_table_sem);

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
    uint32 possible_semid;
    uint32 i;

    if (sem_id == NULL || sem_name == NULL)
        return OS_INVALID_POINTER;

    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */

    if (strlen(sem_name) >= OS_MAX_API_NAME)
            return OS_ERR_NAME_TOO_LONG;

    /* Check Parameters */
    semTake(OS_bin_sem_table_sem,WAIT_FOREVER);

    for (possible_semid = 0; possible_semid < OS_MAX_BIN_SEMAPHORES; possible_semid++)
    {
        if (OS_bin_sem_table[possible_semid].free == TRUE)
            break;
    }

    if((possible_semid >= OS_MAX_BIN_SEMAPHORES) ||
       (OS_bin_sem_table[possible_semid].free != TRUE))
    {
        semGive(OS_bin_sem_table_sem);
        return OS_ERR_NO_FREE_IDS;
    }

    /* Check to see if the name is already taken */
    for (i = 0; i < OS_MAX_BIN_SEMAPHORES; i++)
    {
        if ((OS_bin_sem_table[i].free == FALSE) &&
                strcmp ((char*)sem_name, OS_bin_sem_table[i].name) == 0)
        {
            semGive(OS_bin_sem_table_sem);
            return OS_ERR_NAME_TAKEN;
        }
    }
    
    OS_bin_sem_table[possible_semid].free  = FALSE;
    semGive(OS_bin_sem_table_sem);

    /* Create VxWorks Semaphore */
    OS_bin_sem_table[possible_semid].id = semBCreate(SEM_Q_PRIORITY, sem_initial_value);

    /* check if semBCreate failed */
    if(OS_bin_sem_table[possible_semid].id == NULL)
    {
        semTake(OS_bin_sem_table_sem,WAIT_FOREVER);
        OS_bin_sem_table[possible_semid].free  = TRUE;
        semGive(OS_bin_sem_table_sem);
        return OS_SEM_FAILURE;
    }

    /* Set the sem_id to the one that we found available */
    /* Set the name of the semaphore,creator and free as well */

    *sem_id = possible_semid;

    semTake(OS_bin_sem_table_sem,WAIT_FOREVER);
    OS_bin_sem_table[*sem_id].free = FALSE;
    strcpy(OS_bin_sem_table[*sem_id].name , (char*) sem_name);
    OS_bin_sem_table[*sem_id].creator = OS_FindCreator();
    semGive(OS_bin_sem_table_sem);


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

    if (semDelete(OS_bin_sem_table[sem_id].id) != OK)
        return OS_SEM_FAILURE;

    /* Remove the Id from the table, and its name, so that it cannot be found again */
    semTake(OS_bin_sem_table_sem,WAIT_FOREVER);
    OS_bin_sem_table[sem_id].free = TRUE;
    strcpy(OS_bin_sem_table[sem_id].name , "");
    OS_bin_sem_table[sem_id].creator = UNINITIALIZED;
    OS_bin_sem_table[sem_id].id = NULL;
    semGive(OS_bin_sem_table_sem);

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

    /* Give VxWorks Semaphore */

    if(semGive(OS_bin_sem_table[sem_id].id) != OK)
        return OS_SEM_FAILURE;

    return OS_SUCCESS;

}/* end OS_BinSemGive */

/*---------------------------------------------------------------------------------------
    Name: OS_BinSemFlush

    Purpose: The function unblocks all tasks pending on the specified semaphore. However,
             this function does not change the state of the semaphore.

    
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

    /* Give VxWorks Semaphore */

    if(semFlush(OS_bin_sem_table[sem_id].id) != OK)
        return OS_SEM_FAILURE;

    return OS_SUCCESS;

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

    /* Give VxWorks Semaphore */

    if(semTake(OS_bin_sem_table[sem_id].id, WAIT_FOREVER) != OK)
        return OS_SEM_FAILURE;

    return OS_SUCCESS;

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
    uint32 sys_ticks;

    sys_ticks = OS_Milli2Ticks(msecs);

    /* Check Parameters */

    if( (sem_id >= OS_MAX_BIN_SEMAPHORES) || (OS_bin_sem_table[sem_id].free == TRUE) )
        return OS_ERR_INVALID_ID;

    /* Give VxWorks Semaphore */

    if(semTake(OS_bin_sem_table[sem_id].id , sys_ticks) != OK)
        return OS_SEM_TIMEOUT;

    return OS_SUCCESS;

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
        if ( OS_bin_sem_table[i].free != TRUE &&
           ( strcmp (OS_bin_sem_table[i].name , (char*) sem_name) == 0)
   )
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
    semTake(OS_bin_sem_table_sem,WAIT_FOREVER);

    bin_prop ->creator =    OS_bin_sem_table[sem_id].creator;
    strcpy(bin_prop-> name, OS_bin_sem_table[sem_id].name);
    semGive(OS_bin_sem_table_sem);

    return OS_SUCCESS;

} /* end OS_BinSemGetInfo */

/*---------------------------------------------------------------------------------------
   Name: OS_CountSemCreate

   Purpose: Creates a counting semaphore with initial value specified by
            sem_initial_value and name specified by sem_name. sem_id will be
            returned to the caller

   Returns: OS_INVALID_POINTER if sen name or sem_id are NULL
            OS_ERR_NAME_TOO_LONG if the name given is too long
            OS_ERR_NO_FREE_IDS if all of the semaphore ids are taken
            OS_ERR_NAME_TAKEN if this is already the name of a counting semaphore
            OS_SEM_FAILURE if the OS call failed
            OS_SUCCESS if success


   Notes: options is an unused parameter
---------------------------------------------------------------------------------------*/

int32 OS_CountSemCreate (uint32 *sem_id, const char *sem_name, uint32 sem_initial_value,
                        uint32 options)
{
    /* the current candidate for the new sem id */
    uint32 possible_semid;
    uint32 i;

    if (sem_id == NULL || sem_name == NULL)
        return OS_INVALID_POINTER;

    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */

    if (strlen(sem_name) >= OS_MAX_API_NAME)
            return OS_ERR_NAME_TOO_LONG;

    /* Check Parameters */
    semTake(OS_count_sem_table_sem,WAIT_FOREVER);

    for (possible_semid = 0; possible_semid < OS_MAX_COUNT_SEMAPHORES; possible_semid++)
    {
        if (OS_count_sem_table[possible_semid].free == TRUE)
            break;
    }

    if((possible_semid >= OS_MAX_COUNT_SEMAPHORES) ||
       (OS_count_sem_table[possible_semid].free != TRUE))
    {
        semGive(OS_count_sem_table_sem);
        return OS_ERR_NO_FREE_IDS;
    }

    /* Check to see if the name is already taken */
    for (i = 0; i < OS_MAX_COUNT_SEMAPHORES; i++)
    {
        if ((OS_count_sem_table[i].free == FALSE) &&
                strcmp ((char*)sem_name, OS_count_sem_table[i].name) == 0)
        {
            semGive(OS_count_sem_table_sem);
            return OS_ERR_NAME_TAKEN;
        }
    }
    OS_count_sem_table[possible_semid].free = FALSE;
    semGive(OS_count_sem_table_sem);

    /* Create VxWorks Semaphore */
    OS_count_sem_table[possible_semid].id = semCCreate(SEM_Q_PRIORITY, sem_initial_value);

    /* check if semCCreate failed */
    if(OS_count_sem_table[possible_semid].id == NULL)
    {
        semTake(OS_count_sem_table_sem,WAIT_FOREVER);    
        OS_count_sem_table[possible_semid].free =TRUE;
        semGive(OS_count_sem_table_sem);
        return OS_SEM_FAILURE;
    }

    /* Set the sem_id to the one that we found available */
    /* Set the name of the semaphore,creator and free as well */

    *sem_id = possible_semid;

    semTake(OS_count_sem_table_sem,WAIT_FOREVER);
    OS_count_sem_table[*sem_id].free = FALSE;
    strcpy(OS_count_sem_table[*sem_id].name , (char*) sem_name);
    OS_count_sem_table[*sem_id].creator = OS_FindCreator();
    semGive(OS_count_sem_table_sem);


    return OS_SUCCESS;

}/* end OS_CountSemCreate */


/*--------------------------------------------------------------------------------------
     Name: OS_CountSemDelete

    Purpose: Deletes the specified Counting Semaphore.

    Returns: OS_ERR_INVALID_ID if the id passed in is not a valid counting semaphore
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


    if (semDelete(OS_count_sem_table[sem_id].id) != OK)
        return OS_SEM_FAILURE;

    /* Remove the Id from the table, and its name, so that it cannot be found again */
    semTake(OS_count_sem_table_sem,WAIT_FOREVER);
    OS_count_sem_table[sem_id].free = TRUE;
    strcpy(OS_count_sem_table[sem_id].name , "");
    OS_count_sem_table[sem_id].creator = UNINITIALIZED;
    OS_count_sem_table[sem_id].id = NULL;
    semGive(OS_count_sem_table_sem);

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
             OS_ERR_INVALID_ID if the id passed in is not a counting semaphore
             OS_SUCCESS if success

---------------------------------------------------------------------------------------*/
int32 OS_CountSemGive (uint32 sem_id)
{
    /* Check Parameters */

    if(sem_id >= OS_MAX_COUNT_SEMAPHORES || OS_count_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    /* Give VxWorks Semaphore */

    if(semGive(OS_count_sem_table[sem_id].id) != OK)
        return OS_SEM_FAILURE;

    return OS_SUCCESS;

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

    /* Give VxWorks Semaphore */

    if(semTake(OS_count_sem_table[sem_id].id, WAIT_FOREVER) != OK)
        return OS_SEM_FAILURE;

    return OS_SUCCESS;

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
    uint32 sys_ticks;

    sys_ticks = OS_Milli2Ticks(msecs);

    /* Check Parameters */

    if( (sem_id >= OS_MAX_COUNT_SEMAPHORES) || (OS_count_sem_table[sem_id].free == TRUE) )
        return OS_ERR_INVALID_ID;

    /* Give VxWorks Semaphore */

    if(semTake(OS_count_sem_table[sem_id].id , sys_ticks) != OK)
        return OS_SEM_TIMEOUT;

    return OS_SUCCESS;

}/* end OS_CountSemTimedWait */


/*--------------------------------------------------------------------------------------
    Name: OS_CountSemGetIdByName

    Purpose: This function tries to find a counting sem Id given the name of a count_sem
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
        if ( OS_count_sem_table[i].free != TRUE &&
           ( strcmp (OS_count_sem_table[i].name , (char*) sem_name) == 0)
   )
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
             all of the relevant info( name and creator) about the specified counting
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
    semTake(OS_count_sem_table_sem,WAIT_FOREVER);

    count_prop ->creator =    OS_count_sem_table[sem_id].creator;
    strcpy(count_prop-> name, OS_count_sem_table[sem_id].name);
    semGive(OS_count_sem_table_sem);

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
    uint32 possible_semid;
    uint32 i;

    /* Check Parameters */

    if (sem_id == NULL || sem_name == NULL)
        return OS_INVALID_POINTER;

    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */

    if (strlen(sem_name) >= OS_MAX_API_NAME)
            return OS_ERR_NAME_TOO_LONG;

    semTake(OS_mut_sem_table_sem,WAIT_FOREVER);

    for (possible_semid = 0; possible_semid < OS_MAX_MUTEXES; possible_semid++)
    {
        if (OS_mut_sem_table[possible_semid].free == TRUE)
            break;
    }

    if( (possible_semid >= OS_MAX_MUTEXES) ||
        (OS_mut_sem_table[possible_semid].free != TRUE) )
    {
        semGive(OS_mut_sem_table_sem);
        return OS_ERR_NO_FREE_IDS;
    }

    /* Check to see if the name is already taken */
    for (i = 0; i < OS_MAX_MUTEXES; i++)
    {
        if ((OS_mut_sem_table[i].free == FALSE) &&
            strcmp ((char*) sem_name, OS_mut_sem_table[i].name) == 0)
        {
            semGive(OS_mut_sem_table_sem);
            return OS_ERR_NAME_TAKEN;
        }
    }
    OS_mut_sem_table[possible_semid].free = FALSE;
    semGive(OS_mut_sem_table_sem);

    /* Create VxWorks Semaphore */

    OS_mut_sem_table[possible_semid].id = semMCreate(SEM_Q_PRIORITY);

     /* check if semMCreate failed */
    if(OS_mut_sem_table[possible_semid].id == NULL)
    {
        semTake(OS_mut_sem_table_sem,WAIT_FOREVER);
        OS_mut_sem_table[possible_semid].free = TRUE;
        semGive(OS_mut_sem_table_sem);
        return OS_SEM_FAILURE;
    }

    /* Set the sem_id to the one that we found open */
    /* Set the name of the semaphore, creator, and free as well */

    *sem_id = possible_semid;
    semTake(OS_mut_sem_table_sem,WAIT_FOREVER);

    strcpy(OS_mut_sem_table[*sem_id].name, (char*)sem_name);
    OS_mut_sem_table[*sem_id].free = FALSE;
    OS_mut_sem_table[*sem_id].creator = OS_FindCreator();
    semGive(OS_mut_sem_table_sem);

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
    /* Check to see if this sem_id is valid   */
    if (sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        return OS_ERR_INVALID_ID;

    if (semDelete(OS_mut_sem_table[sem_id].id) != OK)
        return OS_SEM_FAILURE;

    /* Delete its presence in the table */

    semTake(OS_mut_sem_table_sem,WAIT_FOREVER);

    OS_mut_sem_table[sem_id].free = TRUE;
    OS_mut_sem_table[sem_id].id = NULL;
    strcpy(OS_mut_sem_table[sem_id].name , "");
    OS_mut_sem_table[sem_id].creator = UNINITIALIZED;
    semGive(OS_mut_sem_table_sem);


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

    /* Give VxWorks Semaphore */

    if(semGive(OS_mut_sem_table[sem_id].id) != OK)
        return OS_SEM_FAILURE;

    return OS_SUCCESS;

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

    /* Take VxWorks Semaphore */

    if(semTake(OS_mut_sem_table[sem_id].id, WAIT_FOREVER) != OK)
        return OS_SEM_FAILURE;

    return OS_SUCCESS;

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
    semTake(OS_mut_sem_table_sem,WAIT_FOREVER);
    mut_prop -> creator =   OS_mut_sem_table[sem_id].creator;
    strcpy(mut_prop-> name, OS_mut_sem_table[sem_id].name);
    semGive(OS_mut_sem_table_sem);

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
    return( ((sysClkRateGet() * milli_seconds) / 1000) );
    /*
     * this function can be modified - it gives a good approx without any
     * floating point (">>10" ~= "/1000")
    */

}/* end OS_Milli2Ticks */


/*---------------------------------------------------------------------------------------
   Name: OS_InfoGetTicks

   Purpose: This function returns the duration of a system tick in micro seconds.
---------------------------------------------------------------------------------------*/

int32 OS_Tick2Micros (void)
{
    return( 1000000 / sysClkRateGet() );

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
    if (status != OK)
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

   Notes: VxWorks wants the interrupt number to be the interrupt vector - i.e. the address
          that the the hardware reads from to grab the address of the corresponding ISR.
          On the PPC, the macro INUM_TO_IVEC does the conversion.  The file
          "arch/ppc/ivPpc.h" in the Tornado2.2 tree has the definition.
---------------------------------------------------------------------------------------*/

int32 OS_IntAttachHandler (uint32 InterruptNumber, void * InterruptHandler,
                            int32 parameter)
{
    if (InterruptHandler == NULL)
        return OS_INVALID_POINTER;

    if(intConnect(INUM_TO_IVEC(InterruptNumber),
      (VOIDFUNCPTR)InterruptHandler, parameter) != OK)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;

}/* end OS_IntAttachHandler */


/*---------------------------------------------------------------------------------------
   Name: OS_IntDisable

   Purpose: Disable interrupts

   Parameters:
        Level : The Interrupt vectors to be disabled
---------------------------------------------------------------------------------------*/

int32 OS_IntDisable( int32 Level)
{
    int32 Status;
    int32 RetCode;

    Status = intDisable(Level);

    if(Status == OK)
    {
        RetCode = OS_SUCCESS;
    }
    else
    {
        RetCode = OS_ERROR;
    }

    return RetCode;
} /* end OS_IntDisable() */

/*---------------------------------------------------------------------------------------
   Name: OS_IntEnable

   Purpose: Enable interrupts

   Parameters:
        Level : The Interrupt vectors to be Enabled
---------------------------------------------------------------------------------------*/

int32 OS_IntEnable( int32 Level)
{
    int32 Status;
    int32 RetCode;

    Status = intEnable(Level);

    if(Status == OK)
    {
        RetCode = OS_SUCCESS;
    }
    else
    {
        RetCode = OS_ERROR;
    }

    return RetCode;
} /* end OS_IntEnable() */


/*---------------------------------------------------------------------------------------
   Name: OS_IntUnlock

   Purpose: Enable interrupts

   Parameters:
        IntLevel : The Interrupt vectors to be unlocked 
---------------------------------------------------------------------------------------*/
/* The prototype was changed for intUnlock in vxWorks 6.4 */
#if ( _WRS_VXWORKS_MINOR > 3 )
   int32 OS_IntUnlock (int32 IntLevel)
   {
       intUnlock(IntLevel);
       return(OS_SUCCESS);
   }
#else
   int32 OS_IntUnlock (int32 IntLevel)
   {
       int32 Status;
       int32 ReturnCode;
       Status = intUnlock(IntLevel);

       if (Status == ERROR)
       {
           ReturnCode = OS_ERROR;
       }
       else
       {
           ReturnCode = OS_SUCCESS;
       }

       return ReturnCode;

   }/* end OS_IntUnlock */
#endif

/*---------------------------------------------------------------------------------------
   Name: OS_IntLock

   Purpose: Lock the all interupts out

   Parameters:
   
   Returns: Interrupt level before OS_IntLock Call   
---------------------------------------------------------------------------------------*/

int32 OS_IntLock (void)
{
    return  (intLock());


}/* end OS_Intlock */

/*---------------------------------------------------------------------------------------
 *  Name: OS_GetErrorName()
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
 * OS_FindCreator()
 *  This function will return the OSAL ID of the task that created the calling function;
 *  This is an internal call, not to be used by the user.
---------------------------------------------------------------------------------------*/
uint32 OS_FindCreator()
{
    int VxWorks_Id;
    int i;
    VxWorks_Id = taskIdSelf();

    for (i = 0; i < OS_MAX_TASKS; i++)
    {
        if (VxWorks_Id == OS_task_table[i].id)
            break;
    }

    return i;
}
/* ---------------------------------------------------------------------------
 * Name: OS_printf 
 * 
 * Purpose: This function abstracts out the printf type statements. This is 
 *          useful for using OS- specific thats that will allow non-polled
 *          print statements for the real time systems.
 *
 * Note:    This function uses a utility task that gets passed the print
 *          messages on a queue. This allows that task to block (if
 *          necessary), so the calling task does not.
 *
 ---------------------------------------------------------------------------*/
void OS_printf( const char *String, ...)
{
    va_list     ptr;
    char        msg_buffer[OS_BUFFER_SIZE];
    int         ret;

       memset(msg_buffer,0,OS_BUFFER_SIZE);
       va_start(ptr,String);
       vsnprintf(msg_buffer, (size_t)OS_BUFFER_SIZE, String, ptr);
       va_end(ptr);


    #ifdef OS_UTILITY_TASK_ON

       ret = msgQSend(buffQ, msg_buffer, strlen(msg_buffer), NO_WAIT, MSG_PRI_NORMAL);
       if(ret != OK){
        OS_DroppedMessages++;
       }

    #else

       logMsg( &msg_buffer[0],0,0,0,0,0,0);

    #endif


}/* end OS_printf*/

/*---------------------------------------------------------------------------
 * Name:    UtilityTask
 * Purpose: If turned on, this task will print out the messages from
 *          the OS_printf buffer at a low priority. This will mean
 *          that the functions calling OS_printf will not block due to
 *          writing data to the UART
 ----------------------------------------------------------------------------*/ 
#ifdef OS_UTILITY_TASK_ON 
void UtilityTask()
{

  char  message_buff[OS_BUFFER_SIZE];
  int   status;
  
  /* 
  ** give the disk initialization time to start up 
  */
  OS_TaskDelay(3000);
  
   while(TRUE)
   {    
       
       if(OS_DroppedMessages > 0)
       {
	      printf("Dropped Msgs to UART = %d\n",OS_DroppedMessages);
          OS_DroppedMessages=0;
       }


       memset(message_buff, 0, OS_BUFFER_SIZE); 
       status = msgQReceive(buffQ, message_buff, OS_BUFFER_SIZE, WAIT_FOREVER);
       if ( status != ERROR )
          printf("%s",message_buff);
   }
            
}/* End UtilityTask */ 
#endif


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

#ifdef _PPC_
    vxFpscrSet( mask);
#endif 

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

#ifdef _PPC_
    uint32 tempMask;

    tempMask = vxFpscrGet();
    *mask = tempMask;
#endif

    return(OS_SUCCESS);
}

