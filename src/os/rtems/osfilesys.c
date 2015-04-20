/*
** File   : osfilesys.c

** Author : Nicholas Yanchik / NASA GSFC Code 582.0

** Purpose: This file has the api's for all of the making
            and mounting type of calls for file systems
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
#include "osconfig.h"


/****************************************************************************************
                                     DEFINES
****************************************************************************************/

# define ERROR (-1)


/* 
** This is the volume table reference. It is defined in the BSP/startup code for the board
*/
extern OS_VolumeInfo_t OS_VolumeTable [NUM_TABLE_ENTRIES]; 
/****************************************************************************************
                                   GLOBAL DATA
****************************************************************************************/
int32 OS_check_name_length(const char *path);
int32 OS_NameChange(char* name);



/* this is the volume table. This may be moved to the bsp in the future */
OS_VolumeInfo_t OS_VolumeTable [NUM_TABLE_ENTRIES];

/****************************************************************************************
                                    Filesys API
****************************************************************************************/
/*
** System Level API 
*/

/*---------------------------------------------------------------------------------------
    Name: OS_mkfs

    Purpose: Makes a RAM disk on the target with a dos file system
    
    Returns: OS_FS_ERR_INVALID_POINTER if devname is NULL
             OS_FS_DRIVE_NOT_CREATED if the OS calls to create the the drive failed
             OS_FS_SUCCESS on creating the disk

    Note: if address == 0, then a malloc will be called to create the disk
---------------------------------------------------------------------------------------*/

int32 OS_mkfs (char *address, char *devname,char * volname, uint32 blocksize, 
               uint32 numblocks)
{
    int i;
    char FolderName[OS_MAX_PATH_LEN];
    mode_t mode;
    
    /* find an open entry in the Volume Table */
    for (i = 0; i < NUM_TABLE_ENTRIES; i++)
    {
        if (OS_VolumeTable[i].FreeFlag == TRUE && OS_VolumeTable[i].IsMounted == FALSE
            && strcmp(OS_VolumeTable[i].DeviceName, devname) == 0)
            break;
    }

    if (i >= NUM_TABLE_ENTRIES)
        return OS_FS_ERR_DEVICE_NOT_FREE;

    /* now enter the info in the table */

    OS_VolumeTable[i].FreeFlag = FALSE;
    strcpy(OS_VolumeTable[i].VolumeName, volname);
    OS_VolumeTable[i].BlockSize = blocksize;
    
    /* note we don't know the mount point yet */

    
    /* for linux we need to make the fold where this drive is located */
    strcpy(FolderName, OS_VolumeTable[i].PhysDevName);
    strcat(FolderName, devname);

    
    /* make the directory where the file system lives */
    mode = S_IFDIR |S_IRWXU | S_IRWXG | S_IRWXO;
     mkdir(FolderName, mode);
  
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

Is there such a thing as mounting in VxWorks? After the Drive is initialized, it is
mounted. After an Unmount, on the next subsequent I/O operation, the volume will
remount automatically, according to page 2 -99 in the VxWorks 5.3 Reference manual

    Returns: Because of above, OS_mount in VxWorks is basically unimplemenented
---------------------------------------------------------------------------------------*/

int32 OS_mount (const char *devname, char* mountpoint)
{
   int i;
   /* find the device in the table */
    for (i = 0; i < NUM_TABLE_ENTRIES; i++)
    {
        if (OS_VolumeTable[i].FreeFlag == FALSE && 
            strcmp(OS_VolumeTable[i].DeviceName, devname) == 0)
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
    char local_path [OS_MAX_PATH_LEN];
    int i;
    
    if (mountpoint == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    if (strlen(mountpoint) >= OS_MAX_PATH_LEN)
        return OS_FS_ERR_PATH_TOO_LONG;

    strcpy(local_path,mountpoint);
    OS_NameChange(local_path);
    
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
    /*
    int fd;
    STATUS status;
    uint32 free_bytes;

    if (name == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    fd = open (name, O_RDONLY, 0);
    if (fd == ERROR)
        return OS_FS_ERROR;

    status = ioctl(fd,FIONFREE, (unsigned long) &free_bytes);
    if (status == ERROR)
        return OS_FS_ERROR;
    
    return free_bytes;

    */ return OS_FS_ERROR;
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
/*
    STATUS chk_status;
    int fd;

    if (name == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    
    fd = open (name, O_RDONLY, 0);
    if (fd == ERROR)
        return OS_FS_ERROR;
    
    // Fix the disk if there are errors 
    if (repair == 1)
    {
        chk_status = ioctl(fd, FIOCHKDSK, DOS_CHK_REPAIR | DOS_CHK_VERB_SILENT);
        close(fd);
        if (chk_status == OK)
            return OS_FS_SUCCESS;
        else
            return OS_FS_ERROR;
    }
    
    // only check the disk, don't fix anything 
    else
    {
        
        chk_status = ioctl(fd, FIOCHKDSK, DOS_CHK_ONLY | DOS_CHK_VERB_SILENT);
        close(fd);
        if (chk_status == OK)
           return OS_FS_SUCCESS;
        else
            return OS_FS_ERROR;
    }
*/
    /* code should never get here */
    return OS_FS_ERROR;
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

    /*printf("Local_path: $%s\n",LocalName);*/

    
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
    /* concat that with the folder name */
    strcat(newname,OS_VolumeTable[i].DeviceName);

    strcat(newname, filename);
    /* push it back to the caller */
    strncpy(name,newname,OS_MAX_PATH_LEN);

    
    /*printf("new_path  : $%s\n",newname);*/

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

