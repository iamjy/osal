/*
** File   : osfileapi.c

** Author : Nicholas Yanchik

** Purpose: This file Contains all of the api calls for manipulating
            files in a file system for vxworks
** $Date: 2007/10/16 16:15:01EDT $
** $Revision: 1.1 $
** $Log: osfileapi.c  $
** Revision 1.1 2007/10/16 16:15:01EDT apcudmore 
** Initial revision
** Member added to project d:/mksdata/MKS-OSAL-REPOSITORY/src/os/vxworks6/project.pj
** Revision 1.21 2007/08/22 08:31:47EDT njyanchik 
** Add OS config Parameter for the shell input cmd file name for VxWorks6
** Revision 1.20 2007/08/21 11:00:21EDT njyanchik 
** I added a delay in the telemetry sending of the output so the pipe doesn't get flooded on large 
** messages, I also fixed the file descriptor implementation on vxworks (it was not updated with 
** previous file system updates), so that the shell is now reading and writing the correct files.
** Revision 1.19 2007/07/05 14:58:59EDT njyanchik 
** I sem-protected a bigger block in each of the create functions, and set the free flag (or IsValid 
** flag) to false before sem protection ends. That way another task can't get the same ID. This
** was done for the OS_open, OS_creat, and all OS_*Create calls.
** Revision 1.18 2007/04/03 09:54:35EDT njyanchik 
** There were errors in the prologue information for OS_cp and OS_mv. These erros were fixed
** Revision 1.17 2007/03/21 14:33:57EST njyanchik 
** I had a bad way of dereferencing a pointer which caused a compile error in OS_FDGetInfo.
** The error has been fixed in all OS's
** Revision 1.16 2007/03/21 10:15:30EST njyanchik 
** I mistakenly put the wrong length in for the path in the OS_FDTableEntry structure, and I added 
** some code that will set and out of range file descriptors .IsValid flag to false in OS_FDGetInfo
** Revision 1.15 2007/03/16 10:04:02EST njyanchik 
** There were a couple of warnings about a variable not being use ( those variables were removed),
** and a #include was not included, so there were two implicitly defined functions.
** Revision 1.14 2007/03/07 14:24:17EST njyanchik 
** Update to make shell command return an error if there if creation of file fails
** Revision 1.13 2007/03/06 11:50:56EST njyanchik 
** The function OS_FDGetInfo's parameters were backwards compared to the other OS*GetInfo
** functions. Its parameters were changed to match the other functions
*/

/****************************************************************************************
                                    INCLUDE FILES
****************************************************************************************/

#include "vxWorks.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "ioLib.h"
#include "ramDrv.h"
#include "dosFsLib.h"

#include "dirent.h"
#include "stat.h"

#include "shellLib.h"
#include "taskLib.h"
#include "sysLib.h"
#include "semLib.h"
#include "usrLib.h"

#include "cbioLib.h"
#include "ramDiskCbio.h"

#include "common_types.h"
#include "osapi.h"





/****************************************************************************************
                                     DEFINES
****************************************************************************************/



/****************************************************************************************
                                   GLOBAL DATA
****************************************************************************************/
OS_FDTableEntry OS_FDTable[OS_MAX_NUM_OPEN_FILES];
SEM_ID  OS_FDTableMutex;

extern uint32 OS_FindCreator(void);
int32 OS_check_name_length(const char *path);
int32 OS_NameChange( char* name);


/****************************************************************************************
                                INITIALIZATION FUNCTION
****************************************************************************************/

void OS_FS_Init(void)
{
    int i;
    /* Initialize the file system constructs */
    for (i =0; i < OS_MAX_NUM_OPEN_FILES; i++)
    {
        OS_FDTable[i].OSfd =       -1;
        strcpy(OS_FDTable[i].Path, "\0");
        OS_FDTable[i].User =       0;
        OS_FDTable[i].IsValid =    FALSE;
    }
    
   OS_FDTableMutex = semMCreate(SEM_Q_PRIORITY); 

}
/****************************************************************************************
                                    Filesys API
****************************************************************************************/

/*
** Standard File system API
*/

/*--------------------------------------------------------------------------------------
    Name: OS_creat
    
    Purpose: creates a file specified by const char *path, with read/write 
             permissions by access. The file is also automatically opened by the
             create call.
    
    Returns: OS_FS_INVALID_POINTER if path is NULL
             OS_FS_ERR_PATH_INVALID if path cannot be parsed
             OS_FS_PATH_TOO_LONG if path exceeds the maximum number of chars
             OS_FS_ERR_NAME_TOO_LONG if the name of the file is too long
             OS_FS_ERROR if permissions are unknown or OS call fails
             OS_FS_ERR_NO_FREE_FDS if there are no free file descriptors
             a file descriptor if success
    
---------------------------------------------------------------------------------------*/

int32 OS_creat  (const char *path, int32  access)
{
    int status;
    char local_path[OS_MAX_PATH_LEN];
    uint32 PossibleFD;

    if (path == NULL)
        return OS_FS_ERR_INVALID_POINTER;
   
    if (strlen(path) >= OS_MAX_PATH_LEN)
        return OS_FS_ERR_PATH_TOO_LONG;
   
    /* make a local copy of the path */
    strcpy(local_path, path);
    if(OS_NameChange(local_path) != OS_FS_SUCCESS)
    {
        return OS_FS_ERR_PATH_INVALID;
    }


    /* check if the name of the file is too long */
    if (OS_check_name_length(local_path) != OS_FS_SUCCESS)
        return OS_FS_ERR_NAME_TOO_LONG;

    semTake(OS_FDTableMutex,WAIT_FOREVER);
    for ( PossibleFD = 0; PossibleFD < OS_MAX_NUM_OPEN_FILES; PossibleFD++)
    {
        if( OS_FDTable[PossibleFD].IsValid == FALSE)
        {
            break;
        }
    }

    if (PossibleFD >= OS_MAX_NUM_OPEN_FILES)
    {
        semGive(OS_FDTableMutex);
        return OS_FS_ERR_NO_FREE_FDS;
    }

    /* Mark the table entry as valid so no other 
     * task can take that ID */
    OS_FDTable[PossibleFD].IsValid =    TRUE; 
    
    semGive(OS_FDTableMutex);

    status = creat(local_path, (int) access);

    semTake(OS_FDTableMutex,WAIT_FOREVER);

    if (status != ERROR)
    {
        /* fill in the table before returning */

        OS_FDTable[PossibleFD].OSfd =       status;
        strncpy(OS_FDTable[PossibleFD].Path, path, OS_MAX_API_NAME);
        OS_FDTable[PossibleFD].User =       OS_FindCreator();
        semGive(OS_FDTableMutex);

        return PossibleFD;
    }
    else
    {   OS_FDTable[PossibleFD].IsValid = FALSE; 
        semGive(OS_FDTableMutex);
        return OS_FS_ERROR;
    }
} /* end OS_creat */

/*--------------------------------------------------------------------------------------
    Name: OS_open
    
    Purpose: Opens a file. access parameters are OS_READ_ONLY,OS_WRITE_ONLY, or 
             OS_READ_WRITE

    Returns: OS_FS_INVALID_POINTER if path is NULL
             OS_FS_ERR_PATH_INVALID if path cannot be parsed
             OS_FS_PATH_TOO_LONG if path exceeds the maximum number of chars
             OS_FS_ERR_NAME_TOO_LONG if the name of the file is too long
             OS_FS_ERROR if permissions are unknown or OS call fails
             OS_FS_ERR_NO_FREE_FDS if there are no free file descriptors
             a file descriptor if success
---------------------------------------------------------------------------------------*/

int32 OS_open   (const char *path,  int32 access,  uint32  mode)
{
    int status;
    char local_path[OS_MAX_PATH_LEN];
    uint32 PossibleFD;
    
    
    if(path == NULL)
       return OS_FS_ERR_INVALID_POINTER;

    if (strlen(path) >= OS_MAX_PATH_LEN)
        return OS_FS_ERR_PATH_TOO_LONG;
   
    /* make a local copy of the path */
    strcpy(local_path, path);
    if(OS_NameChange(local_path) != OS_FS_SUCCESS)
    {
        return OS_FS_ERR_PATH_INVALID;
    }


    
    /* check if the name of the file is too long */
    if (OS_check_name_length(local_path) == OS_FS_ERROR)
        return OS_FS_ERR_NAME_TOO_LONG;
    
    semTake(OS_FDTableMutex,WAIT_FOREVER);

    for ( PossibleFD = 0; PossibleFD < OS_MAX_NUM_OPEN_FILES; PossibleFD++)
    {
        if( OS_FDTable[PossibleFD].IsValid == FALSE)
        {
            break;
        }
    }

    if (PossibleFD >= OS_MAX_NUM_OPEN_FILES)
    {
        semGive(OS_FDTableMutex);
        return OS_FS_ERR_NO_FREE_FDS;
    }

    /* Mark the table entry as valid so no other 
     * task can take that ID */
    OS_FDTable[PossibleFD].IsValid =    TRUE; 
    
    semGive(OS_FDTableMutex);  

    /* mode is not used in dosFs, just in NFS  drives */
    status = open(local_path, (int) access, (int) mode);

    semTake(OS_FDTableMutex,WAIT_FOREVER);

    if (status != ERROR)
    {
        /* fill in the table before returning */

        OS_FDTable[PossibleFD].OSfd =       status;
        strncpy(OS_FDTable[PossibleFD].Path, path, OS_MAX_API_NAME);
        OS_FDTable[PossibleFD].User =       OS_FindCreator();
        semGive(OS_FDTableMutex);

        return PossibleFD;
    }
    else
    {   OS_FDTable[PossibleFD].IsValid = FALSE; 
        semGive(OS_FDTableMutex);
        return OS_FS_ERROR;
    }

} /* end OS_open */

/*--------------------------------------------------------------------------------------
    Name: OS_close
    
    Purpose: Closes a file. 

    Returns: OS_FS_ERROR if file  descriptor could not be closed
             OS_FS_ERR_INVALID_FD if the file descriptor passed in is invalid
             OS_FS_SUCCESS if success
---------------------------------------------------------------------------------------*/

int32 OS_close (int32  filedes)
{
      int status;

    /* Make sure the file descriptor is legit before using it */
    if (filedes < 0 || filedes >= OS_MAX_NUM_OPEN_FILES || OS_FDTable[filedes].IsValid == FALSE)
    {
        return OS_FS_ERR_INVALID_FD;
    }
    else
    {    
        status = close ((int) OS_FDTable[filedes].OSfd);
        if (status == ERROR)
        {
            return OS_FS_ERROR;
        }
        else
        {
            /* fill in the table before returning */
            semTake(OS_FDTableMutex,WAIT_FOREVER);
            OS_FDTable[filedes].OSfd =       -1;
            strcpy(OS_FDTable[filedes].Path, "\0");
            OS_FDTable[filedes].User =       0;
            OS_FDTable[filedes].IsValid =    FALSE;
            semGive(OS_FDTableMutex);

            return OS_FS_SUCCESS;
        }

    }    
}/* end OS_close */

/*--------------------------------------------------------------------------------------
    Name: OS_read
    
    Purpose: reads up to nbytes from a file, and puts them into buffer. 
    
    Returns: OS_FS_ERR_INVALID_POINTER if buffer is a null pointer
             OS_FS_ERR_INVALID_FD if the file descriptor passed in is invalid
             OS_FS_ERROR if OS call failed
             number of bytes read if success
---------------------------------------------------------------------------------------*/
int32 OS_read  (int32  filedes, void *buffer, uint32 nbytes)
{
    int32 status;

    if (buffer == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    /* Make sure the file descriptor is legit before using it */
    if (filedes < 0 || filedes >= OS_MAX_NUM_OPEN_FILES || OS_FDTable[filedes].IsValid == FALSE)
    {
        return OS_FS_ERR_INVALID_FD;
    }
    else
    { 
        status = read (OS_FDTable[filedes].OSfd, (char*) buffer, (size_t) nbytes);
 
        if (status == ERROR)
            return OS_FS_ERROR;
    }

    return status;
        
}/* end OS_read */

/*--------------------------------------------------------------------------------------
    Name: OS_write

    Purpose: writes to a file. copies up to a maximum of nbtyes of buffer to the file
             described in filedes

    Returns: OS_FS_INVALID_POINTER if buffer is NULL
             OS_FS_ERR_INVALID_FD if the file descriptor passed in is invalid
             OS_FS_ERROR if OS call failed
             number of bytes written if success
---------------------------------------------------------------------------------------*/

int32 OS_write (int32  filedes, void *buffer, uint32 nbytes)
{
    int32 status;

    if (buffer == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    /* Make sure the file descriptor is legit before using it */
    if (filedes < 0 || filedes >= OS_MAX_NUM_OPEN_FILES || OS_FDTable[filedes].IsValid == FALSE)
    {
        return OS_FS_ERR_INVALID_FD;
    }
    else
    {
        status = write(OS_FDTable[filedes].OSfd, buffer, nbytes );
    
        if (status != ERROR)
            return  status;
        else
            return OS_FS_ERROR;
    }    
}/* end OS_write */

/*--------------------------------------------------------------------------------------
    Name: OS_chmod

    Notes: This is not going to be implemented because there is no use for this function.
---------------------------------------------------------------------------------------*/

int32 OS_chmod  (const char *path, uint32 access)
{
     return OS_FS_UNIMPLEMENTED;
} /* end OS_chmod */

/*--------------------------------------------------------------------------------------
    Name: OS_stat
    
    Purpose: returns information about a file or directory in a os_fs_stat structure
    
    Returns: OS_FS_ERR_INVALID_POINTER if path or filestats is NULL
             OS_FS_ERR_PATH_TOO_LONG if the path is too long to be stored locally
             OS_FS_ERR_NAME_TOO_LONG if the name of the file is too long to be stored
             OS_FS_ERR_PATH_INVALID if path cannot be parsed
             OS_FS_ERROR id the OS call failed
             OS_FS_SUCCESS if success

    Note: The information returned is in the structure pointed to by filestats         
---------------------------------------------------------------------------------------*/

int32 OS_stat   (const char *path, os_fstat_t  *filestats)
{
    int ret_val;
    char local_path[OS_MAX_PATH_LEN];
    
    if (path == NULL || filestats == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    if (strlen(path) >= OS_MAX_PATH_LEN)
        return OS_FS_ERR_PATH_TOO_LONG;
   
    /* make a local copy of the path */
    strcpy(local_path, path);
    if(OS_NameChange(local_path) != OS_FS_SUCCESS)
    {
        return OS_FS_ERR_PATH_INVALID;
    }

    
    /* check if the name of the file is too long */
    if (OS_check_name_length(local_path) == OS_FS_ERROR)
        return OS_FS_ERR_NAME_TOO_LONG;

    ret_val = stat( (char*) local_path, filestats);
    if (ret_val == ERROR)
        return OS_FS_ERROR;
    else
        return OS_FS_SUCCESS;
    
} /* end OS_stat */

/*--------------------------------------------------------------------------------------
    Name: OS_lseek

    Purpose: sets the read/write pointer to a specific offset in a specific file. 
             Whence is either OS_SEEK_SET,OS_SEEK_CUR, or OS_SEEK_END

    Returns: the new offset from the beginning of the file
             OS_FS_ERR_INVALID_FD if the file descriptor passed in is invalid
             OS_FS_ERROR if OS call failed
---------------------------------------------------------------------------------------*/

int32 OS_lseek  (int32  filedes, int32 offset, uint32 whence)
{
     int status;
     int where;

    /* Make sure the file descriptor is legit before using it */
    if (filedes < 0 || filedes >= OS_MAX_NUM_OPEN_FILES || OS_FDTable[filedes].IsValid == FALSE)
    {
        return OS_FS_ERR_INVALID_FD;
    }
    else
    {
        switch(whence)
        {
            case OS_SEEK_SET:
                where = SEEK_SET;
                break;
            case OS_SEEK_CUR:
                where = SEEK_CUR;
                break;
            case OS_SEEK_END:
                where = SEEK_END;
                break;
            default:
                return OS_FS_ERROR;
        }

    
        status = lseek( OS_FDTable[filedes].OSfd, (int) offset, (int) where );

        if ( (int) status != ERROR)
            return (int32) status;
        else
            return OS_FS_ERROR;
    } 
}/* end OS_lseek */

/*--------------------------------------------------------------------------------------
    Name: OS_remove

    Purpose: removes a given filename from the drive
 
    Returns: OS_FS_SUCCESS if the driver returns OK
             OS_FS_ERROR if there is no device or the driver returns error
             OS_FS_ERR_INVALID_POINTER if path is NULL
             OS_FS_ERR_PATH_INVALID if path cannot be parsed
             OS_FS_ERR_PATH_TOO_LONG if path is too long to be stored locally
             OS_FS_ERR_NAME_TOO_LONG if the name of the file to remove is too long to be
             stored locally
---------------------------------------------------------------------------------------*/

int32 OS_remove (const char *path)
{
    int status;
    char local_path[OS_MAX_PATH_LEN];
    
    if (path == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    if (strlen(path) >= OS_MAX_PATH_LEN)
        return OS_FS_ERR_PATH_TOO_LONG;
   
    /* make a local copy of the path */
    strcpy(local_path, path);
    if(OS_NameChange(local_path) != OS_FS_SUCCESS)
    {
        return OS_FS_ERR_PATH_INVALID;
    }


    
    /* check if the name of the file is too long */
    if (OS_check_name_length(local_path) == OS_FS_ERROR)
        return OS_FS_ERR_NAME_TOO_LONG;

    status = remove (local_path);
    if (status == OK)
        return OS_FS_SUCCESS;
    else
        return OS_FS_ERROR;
    
} /* end OS_remove */

/*--------------------------------------------------------------------------------------
    Name: OS_rename
    
    Purpose: renames a file

    Returns: OS_FS_SUCCESS if the rename works
             OS_FS_ERROR if the file could not be opened or renamed.
             OS_FS_INVALID_POINTER if old or new are NULL
             OS_FS_ERR_PATH_INVALID if path cannot be parsed
             OS_FS_ERR_PATH_TOO_LONG if the paths given are too long to be stored locally
             OS_FS_ERR_NAME_TOO_LONG if the new name is too long to be stored locally
---------------------------------------------------------------------------------------*/
int32 OS_rename (const char *old, const char *new)
{
    int status;
    char old_path[OS_MAX_PATH_LEN];
    char new_path[OS_MAX_PATH_LEN];
    
    if (old == NULL || new == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    if (strlen(old) >= OS_MAX_PATH_LEN)
        return OS_FS_ERR_PATH_TOO_LONG;
    
    if (strlen(new) >= OS_MAX_PATH_LEN)
        return OS_FS_ERR_PATH_TOO_LONG;

    /* make a local copy of the path */
    strcpy(old_path, old);
    if(OS_NameChange(old_path) != OS_FS_SUCCESS)
    {
        return OS_FS_ERR_PATH_INVALID;
    }


    strcpy(new_path, new);
    if(OS_NameChange(new_path) != OS_FS_SUCCESS)
    {
        return OS_FS_ERR_PATH_INVALID;
    }


    /* check if the name of the file is too long */
    if (OS_check_name_length(new_path) == OS_FS_ERROR)
        return OS_FS_ERR_NAME_TOO_LONG;

     status = rename (old_path, new_path);
     if (status == OK)
         return OS_FS_SUCCESS;
     else
         return OS_FS_ERROR;
     
}/*end OS_rename */
/*--------------------------------------------------------------------------------------
    Name: OS_cp
    
    Purpose: Copies a single file from src to dest

    Returns: OS_FS_SUCCESS if the operation worked
             OS_FS_ERROR if the file could not be accessed
             OS_FS_INVALID_POINTER if src or dest are NULL
             OS_FS_ERR_PATH_INVALID if path cannot be parsed
             OS_FS_ERR_PATH_TOO_LONG if the paths given are too long to be stored locally
             OS_FS_ERR_NAME_TOO_LONG if the dest name is too long to be stored locally
---------------------------------------------------------------------------------------*/

int32 OS_cp (const char *src, const char *dest)
{
    int status;
    char src_path[OS_MAX_PATH_LEN];
    char dest_path[OS_MAX_PATH_LEN];

    if (src == NULL || dest == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    if (strlen(src) >= OS_MAX_PATH_LEN)
        return OS_FS_ERR_PATH_TOO_LONG;
    
    if (strlen(dest) >= OS_MAX_PATH_LEN)
        return OS_FS_ERR_PATH_TOO_LONG;

    /* make a local copy of the path */
    strcpy(src_path, src);
    strcpy(dest_path, dest);
    if(OS_NameChange(src_path) != OS_FS_SUCCESS)
    {
        return OS_FS_ERR_PATH_INVALID;
    }
    if(OS_NameChange(dest_path) != OS_FS_SUCCESS)
    {
        return OS_FS_ERR_PATH_INVALID;
    }

     status = cp(src_path, dest_path);
     
     if (status != ERROR)
         return OS_FS_SUCCESS;
     else
         return OS_FS_ERROR;
     
}/*end OS_cp */

/*--------------------------------------------------------------------------------------
    Name: OS_mv
    
    Purpose: moves a single file from src to dest

    Returns: OS_FS_SUCCESS if the operation worked
             OS_FS_ERROR if the file could not be accessed
             OS_FS_INVALID_POINTER if src or dest are NULL
             OS_FS_ERR_PATH_INVALID if path cannot be parsed
             OS_FS_ERR_PATH_TOO_LONG if the paths given are too long to be stored locally
             OS_FS_ERR_NAME_TOO_LONG if the dest name is too long to be stored locally
---------------------------------------------------------------------------------------*/

int32 OS_mv (const char *src, const char *dest)
{
    int status;
    char src_path[OS_MAX_PATH_LEN];
    char dest_path[OS_MAX_PATH_LEN];

    if (src == NULL || dest == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    if (strlen(src) >= OS_MAX_PATH_LEN)
        return OS_FS_ERR_PATH_TOO_LONG;
    
    if (strlen(dest) >= OS_MAX_PATH_LEN)
        return OS_FS_ERR_PATH_TOO_LONG;

    /* make a local copy of the path */
    strcpy(src_path, src);
    strcpy(dest_path, dest);
    if(OS_NameChange(src_path) != OS_FS_SUCCESS)
    {
        return OS_FS_ERR_PATH_INVALID;
    }
    if(OS_NameChange(dest_path) != OS_FS_SUCCESS)
    {
        return OS_FS_ERR_PATH_INVALID;
    }

    status = mv(src_path, dest_path);
    
     if (status != ERROR)
         return OS_FS_SUCCESS;
     else
         return OS_FS_ERROR;
     
}/*end OS_mv */


/*
** Directory API 
*/
/*--------------------------------------------------------------------------------------
    Name: OS_mkdir

    Purpose: makes a directory specified by path.

    Returns: OS_FS_ERR_INVALID_POINTER if path is NULL
             OS_FS_ERR_PATH_TOO_LONG if the path is too long to be stored locally
             OS_FS_ERROR if the OS call fails
             OS_FS_ERR_PATH_INVALID if path cannot be parsed
             OS_FS_SUCCESS if success

    Note: The access parameter is currently unused.
---------------------------------------------------------------------------------------*/
int32 OS_mkdir (const char *path, uint32 access)
{
   STATUS status;
      char local_path[OS_MAX_PATH_LEN];

    if (path == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    if (strlen(path) >= OS_MAX_PATH_LEN)
        return OS_FS_ERR_PATH_TOO_LONG;
   
    /* make a local copy of the path */
    strcpy(local_path, path);
    if(OS_NameChange(local_path) != OS_FS_SUCCESS)
    {
        return OS_FS_ERR_PATH_INVALID;
    }


    status = mkdir(local_path);

    if (status == OK)
        return OS_FS_SUCCESS;
    else
        return OS_FS_ERROR;
    
}/* end OS_mkdir */

/*--------------------------------------------------------------------------------------
    Name: OS_opendir

    Purpose: opens a directory for searching

    Returns: NULL if path is NULL,path is too long, OS call fails
             a pointer to a directory if success
---------------------------------------------------------------------------------------*/

os_dirp_t OS_opendir (const char *path)
{

    os_dirp_t dirdescptr;
    char local_path[OS_MAX_PATH_LEN];
    
    if (path == NULL)
        return NULL;

    if (strlen(path) > OS_MAX_PATH_LEN)
        return NULL;
   
    /* make a local copy of the path */
    strcpy(local_path, path);
    if(OS_NameChange(local_path) != OS_FS_SUCCESS)
    {
        return NULL;
    }

    dirdescptr = opendir( (char*) local_path);
    
    /* explicitly returns null for clarity */
    if (dirdescptr == NULL)
        return NULL;
    else
        return dirdescptr;
    
} /* end OS_opendir */

/*--------------------------------------------------------------------------------------
    Name: OS_closedir
    
    Purpose: closes a directory

    Returns: OS_FS_SUCCESS if success
             OS_FS_ERROR if close failed
---------------------------------------------------------------------------------------*/
int32 OS_closedir (os_dirp_t directory)
{ 
    int status;

    if (directory == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    status = closedir(directory);
    
    if (status == OK)
        return OS_FS_SUCCESS;
    else
        return OS_FS_ERROR;

} /* end OS_closedir */

/*--------------------------------------------------------------------------------------
    Name: OS_readdir

    Purpose: obtains directory entry data for the next file from an open directory

    Returns: a pointer to the next entry for success
             NULL if error or end of directory is reached
---------------------------------------------------------------------------------------*/

os_dirent_t *  OS_readdir (os_dirp_t directory)
{ 
    os_dirent_t *tempptr;

    errno = OK;
    
    if (directory == NULL)
        return NULL;

    tempptr = readdir( directory);
    
    if (tempptr != NULL)
        return tempptr;

    else
    {
        if (errno != OK)
            return NULL; 
        else
            return NULL;
    }

    /* should never reach this point in the code */
    return NULL;
    
} /* end OS_readdir */

/*--------------------------------------------------------------------------------------
    Name: OS_rmdir
    
    Purpose: removes a directory from  the structure (must be an empty directory)

    Returns: OS_FS_ERR_INVALID_POINTER if path us NULL
             OS_FS_ER_PATH_TOO_LONG
             OS_FS_ERR_PATH_INVALID if path cannot be parsed             
---------------------------------------------------------------------------------------*/

int32  OS_rmdir (const char *path)
{
    STATUS status;
    char local_path [OS_MAX_PATH_LEN];
    
    if (path == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    if (strlen(path) >= OS_MAX_PATH_LEN)
        return OS_FS_ERR_PATH_TOO_LONG;

    strcpy(local_path,path);
    if(OS_NameChange(local_path) != OS_FS_SUCCESS)
    {
        return OS_FS_ERR_PATH_INVALID;
    }

    
    status = rmdir(local_path);
    
    if (status == OK)
        return OS_FS_SUCCESS;
    else
        return OS_FS_ERROR;
    
}/* end OS_rmdir */


/* --------------------------------------------------------------------------------------
    Name: OS_check_path_length
    
    Purpose: Checks the length of the file name at the end of the path.
    
    Returns: OS_FS_ERROR if path is NULL, path is too long, there is no '/' in the path
             name, the name is too long
             OS_SUCCESS if success
            
    NOTE: This is only an internal function and is not intended for use by the user
 ---------------------------------------------------------------------------------------*/
int32 OS_check_name_length(const char *path)
{
    char* name_ptr;
    char* end_of_path;
    int name_len;

    if (path == NULL)
        return OS_FS_ERROR;

    if (strlen(path) > OS_MAX_PATH_LEN)
        return OS_FS_ERROR;
    
    /* checks to see if there is a '/' somewhere in the path */
    name_ptr = strrchr(path, '/');
    
    if (name_ptr == NULL)
        return OS_FS_ERROR;
    
    /* strrchr returns a pointer to the last '/' char, so we advance one char */
    name_ptr = name_ptr + sizeof(char);
    
    /* end_of_path points to the null terminator at the end of the path */
    end_of_path = strrchr(path,'\0');

    /* pointer subraction to see how many characters there are in the name */
    name_len = ((int) end_of_path - (int)name_ptr) / sizeof(char);
    
    if( name_len > OS_MAX_FILE_NAME)
        return OS_FS_ERROR;

    return OS_FS_SUCCESS;
    
}/* end OS_check_name_length */

/* --------------------------------------------------------------------------------------
Name: OS_ShellOutputToFile
    
Purpose: Takes a shell command in and writes the output of that command to the specified file
    
Returns: OS_SUCCESS if success
         OS_FS_ERR_INVALID_FD if the file descriptor passed in is invalid
         OS_FS_ERROR if Error 
---------------------------------------------------------------------------------------*/
int32 OS_ShellOutputToFile(char* Cmd, int32 OS_fd)
{
    char LocalCmd [OS_MAX_CMD_LEN];
    int32 Result;
    int32 ReturnCode = OS_SUCCESS;
    int32 fdCmd;
    char * shellName;

    /* Make sure the file descriptor is legit before using it */
    if (OS_fd < 0 || OS_fd >= OS_MAX_NUM_OPEN_FILES || OS_FDTable[OS_fd].IsValid == FALSE)
    {
        ReturnCode = OS_FS_ERR_INVALID_FD;
    }
    else
    {
        /* Create a file to write the command to (or write over the old one) */
        fdCmd = OS_creat(OS_SHELL_CMD_INPUT_FILE_NAME,OS_READ_WRITE);

        if (fdCmd < OS_FS_SUCCESS)
        {
            Result = OS_FS_ERROR;
        }

        else
        {
            /* copy the command to the file, and then seek back to the beginning of the file */
        
            strncpy(LocalCmd,Cmd, OS_MAX_CMD_LEN);
            OS_write(fdCmd,Cmd, strlen(LocalCmd));
            OS_lseek(fdCmd,0,OS_SEEK_SET);	

            /* Create a shell task the will run the command in the file, push output to OS_fd */
            Result = shellGenericInit("INTERPRETER=Cmd",0,NULL, &shellName, FALSE, FALSE, OS_FDTable[fdCmd].OSfd,	OS_FDTable[OS_fd].OSfd, OS_FDTable[OS_fd].OSfd);

            /* Wait for the command to terminate */
           do{
              taskDelay(sysClkRateGet());
            }while (taskNameToId(shellName) != ERROR);
    
            /* Close the file descriptor */	
            OS_close(fdCmd);

        } /* else */
        
        if (Result != OK)
        {
             ReturnCode =  OS_FS_ERROR;
        }
        
    }
    
    return ReturnCode;
}/* end OS_ShellOutputToFile */

/* --------------------------------------------------------------------------------------
Name: OS_FDGetInfo
    
Purpose: Copies the information of the given file descriptor into a structure passed in
    
Returns: OS_FS_ERR_INVALID_FD if the file descriptor passed in is invalid
         OS_FS_SUCCESS if the copying was successfull
 ---------------------------------------------------------------------------------------*/

int32 OS_FDGetInfo (int32 filedes, OS_FDTableEntry *fd_prop)
{
    /* Make sure the file descriptor is legit before using it */
    if (filedes < 0 || filedes >= OS_MAX_NUM_OPEN_FILES || OS_FDTable[filedes].IsValid == FALSE)
    {
        /* Makse sure user knows this is nota valid descriptor if he forgets to check the
         * return code */
        (*(fd_prop)).IsValid = FALSE;
        return OS_FS_ERR_INVALID_FD;
    }
    else
    { 
        *fd_prop = OS_FDTable[filedes];
        return OS_FS_SUCCESS;
    }

}/* end OS_FDGetInfo */
