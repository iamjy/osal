/*
** File   :	osapiarch.c
**
**
**      This is governed by the NASA Open Source Agreement and may be used, 
**      distributed and modified only pursuant to the terms of that agreement. 
**
**      Copyright © 2004-2006, United States government as represented by the 
**      administrator of the National Aeronautics Space Administration.  
**      All rights reserved. 
**
** Author :	Ezra Yeheskeli
**
** Purpose:
**		   This file  contains some of the OS APIs abstraction layer.
**         It contains the processor architecture specific calls.
**
**  16-Nov-2003 Ezra Yeheskeli
**          - First Creation.
**
*/

/*
** Include section
*/

#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

/*
** User defined include files
*/

#include "common_types.h"
#include "osapi.h"


/*
** EEPROM Defines
*/

#define EEPROM_WRITE_TIMEOUT	0x0000FFFF

/*
** global memory
*/

/*
** Global Memory buffers that represents the hardware memory, eeprom, and I/O space
** 1 Megabyte of memory for mem read/write
*/
unsigned long int os_register_bank[10];

unsigned long int os_sram_bank  [128 * 1024];
unsigned long int os_eeprom_bank[128 * 1024];



/*
** Name: OS_MemRead8
**
** Purpose:
**         Read one byte of memory.
**
**
** Parameters:
**	MemoryAddress : Address to be read
**  ByteValue  : The address content will be copied to the location pointed by this argument
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values: OS_SUCCESS
*/
int32 OS_MemRead8( uint32 MemoryAddress, uint8 *ByteValue )
{

	(*ByteValue) = (uint8)*((uint8 *)MemoryAddress) ;

	return(OS_SUCCESS) ;
}

/*
** Name: OS_MemWrite8
**
** Purpose:
**         Write one byte of memory.
**
**
** Parameters:
**	MemoryAddress : Address to be written to
**  ByteValue  : The content pointed by this argument will be copied to the address
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
*/
int32 OS_MemWrite8 ( uint32 MemoryAddress, uint8 ByteValue )
{
   *((uint8 *)MemoryAddress) = ByteValue;
	return(OS_SUCCESS) ;

}

/*
** Name: OS_MemRead16
**
** Purpose:
**         Read  2 bytes of memory.
**
**
** Parameters:
**	MemoryAddress : Address to be read
**  uint16Value : The address content will be copied to the location pointed by
**            this argument
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_MemRead16( uint32 MemoryAddress, uint16 *uint16Value )
{
	/* check 16 bit alignment  , check the 1st lsb */
	if( MemoryAddress & 0x00000001)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
	(*uint16Value) = (uint16)*((uint16 *)MemoryAddress) ;
	return(OS_SUCCESS) ;

}


/*
** Name: OS_MemWrite16
**
** Purpose:
**         Write 2 byte of memory.
**
**
** Parameters:
**	MemoryAddress : Address to be written to
**  uint16Value : The content pointed by this argument will be copied to the
**            address
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**      OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The MemoryAddress is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_MemWrite16 ( uint32 MemoryAddress, uint16 uint16Value )
{
	/* check 16 bit alignment  , check the 1st lsb */
	if( MemoryAddress & 0x00000001)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
   *((uint16 *)MemoryAddress) = uint16Value;
	return(OS_SUCCESS) ;
}

/*
** Name: OS_MemRead32
**
** Purpose:
**         Read 4 bytes of memory.
**
**
** Parameters:
**	MemoryAddress : Address to be read
**  uint32Value : The address content will be copied to the location pointed by
**            this argument
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_MemRead32( uint32 MemoryAddress, uint32 *uint32Value )
{
	/* check 32 bit alignment  */
	if( MemoryAddress & 0x00000003)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
	(*uint32Value) = *((uint32 *)MemoryAddress) ;
	return(OS_SUCCESS) ;

}

/*
** Name: OS_MemWrite32
**
** Purpose:
**         Write 4 byte of memory.
**
**
** Parameters:
**	MemoryAddress : Address to be written to
**  uint32Value : The content pointed by this argument will be copied to the
**            address
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_MemWrite32 ( uint32 MemoryAddress, uint32 uint32Value )
{
	/* check 32 bit alignment  */
	if( MemoryAddress & 0x00000003)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
   *((uint32 *)MemoryAddress) = uint32Value;
	return(OS_SUCCESS) ;

}



/*
** Name: OS_PortRead8
**
** Purpose:
**         Read one byte of memory.
**
**
** Parameters:
**	PortAddress : Address to be read
**  ByteValue  : The address content will be copied to the location pointed by
**            this argument
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values: OS_SUCCESS
*/
int32 OS_PortRead8( uint32 PortAddress, uint8 *ByteValue )
{

	(*ByteValue) = (uint8)*((uint8 *)PortAddress) ;

	return(OS_SUCCESS) ;
}

/*
** Name: OS_PortWrite8
**
** Purpose:
**         Write one byte of memory.
**
**
** Parameters:
**	PortAddress : Address to be written to
**  ByteValue  : The content pointed by this argument will be copied to the
**            address
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
*/
int32 OS_PortWrite8 ( uint32 PortAddress, uint8 ByteValue )
{
   *((uint8 *)PortAddress) = ByteValue;
	return(OS_SUCCESS) ;

}

/*
** Name: OS_PortRead16
**
** Purpose:
**         Read  2 bytes of memory.
**
**
** Parameters:
**	PortAddress : Address to be read
**  uint16Value : The address content will be copied to the location pointed by
**            this argument
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_PortRead16( uint32 PortAddress, uint16 *uint16Value )
{
	/* check 16 bit alignment  , check the 1st lsb */
	if( PortAddress & 0x00000001)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
	(*uint16Value) = (uint16)*((uint16 *)PortAddress) ;
	return(OS_SUCCESS) ;

}


/*
** Name: OS_PortWrite16
**
** Purpose:
**         Write 2 byte of memory.
**
**
** Parameters:
**	PortAddress : Address to be written to
**  uint16Value : The content pointed by this argument will be copied to the
**            address
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**      OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_PortWrite16 ( uint32 PortAddress, uint16 uint16Value )
{
	/* check 16 bit alignment  , check the 1st lsb */
	if( PortAddress & 0x00000001)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
   *((uint16 *)PortAddress) = uint16Value;
	return(OS_SUCCESS) ;
}

/*
** Name: OS_PortRead32
**
** Purpose:
**         Read 4 bytes of memory.
**
**
** Parameters:
**	PortAddress : Address to be read
**  uint32Value : The address content will be copied to the location pointed by
**            this argument
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_PortRead32( uint32 PortAddress, uint32 *uint32Value )
{
	/* check 32 bit alignment  */
	if( PortAddress & 0x00000003)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
	(*uint32Value) = *((uint32 *)PortAddress) ;
	return(OS_SUCCESS) ;

}

/*
** Name: OS_PortWrite32
**
** Purpose:
**         Write 4 byte of memory.
**
**
** Parameters:
**	PortAddress : Address to be written to
**  uint32Value : The content pointed by this argument will be copied to the
**            address
**
** Global Inputs: None
**
** Global Outputs: None
**
**
**
** Return Values:
**		OS_SUCCESS
**		OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit
**      addressing scheme.
*/
int32 OS_PortWrite32 ( uint32 PortAddress, uint32 uint32Value )
{
	/* check 32 bit alignment  */
	if( PortAddress & 0x00000003)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}
   *((uint32 *)PortAddress) = uint32Value;
	return(OS_SUCCESS) ;

}

/*
** Name: OS_MemCpy
**
** Purpose:
**	Copies 'size' byte from memory address pointed by 'src' to memory
**  address pointed by ' dst' For now we are using the standard c library
**  call 'memcpy' but if we find we need to make it more efficient then
**  we'll implement it in assembly.
**
** Assumptions and Notes:
**
** Parameters:
**	dst : pointer to an address to copy to
**  src : pointer address to copy from
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values: OS_SUCCESS
*/
int32 OS_MemCpy ( void *dst, void *src, uint32 size)
{
	memcpy( dst, src, size);
	return(OS_SUCCESS) ;
} 



/*
** Name: OS_MemSet
**
** Purpose:
**	Copies 'size' number of byte of value 'value' to memory address pointed
**  by 'dst' .For now we are using the standard c library call 'memset'
**  but if we find we need to make it more efficient then we'll implement
**  it in assembly.
**
**
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values: OS_SUCCESS
*/
/*
** OS_MemSet
*/
int32 OS_MemSet ( void *dst, uint8 value , uint32 size)
{
    memset( dst, (int)value, (size_t)size);
	return(OS_SUCCESS) ;
} 




/*
** Name: OS_EepromWrite32
**
** Purpose:
**
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values:
**	 OS_SUCCESS
**	 OS_ERROR_TIMEOUT write operation did not go through after a specific
**   timeout.
**	 OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit addressing
**   scheme.
*/
int32 OS_EepromWrite32( uint32 MemoryAddress, uint32 uint32Value )
{
    uint32 ret_value = OS_SUCCESS;

	/* check 32 bit alignment  */
	if( MemoryAddress & 0x00000003)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}

   /* make the Write */
   *((uint32 *)MemoryAddress) = uint32Value;

	return(ret_value) ;
}


/*
** Name: OS_EepromWrite16
**
** Purpose:
**
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values:
**   OS_SUCCESS
**	 OS_ERROR_TIMEOUT write operation did not go through after a specific
**   timeout.
**   OS_ERROR_ADD_MISALIGNED The Address is not aligned to 16 bit addressing
**   scheme.
*/
int32 OS_EepromWrite16( uint32 MemoryAddress, uint16 uint16Value )
{
	uint32 write32 ,temp32 ;
	uint32 aligned_address ;


	/* check 16 bit alignment  , check the 1st lsb */
	if( MemoryAddress & 0x00000001)
	{
		return(OS_ERROR_ADDRESS_MISALIGNED) ;
	}


	temp32 = uint16Value ;

    /* check the 2nd lsb */
	if( MemoryAddress & 0x00000002 )
	{
		/* writting the 16 high bit order of 32 bit field */
		aligned_address = MemoryAddress-2 ;

		OS_MemRead32 ( aligned_address  ,&write32)  ;
		write32 = (write32 & 0x0000FFFF) | (temp32<<16 ) ;
	}
	else
	{
		/* writting the 16 low bit order of 32 bit field */
		aligned_address = MemoryAddress ;

		OS_MemRead32 (  aligned_address, &write32 ) ;
		write32 = (write32 ) | (temp32 & 0xFFFF0000 ) ;
	}


	return(OS_EepromWrite32(aligned_address,write32)) ;

}


/*
** Name: OS_EepromWrite8
**
** Purpose:
**
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values:
**   OS_SUCCESS
**	 OS_ERROR_TIMEOUT write operation did not go through after a specific
**   timeout.
*/
int32 OS_EepromWrite8( uint32 MemoryAddress, uint8 ByteValue )
{
	uint32 aligned_address ;
	uint16 write16 ,temp16;



	temp16 = ByteValue ;

    /* check the 1st lsb */
	if( MemoryAddress & 0x00000001)
	{
		/* writting the 8 high bit order of 16 bit field */
		aligned_address = MemoryAddress-1 ;

		OS_MemRead16 ( aligned_address  ,&write16)  ;
		write16 = (write16 & 0x00FF) | (temp16<<8 ) ;
	}
	else
	{
		/* writting the 8 low bit order of 16 bit field */
		aligned_address = MemoryAddress ;

		OS_MemRead16 (  aligned_address, &write16 ) ;
		write16 = (write16 ) | (temp16 & 0xFF00 ) ;
	}


	return(OS_EepromWrite16(aligned_address,write16)) ;

}

/*
** Name: OS_EepromWriteEnable
**
** Purpose:
**		Eable the eeprom for write operation
**
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values:
**   OS_SUCCESS
*/
int32 OS_EepromWriteEnable()
{

	return(OS_SUCCESS) ;
}

/*
** Name: OS_EepromWriteDisable
**
** Purpose:
**		Disable  the eeprom from write operation
**
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values:
**   OS_SUCCESS
*/
int32 OS_EepromWriteDisable()
{

	return(OS_SUCCESS) ;

}


/*
** Name: OS_EepromPowerUp
**
** Purpose:
**		Power up the eeprom
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values:
**   OS_SUCCESS
*/
int32 OS_EepromPowerUp()
{
	return(OS_SUCCESS) ;
}



/*
** Name: OS_EepromPowerDown
**
** Purpose:
**		Power down the eeprom
** Assumptions and Notes:
**
** Parameters:
**
** Global Inputs: None
**
** Global Outputs: None
**
**
** Return Values:
**   OS_SUCCESS
*/
int32 OS_EepromPowerDown()
{
	return(OS_SUCCESS) ;
}
