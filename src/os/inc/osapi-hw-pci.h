/*
** File Name:  os-hw-pci.h
**
** Project:    Common Flight Software
**
** Title:      OS PCI API Library Header File
**
** Revision:   $Revision: 1.1 $
**
** Date:       $Date: 2007/10/16 16:14:51EDT $
**
** Purpose:    This header file contains all the structures, type #defines
**             and prototypes required for the os pci api.
**
** Assumptions and Notes:
**
** Modification History:
**   MM/DD/YY       Change ID:     Author:                                 Description:
**   05/19/04       None           Dwaine Molock, NASA/GSFC Code 582       Initial Release
**
** $Log: osapi-hw-pci.h  $
** Revision 1.1 2007/10/16 16:14:51EDT apcudmore 
** Initial revision
** Member added to project d:/mksdata/MKS-OSAL-REPOSITORY/src/os/inc/project.pj
** Revision 1.1 2007/08/24 13:43:23EDT apcudmore 
** Initial revision
** Member added to project d:/mksdata/MKS-CFE-PROJECT/fsw/cfe-core/os/inc/project.pj
** Revision 1.2 2006/01/20 11:56:17EST njyanchik 
** Fixed header file information to match api document
** Revision 1.1  2005/03/15 18:26:32  nyanchik
** *** empty log message ***
**
** Revision 1.9  2004/11/19 22:20:30  pkutt
** Added newline to end of file.
**
** Revision 1.8  2004/11/18 18:26:17  jswinski
** added shared memory routines to api to account for byte swapping present on PowerPC
**
** Revision 1.7  2004/07/22 16:02:49  dwilson
** Bug#0000177:   Updated OS_PCI_VENDOR_ID and OS_PCI_DEVICE_ID in ospci.h
** to OS_PCI_VENDOR_OFFSET and OS_PCI_DEVICE_OFFSET
**
** Revision 1.6  2004/07/06 18:10:06  dmolock
** Updated OS PCI API routines.
**
** Revision 1.5  2004/06/04 14:24:48  pkutt
** Fixed guard macros in header files.  Added includes where needed.
**
** Revision 1.4  2004/05/27 21:49:10  dmolock
** Modifications to PCI bus API target ids and functions prototypes.
**
** Revision 1.3  2004/05/27 21:07:40  dmolock
** Modifications to PCI bus API: Added target ids and interrupt handler access routines.
**
** Revision 1.3  2004/05/27 20:45:22  dmolock
** Modifications to PCI bus API: Added target ids and interrupt handler access routines.
**
** Revision 1.2  2004/05/20 17:00:32  dmolock
** Added OS PCI Routines & Updated Makefiles
**
** Revision 1.1  2004/05/20 16:06:20  dmolock
** Added PCI stub routines and semaphores
**
*/
#ifndef _ospci_
#define _ospci_

#define OS_PCI_INVALID_VENDORDEVICEID 0xffffffff
#define OS_PCI_MULTI_FUNCTION         0x80

/*
** Under PCI, each device has 256 bytes of configuration address space,
** of which the first 64 bytes are standardized as follows:
**/
#define OS_PCI_VENDOR_ID_OFFSET           0x00                    /* 16 bits */
#define OS_PCI_DEVICE_ID_OFFSET           0x02                    /* 16 bits */
#define OS_PCI_COMMAND                    0x04                    /* 16 bits */
#define OS_PCI_STATUS                     0x06                    /* 16 bits */
#define OS_PCI_CLASS_REVISION             0x08    /* High 24 bits are class  */
                                                  /* Low 8 bits are revision */
#define OS_PCI_REVISION_ID                0x08                /* Revision ID */
#define OS_PCI_CLASS_PROG                 0x09     /* Reg. Level Programming */
                                                   /*              Interface */
#define OS_PCI_CLASS_DEVICE               0x0a               /* Device class */
#define OS_PCI_CACHE_LINE_SIZE            0x0c                     /* 8 bits */
#define OS_PCI_LATENCY_TIMER              0x0d                     /* 8 bits */
#define OS_PCI_HEADER_TYPE                0x0e                     /* 8 bits */
#define OS_PCI_BIST                       0x0f                     /* 8 bits */

/*
** Base addresses specify locations in memory or I/O space.
** Decoded size can be determined by writing a value of
** 0xffffffff to the register, and reading it back.  Only
** 1 bits are decoded.
**/
#define OS_PCI_BASE_ADDRESS_0    0x10 /* 32 bits */
#define OS_PCI_BASE_ADDRESS_1    0x14 /* 32 bits [htype 0,1 only] */
#define OS_PCI_BASE_ADDRESS_2    0x18 /* 32 bits [htype 0 only] */
#define OS_PCI_BASE_ADDRESS_3    0x1c /* 32 bits */
#define OS_PCI_BASE_ADDRESS_4    0x20 /* 32 bits */
#define OS_PCI_BASE_ADDRESS_5    0x24 /* 32 bits */

/* Header type 0 (normal devices) */
#define OS_PCI_CARDBUS_CIS            0x28
#define OS_PCI_SUBSYSTEM_VENDOR_ID    0x2c
#define OS_PCI_SUBSYSTEM_DEVICE_ID    0x2e
#define OS_PCI_ROM_ADDRESS            0x30 /* Bits 31..11 are address, 10..1 reserved */
#define OS_PCI_INTERRUPT_LINE         0x3c /* 8 bits */
#define OS_PCI_INTERRUPT_PIN          0x3d /* 8 bits */
#define OS_PCI_MIN_GNT                0x3e /* 8 bits */
#define OS_PCI_MAX_LAT                0x3f /* 8 bits */
#define OS_PCI_INT_CNTRL_STATUS_REG   0x48 /* 32 bits */
#define OS_PCI_HOT_SWAP_REG           0x80 /* 32 bits */

#define OS_PCI_COMMAND_IO             0x001 /* Enable response in I/O space */
#define OS_PCI_COMMAND_MEMORY         0x002 /* Enable response in Memory space */
#define OS_PCI_COMMAND_MASTER         0x004 /* Enable bus mastering */
#define OS_PCI_COMMAND_SPECIAL        0x008 /* Enable response to special cycles */
#define OS_PCI_COMMAND_INVALIDATE     0x010 /* Use memory write and invalidate */
#define OS_PCI_COMMAND_VGA_PALETTE    0x020 /* Enable palette snooping */
#define OS_PCI_COMMAND_PARITY         0x040 /* Enable parity checking */
#define OS_PCI_COMMAND_WAIT           0x080 /* Enable address/data stepping */
#define OS_PCI_COMMAND_SERR           0x100 /* Enable SERR */
#define OS_PCI_COMMAND_FAST_BACK      0x200 /* Enable back-to-back writes */
#define OS_PCI_COMMAND_INT_DISABLE    0x400 /* Disables Target Interrupt */

#define OS_PCI_STATUS_INTERRUPT_STATE      0x0004 /* Indicates Current Interrupt State */
#define OS_PCI_STATUS_HOT_SWAP_ENABLE      0x0010
#define OS_PCI_STATUS_66MHZ                0x0020 /* Support 66 Mhz PCI 2.1 bus */
#define OS_PCI_STATUS_UDF                  0x0040 /* Support User Definable Features */
#define OS_PCI_STATUS_FAST_BACK            0x0080 /* Accept fast-back to back */
#define OS_PCI_STATUS_PARITY               0x0100 /* Detected parity error */
#define OS_PCI_STATUS_DEVSEL_MASK          0x0600 /* DEVSEL timing */
#define OS_PCI_STATUS_DEVSEL_FAST          0x0000
#define OS_PCI_STATUS_DEVSEL_MEDIUM        0x0200
#define OS_PCI_STATUS_DEVSEL_SLOW          0x0400
#define OS_PCI_STATUS_SIG_TARGET_ABORT     0x0800 /* Set on target abort */
#define OS_PCI_STATUS_REC_TARGET_ABORT     0x1000 /* Master ack of " */
#define OS_PCI_STATUS_REC_MASTER_ABORT     0x2000 /* Set on master abort */
#define OS_PCI_STATUS_SIG_SYSTEM_ERROR     0x4000 /* Set when we drive SERR */
#define OS_PCI_STATUS_DETECTED_PARITY      0x8000 /* Set on parity error */

#define OS_PCI_HEADER_TYPE_NORMAL     0
#define OS_PCI_HEADER_TYPE_BRIDGE     1
#define OS_PCI_HEADER_TYPE_CARDBUS    2

#define OS_PCI_BIST_CODE_MASK         0x0f /* Return result */
#define OS_PCI_BIST_START             0x40 /* 1 to start BIST, 2 secs or less */
#define OS_PCI_BIST_CAPABLE           0x80 /* 1 if BIST capable */

#define OS_PCI_BASE_ADDRESS_SPACE          0x01 /* 0 = memory, 1 = I/O */
#define OS_PCI_BASE_ADDRESS_SPACE_IO       0x01
#define OS_PCI_BASE_ADDRESS_SPACE_MEMORY   0x00
#define OS_PCI_BASE_ADDRESS_MEM_TYPE_MASK  0x06
#define OS_PCI_BASE_ADDRESS_MEM_TYPE_32    0x00 /* 32 bit address */
#define OS_PCI_BASE_ADDRESS_MEM_TYPE_1M    0x02 /* Below 1M */
#define OS_PCI_BASE_ADDRESS_MEM_TYPE_64    0x04 /* 64 bit address */
#define OS_PCI_BASE_ADDRESS_MEM_PREFETCH   0x08 /* prefetchable? */
#define OS_PCI_BASE_ADDRESS_MEM_MASK       (~0x0f)
#define OS_PCI_BASE_ADDRESS_IO_MASK        (~0x03)

#define OS_PCI_ROM_ADDRESS_ENABLE          0x01
#define OS_PCI_ROM_ADDRESS_MASK            (~0x7ff)

#define OS_PCI_INT_CNTRL_EXT_INT_ACTIVE    0x0100
#define OS_PCI_INT_CNTRL_EXT_INT_ENABLE    0x0200

/* Header type 1 (PCI-to-PCI bridges) */
#define OS_PCI_PRIMARY_BUS            0x18 /* Primary bus number */
#define OS_PCI_SECONDARY_BUS          0x19 /* Secondary bus number */
#define OS_PCI_SUBORDINATE_BUS        0x1a /* Highest bus number behind the bridge */
#define OS_PCI_SEC_LATENCY_TIMER      0x1b /* Latency timer for secondary interface */
#define OS_PCI_IO_BASE                0x1c /* I/O range behind the bridge */
#define OS_PCI_IO_LIMIT               0x1d
#define OS_PCI_SEC_STATUS             0x1e /* Secondary status register, only bit 14 used */
#define OS_PCI_MEMORY_BASE            0x20 /* Memory range behind */
#define OS_PCI_MEMORY_LIMIT           0x22
#define OS_PCI_PREF_MEMORY_BASE       0x24 /* Prefetchable memory range behind */
#define OS_PCI_PREF_MEMORY_LIMIT      0x26
#define OS_PCI_PREF_BASE_UPPER32      0x28 /* Upper half of prefetchable memory range */
#define OS_PCI_PREF_LIMIT_UPPER32     0x2c
#define OS_PCI_IO_BASE_UPPER16        0x30 /* Upper half of I/O addresses */
#define OS_PCI_IO_LIMIT_UPPER16       0x32
#define OS_PCI_ROM_ADDRESS1           0x38 /* Same as OS_PCI_ROM_ADDRESS, but for htype 1 */
#define OS_PCI_BRIDGE_CONTROL         0x3e

#define  OS_PCI_IO_RANGE_TYPE_MASK         0x0f /* I/O bridging type */
#define  OS_PCI_IO_RANGE_TYPE_16           0x00
#define  OS_PCI_IO_RANGE_TYPE_32           0x01
#define  OS_PCI_IO_RANGE_MASK              ~0x0f

#define  OS_PCI_MEMORY_RANGE_TYPE_MASK     0x0f
#define  OS_PCI_MEMORY_RANGE_MASK          ~0x0f

#define  OS_PCI_PREF_RANGE_TYPE_MASK       0x0f
#define  OS_PCI_PREF_RANGE_TYPE_32         0x00
#define  OS_PCI_PREF_RANGE_TYPE_64         0x01
#define  OS_PCI_PREF_RANGE_MASK            ~0x0f

#define  OS_PCI_BRIDGE_CTL_PARITY          0x01 /* Enable parity detection on secondary interface */
#define  OS_PCI_BRIDGE_CTL_SERR            0x02 /* The same for SERR forwarding */
#define  OS_PCI_BRIDGE_CTL_NO_ISA          0x04 /* Disable bridging of ISA ports */
#define  OS_PCI_BRIDGE_CTL_VGA             0x08 /* Forward VGA addresses */
#define  OS_PCI_BRIDGE_CTL_MASTER_ABORT    0x20 /* Report master aborts */
#define  OS_PCI_BRIDGE_CTL_BUS_RESET       0x40 /* Secondary bus reset */
#define  OS_PCI_BRIDGE_CTL_FAST_BACK       0x80 /* Fast Back2Back enabled on secondary interface */

/* Header type 2 (CardBus bridges) */
#define OS_PCI_CB_SEC_STATUS          0x16 /* Secondary status */
#define OS_PCI_CB_PRIMARY_BUS         0x18 /* PCI bus number */
#define OS_PCI_CB_CARD_BUS            0x19 /* CardBus bus number */
#define OS_PCI_CB_SUBORDINATE_BUS     0x1a /* Subordinate bus number */
#define OS_PCI_CB_LATENCY_TIMER       0x1b /* CardBus latency timer */
#define OS_PCI_CB_MEMORY_BASE_0       0x1c
#define OS_PCI_CB_MEMORY_LIMIT_0      0x20
#define OS_PCI_CB_MEMORY_BASE_1       0x24
#define OS_PCI_CB_MEMORY_LIMIT_1      0x28
#define OS_PCI_CB_IO_BASE_0           0x2c
#define OS_PCI_CB_IO_BASE_0_HI        0x2e
#define OS_PCI_CB_IO_LIMIT_0          0x30
#define OS_PCI_CB_IO_LIMIT_0_HI       0x32
#define OS_PCI_CB_IO_BASE_1           0x34
#define OS_PCI_CB_IO_BASE_1_HI        0x36
#define OS_PCI_CB_IO_LIMIT_1          0x38
#define OS_PCI_CB_IO_LIMIT_1_HI       0x3a
#define OS_PCI_CB_BRIDGE_CONTROL      0x3e
#define OS_PCI_CB_SUBSYSTEM_VENDOR_ID 0x40
#define OS_PCI_CB_SUBSYSTEM_ID        0x42
#define OS_PCI_CB_LEGACY_MODE_BASE    0x44 /* 16-bit PC Card legacy mode base address (ExCa) */

#define  OS_PCI_CB_IO_RANGE_MASK                ~0x03

#define  OS_PCI_CB_BRIDGE_CTL_PARITY            0x001 /* Similar to standard bridge control register */
#define  OS_PCI_CB_BRIDGE_CTL_SERR              0x002
#define  OS_PCI_CB_BRIDGE_CTL_ISA               0x004
#define  OS_PCI_CB_BRIDGE_CTL_VGA               0x008
#define  OS_PCI_CB_BRIDGE_CTL_MASTER_ABORT      0x020
#define  OS_PCI_CB_BRIDGE_CTL_CB_RESET          0x040 /* CardBus reset */
#define  OS_PCI_CB_BRIDGE_CTL_16BIT_INT         0x080 /* Enable interrupt for 16-bit cards */
#define  OS_PCI_CB_BRIDGE_CTL_PREFETCH_MEM0     0x100 /* Prefetch enable for both memory regions */
#define  OS_PCI_CB_BRIDGE_CTL_PREFETCH_MEM1     0x200
#define  OS_PCI_CB_BRIDGE_CTL_POST_WRITES       0x400

     /*
     ** PCI Defines
     */
typedef   void (*OSPCIINTHANDLER)(unsigned long);   /* pointer to function that receives an unsigned long */

#define OS_PCI_TARGET_NOT_PRESENT          -1
#define OS_PCI_TARGET_PRESENT              0

#define OS_PCI_TARGET_INT_ENABLED          OS_PCI_TARGET_PRESENT
#define OS_PCI_TARGET_INT_DISABLED         OS_PCI_TARGET_PRESENT

#define OS_PCI_TARGET_ISR_CONNECTED        OS_PCI_TARGET_PRESENT
#define OS_PCI_TARGET_ISR_DISCONNECTED     OS_PCI_TARGET_PRESENT

#define OS_PCI_TARGET_ENABLED              OS_PCI_TARGET_PRESENT
#define OS_PCI_TARGET_DISABLED             OS_PCI_TARGET_PRESENT

#define OS_PCI_TARGET_RESET                OS_PCI_TARGET_PRESENT

#define OS_PCI_ACCESS_TERMINATED           -1
#define OS_PCI_ACCESS_SUCCESSFUL           0

#define OS_PCI_RESOURCE_NOT_PRESENT        -1
#define OS_PCI_RESOURCE_IO                 0
#define OS_PCI_RESOURCE_MEM                1

#define OS_PCI_XFER_MODE_FIFO              FALSE
#define OS_PCI_XFER_MODE_SRAM              TRUE

#define OS_PCI_MAX_TARGET_DEVICES          7
#define OS_PCI_MAX_BASE_ADDRESSES          6

#define OS_PCI_TARGET_SLOT2_INDEX          0
#define OS_PCI_TARGET_SLOT3_INDEX          1
#define OS_PCI_TARGET_SLOT4_INDEX          2
#define OS_PCI_TARGET_SLOT5_INDEX          3
#define OS_PCI_TARGET_SLOT6_INDEX          4
#define OS_PCI_TARGET_SLOT7_INDEX          5
#define OS_PCI_TARGET_SLOT8_INDEX          6


     /*
     ** OS PCI API
     ** Function Prototypes
     */

int OS_PciScanAndConfigureBus(unsigned long base);
int OS_PciFindDevice(uint16 vendor, uint16 device, int index, uint32 *target);
int OS_PciFindSubsystemDevice(uint16 subvendor, uint16 subdevice, int index, uint32 *target);
int OS_PciFindTargetDevice(uint16 vendor, uint16 device, uint16 subvendor, uint16 subdevice, int index, uint32 *target);

int OS_PciReadConfigurationByte(uint32 target, int offset, uint8 *ptr);
int OS_PciReadConfigurationWord(uint32 target, int offset, uint16 *ptr);
int OS_PciReadConfigurationDword(uint32 target, int offset, uint32 *ptr);
int OS_PciWriteConfigurationByte(uint32 target, int offset, uint8 value);
int OS_PciWriteConfigurationWord(uint32 target, int offset, uint16 value);
int OS_PciWriteConfigurationDword(uint32 target, int offset, uint32 value);

unsigned long OS_PciGetResourceStartAddr(uint32 target, int bar);
unsigned long OS_PciGetResourceEndAddr(uint32 target, int bar);
unsigned long OS_PciGetResourceLen(uint32 target, int bar);
unsigned long OS_PciGetResourceFlags(uint32 target, int bar);

void OS_PciInterruptServiceRoutine(unsigned long parameter);

int OS_PciEnableTargetInterrupt(uint32 target);
int OS_PciDisableTargetInterrupt(uint32 target);
int OS_PciConnectTargetIsr(uint32 target, OSPCIINTHANDLER handler, unsigned long parameter);
int OS_PciDisconnectTargetIsr(uint32 target);
int OS_PciEnableTarget(uint32 target);
int OS_PciDisableTarget(uint32 target);

int OS_PciGetBaseAddress(uint16 subvendor, uint16 subdevice, int index, uint32 *target, uint32 *address, int bar);
int OS_PciResetTarget(uint32 target);
int OS_PciBusWrite(uint32 pci, uint32 src, int items);
int OS_PciBusRead(uint32 pci, uint32 dst, int items);
int OS_PciInterruptHandlerWrite(uint32 address, uint32 data);
int OS_PciInterruptHandlerRead(uint32 address, uint32 *data);

void OS_PciMemWriteByte(uint8* addr, uint8 val);
void OS_PciMemWriteWord(uint16* addr, uint16 val);
void OS_PciMemWriteDword(uint32* addr, uint32 val);

uint8 OS_PciMemReadByte(uint8* addr);
uint16 OS_PciMemReadWord(uint16* addr);
uint32 OS_PciMemReadDword(uint32* addr);

#endif

