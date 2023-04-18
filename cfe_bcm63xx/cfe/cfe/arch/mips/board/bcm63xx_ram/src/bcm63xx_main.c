/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Main Module				File: cfe_main.c       
    *  
    *  This module contains the main "C" routine for CFE and 
    *  the main processing loop.  There should not be any board-specific
    *  stuff in here.
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


#include "lib_types.h"
#include "lib_string.h"
#include "lib_malloc.h"
#include "lib_printf.h"

#include "cfe_iocb.h"
#include "cfe_device.h"
#include "cfe_console.h"
#include "cfe_timer.h"

#include "env_subr.h"
#include "ui_command.h"
#include "cfe_mem.h"
#include "cfe.h"

#include "exception.h"

#include "bsp_config.h"
#include "board.h"
#include "boardparms.h"
#include "bcm_map.h"
#include "dev_bcm63xx_flash.h"

#include "segtable.h"

#include "initdata.h"

#if CFG_PCI
#include "pcivar.h"
#endif


/*  *********************************************************************
    *  Constants
    ********************************************************************* */

#ifndef CFG_STACK_SIZE
#define STACK_SIZE	8192
#else
#define STACK_SIZE	((CFG_STACK_SIZE+1023) & ~1023)
#endif

/*  *********************************************************************
    *  Externs
    ********************************************************************* */

void cfe_main(int,int);
void cfe_command_restart(uint64_t status);

extern void cfe_device_poll(void *x);

extern int cfe_web_check(void);
extern void cfe_web_fg_process(void);
extern void cfe_web_poll(void *x);

extern segtable_t *_getsegtbl(void);

extern const char *builddate;
extern const char *builduser;

extern void AlertLed_On(void);//roy
extern void AlertLed_Off(void);//roy
#define GPIODIR 0xFFFE0404  //roy GPIO31~GPIO0
#define GPIODATA 0xFFFE040C //roy GPIO31~GPIO0

/*  *********************************************************************
    *  Globals
    ********************************************************************* */

const char *cfe_boardname = CFG_BOARDNAME;
unsigned int cfe_startflags =
#if CFG_PCI
    CFE_INIT_PCI |
#endif
    0;
int cfe_bus_speed = 0;
int cfe_ref_speed = 0;
unsigned long cfe_sdramsize = 8 * 1024 * 1024;

#if defined (_BCM96348_) || defined (_BCM96358_)
static void calculateCpuSpeed(void);
#endif

/*  *********************************************************************
    *  cfe_setup_default_env()
    *  
    *  Initialize the default environment for CFE.  These are all
    *  the temporary variables that do not get stored in the NVRAM
    *  but are available to other programs and command-line macros.
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

static void cfe_setup_default_env(void)
{
    char buffer[80];

    xsprintf(buffer,"%d.%d.%d",CFE_VER_MAJOR,CFE_VER_MINOR,CFE_VER_BUILD);
    env_setenv("CFE_VERSION",buffer,ENV_FLG_BUILTIN | ENV_FLG_READONLY);

    if (cfe_boardname) {
	env_setenv("CFE_BOARDNAME",(char *) cfe_boardname,
		   ENV_FLG_BUILTIN | ENV_FLG_READONLY);
	}
//  No more memory display to reduce code size
//    xsprintf(buffer,"%d",mem_totalsize);
//    env_setenv("CFE_MEMORYSIZE",buffer,ENV_FLG_BUILTIN | ENV_FLG_READONLY);

}


/*  *********************************************************************
    *  cfe_ledstr(leds)
    *  
    *  Display a string on the board's LED display, if it has one.
    *  This routine depends on the board-support package to
    *  include a "driver" to write to the actual LED, if the board
    *  does not have one this routine will do nothing.
    *  
    *  The LEDs are written at various places in the initialization
    *  sequence, to debug board problems.
    *  
    *  Input parameters: 
    *  	   leds - pointer to four-character ASCII string
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

void cfe_ledstr(const char *leds)
{
#if 0
    unsigned int val;

    val = ((((unsigned int) leds[0]) << 24) |
	   (((unsigned int) leds[1]) << 16) |
	   (((unsigned int) leds[2]) << 8) |
	   ((unsigned int) leds[3]));

    cfe_leds(val);
#endif
}


/*  *********************************************************************
    *  cfe_say_hello()
    *  
    *  Print out the CFE startup message and copyright notice
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

void cfe_command_restart(uint64_t status)
{
}

static void cfe_say_hello(void)
{
    xprintf("\n\n");
    xprintf("CFE version %d.%d.%d-%d.%d"
#ifdef CFE_VER_RELEASE
	    ".%d"
#endif
	    " for %s (%s)\n",
	    CFE_VER_MAJOR,CFE_VER_MINOR,CFE_VER_BUILD, BCM63XX_MAJOR, BCM63XX_MINOR,
#ifdef CFE_VER_RELEASE
	    CFE_VER_RELEASE,
#endif
	    cfe_boardname,
#ifdef __long64
	    "64bit,"
#else
	    "32bit,"
#endif	
#if CFG_MULTI_CPUS
	    "MP,"
#else
	    "SP,"
#endif
#ifdef __MIPSEL
	    "LE"
#endif
#ifdef __MIPSEB
	    "BE"
#endif
#if CFG_VAPI
	    ",VAPI"
#endif
	);

    xprintf("Build Date: %s (%s)\n",builddate,builduser);
    xprintf("Copyright (C) 2000-2005 Broadcom Corporation.\n");
    xprintf("\n");
}


/*  *********************************************************************
    *  cfe_restart()
    *  
    *  Restart CFE from scratch, jumping back to the boot vector.
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   does not return
    ********************************************************************* */

void cfe_restart(void)
{
    _exc_restart();
}


/*  *********************************************************************
    *  cfe_start(ept)
    *  
    *  Start a user program
    *  
    *  Input parameters: 
    *  	   ept - entry point
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */
void cfe_start(unsigned long ept)
{
    cfe_launch(ept);
}

/*  *********************************************************************
    *  cfe_get_sdram_size(void)
    *  
    *  Return amount of SDRAM on the board.
    *  
    *  Input parameters: 
    *  	   None.
    *  	   
    *  Return value:
    *  	   Amount of SDRAM on the board.
    ********************************************************************* */
static unsigned long cfe_get_sdram_size(void)
{
    unsigned long size = 8 * 1024 * 1024;

    switch( kerSysMemoryTypeGet() )
    {
    case BP_MEMORY_16MB_1_CHIP:
    case BP_MEMORY_16MB_2_CHIP:
        size = 16 * 1024 * 1024;
        break;
    case BP_MEMORY_32MB_1_CHIP:
    case BP_MEMORY_32MB_2_CHIP:
        size = 32 * 1024 * 1024;
        break;
    case BP_MEMORY_64MB_2_CHIP:
        size = 64 * 1024 * 1024;
        break;
    /*case BP_MEMORY_8MB_1_CHIP:*/
    default:
        size = 8 * 1024 * 1024;
        break;
    }

    return( size );
}

/*  *********************************************************************
    *  cfe_startup_info()
    *  
    *  Display startup memory configuration messages
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

static void cfe_startup_info(void)
{
    segtable_t *segtbl;

    segtbl = _getsegtbl();
    xprintf("CPU type 0x%X: ",(uint32_t)cpu_prid);
    if (cfe_cpu_speed < 1000000) xprintf("%dKHz\n",cfe_cpu_speed/1000);
    else xprintf("%dMHz",cfe_cpu_speed/1000000);
#if defined (_BCM96348_) || defined (_BCM96358_)
    xprintf(", Bus: %dMHz, Ref: %dMHz\n", cfe_bus_speed/1000000,
        cfe_ref_speed/1000000);
#else
    xprintf("\n");
#endif
    cfe_sdramsize = cfe_get_sdram_size();
    xprintf("Total memory: %lu bytes (%luMB)\n",
	    cfe_sdramsize, cfe_sdramsize >> 20);
    xprintf("\n");
    xprintf("Total memory used by CFE:  0x%08X - 0x%08X (%d)\n",
	    (uint32_t) mem_bottomofmem,
	    (uint32_t) mem_topofmem,
	    (uint32_t) mem_topofmem-mem_bottomofmem);
    xprintf("Initialized Data:          0x%08X - 0x%08X (%d)\n",
	    (uint32_t) (segtbl[R_SEG_FDATA] + mem_datareloc),
	    (uint32_t) (segtbl[R_SEG_EDATA] + mem_datareloc),
	    (uint32_t) (segtbl[R_SEG_EDATA] - segtbl[R_SEG_FDATA]));
    xprintf("BSS Area:                  0x%08X - 0x%08X (%d)\n",
	    (uint32_t) (segtbl[R_SEG_FBSS] + mem_datareloc),
	    (uint32_t) (segtbl[R_SEG_END] + mem_datareloc),
	    (uint32_t) (segtbl[R_SEG_END] - segtbl[R_SEG_FBSS]));
    xprintf("Local Heap:                0x%08X - 0x%08X (%d)\n",
	    (uint32_t)(mem_heapstart),
	    (uint32_t)(mem_heapstart + (CFG_HEAP_SIZE*1024)),
	    (CFG_HEAP_SIZE*1024));
    xprintf("Stack Area:                0x%08X - 0x%08X (%d)\n",
	    (uint32_t)(mem_heapstart + (CFG_HEAP_SIZE*1024)),
	    (uint32_t)(mem_heapstart + (CFG_HEAP_SIZE*1024) + STACK_SIZE),
	    STACK_SIZE);
    xprintf("Text (code) segment:       0x%08X - 0x%08X (%d)\n",
	    (uint32_t)mem_textbase,
	    (uint32_t)(mem_textbase+mem_textsize),
	    (uint32_t) mem_textsize);
    xprintf("Boot area (physical):      0x%08X - 0x%08X\n",
	    mem_bootarea_start,mem_bootarea_start+mem_bootarea_size);
    xprintf("Relocation Factor:         I:%08X - D:%08X\n",
	    (uint32_t) mem_textreloc,(uint32_t) mem_datareloc);

}


/*  *********************************************************************
    *  cfe_main(a,b)
    *  
    *  It's gotta start somewhere.
    *  
    *  Input parameters: 
    *  	   a,b - not used
    *  	   
    *  Return value:
    *  	   does not return
    ********************************************************************* */

void cfe_main(int a,int b)
{
    /*
     * By the time this routine is called, the following things have
     * already been done:
     *
     * 1. The processor(s) is(are) initialized.
     * 2. The caches are initialized.
     * 3. The memory controller is initialized.
     * 4. BSS has been zeroed.
     * 5. The data has been moved to R/W space.
     * 6. The "C" Stack has been initialized.
     */
AlertLed_On();//roy
    cfe_bg_init();		       		/* init background processing */
    cfe_attach_init();
#if defined (_BCM96348_) || defined (_BCM96358_)
    calculateCpuSpeed();
#endif
    cfe_timer_init();				/* Timer process */
    cfe_bg_add(cfe_device_poll,NULL);

    /*
     * Initialize the memory allocator
     */
    KMEMINIT((unsigned char *) (uintptr_t) mem_heapstart,
	     ((CFG_HEAP_SIZE)*1024));

    /*
     * Initialize the console.  It is done before the other devices
     * get turned on.  The console init also sets the variable that
     * contains the CPU speed.
     */
    board_console_init();
    /*
     * Set up the exception vectors
     */

    cfe_setup_exceptions();
    /* 
     * Say hello.
     */
    cfe_say_hello();

    xprintf("Boot Address 0x%x\n\n", FLASH_BASE);

    /*
     * Initialize the other devices.
     */
    xprintf("Initializing Arena.\n");
    cfe_arena_init();

    xprintf("Initializing Devices.\n");
	
    board_device_init();
	
    cfe_startup_info();
	
    cfe_setup_default_env();
	
    xprintf("\n");

    ui_init_cmddisp();
	
    board_final_init();
	
    cfe_command_loop();
	
}


void AlertLed_On(void)
{ 
    unsigned int GPIOVal1Mask1= 0xffffffdf; //GPIODIR use, bit31(GPIO31)~bit0(GPIO0), control GPIO5
    unsigned int GPIOVal2Mask1= 0xffffffdf; //GPIODATA use, bit31(GPIO31)~bit0(GPIO0), control GPIO5
    unsigned int GPIOVal2Mask2= 0x00000000; //GPIO5=0=>On, low activate
	
		//Set GPIODIR====================================================================
		*((volatile unsigned int *)GPIODIR)=((*((volatile unsigned int *)GPIODIR)& GPIOVal1Mask1) | ~(GPIOVal1Mask1));
		//Alert LED ON (Set GPIODATA)====================================================
	  *((volatile unsigned int *)GPIODATA) = ((*((volatile unsigned int *)GPIODATA)& GPIOVal2Mask1) | (GPIOVal2Mask2));  	
} 

void AlertLed_Off(void)
{ 
    unsigned int GPIOVal1Mask1= 0xffffffdf; //GPIODIR use, bit31(GPIO31)~bit0(GPIO0), control GPIO5
    unsigned int GPIOVal2Mask1= 0xffffffdf; //GPIODATA use, bit31(GPIO31)~bit0(GPIO0), control GPIO5
    unsigned int GPIOVal2Mask3= 0x00000020; //GPIO5=1=>Off, low activate
				
		//Set GPIODIR====================================================================
		*((volatile unsigned int *)GPIODIR)=((*((volatile unsigned int *)GPIODIR)& GPIOVal1Mask1) | ~(GPIOVal1Mask1));
		//Alert LED OFF (Set GPIODATA)====================================================
    *((volatile unsigned int *)GPIODATA) = ((*((volatile unsigned int *)GPIODATA)& GPIOVal2Mask1) | (GPIOVal2Mask3));  	
} 






/*  *********************************************************************
    *  cfe_command_loop()
    *  
    *  This routine reads and processes user commands
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   does not return
    ********************************************************************* */

void cfe_command_loop()
{
    char buffer[300];
    int status;
    char *prompt;

    /* Start Web interface. */
    cfe_bg_add(cfe_web_poll,NULL);

    for (;;) {
        prompt = env_getenv("PROMPT");
        if (!prompt) prompt = "CFE> ";
        console_readline(prompt,buffer,sizeof(buffer));

        if (cfe_web_check())
            cfe_web_fg_process();
        else {
            status = ui_docommands(buffer);
            if (status != CMD_ERR_BLANK) {
                xprintf("*** command status = %d\n", status);
            }
        }
    }
}



/*  *********************************************************************
    *  cfe_errortext(err)
    *  
    *  Returns an error message with a number in it.  The number can be
    *  looked up in cfe_error.h. 
    *  
    *  Input parameters: 
    *  	   err - error code
    *  	   
    *  Return value:
    *  	   string description of error
    ********************************************************************* */

const char *cfe_errortext(int err)
{
    static char err_buf[20];

    sprintf(err_buf, "CFE error %d", err);
    return (const char *) err_buf;
}
#if defined (_BCM96348_)
/*  *********************************************************************
    *  calculateCpuSpeed()
    *      Calculate the BCM6348 CPU speed by reading the PLL strap register
    *      and applying the following formula:
    *      Fref_clk = (0.25 * FREF) * N / Mr,
    *          where FREF = 64MHz and N = (N1 + 1) * (N2 + 2) and
    *                       Mr = (M1r + 1) * (M2r + 4)
    *      Fbus_clk = (0.25 * FREF) * N / Mb,
    *          where FREF = 64MHz and N = (N1 + 1) * (N2 + 2) and
    *                       Mb = (M1b + 1) * (M2b + 4)
    *      Fcpu_clk = (0.25 * FREF) * N / Mc,
    *          where FREF = 64MHz and N = (N1 + 1) * (N2 + 2) and Mc = (M1c + 1)
    *  Input parameters:
    *      none
    *  Return value:
    *      none
    ********************************************************************* */
void static calculateCpuSpeed(void)
{
    extern int cfe_cpu_speed, cfe_bus_speed, cfe_ref_speed;
    const int freq = 64 / 4;
    UINT32 pllStrap = PERF->PllStrap;
    int numerator = freq *
               (((pllStrap & PLL_N1_MASK) >> PLL_N1_SHFT) + 1) *
               (((pllStrap & PLL_N2_MASK) >> PLL_N2_SHFT) + 2);
    int mcpu = (((pllStrap & PLL_M1_CPU_MASK) >> PLL_M1_CPU_SHFT) + 1);
    int mbus = (((pllStrap & PLL_M1_BUS_MASK) >> PLL_M1_BUS_SHFT) + 1) *
               (((pllStrap & PLL_M2_BUS_MASK) >> PLL_M2_BUS_SHFT) + 4);
    int mref = (((pllStrap & PLL_M1_REF_MASK) >> PLL_M1_REF_SHFT) + 1) *
               (((pllStrap & PLL_M2_REF_MASK) >> PLL_M2_REF_SHFT) + 4);

    cfe_cpu_speed = (numerator / mcpu) * 1000000;
    cfe_bus_speed = (numerator / mbus) * 1000000;
    cfe_ref_speed = (numerator / mref) * 1000000;
}
#endif

#if defined (_BCM96358_)
/*  *********************************************************************
    *  calculateCpuSpeed()
    *      Calculate the BCM6358 CPU speed by reading the PLL Config register
    *      and applying the following formula:
    *      Fref_clk = (25 * MIPSDDR_NDIV) / REF_MDIV
    *      Fbus_clk = (25 * MIPSDDR_NDIV) / DDR_MDIV
    *      Fcpu_clk = (25 * MIPSDDR_NDIV) / MIPS_MDIV
    *  Input parameters:
    *      none
    *  Return value:
    *      none
    ********************************************************************* */
void static calculateCpuSpeed(void)
{
    extern int cfe_cpu_speed, cfe_bus_speed, cfe_ref_speed;
    int numerator;
    UINT32 pllConfig = DDR->MIPSDDRPLLConfig;

    cfe_ref_speed = 64000000;
    cfe_cpu_speed = cfe_ref_speed / ((pllConfig & MIPS_MDIV_MASK) >> MIPS_MDIV_SHFT);
    cfe_bus_speed = cfe_ref_speed / ((pllConfig & DDR_MDIV_MASK) >> DDR_MDIV_SHFT);
    numerator = (((pllConfig & MIPSDDR_N2_MASK) >> MIPSDDR_N2_SHFT) * ((pllConfig & MIPSDDR_N1_MASK) >> MIPSDDR_N1_SHFT));
    cfe_cpu_speed = (cfe_cpu_speed * numerator) / 4;
    cfe_bus_speed = (cfe_bus_speed * numerator) / 4;
}
#endif
