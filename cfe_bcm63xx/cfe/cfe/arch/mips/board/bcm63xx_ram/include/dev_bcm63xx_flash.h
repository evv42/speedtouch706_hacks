/***************************************************************************
 * Broadcom Corp. Confidential
 * Copyright 2001 Broadcom Corp. All Rights Reserved.
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED 
 * SOFTWARE LICENSE AGREEMENT BETWEEN THE USER AND BROADCOM. 
 * YOU HAVE NO RIGHT TO USE OR EXPLOIT THIS MATERIAL EXCEPT 
 * SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 ***************************************************************************
 * File Name  : dev_bcm63xx_flash.h 
 *
 * Created on :  04/18/2002  seanl
 ***************************************************************************/

#if !defined(_DEV_BCM63XX_FLASH_)
#define _DEV_BCM63XX_FLASH_

#include "bcmtypes.h"
#include "board.h"

// Used for images that do not contain a FILE_TAG record.
#define FLASH_IMAGE_START_ADDR          (FLASH_BASE + FLASH_LENGTH_BOOT_ROM)

/*****************************************************************************/
/*       NVRAM definition                                                    */
/*****************************************************************************/
typedef struct
{
    unsigned long ulVersion;
    char szBootline[NVRAM_BOOTLINE_LEN];
    char szBoardId[NVRAM_BOARD_ID_STRING_LEN];
    unsigned long ulReserved1[2];
    unsigned long ulNumMacAddrs;
    unsigned char ucaBaseMacAddr[NVRAM_MAC_ADDRESS_LEN];
    char chReserved[2];
    unsigned long ulCheckSum;
} NVRAM_DATA, *PNVRAM_DATA;

typedef struct flashaddrinfo
{
    int flash_persistent_start_blk;
    int flash_persistent_number_blk;
    int flash_persistent_length;
    unsigned long flash_persistent_blk_offset;

    int flash_nvram_start_blk;
    int flash_nvram_number_blk;
    int flash_nvram_length;
    unsigned long flash_nvram_blk_offset;
} FLASH_ADDR_INFO, *PFLASH_ADDR_INFO;

void kerSysFlashInit(void);
void kerSysFlashAddrInfoGet(PFLASH_ADDR_INFO pflash_addr_info);
int kerSysNvRamSet(char *string,int strLen,int offset);
int kerSysNvRamGet(char *string,int strLen,int offset);
int kerSysBcmImageSet( int flash_start_addr, char *string, int size, int fWholeImage);
void kerSysMipsSoftReset(void);
int kerSysErasePsi(int);
int kerSysEraseNvRam(void);
int kerSysPsiSet(char *string,int strLen,int offset);
int kerSysPsiGet(char *string,int strLen,int offset);
int kerSysMemoryTypeSet(int size);
int kerSysMemoryTypeGet(void);
int kerSysThreadNumSet(int thread);
int kerSysThreadNumGet(void);
unsigned long kerSysReadFromFlash( void *toaddr, unsigned long fromaddr,
    unsigned long len );
void dumpHex(unsigned char *start, int len);
int getBootToCfe(void);

#endif /* _DEV_BCM63XX_FLASH_ */

