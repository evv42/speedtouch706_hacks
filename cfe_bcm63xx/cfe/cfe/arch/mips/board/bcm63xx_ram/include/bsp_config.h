/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *
    *  BSP Configuration file			File: bsp_config.h
    *
    *  This module contains global parameters and conditional
    *  compilation settings for building CFE.
    *
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *
    *********************************************************************
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */

#if defined(_BCM96338_)
#define CFG_CPU_SPEED   240000000      /* 240 Mhz in Hz */
#endif
#if defined(_BCM96348_)
#define CFG_CPU_SPEED   240000000      /* 240 Mhz in Hz */
#endif
#if defined(_BCM96358_)
#define CFG_CPU_SPEED   300000000      /* 300 Mhz in Hz */
#define CFG_CMT         1
#endif

#define CFG_INIT_L1             1      /* initialize the L1 cache */
#define CFG_INIT_L2             0      /* there is no L2 cache */

#define CFG_INIT_DRAM           1      /* initialize DRAM controller */
#define CFG_DRAM_SIZE           xxx    /* size of DRAM if you don't initialize */
                                       /* NOTE : Size is in kilobytes. */

#define CFG_NETWORK             1      /* define to include network support */

#define CFG_FATFS               0
#define CFG_UI                  1      /* Define to enable user interface */

#define CFG_MULTI_CPUS          0      /* no multi-cpu support */

#define CFG_HEAP_SIZE           1024   /* heap size in kilobytes */

#define CFG_STACK_SIZE          8192   /* stack size (bytes, rounded up to K) */

#define CFG_SERIAL_BAUD_RATE	115200	/* normal console speed */

#define CFG_VENDOR_EXTENSIONS   0
#define CFG_MINIMAL_SIZE        1

/*
 * These parameters control the flash driver's sector buffer.  
 * If you write environment variables or make small changes to
 * flash sectors from user applications, you
 * need to have the heap big enough to store a temporary sector
 * for merging in small changes to flash sectors, so you
 * should set CFG_FLASH_ALLOC_SECTOR_BUFFER in that case.
 * Otherwise, you can provide an address in unallocated memory
 * of where to place the sector buffer.
 */

#define CFG_FLASH_ALLOC_SECTOR_BUFFER 0	/* '1' to allocate sector buffer from the heap */
#define CFG_FLASH_SECTOR_BUFFER_ADDR  (1*1024*1024-128*1024) /* 1MB - 128K */
#define CFG_FLASH_SECTOR_BUFFER_SIZE  (128*1024)

/*
 * The flash staging buffer is where we store a flash image before we write
 * it to the flash.  It's too big for the heap.
 */

#define CFG_FLASH_STAGING_BUFFER_ADDR (1*1024*1024)
#define CFG_FLASH_STAGING_BUFFER_SIZE (1*1024*1024)
