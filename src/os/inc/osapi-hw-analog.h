/*
** File Name:  os-hw-analog.h
**
** Project:    Common Flight Software
**
** Title:      OS ADC API Library Header File
**
** Revision:   $Revision: 1.1 $
**
** Date:       $Date: 2007/10/16 16:14:51EDT $
**
** Purpose:    This header file contains all the structures, type #defines
**             and prototypes required for the os adc api.
**
** Assumptions and Notes:
**
** Modification History:
**   MM/DD/YY       Change ID:     Author:                                 Description:
**   07/02/04       None           Dwaine Molock, NASA/GSFC Code 582       Initial Release
**
** $Log: osapi-hw-analog.h  $
** Revision 1.1 2007/10/16 16:14:51EDT apcudmore 
** Initial revision
** Member added to project d:/mksdata/MKS-OSAL-REPOSITORY/src/os/inc/project.pj
** Revision 1.1 2007/08/24 13:43:22EDT apcudmore 
** Initial revision
** Member added to project d:/mksdata/MKS-CFE-PROJECT/fsw/cfe-core/os/inc/project.pj
** Revision 1.2 2006/01/20 11:56:17EST njyanchik 
** Fixed header file information to match api document
** Revision 1.1  2005/03/15 18:26:32  nyanchik
** *** empty log message ***
**
** Revision 1.6  2004/11/18 16:17:47  aschoening
** Increased number of registers to 7
**
** Revision 1.5  2004/09/03 20:24:15  aschoening
** Updated ADC stuff again
**
** Revision 1.4  2004/09/03 19:49:17  aschoening
** Added hang prevention when checking ADC completion status
**
** Revision 1.3  2004/08/20 20:23:11  aschoening
** Integrated new ADC interface function for external mux setup
**
** Revision 1.2  2004/07/22 21:48:56  aschoening
** Added ADC Table Processing function
**
** Revision 1.1  2004/07/04 03:28:34  dmolock
** Added analog collection routines to OS API.
**
**
*/
#ifndef _osadc_
#define _osadc_

#define OS_ADC_EXTERN_REGISTERS 7
#define OS_ADC_NULL 0
#define OS_ADC_SDN  1
#define OS_ADC_PCI  2

#define OS_ADC_PCI_CARD_NO_REG_ERROR  0
#define OS_ADC_PCI_CARD_OLD_REG_ERROR 1
#define OS_ADC_PCI_CARD_NEW_REG_ERROR 2

/* Set maximum delay time as 1000 microseconds */
#define OS_ADC_MAX_TIME 1000

/* Set maximum time to wait for completion status at 50 microseconds to prevent hanging system */
#define OS_ADC_MAX_STATUS_TIME 50

typedef struct {
   uint32             Location; /* Location (SDN, PCI, or NULL) */
   uint32             Offset;   /* Offset (Address if SDN) */
   uint32             Data;     /* Data to write to Offset */
   uint16             Vendor;   /* Vendor ID */
   uint16             Device;   /* Device ID */
} OS_ADCSetupRegister_t;  /* Struct for setting up muxes */

typedef struct {
   OS_ADCSetupRegister_t  Register[OS_ADC_EXTERN_REGISTERS];  /* set up other muxes */
   uint32                 ADCMSRValue;  /* ADC Mux Select Register value  */
   uint32                 Delay;        /* us delay before reading analog */
} OS_ADCInputTable_t;  /* Table Structure for ADCProcessTable input  */

     /*
     ** OS ADC API
     ** Function Prototypes
     */
uint16 OS_ADCGetRawSample(uint32 mux, uint32 time, uint16 *value);

uint16 OS_ADCProcessTable(OS_ADCInputTable_t InputTable[], uint16 OutputTable[], uint32 Entries);

#endif

