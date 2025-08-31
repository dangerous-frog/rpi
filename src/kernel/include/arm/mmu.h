#ifndef _MMU_H
#define _MMU_H

/*
    Bro, just so I understand
    We have PGD (Page Global Directory), this points below to
    PUD (Page Upper Directory), this points below to
    PMD (Page Middle Directory), this points below to
    PTE (Page Table Entry), which tells us where to look

    Within an entry in those tables we have more info
    in the 47:12 bits we have address and 11:0 is some data
    This is achieved because 4KB (4096 b) pages, so 2^12





*/


// Below is from the ARM manual, my god it has sooooooo muuuuuuuuuuuuuch
#define MM_TYPE_PAGE_TABLE		0x3 // Tells MMU this is a table & valid
#define MM_TYPE_PAGE 			0x3 // this points to single pages & valid
#define MM_TYPE_BLOCK			0x1 // this points to block and is valid
#define MM_ACCESS			(0x1 << 10) // If this was set to 0 it would do page fault
#define MM_ACCESS_PERMISSION		(0x01 << 6) // Can't be R/W from EL0

/*
 * Memory region attributes:
 *
 *   n = AttrIndx[2:0]
 *			n	MAIR
 *   DEVICE_nGnRnE	000	00000000
 *   NORMAL_NC		001	01000100
 */
#define MT_DEVICE_nGnRnE 		0x0 // separate acces, no reorder, no early ack
#define MT_NORMAL_NC			0x1 // normal non cache
#define MT_DEVICE_nGnRnE_FLAGS		0x00
#define MT_NORMAL_NC_FLAGS  		0x44
#define MAIR_VALUE			(MT_DEVICE_nGnRnE_FLAGS << (8 * MT_DEVICE_nGnRnE)) | (MT_NORMAL_NC_FLAGS << (8 * MT_NORMAL_NC))

// Normal MMU Flags for just memory, let's say kernel
#define MMU_FLAGS	 		(MM_TYPE_BLOCK | (MT_NORMAL_NC << 2) | MM_ACCESS)	
// Device for the funky MMIO used by devices, caching that ain't gonna cut it
#define MMU_DEVICE_FLAGS		(MM_TYPE_BLOCK | (MT_DEVICE_nGnRnE << 2) | MM_ACCESS)
// Those ones are for user space, the dangerous bastards	
#define MMU_PTE_FLAGS			(MM_TYPE_PAGE | (MT_NORMAL_NC << 2) | MM_ACCESS | MM_ACCESS_PERMISSION)	

#define TCR_T0SZ			(64 - 48) 
#define TCR_T1SZ			((64 - 48) << 16)
#define TCR_TG0_4K			(0 << 14)
#define TCR_TG1_4K			(2 << 30)
#define TCR_VALUE			(TCR_T0SZ | TCR_T1SZ | TCR_TG0_4K | TCR_TG1_4K)

#endif