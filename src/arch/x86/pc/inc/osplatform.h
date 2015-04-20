/*
** osplatform.h
**
**
**      This is governed by the NASA Open Source Agreement and may be used, 
**      distributed and modified only pursuant to the terms of that agreement. 
**
**      Copyright © 2004-2006, United States government as represented by the 
**      administrator of the National Aeronautics Space Administration.  
**      All rights reserved. 
**
**
*/

#ifndef _osplatform_
#define _osplatform_

/*
** external definitions for simulated registers
** There should probably be a base set of generic 
** registers for core fsw
*/
extern unsigned long int os_register_bank[10];

extern unsigned long int os_sram_bank  [128 * 1024];
extern unsigned long int os_eeprom_bank[128 * 1024];

/*
** Address ranges for all memory banks.
*/

#define OS_SRAM_BANK_START_ADDRESS     ((uint32 )(&(os_sram_bank[0])))
#define OS_SRAM_BANK_END_ADDRESS	   ((uint32 )(&(os_sram_bank[0]) + sizeof(os_sram_bank)))

#define OS_EEPROM_BANK_START_ADDRESS   ((uint32 )(&(os_eeprom_bank[0])))
#define OS_EEPROM_BANK_END_ADDRESS     ((uint32 )(&(os_eeprom_bank[0]) + sizeof(os_eeprom_bank)))


/*
** EEPROM characteristic
*/
#define OS_EEPROM_WRITE_DELAY          20 /* time required for the EEPROM to write itself */


#endif

