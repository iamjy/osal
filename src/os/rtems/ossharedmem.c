/*RTEMS */

/* Since RTEMS does not support shared memory, so it is going to be "faked"
 * until there is a time when it is supported
 * */

#include <stdio.h>


/*Internal API's */
int32 OS_InternalMemTableSemTake(void);
int32 OS_InternalMemTableSemGive(void);


typedef struct
{
    boolean         IsFree;                 /* mem partition is in use */
    char            Name [OS_MAX_API_NAME]; /* name of memory partition */
    uint32          Size;                   /* size of the partition */
    void*           ShMemAddress;           /* The area of memory where the data starts */
    sem_t           ShMemSem;               /* One Shared Semaphore for each mem segment */
        
}OS_MemPartition_t

/* Table of Shared Memory Segments */
OS_MemPartition_t OS_ShMemTable [OS_MAX_SHARED_MEM_SEGS];

/* semaphore for accessing the table */
sem_t   OS_ShMemTableSem;

/*---------------------------------------------------------------------------------------
 * Name:    OS_ShMemInit
 *
 * Purpose: This function initializes the table for managing shared memory segments.
 *
 * Returns: OS_SUCCESS
 *          OS_ERROR if the semaphore's can't be created
 * ------------------------------------------------------------------------------------*/
int32 OS_ShMemInit (void)
{
    uint32 i;
    int32 return_code;

    for (i =0; i < OS_MAX_SHARED_MEM_SEGS; i++)
    {
        OS_ShMemTable[i].IsFree = TRUE;
        OS_ShMemTable[i].ShMemId = 0
        OS_ShMemTable[i].Size = 0;
        OS_ShMemTable[i].ShMemAddress = NULL;
        strcpy(OS_ShMemTable[i].Name, "\0");
    }

    
    /* Create RTEMS Semaphore */
	return_code = sem_init(&(OS_ShMemTableSem ),
		1 ,                   /* process sharing */
		OS_SEM_FULL ) ; /* initial value of sem_initial value */
    

    /* check if Create failed */
	if ( return_code != 0 )
    {
		return OS_SEM_FAILURE;
    }

    return OS_SUCCESS;
}/* end OS_ShMemInit */

/*---------------------------------------------------------------------------------------
 * Name:    OS_ShMemCreate
 *
 * Purpose: This function will create a shared memory segment specified by the user,
 *          and will initialize the semaphore that will accompany the segment.
 *          All of the data will be placed into the shared memory table
 *
 * Returns: OS_INVALID_POINTER if the pointers to Id or SegName are NULL
 *          OS_ERR_NAME_TOO_LONG if the name of the segment is too long
 *          OS_ERR_NO_FREE_IDS if the shared memory table is full
 *          OS_ERR_NAME_TAKEN if the name of the segment is already in use
 *          OS_ERROR if an underlying OS call fails
 *          OS_SUCCESS if success, the new shared mem segment Id is passed back 
 *          through Id
 * -----------------------------------------------------------------------------------*/
int32 OS_ShMemCreate( uint32 *Id, uint32 NBytes, char* SegName )
{
    
    int32 PossiblePartId;
    int32 i;
    uint32 FoundSlot = FALSE;
    uint32 FreeName = TRUE;
    int32 PartIdFound;
    int32  Returned;
    void*  TempAddr;
   
    
    if( Id == NULL || SegName == NULL )
    {
        return OS_INVALID_POINTER;
    }
    
    /* We don't want names that are too long */
    if (strlen(SegName) >= OS_MAX_API_NAME)
            return OS_ERR_NAME_TOO_LONG;

    /* Check to make sure the name is not already in use and there is an open slot */

    OS_InternalShMemSemTake();

    /* 
     * The reason why this loop count down is because every time we find an available
     * slot in the table, it will over-write the previously found one. For clarity 
     * for the user, I want the first one created to have an ID of 0, the second 1, etc 
     * Instead of the other way around 
     * */
    
    for(PossiblePartId = OS_MAX_SHARED_MEM_SEGS -1; PossiblePartId >= 0; PossiblePartId--)
    {
        if (OS_ShMemTable[PossiblePartId].IsFree  == TRUE)
        {
            FoundSlot = TRUE;
            PartIdFound = PossiblePartId;
        }

        /* if the Id is in use and the name is the same, we do not have a free name */
        if ( (OS_ShMemTable[PossiblePartId].IsFree == FALSE) &&
                (strcmp (SegName, OS_ShMemTable[PossiblePartId].Name ) == 0 ) )
        {
            FreeName = FALSE;
        }
    }
    
    OS_InternalShMemSemGive();

    
    if (FoundSlot == FALSE)
    {
        return OS_ERR_NO_FREE_IDS;
    }

    if (FreeName == FALSE)
    {
        return OS_ERR_NAME_TAKEN;
    }

    /* Now we can create a OS Shared memory Segment */


    TempAddr = malloc(NBytes);

    
    /* Now We can create the shared semaphore that goes along with this piece of memory */

       
    /* Create RTEMS Semaphore */
	Returned = sem_init(&(OS_ShMemTable[PartIdFound].ShMemSem),
		1 ,                   /* process sharing */
		OS_SEM_FULL ) ; /* initial value of sem_initial value */
    
    /* Now we can add all of the info we need to the table */

    if (Returned != 0)
    {
        free(TempAddr);
        return OS_SEM_FAILURE;
    }

    OS_InternalShMemSemTake();
  
    OS_ShMemTable[PartIdFound].IsFree = FALSE;
    strcpy(OS_ShMemTable[PartIdFound].Name, SegName);
    OS_ShMemTable[PartIdFound].Size = NBytes;                   
    OS_ShMemTable[PartIdFound].ShMemAddress = TempAddr;

    OS_InternalShMemSemGive();

    /* Give the Id found to the caller */
    *Id= PartIdFound;

    return OS_SUCCESS;
}/* end OS_ShMemCreate */

/*---------------------------------------------------------------------------------------
 * Name:    OS_ShMemGetIdByName
 *
 * Purpose: Given the name of a shared memory segment, this function returns the Id of 
 *          that segment
 *
 * Returns: OS_INVALID_POINTER if the parameters are NULL
 *          OS_ERR_NAME_TOO_LONG if the given name is too long
 *          OS_ERR_NAME_NOT_FOUND if the name was not found in the table
 *          OS_SUCCESS if success. The found Id will be passed back through Id
 * ------------------------------------------------------------------------------------*/
int32 OS_ShMemGetIdByName(uint32 *ShMemId, const char *SegName)
{
    uint32 i;

    if (ShMemId == NULL || SegName == NULL)
    {
        return OS_INVALID_POINTER;
    }
    /* we don't want to allow names too long because they won't be found at all */

    if (strlen(SegName) >= OS_MAX_API_NAME)
    {
        return OS_ERR_NAME_TOO_LONG;
    }

    for (i = 0; i < OS_MAX_SHARED_MEM_SEGS; i++)
    {
        if((OS_ShMemTable[i].IsFree != TRUE) &&
          (strcmp(OS_ShMemTable[i].Name,(char *)SegName) == 0 ))
        {
            *ShMemId = i;
            return OS_SUCCESS;
        }
    }
    /* The name was not found in the table,
     *  or it was, and the task_id isn't valid anymore */
    return OS_ERR_NAME_NOT_FOUND;

}/* end OS_ShMemGetIdByName */

/*---------------------------------------------------------------------------------------
 * Name:    OS_ShMemSemTake
 *
 * Purpose: This function takes a semaphore associated with a shared memory segment
 *
 * Returns: OS_ERR_INVALID_ID if the Id give is out of range, or is not in use
 *          OS_SEM_FAILURE if the underlying take command fails
 *          OS_SUCCESS if success
 * ------------------------------------------------------------------------------------*/
int32 OS_ShMemSemTake   (uint32 Id)
{
    uint32   ReturnValue;
    int32    Returned;

    if(Id >= OS_MAX_SHARED_MEM_SEGS || OS_ShMemTable[Id].IsFree == TRUE)
    {
        return OS_ERR_INVALID_ID;
    }       

    /* Take RTEMS Semaphore */
	Returned = sem_wait(&(OS_ShMemTable[Id].ShMemSem ));
    
    if ( Returned == 0 )
    {
       ReturnValue = OS_SUCCESS;
    }
    else
    {
       ReturnValue = OS_SEM_FAILURE;
    }
    
    return ReturnValue;
    
}/* end OS_ShMemSemTake */

/*---------------------------------------------------------------------------------------
 * Name:    OS_ShMemSemGive
 *
 * Purpose: This function gives a semaphore associated with a shared memory segment
 *
 * Returns: OS_ERR_INVALID_ID if the Id give is out of range, or is not in use
 *          OS_SEM_FAILURE if the underlying take command fails
 *          OS_SUCCESS if success
 * ------------------------------------------------------------------------------------*/
int32 OS_ShMemSemGive   (uint32 Id)
{
    uint32 ReturnValue ;
    int32    Returned;
       
    
    if(Id >= OS_MAX_SHARED_MEM_SEGS || OS_ShMemTable[Id].IsFree == TRUE)
    {
        return OS_ERR_INVALID_ID;
    }

    Returned = sem_post(&(OS_ShMemTable[Id].ShMemSem ));

    if ( Returned == 0 )
    {
       ReturnValue = OS_SUCCESS;
    }
    else
    {
       ReturnValue = OS_SEM_FAILURE;
    }
    
    return ReturnValue;

}/* end OS_ShMemSemGive */

/*---------------------------------------------------------------------------------------
 * Name:    OS_ShMemAttach
 *
 * Purpose: This function Attaches the caller to the shared mem segment
 *
 * Returns: OS_ERR_INVALID_ID if the Id passed to it is invalid
 *          OS_SUCCESS if success the address to the memory is passed back inaddress
 *
 * ------------------------------------------------------------------------------------*/
int32 OS_ShMemAttach    (uint32 * Address, uint32 Id)
{
    if(Id >= OS_MAX_SHARED_MEM_SEGS || OS_ShMemTable[Id].IsFree == TRUE)
    {
        return OS_ERR_INVALID_ID;
    }

    *Address = (uint32) (uint32*) OS_ShMemTable[Id].Address;
    
    return OS_SUCCESS;
       
}/* end OS_ShMemAttach */
    

/************ INTERNAL APIS ************************************************************/
/*-------------------------------------------------------------------------------------*/
int32 OS_InternalMemTableSemTake(void)
{
       
	if ( sem_wait(&(OS_ShMemTableSem)) != 0)
    {
	    return OS_SEM_FAILURE;
    }
    else
    {
        return OS_SUCCESS;
    }
}/* End OS_InternalMemTableSemTake */



/*-------------------------------------------------------------------------------------*/
int32 OS_InternalMemTableSemGive(void)
{
	if(  sem_post(&(OS_ShMemTableSem)) != 0)
    {
		return OS_SEM_FAILURE ;
    }
	else
    {
		return  OS_SUCCESS ;    
    }
    
}/* End OS_InternalMemTableSemGive */

