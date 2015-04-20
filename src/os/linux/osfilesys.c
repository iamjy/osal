/*
** File   : osfilesys.c

** Author : Nicholas Yanchik / NASA GSFC Code 582.0

** Purpose: This file has the api's for all of the making
            and mounting type of calls for file systems

** $Date: 2007/10/16 16:14:54EDT $
** $Revision: 1.1 $
** $Log: osfilesys.c  $
** Revision 1.1 2007/10/16 16:14:54EDT apcudmore 
** Initial revision
** Member added to project d:/mksdata/MKS-OSAL-REPOSITORY/src/os/linux/project.pj
** Revision 1.11 2007/05/25 09:17:55EDT njyanchik 
** I added the rmfs call to the OSAL and updated the unit test stubs to match
** Revision 1.10 2007/04/24 11:36:38EDT njyanchik 
** I Implemented the followiing fixes:
** Items 1,2,3 are for vxworks 5.5 , so we don't have to change that at all
** Item 4: fixed by adding a check for the length of the volume name (volname) on entry to the function
** Items 5,6, fixed by making the final strcpy a strncpy in OS_NameChange to make sure the string returned is less than or equal to the maximum number of bytes.
** Item 7: fixed by making the first strcpy in OS_NameChange a strncpy to prevent the input from being too long. This way the string length of LocalName won't be too long to use in line 704.
** Item 9: Fixed by making the error number parameter an int32 instead of a uint32
** Revision 1.9 2007/02/27 15:22:02EST njyanchik 
** This CP has the initial import of the new file descripor table mechanism

*/

/****************************************************************************************
                                    INCLUDE FILES
****************************************************************************************/


#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "sys/types.h"
#include "fcntl.h"
#include "unistd.h"
#include "errno.h"

#include "dirent.h"
#include "sys/stat.h"

#include "common_types.h"
#include "osapi.h"

#include <sys/vfs.h>

/****************************************************************************************
                                     DEFINES
****************************************************************************************/

# define ERROR (-1)


/****************************************************************************************
                                   GLOBAL DATA
****************************************************************************************/

int32 OS_NameChange(char* name);



/* 
** This is the volume table reference. It is defined in the BSP/startup code for the board
*/
extern OS_VolumeInfo_t OS_VolumeTable [NUM_TABLE_ENTRIES]; 


/****************************************************************************************
                                Filesys API
****************************************************************************************/

/*
** System Level API 
*/

/*---------------------------------------------------------------------------------------
    Name: OS_mkfs

    Purpose: Makes a file system on the target
    
    Returns: OS_FS_ERR_INVALID_POINTER if devname is NULL
             OS_FS_DRIVE_NOT_CREATED if the OS calls to create the the drive failed
             OS_FS_SUCCESS on creating the disk
---------------------------------------------------------------------------------------*/



int32 OS_mkfs (char *address, char *devname,char * volname, uint32 blocksize, 
               uint32 numblocks)
{
    int i;
    char Command[ OS_MAX_PATH_LEN +10 ];
    char FolderName[OS_MAX_PATH_LEN];
    
    /* find an open entry in the Volume Table */
    for (i = 0; i < NUM_TABLE_ENTRIES; i++)
    {
        if (OS_VolumeTable[i].FreeFlag == TRUE && OS_VolumeTable[i].IsMounted == FALSE
            && strcmp(OS_VolumeTable[i].DeviceName, devname) == 0)
            break;
    }

    if (i >= NUM_TABLE_ENTRIES)
        return OS_FS_ERR_DEVICE_NOT_FREE;


    /* make a disk if it is FS based */
    /*------------------------------- */
    if (OS_VolumeTable[i].VolumeType == FS_BASED)
    {

       /* now enter the info in the table */

       OS_VolumeTable[i].FreeFlag = FALSE;
       strcpy(OS_VolumeTable[i].VolumeName, volname);
       OS_VolumeTable[i].BlockSize = blocksize;
    
       /* note we don't know the mount point yet */
    
       /* for linux/osx we need to make the folder where this drive is located */
       strcpy(FolderName, OS_VolumeTable[i].PhysDevName);
       strcat(FolderName, devname);

       /* because we are making the filesystem new, delete any that that was there
        * before hand and re-create it. */
    
       if ((strcmp(FolderName,"/") == 0) || (strcmp(FolderName,"~") == 0) ||
           (strcmp(FolderName,"/~") == 0) || (strcmp(FolderName,"~/") == 0))
       {
           printf("WARNING! WARNING! The folder you are trying to use for your filesystem is your home directory or your root directory. I'm sorry, Dave. I can't let you do that.");
           return OS_FS_ERR_DEVICE_NOT_FREE;
       }
       
       sprintf(Command,"rm -rf %s", FolderName);
       system(Command);
   
       /* make the directory where the file system lives */
       sprintf(Command,"mkdir %s", FolderName);
       system(Command);
       
    }   /* VolumeType is something else that is not supported right now */
    else
    {
        return OS_FS_ERROR;
    }

   return OS_FS_SUCCESS; 
    
    
} /* end OS_mkfs */

/*---------------------------------------------------------------------------------------
    Name: OS_rmfs

    Purpose: Inititalizes a file system on the target
    
    Returns: OS_FS_ERR_INVALID_POINTER if devname is NULL
             OS_FS_ERROR is the drive specified cannot be located
             OS_FS_SUCCESS on removing  the disk
---------------------------------------------------------------------------------------*/

int32 OS_rmfs (char *devname)
{
    int i;
    int32 ReturnCode;

    if (devname ==NULL)
    {
        ReturnCode =  OS_FS_ERR_INVALID_POINTER;
    }
    else
    {
    
        /* find this entry in the Volume Table */
        for (i = 0; i < NUM_TABLE_ENTRIES; i++)
        {
            if (OS_VolumeTable[i].FreeFlag == FALSE && OS_VolumeTable[i].IsMounted == FALSE
                && strcmp(OS_VolumeTable[i].DeviceName, devname) == 0)
            {
                break;
            }
        }

        /* We can't find that entry in the table */
        if (i >= NUM_TABLE_ENTRIES)
        {
            ReturnCode =  OS_FS_ERROR;
        }
        else
        {
            /* Free this entry in the table */
            OS_VolumeTable[i].FreeFlag = TRUE;
            
            /* desconstruction of the filesystem to come later */

            ReturnCode = OS_FS_SUCCESS;
        }

    }

    return ReturnCode;
}/* end OS_rmfs */

/*---------------------------------------------------------------------------------------
    Name: OS_initfs

    Purpose: Inititalizes a file system on the target
    
    Returns: OS_FS_ERR_INVALID_POINTER if devname is NULL
             OS_FS_DRIVE_NOT_CREATED if the OS calls to create the the drive failed
             OS_FS_SUCCESS on creating the disk
---------------------------------------------------------------------------------------*/
int32 OS_initfs (char *address,char *devname, char *volname, 
                uint32 blocksize, uint32 numblocks)
{
   int i;
    char Command[ OS_MAX_PATH_LEN +10 ];
    char FolderName[OS_MAX_PATH_LEN];
    
    /* find an open entry in the Volume Table */
    for (i = 0; i < NUM_TABLE_ENTRIES; i++)
    {
        if (OS_VolumeTable[i].FreeFlag == TRUE && OS_VolumeTable[i].IsMounted == FALSE
            && strcmp(OS_VolumeTable[i].DeviceName, devname) == 0)
            break;
    }

    if (i >= NUM_TABLE_ENTRIES)
        return OS_FS_ERR_DEVICE_NOT_FREE;


    if(strlen(devname) > 32 || strlen(volname) > 30) /* 32 - 2 for ':0' in vxworks6 port */
    {
        return OS_FS_ERR_PATH_TOO_LONG;
    }




    /* make a disk if it is FS based */
    /*------------------------------- */
    if (OS_VolumeTable[i].VolumeType == FS_BASED)
    {

       /* now enter the info in the table */

       OS_VolumeTable[i].FreeFlag = FALSE;
       strcpy(OS_VolumeTable[i].VolumeName, volname);
       OS_VolumeTable[i].BlockSize = blocksize;
    
       /* note we don't know the mount point yet */
    
       /* for linux/osx we need to make the folder where this drive is located */
       strcpy(FolderName, OS_VolumeTable[i].PhysDevName);
       strcat(FolderName, devname);

    
       /* make the directory where the file system lives */
       sprintf(Command,"mkdir -p %s", FolderName);
       system(Command);
       
    }   /* VolumeType is something else that is not supported right now */
    else
    {
        return OS_FS_ERROR;
    }

   return OS_FS_SUCCESS; 
}/* end OS_initfs */

/*--------------------------------------------------------------------------------------
    Name: OS_mount
    
    Purpose: mounts a drive.

---------------------------------------------------------------------------------------*/

int32 OS_mount (const char *devname, char* mountpoint)
{
   int i;
   /* find the device in the table */
    for (i = 0; i < NUM_TABLE_ENTRIES; i++)
    {
        if (OS_VolumeTable[i].FreeFlag == FALSE && OS_VolumeTable[i].IsMounted == FALSE
            && strcmp(OS_VolumeTable[i].DeviceName, devname) == 0)
            break;
    }

    /* make sure we found the device */
    if (i >= NUM_TABLE_ENTRIES)
        return OS_FS_ERR_DRIVE_NOT_CREATED;

    /* attach the mountpoint */
    strcpy(OS_VolumeTable[i].MountPoint, mountpoint);
    OS_VolumeTable[i].IsMounted = TRUE;


    return OS_FS_SUCCESS;
}/* end OS_mount */

/*--------------------------------------------------------------------------------------
    Name: OS_unmount
    
    Purpose: unmounts a drive. and therefore makes all file descriptors pointing into
             the drive obsolete.

    Returns: OS_FS_ERR_INVALID_POINTER if name is NULL
             OS_FS_ERR_PATH_TOO_LONG if the absolute path given is too long
             OS_FS_ERROR if the OS calls failed
             OS_FS_SUCCESS if success
---------------------------------------------------------------------------------------*/

int32 OS_unmount (const char *mountpoint)
{
    int i;
    
    if (mountpoint == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    if (strlen(mountpoint) >= OS_MAX_PATH_LEN)
        return OS_FS_ERR_PATH_TOO_LONG;

    for (i = 0; i < NUM_TABLE_ENTRIES; i++)
    {
        if (OS_VolumeTable[i].FreeFlag == FALSE && OS_VolumeTable[i].IsMounted == TRUE
             && strcmp(OS_VolumeTable[i].MountPoint, mountpoint) == 0)
            break;
    }

    /* make sure we found the device */
    if (i >= NUM_TABLE_ENTRIES)
        return OS_FS_ERROR;

    /* release the informationm from the table */
   OS_VolumeTable[i].IsMounted = FALSE;
   strcpy(OS_VolumeTable[i].MountPoint, "");
    
    return OS_FS_SUCCESS;
    
}/* end OS_umount */

/*--------------------------------------------------------------------------------------
    Name: OS_fsBlocksFree

    Purpose: Returns the number of free blocks in a volume
 
    Returns: OS_FS_INVALID_POINTER if name is NULL
             OS_FS_ERROR if the OS call failed
             The number of bytes free in a volume if success
---------------------------------------------------------------------------------------*/


int32 OS_fsBlocksFree (const char *name)
{

   int           status;
   int32         NameStatus;
   struct statfs stat_buf;
   char          tmpFileName[OS_MAX_PATH_LEN +1];
   
   if ( name == NULL )
   {
      return(OS_FS_ERR_INVALID_POINTER);
   }
   strncpy(tmpFileName,name,OS_MAX_PATH_LEN +1);
   NameStatus = OS_NameChange(tmpFileName);
   status = statfs(tmpFileName, &stat_buf);
   
   if ( status == 0 )
   {
      return(stat_buf.f_bfree);
   }
   return OS_FS_ERROR;

}/* end OS_fsBlocksFree */

/*--------------------------------------------------------------------------------------
    Name: OS_chkfs
    
    Purpose: Checks the drives for inconsisenties and either repairs it or not

    Returns: OS_FS_ERR_INVALID_POINTER if name is NULL
             OS_FS_SUCCESS if success
             OS_FS_ERROR if the OS calls fail

---------------------------------------------------------------------------------------*/
os_fshealth_t OS_chkfs (const char *name, boolean repair)
{ 

/* LOOK INTO USING e2fsck TO CHECK THE FILE SYSTEM !!! */
    return OS_FS_UNIMPLEMENTED;
}/* end OS_chkfs */
/*--------------------------------------------------------------------------------------
    Name: OS_FS_GetPhysDriveName
    
    Purpose: Gets the name of the physical volume underneith the drive,
             when given the mount point of the drive

    Returns: OS_FS_ERR_INVALID_POINTER if either  parameter is NULL
             OS__SUCCESS if success
             OS_FS_ERROR if the mountpoint could not be found
---------------------------------------------------------------------------------------*/
int32 OS_FS_GetPhysDriveName(char * PhysDriveName, char * MountPoint)
{
    char LocalDrvName [OS_MAX_PATH_LEN];
    int32 ReturnCode;
    int32 status;
    
    if (MountPoint == NULL || PhysDriveName == NULL)
    {
        return OS_FS_ERR_INVALID_POINTER;
    }
    
    strncpy(LocalDrvName,MountPoint,OS_MAX_PATH_LEN);

    status = OS_NameChange(LocalDrvName);

    if (status != OS_SUCCESS)
    {
        ReturnCode = OS_FS_ERROR;
    }
    else
    {
        ReturnCode = OS_SUCCESS;
        strcpy(PhysDriveName,LocalDrvName);
    }

    return ReturnCode;
}/* end OS_FS_GetPhysDriveName */

/*-------------------------------------------------------------------------------------
 * Name: OS_NameChange
 * 
 * Purpose: Because of the abstraction of the filesystem across OSes, we have to change
 *          the name of the {file, directory, drive} to be what the OS can actually 
 *          accept
---------------------------------------------------------------------------------------*/
int32 OS_NameChange( char* name)
{
    char LocalName [OS_MAX_PATH_LEN];
    char newname [OS_MAX_PATH_LEN];
    char devname [OS_MAX_PATH_LEN];
    char filename[OS_MAX_PATH_LEN];
    int NumChars;
    int i=0;
   
    /* copy the name locally for good measure */
    strncpy(LocalName,name, OS_MAX_PATH_LEN);

    
    /* we want to find the number of chars in to LocalName the second "/" is.
     * Since we know the first one is in spot 0, we star looking at 1, and go until
     * we find it.*/
    
    NumChars=1;
    
    while (strncmp (&LocalName[NumChars],"/",1) != 0 &&
           (NumChars <= strlen(LocalName)))
    {
        NumChars++;
    }
    
    /* don't let it overflow to cause a segfault when trying to get the highest level
     * directory */
    
    if (NumChars >= strlen(LocalName))
            NumChars = strlen(LocalName);
  
    /* copy over only the part that is the device name */
    strncpy(devname,LocalName,NumChars);
    
    strncpy(filename,(LocalName + NumChars*sizeof(char)), strlen(LocalName) - NumChars+1);
    
  /*  printf("LocalName: %s\n",LocalName);
    printf("strlen %d\n",strlen(LocalName));
    printf("NumChars: %d\n",NumChars);
    printf("name in: %s\n",LocalName);
    printf("devname: %s\n",devname);
    printf("filename: %s\n",filename);
   
    */
    /* look for the dev name we found in the VolumeTable */
    for (i = 0; i < NUM_TABLE_ENTRIES; i++)
    {
        if (OS_VolumeTable[i].FreeFlag == FALSE && 
            strncmp(OS_VolumeTable[i].MountPoint, devname,NumChars) == 0)
        {
            break;
        }
    }

    /* Make sure we found a valid drive */
    if (i >= NUM_TABLE_ENTRIES)
        return OS_FS_ERR_DRIVE_NOT_CREATED;
    
    /* copy over the physical first part of the drive */
    strcpy(newname,OS_VolumeTable[i].PhysDevName);
    /* concat that with the file name */
    strcat(newname,OS_VolumeTable[i].DeviceName);

    strcat(newname, filename);
    /* push it back to the caller */
    strncpy(name,newname,OS_MAX_PATH_LEN);
    
    return OS_FS_SUCCESS;
    
} /* end OS_NameChange*/

/*---------------------------------------------------------------------------------------
    Name: OS_FS_GetErrorName()

    Purpose: a handy debugging tool that will copy the name of the error code to a buffer

    Returns: OS_FS_ERROR if given error number is unknown
             OS_FS_SUCCESS if given error is found and copied to the buffer
--------------------------------------------------------------------------------------- */
int32 OS_FS_GetErrorName(int32 error_num, os_fs_err_name_t * err_name)
{
    os_fs_err_name_t local_name;
    int32 return_code;

    return_code = OS_FS_SUCCESS;
    
    switch (error_num)
    {
        case OS_FS_SUCCESS: 
            strcpy(local_name,"OS_FS_SUCCESS"); break;
        case OS_FS_ERROR: 
            strcpy(local_name,"OS_FS_ERROR"); break;
        case OS_FS_ERR_INVALID_POINTER: 
            strcpy(local_name,"OS_FS_ERR_INVALID_POINTER"); break;
        case OS_FS_ERR_PATH_TOO_LONG: 
            strcpy(local_name,"OS_FS_ERR_PATH_TOO_LONG"); break;
        case OS_FS_ERR_NAME_TOO_LONG: 
            strcpy(local_name,"OS_FS_ERR_NAME_TOO_LONG"); break;
        case OS_FS_UNIMPLEMENTED: 
            strcpy(local_name,"OS_FS_UNIMPLEMENTED"); break;
        case OS_FS_ERR_PATH_INVALID:
            strcpy(local_name,"OS_FS_ERR_PATH_INVALID"); break;
        case OS_FS_ERR_DRIVE_NOT_CREATED: 
            strcpy(local_name,"OS_FS_ERR_DRIVE_NOT_CREATED"); break;
    
        default: strcpy(local_name,"ERROR_UNKNOWN");
                 return_code = OS_FS_ERROR;
    }

    strcpy((char*) err_name, local_name);


     return return_code;
}

