/*
** example1.c
**
** This is an example OSAL Application. This Application creates a couple of tasks, 
** and passes messages back and forth using queues
*/

#include <stdio.h>
#include "common_types.h"
#include "osapi.h"


/* Task 1 */

#define TASK_1_ID         1
#define TASK_1_STACK_SIZE 1024
#define TASK_1_PRIORITY   101

uint32 task_1_stack[TASK_1_STACK_SIZE];

void task_1(void);


/* Task 2 */

#define TASK_2_ID         2
#define TASK_2_STACK_SIZE 1024
#define TASK_2_PRIORITY   102

uint32 task_2_stack[TASK_2_STACK_SIZE];

void task_2(void);


/* Task 3 */

#define TASK_3_ID         3
#define TASK_3_STACK_SIZE 1024
#define TASK_3_PRIORITY   103

uint32 task_3_stack[TASK_3_STACK_SIZE];

void task_3(void);


/* OS Constructs */

#define MSGQ_ID           1
#define MSGQ_DEPTH        50
#define MSGQ_SIZE         4

#define MUTEX_ID          1

uint32 task_1_id, task_2_id, task_3_id;

uint32 mutex_id, msgq_id;
/* Global Data */

uint32 shared_resource_x;


/* ********************** MAIN **************************** */

void OS_Application_Startup(void)
{
	printf("********If You see this, we got into OS_Application_Startup****\n");

  /*printf("calling Create Task 1\n");*/
  OS_TaskCreate( &task_1_id, "Task 1", (void *)task_1, task_1_stack, TASK_1_STACK_SIZE, TASK_1_PRIORITY, 0);

  /*printf("calling Create Task 2\n");*/
  OS_TaskCreate( &task_2_id, "Task 2", (void *)task_2, task_2_stack, TASK_2_STACK_SIZE, TASK_2_PRIORITY, 0);

  /*printf("calling Create Task 3\n");*/
  OS_TaskCreate( &task_3_id, "Task 3", (void *)task_3, task_3_stack, TASK_3_STACK_SIZE, TASK_3_PRIORITY, 0);

  OS_QueueCreate( &msgq_id, "MsgQ", MSGQ_DEPTH, MSGQ_SIZE, 0);

  OS_MutSemCreate( &mutex_id, "Mutex", 0);
}


/* ********************** TASK 1 **************************** */

void task_1(void)
{
	printf("In task 1\n");
    OS_TaskRegister();

    while(1)
    {
        OS_MutSemTake(mutex_id);

            shared_resource_x = task_1_id;

            OS_QueuePut(msgq_id, (void*)&shared_resource_x, sizeof(uint32), 0);

            shared_resource_x = task_1_id;
 
            OS_QueuePut(msgq_id, (void*)&shared_resource_x, sizeof(uint32), 0);

        OS_MutSemGive(mutex_id);

        OS_TaskDelay(100);
    }
}


/* ********************** TASK 2 **************************** */

void task_2(void)
{
	printf("In task 2\n");
	OS_TaskRegister();

    while(1)
    {
        OS_MutSemTake(mutex_id);

            shared_resource_x = task_2_id;

            OS_QueuePut(msgq_id, (void*)&shared_resource_x, sizeof(uint32), 0);
              
            OS_TaskDelay(150);

            shared_resource_x = task_2_id;

            OS_QueuePut(msgq_id, (void*)&shared_resource_x, sizeof(uint32), 0);

        OS_MutSemGive(mutex_id);

        OS_TaskDelay(500);
    }  
}


/* ********************** TASK 3 **************************** */

void task_3(void)
{   
    
	uint32 data_received;
    uint32 data_size;
    uint32 status;

    OS_TaskRegister();

    while(1)
    {
        status = OS_QueueGet(msgq_id, (void*)&data_received, MSGQ_SIZE, &data_size, OS_PEND);
   
        if (status == OS_SUCCESS)
        {
            printf("Received - %d\n", (int)data_received+1);
        } 
    }
}
