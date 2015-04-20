/*
** File   : osfilesys.c

** Author : Nicholas Yanchik / NASA GSFC Code 582.0

** Purpose: This file has the api's for all of the making
            and mounting type of calls for file systems
** $Date: 2007/10/16 16:15:01EDT $
** $Revision: 1.1 $
** $Log: osfilesys.c  $
** Revision 1.1 2007/10/16 16:15:01EDT apcudmore 
** Initial revision
** Member added to project d:/mksdata/MKS-OSAL-REPOSITORY/src/os/vxworks6/project.pj
** Revision 1.14 2007/05/25 09:17:54EDT njyanchik 
** I added the rmfs call to the OSAL and updated the unit test stubs to match
** Revision 1.13 2007/04/24 11:36:37EDT njyanchik 
** I Implemented the followiing fixes:
** Items 1,2,3 are for vxworks 5.5 , so we don't have to change that at all
** Item 4: fixed by adding a check for the length of the volume name (volname) on entry to the function
** Items 5,6, fixed by making the final strcpy a strncpy in OS_NameChange to make sure the string returned is less than or equal to the maximum number of bytes.
** Item 7: fixed by making the first strcpy in OS_NameChange a strncpy to prevent the input from being too long. This way the string length of LocalName won't be too long to use in line 704.
** Item 9: Fixed by making the error number parameter an int32 instead of a uint32
** Revision 1.12 2007/02/27 15:21:59EST njyanchik 
** This CP has the initial import of the new file descripor table mechanism

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
#include "xbdBlkDev.h"
#include "xbdRamDisk.h"
#include "errnoLib.h"
#include "taskLib.h"
#include "dirent.h"
#include "stat.h"

#ifdef USE_VXWORKS_ATA_DRIVER
  #include "drv/hdisk/ataDrv.h"
#endif


#include "common_types.h"
#include "osapi.h"

int32 OS_check_name_length(const char *path);
int32 OS_NameChange(char* name);
int32 OS_GetPhysDeviceName(char *PhysDevName, char *LocalVolname);
/****************************************************************************************
                                   GLOBAL DATA
****************************************************************************************/

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

    Purpose: Makes a file systm on the target 
    
    Returns: OS_FS_ERR_INVALID_POINTER if devname is NULL
             OS_FS_DRIVE_NOT_CREATED if the OS calls to create the the drive failed
             OS_FS_SUCCESS on creating the disk

    Note: if address == 0, then a malloc will be called to create the disk
---------------------------------------------------------------------------------------*/

int32 OS_mkfs (char *address, char *devname, char *volname, uint32 blocksize, 
               uint32 numblocks)
{
    int         i;
    int         status;
    char        local_volname[OS_MAX_PATH_LEN];
    uint32      ReturnCode;    
    BLK_DEV     *ramDev;
    device_t    xbd;

#ifdef USE_VXWORKS_ATA_DRIVER
    BLK_DEV     *ataBlkDev;  
#endif
    
    if ( devname == NULL || volname == NULL )
        return OS_FS_ERR_INVALID_POINTER;

    /* 
    ** Find an open entry in the Volume Table 
    */
    for (i = 0; i < NUM_TABLE_ENTRIES; i++)
    {
        if (OS_VolumeTable[i].FreeFlag == TRUE && OS_VolumeTable[i].IsMounted == FALSE
            && strcmp(OS_VolumeTable[i].DeviceName, devname) == 0)
            break;
    }

    if (i >= NUM_TABLE_ENTRIES)
        return OS_FS_ERR_DEVICE_NOT_FREE;
    
    /* 
    ** Now enter the info in the table 
    */
    OS_VolumeTable[i].FreeFlag = FALSE;
    strcpy(OS_VolumeTable[i].VolumeName, volname);
    OS_VolumeTable[i].BlockSize = blocksize;
    
    /* 
    ** Note we don't know the mount point/physical device name yet 
    */
    strcpy(local_volname,volname);

    if (OS_VolumeTable[i].VolumeType == RAM_DISK)
    {
        printf("OSAL: Making a RAM disk at: 0x%08X\n",(unsigned long)address );
        /*
        ** Create the ram disk device 
        ** The 32 is the number of blocks per track. 
        **  Other values dont seem to work here
        */
        ramDev = ramDevCreate (address, blocksize , 32 , numblocks,  0);
        if (ramDev == NULL)
        {
            ReturnCode = OS_FS_ERR_DRIVE_NOT_CREATED;
        }

        /*
        ** Connect the ram drive to the xbd block device 
        */
        xbd = xbdBlkDevCreate(ramDev,local_volname);
        if (xbd == NULLDEV)
        {
            ReturnCode = OS_FS_ERR_DRIVE_NOT_CREATED;
        }
        else
        {
           /*
           ** Delay to allow the XBD operation to complete
           */
           (void) taskDelay(100);
          
           /*
           ** Determine the vxWorks device name and store it 
           ** in the Physical Device field of the volume table.
           ** It will be used for determining the path in OS_NameChange
           */
           strcat(local_volname,":0");
           strncpy(OS_VolumeTable[i].PhysDevName, local_volname, 32 );
           
           /*
           ** Call the dos format routine
           */
           status = dosFsVolFormat(OS_VolumeTable[i].PhysDevName, DOS_OPT_BLANK, NULL);
           if ( status == -1 )
           {
              printf("OSAL: dosFsVolFormat failed. Errno = %d\n",errnoGet());
              ReturnCode = OS_FS_ERR_DRIVE_NOT_CREATED;
           }
           else
           {
              ReturnCode = OS_FS_SUCCESS;
           }
        }
    }
    else if (OS_VolumeTable[i].VolumeType == FS_BASED)
    {
       /*
       ** FS_BASED will map the cFE to an already mounted filesystem
       */
       
       /* 
       ** now enter the info in the table 
       */
       OS_VolumeTable[i].FreeFlag = FALSE;
       strcpy(OS_VolumeTable[i].VolumeName, volname);
       OS_VolumeTable[i].BlockSize = blocksize;

       ReturnCode = OS_FS_SUCCESS;

    }
#ifdef USE_VXWORKS_ATA_DRIVER    
    /*
    ** Format an ATA disk
    ** This code requires an ATA driver in the BSP, so it must be 
    ** left out of the compilation BSPs without. 
    */
    /* --------------------------------------------- */
    else if (OS_VolumeTable[i].VolumeType == ATA_DISK)
    {
        printf("OSAL: Formatting a FLASH DISK\n");
        /*
        ** Create the Flash disk device
        */
        if( (ataBlkDev = ataDevCreate( 0, 0, 0, 0)) == NULL)
        {
            printf("OSAL: Error Creating flash disk device.\n");
            ReturnCode = OS_FS_ERR_DRIVE_NOT_CREATED;
        }
        else
        {
           printf("OSAL: FLASH device initialized.\n");

           /*
           **  Attach to the XBD layer 
           */
           xbd = xbdBlkDevCreate(ataBlkDev,local_volname);
           if (xbd == NULLDEV)
           {
               printf("OSAL: Error Creating XBD device on FLASH disk.\n");
               ReturnCode = OS_FS_ERR_DRIVE_NOT_CREATED;
           }
           else
           {
              /*
              ** Delay to allow the XBD operation to complete
              */
              (void) taskDelay(100);

              /*
              ** Determine the vxWorks device name and store it 
              ** in the Physical Device field of the volume table.
              ** It will be used for determining the path in OS_NameChange
              */
              status = OS_GetPhysDeviceName(OS_VolumeTable[i].PhysDevName, local_volname);
              if ( status == OS_FS_ERROR )
              {
                  ReturnCode = OS_FS_ERR_DRIVE_NOT_CREATED;
              }
              else
              {
                 /* 
                 ** Format the Device with the DOS file system 
                 */
                 if ( dosFsVolFormat(OS_VolumeTable[i].PhysDevName,DOS_OPT_BLANK,NULL) == ERROR )
                 {
                    printf("OSAL: DOS format error on flash disk.\n");
                    ReturnCode = OS_FS_ERR_DRIVE_NOT_CREATED;
                 }
                 else
                 { 
                    ReturnCode = OS_FS_SUCCESS;
                 }
              }
           }
        }    
    }/* if ATA_DISK */
#endif
    else
    {
        /* 
        ** VolumeType is something else that is not supported right now 
        */
        ReturnCode = OS_FS_ERROR;
    }
    
    return ReturnCode;

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

    Purpose: Re-initialize an existing file system. This is used primarily for 
             vxWorks where we need to re-attach to an existing file system on 
             either the RAM, EEPROM, or FLASH drives.
    
    Returns: OS_FS_ERR_INVALID_POINTER if devname is NULL
             OS_FS_DRIVE_NOT_CREATED if the OS calls to create the the drive failed
             OS_FS_SUCCESS on creating the disk

    Note: if address == 0, then a malloc will be called to create the disk
---------------------------------------------------------------------------------------*/
int32 OS_initfs (char *address, char *devname, char *volname, uint32 blocksize, 
               uint32 numblocks)
{
    int         i;
    char        local_volname[OS_MAX_PATH_LEN];
    uint32      ReturnCode;
    BLK_DEV     *ramDev;
    device_t    xbd;

#ifdef USE_VXWORKS_ATA_DRIVER
    BLK_DEV    *ataBlkDev;  
    int32       status;
#endif
    
    if ( devname == NULL || volname == NULL )
        return OS_FS_ERR_INVALID_POINTER;

    if(strlen(devname) > 32 || strlen(volname) > 30) /* 32 - 2 for ':0' in vxworks6 port */
    {
        return OS_FS_ERR_PATH_TOO_LONG;
    }


    /* 
    ** Find an open entry in the Volume Table 
    */
    for (i = 0; i < NUM_TABLE_ENTRIES; i++)
    {
        if (OS_VolumeTable[i].FreeFlag == TRUE && OS_VolumeTable[i].IsMounted == FALSE
            && strcmp(OS_VolumeTable[i].DeviceName, devname) == 0)
            break;
    }

    if (i >= NUM_TABLE_ENTRIES)
        return OS_FS_ERR_DEVICE_NOT_FREE;
    
    /* 
    ** Now enter the info in the table 
    */
    OS_VolumeTable[i].FreeFlag = FALSE;
    strcpy(OS_VolumeTable[i].VolumeName, volname);
    OS_VolumeTable[i].BlockSize = blocksize;
    
    /* 
    ** Note we don't know the mount point yet 
    */
    strcpy(local_volname,volname);

    if (OS_VolumeTable[i].VolumeType == RAM_DISK)
    {
        printf("OSAL: Re-mounting a RAM disk at: 0x%08X\n",(unsigned long)address );
        /*
        ** Create the ram disk device 
        ** The 32 is the number of blocks per track. 
        **  Other values dont seem to work here
        */
        ramDev = ramDevCreate (address, blocksize , 32 , numblocks,  0);
        if (ramDev == NULL)
        {
            ReturnCode = OS_FS_ERR_DRIVE_NOT_CREATED;
        }

        /*
        ** Connect the ram drive to the xbd block device 
        */
        xbd = xbdBlkDevCreate(ramDev,local_volname);
        if (xbd == NULLDEV)
        {
            ReturnCode = OS_FS_ERR_DRIVE_NOT_CREATED;
        }
        else
        {
           /*
           ** Delay to allow the XBD operation to complete
           */
           (void) taskDelay(100);

           /*
           ** Determine the vxWorks device name and store it 
           ** in the Physical Device field of the volume table.
           ** It will be used for determining the path in OS_NameChange
           */
           strcat(local_volname,":0");
           strncpy(OS_VolumeTable[i].PhysDevName, local_volname, 32 );

           /*
           ** Nothing else has to be done here, because the 
           ** XBD call should automatically detect and init the existing DOS file system
           */
           ReturnCode = OS_FS_SUCCESS;
        
        }
    }
    else if (OS_VolumeTable[i].VolumeType == FS_BASED)
    {
       /*
       ** FS_BASED will map the cFE to an already mounted filesystem
       */
       
       /* 
       ** now enter the info in the table 
       */
       OS_VolumeTable[i].FreeFlag = FALSE;
       strcpy(OS_VolumeTable[i].VolumeName, volname);
       OS_VolumeTable[i].BlockSize = blocksize;

       ReturnCode = OS_FS_SUCCESS;

    }
#ifdef USE_VXWORKS_ATA_DRIVER    
    /*
    ** Initialize an ATA disk
    ** This code requires an ATA driver in the BSP, so it must be 
    ** left out of the compilation BSPs without. 
    */
    else if (OS_VolumeTable[i].VolumeType == ATA_DISK)
    {
        printf("OSAL: Re-mounting an ATA/FLASH DISK\n");
        /*
        ** Create the Flash disk device
        */
        if( (ataBlkDev = ataDevCreate( 0, 0, 0, 0)) == NULL)
        {
            printf("OSAL: Error Creating flash disk device.\n");
            ReturnCode = OS_FS_ERR_DRIVE_NOT_CREATED;
        }
        else
        {
           printf("OSAL: FLASH device initialized.\n");

           /*
           **  Attach to the XBD layer 
           */
           xbd = xbdBlkDevCreate(ataBlkDev,local_volname);
           if (xbd == NULLDEV)
           {
               printf("OSAL: Error Creating XBD device on ATA/FLASH disk.\n");
               ReturnCode = OS_FS_ERR_DRIVE_NOT_CREATED;
           }
           else
           {
              /*
              ** Delay to allow the XBD operation to complete
              */
              (void) taskDelay(100);

              /*
              ** Determine the vxWorks device name and store it 
              ** in the Physical Device field of the volume table.
              ** It will be used for determining the path in OS_NameChange
              */
              status = OS_GetPhysDeviceName(OS_VolumeTable[i].PhysDevName, local_volname);
              if ( status == OS_FS_ERROR )
              {
                  ReturnCode = OS_FS_ERR_DRIVE_NOT_CREATED;
              }
              else
              {
                 /*
                 ** Nothing else needs to be done here, the XBD layer should 
                 ** re-init the DOS volume.
                 */
                 ReturnCode = OS_FS_SUCCESS;
              }
           }
        }    
    }/* if ATA_DISK */
#endif
    else
    {
        /* 
        ** VolumeType is something else that is not supported right now 
        */
        ReturnCode = OS_FS_ERROR;
    }
    return ReturnCode;

} /* end OS_initfs */

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
int32 OS_unmount (const char* mountpoint)
{

    int fd;
    STATUS ret_status;
    char local_path [OS_MAX_PATH_LEN];
    int i;

    if (mountpoint == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    if (strlen(mountpoint) >= OS_MAX_PATH_LEN)
        return OS_FS_ERR_PATH_TOO_LONG;
   
    /* make a local copy of the path */
    strcpy(local_path, mountpoint);
    OS_NameChange(local_path); 
    
    /* find the device in the table */
    for (i = 0; i < NUM_TABLE_ENTRIES; i++)
    {
        if (OS_VolumeTable[i].FreeFlag == FALSE && OS_VolumeTable[i].IsMounted == TRUE
             && strcmp(OS_VolumeTable[i].MountPoint, mountpoint) == 0)
            break;
    }

    /* make sure we found the device */
    if (i >= NUM_TABLE_ENTRIES)
        return OS_FS_ERROR;

    fd = open(local_path,O_RDONLY,0);
    
    if (fd == ERROR)
        return OS_FS_ERROR;

    ret_status = ioctl( fd, FIOUNMOUNT,0);

    /* no need to close the fd, because ioctl is doing that for us */

    /* release the informationm from the table */
   OS_VolumeTable[i].IsMounted = FALSE;
   strcpy(OS_VolumeTable[i].MountPoint, "");
    
    if (ret_status == OK)
        return OS_FS_SUCCESS;
    else
        return OS_FS_ERROR;
    
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
    int fd;
    int i;
    STATUS status;
    uint32 free_bytes;
    char local_path [OS_MAX_PATH_LEN];


    if (name == NULL)
        return OS_FS_ERR_INVALID_POINTER;


    for (i = 0; i < NUM_TABLE_ENTRIES; i++)
    {
        if (OS_VolumeTable[i].FreeFlag == FALSE && OS_VolumeTable[i].IsMounted == TRUE
             && strcmp(OS_VolumeTable[i].MountPoint, name) == 0)
            break;
    }

    /* make sure we found the device */
    if (i >= NUM_TABLE_ENTRIES)
        return OS_FS_ERROR;

    
    strcpy(local_path,name);
    OS_NameChange(local_path);

    fd = open (local_path, O_RDONLY, 0);
    if (fd == ERROR)
        return OS_FS_ERROR;

    status = ioctl(fd,FIONFREE, (unsigned long) &free_bytes);
    if (status == ERROR)
        return OS_FS_ERROR;
    
    return (free_bytes / OS_VolumeTable[i].BlockSize);
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
    STATUS chk_status;
    int fd;
    char local_path [OS_MAX_PATH_LEN];

    if (name == NULL)
        return OS_FS_ERR_INVALID_POINTER;

    strcpy(local_path,name);
    OS_NameChange(local_path);

    fd = open (local_path, O_RDONLY, 0);
    if (fd == ERROR)
        return OS_FS_ERROR;
    
    /* Fix the disk if there are errors */
    if (repair == 1)
    {
        chk_status = ioctl(fd, FIOCHKDSK, DOS_CHK_REPAIR | DOS_CHK_VERB_SILENT);
        close(fd);
        if (chk_status == OK)
            return OS_FS_SUCCESS;
        else
            return OS_FS_ERROR;
    }
    
    /* only check the disk, don't fix anything */
    else
    {
        chk_status = ioctl(fd, FIOCHKDSK, DOS_CHK_ONLY | DOS_CHK_VERB_SILENT);
        close(fd);
        if (chk_status == OK)
           return OS_FS_SUCCESS;
        else
            return OS_FS_ERROR;
    }

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
     * Since we know the first one is in spot 0, we start looking at 1, and go until
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
   
    /* copy over the physical part of the drive */
    strcpy(newname,OS_VolumeTable[i].PhysDevName);
    strcat(newname, filename);
    
    /* push it back to the caller */
    strncpy(name,newname,OS_MAX_PATH_LEN);
    
    /* printf("new_path  : $%s\n",newname); */

    return OS_FS_SUCCESS;
    
} /* end OS_NameChange*/

/*-------------------------------------------------------------------------------------
 * Name: OS_GetPhysDeviceName
 * 
 * Purpose: This function will get the vxWorks physical volume name. In vxWorks 6.2
 *          the disk XBD code will add ":X" to the volume name you give to a disk, where
 *          X is the partition number. While RAM disks are always 0 ( "RAM:0" ), 
 *          a physical disk such as a compact flash disk can be ":0", or ":1" etc, 
 *          depending on how the disk was partitioned.
 *          This code will figure out which partition number to use and return that name.
 *
---------------------------------------------------------------------------------------*/
int32 OS_GetPhysDeviceName(char *PhysDevName, char *LocalVolname)
{   
    char tempName[32];
    int  deviceFound = FALSE;
    int  tempFd;
    int  i;      

    /*
    ** Copy the local volume over to a temporary volume name
    */
    strncpy(tempName, LocalVolname, 32 );
    
    /*
    ** Loop through and determine the correct physical volume name
    */
    for ( i = 0; i < 4; i++ )
    {
       snprintf(tempName, 32, "%s:%d", LocalVolname, i );
       tempFd = open ( tempName, O_RDONLY, 0644 );
       if ( tempFd != ERROR )
       {
          close(tempFd);
          deviceFound = TRUE;
          break;
       }
    }

    if ( deviceFound == TRUE )
    { 
       strncpy(PhysDevName, tempName, 32);
       return(OS_FS_SUCCESS);
    }
    else
    {
       strncpy(PhysDevName, LocalVolname, 32);       
       return (OS_FS_ERROR);
    }
    
} /* end OS_GetPhysDeviceName*/
