#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common_types.h"
#include "osapi.h"


/* OS Constructs */
int TestCreatRemove(void);
int TestOpenClose(void);
int TestReadWriteLseek(void);
int TestMkRmDirFreeBytes(void);
int TestOpenReadCloseDir(void);
int TestRename(void);

os_fs_err_name_t errname;

/* *************************************** MAIN ************************************** */

void OS_Application_Startup(void)
{
    int status;
    printf("This is Example 2\n");

    /* make the file system */
    status = OS_mkfs(0,"/ramdev0","RAM",512,200);
    printf("status after mkfs = %d\n",status);

    status =OS_mount("/ramdev0","/drive0");
    printf("status after mount = %d\n",status);

/*
    status = OS_mkfs(0,"/eedev1","RAM",512,200);
    printf("status after mkfs = %d\n",status);
*/

    printf("*******************\n");
    TestCreatRemove();
    printf("*******************\n");    
    TestOpenClose();
    printf("*******************\n");
    TestReadWriteLseek();
    printf("*******************\n");
    TestMkRmDirFreeBytes();
    printf("*******************\n");
   TestOpenReadCloseDir();
    printf("********************\n");

   /*try unmounting the drive, and then remounting it with a different name */
  
    status = OS_unmount("/drive0");
    if (status < OS_FS_SUCCESS)
       printf("ERROR from OS_unmount\n");
    else 
        printf("Drive Unmounted Sucessfully\n");

    status =OS_mount("/ramdev0","/drive1");
    printf("status after mount = %d\n",status);
    
    TestRename();
    printf("********************\n");  
    
    printf("End of OS_Application_Startup\n");
    printf("Test Complete: On a Desktop System, hit Control-C to return to command shell\n");
    
    return;

} /* end OS_Application Startup */
/*---------------------------------------------------------------------------------------
 * Name Test CreatRemove()
 * This function tests the basic create / remove capabilities of the filesystem. It tries
 *  to create files of reasonable length, OS_MAX_FILE_NAME length, and OS_MAX_FILE_NAME+1
 *  length.
 *
 *  The function then tries to remove the files created above, as well as remove a file
 *  that was never on the drive to begin with.
---------------------------------------------------------------------------------------*/

int TestCreatRemove(void)
{
    char filename [OS_MAX_PATH_LEN];
    char maxfilename[OS_MAX_PATH_LEN];
    char longfilename [OS_MAX_PATH_LEN];
    int status;
 
    printf("In TestCreatRemove()\n");
    strcpy(filename,"/drive0/test11chars");

    /* make the two really long filenames */
    
    /* 1 '/' and 40 'm' */
    strcpy(maxfilename,"/drive0/mmmmmmmmmmmmmmmmmmmm");/*mmmmmmmm");*/ 

    /* 1 '/' and 41 'l' */
    strcpy(longfilename,"/drive0/lllllllllllllllllllllllllllllllllllllllll");
    
    /* create a file of reasonable length (but over 8 chars) */
    status = OS_creat(filename,OS_READ_WRITE);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_creat 1\n");
    }
    else
        printf("OK- OS_creat 1 \n");

    /* create a file of OS_max_file_name size */
    
    status = OS_creat(maxfilename,OS_READ_WRITE);
    
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_creat 3 %s\n", errname);
    }
    else
        printf("OK- OS_creat 3\n");
    /* try removing the file from the drive */
    
    status = OS_remove(filename);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_remove 1 %s \n",errname);
    }
    else
        printf("OK- OS_remove 1\n");
    
    /* try removing the file from the drive */
    status = OS_remove(maxfilename);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_remove 2 %s \n",errname);
    }
    else
        printf("OK- OS_remove 2\n");

    /* try removing the file from the drive. Should Fail */
    status = OS_remove(longfilename);
    if (status >= OS_FS_SUCCESS)
    {
        printf("ERROR from OS_remove 3 %s \n",errname);
    }
    else
        printf("OK- OS_remove 3\n");

    /* try removing the file from the drive. Should Fail */
    status = OS_remove("/FileNotOnDrive");
    if (status != OS_FS_ERROR)
    {
        printf("ERROR from OS_remove 4 %s \n",errname);
    }
    else
        printf("OK- OS_remove 4\n");

    printf("Leaving TestCreatRemove()\n");
  
    return 0;
}

/*---------------------------------------------------------------------------------------
 *  Name: TestOpenClose
 * This functions tests the basic functionality of OS_open and OS_close.
---------------------------------------------------------------------------------------*/
int TestOpenClose(void)
{
    char filename [OS_MAX_PATH_LEN];
    int status;
    int fd;

    printf("In TestOpenClose()\n");
    
    strcpy(filename,"/drive0/Filename1");

    /* create a file of reasonable length (but over 8 chars) */
    
    status = OS_creat(filename,OS_READ_WRITE);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_creat 1 %s \n", errname);
    }
    else
        printf("OK- OS_creat 1 \n");
        
    fd = status;
   
     /*try to close the file*/
    status = OS_close(fd);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_close 1 %s \n",errname);
    }
    else
        printf("OK- OS_close 1\n");

    /*  reopen the file */
    status = OS_open(filename,OS_READ_WRITE,644);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_open 1 %s \n", errname);
    }
    else
        printf("OK- OS_open 1\n");

    fd = status;
   
     /*try to close the file again*/
    status = OS_close(fd);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_close 2 %s \n",errname);
    }
    else
        printf("OK- OS_close 2\n");
   
     /*try to close the file again. Should Fail*/
    status = OS_close(fd);
    if (status >= OS_FS_SUCCESS)
    {
        printf("ERROR from OS_close 3 %s \n",errname);
    }
    else
        printf("OK- OS_close 3\n");
    
     /*try to close a file not on the system. Should Fail */
    status = OS_close(43);
    if (status >= OS_FS_SUCCESS)
    {
        printf("ERROR from OS_close 4 %s \n",errname);
    }
    else
        printf("OK- OS_close 4\n");

    /*  open a file that was never in the system */
    status = OS_open("/drive0/FileNotHere",OS_READ_WRITE,644);
    if (status >= OS_FS_SUCCESS)
    {
        printf("ERROR from OS_open 2 %s \n", errname);
    }
    else
        printf("OK- OS_open 2\n");

    /* try removing the file from the drive  to end the function */

    status = OS_remove(filename);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_remove 1 %s \n",errname);
    }
    else
        printf("OK- OS_remove 1\n");

    printf("Leaving TestOpenClose()\n");
    
    return 0;
}
/*---------------------------------------------------------------------------------------
 * Name TestReadWriteLseek
---------------------------------------------------------------------------------------*/

int TestReadWriteLseek(void)
{
    char filename [OS_MAX_PATH_LEN];
    char buffer [30];
    char copyofbuffer[30];
    char seekbuffer[30];
    char newbuffer[30];
    int offset;
    int size;
    int status;
    int fd;

    printf("In TestReadWriteLseek()\n");

    strcpy(filename,"/drive0/Filename1");
    strcpy(buffer,"ValueToWriteInTheFile");
    strcpy(copyofbuffer,buffer); /* hold a copy of the buffer */
    offset = 12;

    /* start copying offset chars in buffer into seekbuffer */
    strcpy(seekbuffer, buffer + sizeof(char)* offset);
    
    /* create a file of reasonable length (but over 8 chars) */
    
    /* Open In R/W mode */
    status = OS_creat(filename,OS_READ_WRITE);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_creat 1 %s \n", errname);
    }
    else
        printf("OK- OS_creat 1 \n");
    
    fd = status;
    size = strlen(buffer);
    
    /* test write portion of R/W mode */
    status = OS_write(fd, (void*)buffer, size);
    if (status < OS_FS_SUCCESS)
    {
         printf("ERROR from OS_write 1 %s\n",errname);
    }
    else 
        printf("OK- OS_write 1 \n");
    
    strcpy(buffer,"");

    /* lseek back to the beginning of the file */
    status = OS_lseek(fd,0, 0);
    if(status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_lseek 1 %s \n",errname);
    }
    else
        printf("OK- OS_lseek 1\n");
    
    /*Read what we wrote to the file */
    status = OS_read(fd,(void*)buffer, size);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_read 1 %s \n",errname);
    }
    else
    {
        printf("OK- OS_read 1 ");
        if (strcmp(buffer,copyofbuffer) != 0)
        {
            printf("However, the read and written values differ\n");
            printf("Read: %s, Written: %s\n",buffer,copyofbuffer);
        }
        else
            printf("Read and Written values are equal\n");
    }

    /* close the file */
    status = OS_close(fd);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_close 1 %s\n",errname);
    }
    else
        printf("OK- OS_close 1\n");

    /*  open a file again, but only in READ mode */
    status = OS_open(filename,OS_READ_ONLY,644);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_open 1 %s \n", errname);
    }
    else
        printf("OK- OS_open 1\n");

    /* test write in READ ONLY mode */
    status = OS_write(fd, (void*)buffer, size);
    if (status >= OS_FS_SUCCESS)
    {
         printf("ERROR from OS_write 2 %s\n",errname);
    }
    else 
        printf("OK- OS_write 2 \n");

    /* try to read in READ ONLY MODE */
    
    status = OS_read(fd,(void*)buffer, size);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_read 2 %s \n",errname);
    }
    else
    {
        printf("OK- OS_read 2 ");
        if (strcmp(buffer,copyofbuffer) != 0)
        {
            printf("However, the read and written values differ.\n");
            printf("Read: %s, Written: %s\n",buffer,copyofbuffer);
        }
        else
            printf("Read and Written values are equal\n");
    }

    /* seek to a spot in the middle of the file to test lseek */
    status = OS_lseek(fd,offset,0);

    /* now try to read out only the last chars of the file */
    
    status = OS_read(fd,(void*)newbuffer, (size - offset));
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_read 3 %s \n",errname);
    }
    else
    {
        printf("OK- OS_read 3. ");
        if (strncmp(newbuffer,seekbuffer, size - offset) != 0)
        {
            printf("However, the read and written values differ.\n");
            printf("Read: %s, Written: %s\n",newbuffer,seekbuffer);
        }
        else
            printf("Read and written values are equal\n");
    }

    /* close the file */
    status = OS_close(fd);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_close 2 %s\n",errname);
    }
    else
        printf("OK- OS_close 2\n");

    /*  open a file again, but only in WRITE mode */
    status = OS_open(filename,OS_WRITE_ONLY,644);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_open 2 %s\n", errname);
    }
    else
        printf("OK- OS_open 2\n");

    /* test write in WRITE ONLY mode */
    status = OS_write(fd, (void*)buffer, size);
    if (status < OS_FS_SUCCESS)
    {
         printf("ERROR from OS_write 3 %s\n",errname);
    }    
    else 
        printf("OK- OS_write 3 \n");

    /* try to read in WRITE ONLY MODE */
    
    status = OS_read(fd,(void*)buffer, size);
    if (status >= OS_FS_SUCCESS)
        printf("ERROR from OS_read 4 %s \n",errname);
    else
        printf("OK- OS_read 4.\n");

    /* seek to a spot in the middle of the file to test lseek */
    status = OS_lseek(fd,offset,0);

    /* now try to read out only the last chars of the file */
    status = OS_read(fd,(void*)newbuffer, (size - offset));
    if (status >= OS_FS_SUCCESS)
    {
        printf("ERROR from OS_read 5 %s\n",errname);
    }
    else
        printf("OK- OS_read 5.\n");

    /* close the file */
    status = OS_close(fd);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_close 3 %s\n",errname);
    }
    else
        printf("OK- OS_close 3\n");
    /* try removing the file from the drive */
    status = OS_remove(filename);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_remove 1 %s\n",errname);
    }
    else
        printf("OK- OS_remove 1\n");
    printf("Leaving TestReadWriteLseek()\n");
    return 0;
}
/*---------------------------------------------------------------------------------------
 *  Name TestMkRmDir()
---------------------------------------------------------------------------------------*/
int TestMkRmDirFreeBytes(void)
{
    int status;
    char filename1[OS_MAX_PATH_LEN];
    char filename2[OS_MAX_PATH_LEN];
    char dir1 [OS_MAX_PATH_LEN];
    char dir2 [OS_MAX_PATH_LEN];
    char buffer1 [OS_MAX_PATH_LEN];
    char buffer2 [OS_MAX_PATH_LEN];
    char copybuffer1 [OS_MAX_PATH_LEN];
    char copybuffer2 [OS_MAX_PATH_LEN];
    int fd1;
    int fd2;

    printf("In TestMkRmDirFreeBytes()\n");
    /* make the directory names for testing, as well as the filenames and the buffers
     * to put in the files */
    strcpy(dir1,"/drive0/DIRECTORY_ONE");
    strcpy(dir2,"/drive0/directory_two");
    strcpy(filename1,"/drive0/DIRECTORY_ONE/MyFile1");
    strcpy(filename2,"/drive0/directory_two/MyFile2");
    strcpy(buffer1,"111111111111");
    strcpy(copybuffer1,buffer1);
    strcpy(buffer2,"222222222222");
    strcpy(copybuffer2,buffer2);
   
    printf("Checking Free Blocks: %d \n",(int)OS_fsBlocksFree("/drive0"));

    /* make the two directories */
    
    status = OS_mkdir(dir1,0);
    if (status >= OS_FS_SUCCESS)
        printf("OK- OS_mkdir 1\n");
    else
    {
        printf("ERROR OS_mkdir 1, %s\n",errname);
    }
           
    status = OS_mkdir(dir2,0);
    if (status >= OS_FS_SUCCESS)
        printf("OK- OS_mkdir 2\n");
    else
    {
        printf("ERROR OS_mkdir 2, %s\n",errname);
    }
    
    /* now create two files in the two directories (1 file per directory) */

    status = OS_creat(filename1,OS_READ_WRITE);
    if (status >= OS_FS_SUCCESS)
        printf("OK- OS_creat 1\n");
    else
    {
        printf("ERROR OS_creat 1, %s\n",errname);
    }
    
    fd1 = status;

    status = OS_creat(filename2,OS_READ_WRITE);
    if (status >= OS_FS_SUCCESS)
        printf("OK- OS_creat 2\n");
    else
    {
        printf("ERROR OS_creat 2, %s \n",errname);
    }
    
    fd2 = status;

    /* write the propper buffers into each of the files */
    
    status = OS_write(fd1, buffer1, strlen(buffer1));
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR OS_write 1, %s \n",errname);
    }
    else
        printf("OK- OS_write 1\n");
    
    status = OS_write(fd2, buffer2, strlen(buffer2));
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR OS_write 2, %s\n",errname);
    }
    else
        printf("OK- OS_write 2\n");

    /* lseek back to the beginning of the file */
    status = OS_lseek(fd1,0, 0);
    if(status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_lseek 1, %s \n",errname);
    }
    else
        printf("OK- OS_lseek 1\n");
    
    status = OS_lseek(fd2,0, 0);
    if(status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_lseek 2, %s \n",errname);
    }
    else
        printf("OK- OS_lseek 2\n");

    bzero(buffer1, strlen(buffer1));
    bzero(buffer2, strlen(buffer2));
    printf("Checking Free Blocks: %d \n",(int)OS_fsBlocksFree("/drive0"));

    /* read back out of the files what we wrote into them */
    status = OS_read(fd1,(void*)buffer1, strlen(copybuffer1));
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_read 1 %s\n",errname);
    }
    else
    {
        printf("OK- OS_read 1. ");
        if (strncmp(buffer1,copybuffer1,strlen(copybuffer1)) != 0)
        {
            printf("However, the read and written values differ.\n");
            printf("Read: %s, Written: %s\n",buffer1,copybuffer1);
        }
        else
            printf("Read and written values are equal\n");
    }
    
    status = OS_read(fd2,(void*)buffer2, strlen(copybuffer2));
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_read 2 %s\n",errname);
    }
    else
    {
        printf("OK- OS_read 2. ");
        if (strncmp(buffer2,copybuffer2,strlen(copybuffer2)) != 0)
        {
            printf("However, the read and written values differ.\n");
            printf("Read: %s, Written: %s\n",buffer2,copybuffer2);
        }
        else
            printf("Read and written values are equal\n");
    }
        /* close the files */

    status = OS_close(fd1);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_close 1 %s\n",errname);
    }
    else
        printf("OK- OS_close 1\n");

    status = OS_close(fd2);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_close 2 %s\n",errname);
    }
    else
        printf("OK- OS_close 2\n");

    /* try removing the file from the drive */
    status = OS_remove(filename1);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_remove 1 %s\n",errname);
    }
    else
        printf("OK- OS_remove 1\n");

    status = OS_remove(filename2);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_remove 2 %s\n",errname);
    }
    else
        printf("OK- OS_remove 2\n");

    status = OS_rmdir(dir1);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_rmdir 1 %s\n",errname);
    }
    else
        printf("OK- OS_rmdir 1\n");

    status = OS_rmdir(dir2);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_rmdir 2 %s\n",errname);
    }
    else
        printf("OK- OS_rmdir 2\n");

    printf("Checking Free Blocks: %d \n",(int)OS_fsBlocksFree("/"));

    printf("Leaving TestMkRmDirFreeBytes()\n");
    return 0;
}
/*---------------------------------------------------------------------------------------
 * Name TestOpenReadCloseDir();
---------------------------------------------------------------------------------------*/
int TestOpenReadCloseDir(void)
{
    int status;
    char filename1[OS_MAX_PATH_LEN];
    char filename2[OS_MAX_PATH_LEN];
    char dir0 [OS_MAX_PATH_LEN];
    char dir1 [OS_MAX_PATH_LEN];
    char dir2 [OS_MAX_PATH_LEN];
    char buffer1 [OS_MAX_PATH_LEN];
    char buffer2 [OS_MAX_PATH_LEN];
    int fd1;
    int fd2;
    os_dirp_t dirp0;
    os_dirp_t dirp1;
    os_dirp_t dirp2;
    os_dirent_t *dirent_ptr0;
    os_dirent_t *dirent_ptr1;
    os_dirent_t *dirent_ptr2;
    
    printf("In TestOpenReadCloseDir()\n");


    /* make the directory names for testing, as well as the filenames and the buffers
     * to put in the files */
    strcpy(dir0,"/drive0");
    strcpy(dir1,"/drive0/DIRECTORY_ONE");
    strcpy(dir2,"/drive0/directory_two");
    strcpy(filename1,"/drive0/DIRECTORY_ONE/MyFile1");
    strcpy(filename2,"/drive0/directory_two/MyFile2");
    strcpy(buffer1,"111111111111");
    strcpy(buffer2,"222222222222");

    /* make the two directories */
    
    status = OS_mkdir(dir1,0);
    if (status >= OS_FS_SUCCESS)
        printf("OK- OS_mkdir 1\n");
    else
    {
        printf("ERROR OS_mkdir 1, %s\n",errname);
    }
           
    status = OS_mkdir(dir2,0);
    if (status >= OS_FS_SUCCESS)
        printf("OK- OS_mkdir 2\n");
    else
    {
        printf("ERROR OS_mkdir 2, %s\n",errname);
    }
    
    /* now create two files in the two directories (1 file per directory) */

    status = OS_creat(filename1,OS_READ_WRITE);
    if (status >= OS_FS_SUCCESS)
        printf("OK- OS_creat 1\n");
    else
    {
        printf("ERROR OS_creat 1, %s \n",errname);
    }
    
    fd1 = status;

    status = OS_creat(filename2,OS_READ_WRITE);
    if (status >= OS_FS_SUCCESS)
        printf("OK- OS_creat 2\n");
    else
    {
        printf("ERROR OS_creat 2, %s \n",errname);
    }
    
    fd2 = status;

    /* write the propper buffers into each of the files */
    
    status = OS_write(fd1, buffer1, strlen(buffer1));
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR OS_write 1, %s\n",errname);
    }
    else
        printf("OK- OS_write 1\n");
    
    status = OS_write(fd2, buffer2, strlen(buffer2));
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR OS_write 2, %s\n",errname);
    }
    else
        printf("OK- OS_write 2\n");

    /* close the files */

    status = OS_close(fd1);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_close 1 %s \n",errname);
    }
    else
        printf("OK- OS_close 1\n");

    status = OS_close(fd2);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_close 2 %s\n",errname);
    }
    else
        printf("OK- OS_close 2\n");

    /* Now both of the directories are there and have files in them. Also both files
     have stuff written in them */
    
    
    /* try to open the base directory "/" */

    dirp0 = OS_opendir(dir0);
    if (dirp0 == NULL)
        printf("ERROR from OS_opendir 1\n");
    else
        printf("OK- OS_opendir 1\n");
    
    /* try to read the two folders that are in the "/" */
    /* Have to make it a loop to see if we can find the directories in question */

    dirent_ptr0 = OS_readdir (dirp0);

    while (dirent_ptr0 != NULL)
    {
        if ( strncmp(dirent_ptr0 -> d_name,"DIRECTORY_ONE",strlen("DIRECTORY_ONE")) == 0)
        {
            printf("OK- OS_readdir 1 Read and Written Values are equal\n"); 
            break;
        }
        else
            dirent_ptr0 = OS_readdir(dirp0);
    }
    
    if (dirent_ptr0 == NULL)
        printf("ERROR from OS_readdir 1 \n");


    status = OS_closedir(dirp0);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_closedir 1 %s\n",errname);
    }
    else
        printf("OK- OS_closedir 1\n");

    dirp0 = OS_opendir(dir0);
    if (dirp0 == NULL)
        printf("ERROR from OS_opendir 2\n");
    else
        printf("OK- OS_opendir 2\n");

    dirent_ptr0 = OS_readdir (dirp0);
    while (dirent_ptr0 != NULL)
    {
        if ( strncmp(dirent_ptr0 -> d_name,"directory_two",strlen("directory_two")) == 0)
        {
            printf("OK- OS_readdir 2 Read and Written Values are equal\n"); 
            break;
        }
        else
            dirent_ptr0 = OS_readdir(dirp0);
    }
    
    if (dirent_ptr0 == NULL)
        printf("ERROR from OS_readdir 2 \n");
      status = OS_closedir(dirp0);

      
      if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_closedir 2 %s\n",errname);
    }
    else
        printf("OK- OS_closedir 2\n");

    
    /* try and read a nonexisitant 3rd directory */ 
        
    dirp0 = OS_opendir(dir0);
    if (dirp0 == NULL)
        printf("ERROR from OS_opendir 3\n");
    else
        printf("OK- OS_opendir 3\n");

    dirent_ptr0 = OS_readdir (dirp0);
    while (dirent_ptr0 != NULL)
    {
        if ( strncmp(dirent_ptr0 -> d_name,"NOTHERE",strlen("NOTHERE")) == 0)
        {
            printf("ERROR from OS_readdir 3 \n"); 
            break;
        }
        else
            dirent_ptr0 = OS_readdir(dirp0);
    }
    
    if (dirent_ptr0 == NULL)
        printf("OK- OS_readdir 3 \n");

    
    status = OS_closedir(dirp0);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_closedir 3 %s\n",errname);
    }
    else
        printf("OK- OS_closedir 3\n");

    /* Now test the open/ read close for one of the sub directories */
    
    dirp1 = OS_opendir(dir1);
    if (dirp1 == NULL)
        printf("ERROR from OS_opendir 4\n");
    else
        printf("OK- OS_opendir 4\n");
    
   
    /* try to read the next file within the first directory "MyFile1" */

    dirent_ptr1 = OS_readdir (dirp1);
    while (dirent_ptr1 != NULL)
    {
        if ( strncmp(dirent_ptr1 -> d_name,"MyFile1",strlen("MyFile1")) == 0)
        {
            printf("OK- OS_readdir 4 Read and Written Values are equal\n"); 
            break;
        }
        else
            dirent_ptr1 = OS_readdir(dirp1);
    }
    
    if (dirent_ptr1 == NULL)
        printf("ERROR from OS_readdir 4 \n");
    
    /* Close the file */
    
    status = OS_closedir(dirp1);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_closedir 4 %s\n",errname);
    }
    else
        printf("OK- OS_closedir 4\n");
    
    /* try to read the file within the first directory "." */

    dirp2 = OS_opendir(dir2);
    if (dirp2 == NULL)
        printf("ERROR from OS_opendir 5\n");
    else
        printf("OK- OS_opendir 5\n");
 
    /* try to read the next file within the first directory "MyFile2" */
    
    dirent_ptr2 = OS_readdir (dirp2);
    while (dirent_ptr2 != NULL)
    {
        if ( strncmp(dirent_ptr2 -> d_name,"MyFile2",strlen("MyFile2")) == 0)
        {
            printf("OK- OS_readdir 5 Read and Written Values are equal\n"); 
            break;
        }
        else
            dirent_ptr2 = OS_readdir(dirp2);
    }

    if (dirent_ptr2 == NULL)
        printf("ERROR from OS_readdir 5 \n");

    /*Close the file */
    
    status = OS_closedir(dirp2);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_closedir 5 %s\n",errname);
    }
    else
        printf("OK- OS_closedir 5\n");

/* remove the files */
    status = OS_remove(filename1);
    if(status < OS_FS_SUCCESS)
    {   
        printf("ERROR from OS_remove 1: %s \n",errname);
    }
    else
        printf("OK- OS_remove 1\n");

    status = OS_remove(filename2);
    if(status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_remove 2: %s\n",errname);
    }
    else
        printf("OK- OS_remove 2\n");

/* remove the directories */
    status = OS_rmdir(dir1);
    if(status < OS_FS_SUCCESS)
    {   
        printf("ERROR from OS_rmdir 1: %s \n",errname);
    }
    else
        printf("OK- OS_rmdir 1\n");

    status = OS_rmdir(dir2);
    if(status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_rmdir 2: %s\n",errname);
    }
    else
        printf("OK- OS_rmdir 2\n");


    
    printf("Leaving TestOpenReadCloseDir()\n");
    return 0;
}

/*---------------------------------------------------------------------------------------
 * Name TestRename
---------------------------------------------------------------------------------------*/
int TestRename(void)
{
    int status;
    char filename1[OS_MAX_PATH_LEN];
    char dir0 [OS_MAX_PATH_LEN];
    char dir1 [OS_MAX_PATH_LEN];

    char buffer1 [OS_MAX_PATH_LEN];
    char copybuffer1 [OS_MAX_PATH_LEN];

    char newdir1 [OS_MAX_PATH_LEN];
    char midname1 [OS_MAX_PATH_LEN];
    char newfilename1 [OS_MAX_PATH_LEN];
    
    int fd1;
    
    printf("In TestRename()\n");

    /* make the directory names for testing, as well as the filenames and the buffers
     * to put in the files */
    strcpy(dir0,"/drive1/");
    strcpy(dir1,"/drive1/DIRECTORY_ONE");
    strcpy(filename1,"/drive1/DIRECTORY_ONE/MyFile1");
    strcpy(buffer1,"111111111111");
    strcpy(copybuffer1,buffer1);

    strcpy(newdir1,"/drive1/NEW_DIR_ONE");
    strcpy(newfilename1,"/drive1/NEW_DIR_ONE/NEW_myfile1");
    
    strcpy(midname1,"/drive1/NEW_DIR_ONE/MyFile1");
    
    /* make the directory */
    
    status = OS_mkdir(dir1,0);
    if (status >= OS_FS_SUCCESS)
        printf("OK- OS_mkdir 1\n");
    else
    {
        printf("ERROR OS_mkdir 1, %s\n",errname);
    }
               
    /* now create a file in the directory */

    status = OS_creat(filename1,OS_READ_WRITE);
    if (status >= OS_FS_SUCCESS)
        printf("OK- OS_creat 1\n");
    else
    {
        printf("ERROR OS_creat 1, %s\n",errname);
    }
    
    fd1 = status;

    /* write the propper buffes into  the file */
    
    status = OS_write(fd1, buffer1, strlen(buffer1));
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR OS_write 1, %s\n",errname);
    }
    else
        printf("OK- OS_write 1\n");
  
    /* close the file */

    status = OS_close(fd1);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_close 1 %s\n",errname);
    }
    else
        printf("OK- OS_close 1\n");

/* now there is 1 subdirectory with 1 file in it */

    /* Rename the directory */

    status = OS_rename(dir1,newdir1);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_rename 1: %s,%d\n",errname,status);
    }
    else
        printf("OK- OS_rename 1\n");

    status = OS_rename(midname1,newfilename1);
    if(status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_rename 2 %s\n",errname);
    }
    else
        printf("OK- OS_rename 2\n");

    bzero(buffer1, strlen(buffer1));

    /* try to read the new file out */

    fd1 = OS_open(newfilename1,OS_READ_ONLY,644);
    if (fd1 < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_open 1 %s\n",errname);
    }
    else
        printf("OK- OS_open 1 \n");
    
    status = OS_read(fd1,buffer1,strlen(copybuffer1));
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_read 1 %s \n",errname);
    }
    else
    {
        printf("OK- OS_read 1 ");
        if (strncmp(buffer1,copybuffer1, strlen(copybuffer1)) == 0)
            printf("Read and Written Results are equal\n");
        else
            printf("Read and Written results differ \n");
    }
    
    status = OS_close(fd1);
    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_close 1 %s\n",errname);
    }
    else
        printf("OK- OS_close 1\n");

    /* try removing the file from the drive */
#if 0
    status = OS_remove(newfilename1);

    if (status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_remove 1 %s\n",errname);
    }
    else
        printf("OK- OS_remove 1\n");

    status = OS_rmdir(newdir1);
    if(status < OS_FS_SUCCESS)
    {
        printf("ERROR from OS_rmdir 1 %s \n",errname);
    }
    else
        printf("OK- OS_rmdir 1\n");
#endif
    printf("Leaving TestRename\n");
    return 0;
}

