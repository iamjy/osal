/*
** File: osapi-os-net.h
**
** Author:  Alan Cudmore Code 582
**
** Purpose: Contains functions prototype definitions and variables declarations
**          for the OS Abstraction Layer, Network Module
**
** $Revision: 1.1 $ 
**
** $Date: 2007/10/16 16:14:52EDT $
**
** $Log: osapi-os-net.h  $
** Revision 1.1 2007/10/16 16:14:52EDT apcudmore 
** Initial revision
** Member added to project d:/mksdata/MKS-OSAL-REPOSITORY/src/os/inc/project.pj
** Revision 1.1 2007/08/24 13:43:25EDT apcudmore 
** Initial revision
** Member added to project d:/mksdata/MKS-CFE-PROJECT/fsw/cfe-core/os/inc/project.pj
** Revision 1.3 2006/01/20 11:56:18EST njyanchik 
** Fixed header file information to match api document
** Revision 1.4  2005/06/07 16:49:31  nyanchik
** changed returns code for osapi.c to all int32 from uint32
**
** Revision 1.3  2005/03/22 19:04:54  acudmore
** fixed uint type
**
** Revision 1.2  2005/03/22 18:59:33  acudmore
** updated prototype
**
** Revision 1.1  2005/03/22 18:58:51  acudmore
** added osapi network interface
**
** Revision 1.1  2005/03/15 18:26:32  nyanchik
** *** empty log message ***
**
**
** Date Written:
**
**    
*/
#ifndef _osapi_network_
#define _osapi_network_

int32 OS_NetworkGetID             (void);
int32 OS_NetworkGetHostName       (char *host_name, uint32 name_len);

#endif
