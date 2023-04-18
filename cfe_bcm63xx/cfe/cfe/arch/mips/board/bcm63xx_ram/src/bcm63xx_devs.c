/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *
    *  Board device initialization      File: bcm94710_devs.c
    *
    *  This is the "C" part of the board support package.  The
    *  routines to create and initialize the console, wire up
    *  device drivers, and do other customization live here.
    *
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *
    *********************************************************************
    *
    *  XX Copyright 2000,2001
    *  Broadcom Corporation. All rights reserved.
    *
    *  BROADCOM PROPRIETARY AND CONFIDENTIAL
    *
    *  This software is furnished under license and may be used and
    *  copied only in accordance with the license.
    ********************************************************************* */

#include "lib_types.h"
#include "lib_printf.h"
#include "cfe_timer.h"
#include "cfe.h"
#include "bcm_map.h"
#include "board.h"
#include "bcmTag.h"
#include "dev_bcm63xx_flash.h"
#include "bcm63xx_util.h"
#include "flash_api.h"

int checkForResetHold( unsigned short pahrGpio );
void kerSysMipsSoftReset(void);

/*  *********************************************************************
    *  Devices we're importing
    ********************************************************************* */

extern cfe_driver_t bcm63xx_uart;
extern cfe_driver_t bcm6348_enet;

/*  *********************************************************************
    *  Some board-specific parameters
    ********************************************************************* */


/*  *********************************************************************
    *  board_console_init()
    *
    *  Add the console device and set it to be the primary
    *  console.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void board_console_init(void)
{
    /* Add the serial port driver. */
    cfe_add_device(&bcm63xx_uart,0,0,0);

    cfe_set_console( "uart0" );
}


/*  *********************************************************************
    *  board_device_init()
    *
    *  Initialize and add other devices.  Add everything you need
    *  for bootstrap here, like disk drives, flash memory, UARTs,
    *  network controllers, etc.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void board_device_init(void)
{
    unsigned short gpio;

    kerSysFlashInit();
    /* Add the ethernet driver. */
    cfe_add_device( &bcm6348_enet, 0, 0, 0);
    setPowerOnLedOn();
    /* Set blink rate for hardware LEDs. */
    GPIO->LEDCtrl &= ~LED_INTERVAL_SET_MASK;
    GPIO->LEDCtrl |= LED_INTERVAL_SET_80MS;

#if defined(_BCM96358_)
    /* Enable LED Outputs */
    GPIO->GPIOMode |= GPIO_MODE_LED_OVERLAY;
    GPIO->GPIODir |= 0x000f;
    /* Enable Serial LED Outputs */
    GPIO->GPIOMode |= GPIO_MODE_SERIAL_LED_OVERLAY;
    GPIO->GPIODir |= 0x00c0;
    GPIO->SerialLed = 0xffffffff;
#endif

    if( runFromFlash() && getBootImageTag() == NULL )
    {
        printf("\n** Image information not found. **\n\n");
        kerSysErasePsi(1);
        if( BpGetBootloaderResetCfgLedGpio( &gpio ) == BP_SUCCESS )
            setLedOn( gpio );
        setBreakIntoCfeLed();
    }
}


/*  *********************************************************************
    *  board_final_init()
    *
    *  Do any final initialization, such as adding commands to the
    *  user interface.
    *
    *  If you don't want a user interface, put the startup code here.
    *  This routine is called just before CFE starts its user interface.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void board_final_init(void)
{
    unsigned short gpio;
    int breakIntoCfe = 0;

    if( BpGetBootloaderResetCfgLedGpio( &gpio ) == BP_SUCCESS )
        setLedOff( gpio );

    if( BpGetBootloaderStopLedGpio( &gpio ) == BP_SUCCESS )
        setLedOff( gpio );

    if( BpGetPressAndHoldResetGpio( &gpio ) == BP_SUCCESS )
        breakIntoCfe = checkForResetHold( gpio );

    if( breakIntoCfe )
    {
        /* The board may or may not reset now.  Wait a second.  If the board
         * does reset, code execution will still come back to this function.
         */
        cfe_sleep(CFE_HZ);
    }

    if( getBootToCfe() == 1 )
    {
        kerSysErasePsi(0);
        breakIntoCfe = 1;
    }

    bcm63xx_run(breakIntoCfe);
}

/*  *********************************************************************
    * Miscellaneous Board Functions
    ********************************************************************* */

/*  *********************************************************************
    *  checkForResetHold()
    *
    *  Determines if the user is holding the reset button in a depressed
    *  state.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      1 - break into the CFE, 0 - continue boot sequence
    ********************************************************************* */

int checkForResetHold( unsigned short pahrGpio )
{
    const int nInitPsiDelay = 2;
    const int nBreakIntoCfeDelay = 5; //roy
    unsigned short gpio = 0;
    int ret = 0;
    int reinit = 0;
    int i;

    unsigned long gpio_mask = GPIO_NUM_TO_MASK(pahrGpio);
    volatile unsigned long *gpio_reg = &GPIO->GPIOio;

#if !defined(_BCM96338_)
    if( (pahrGpio & BP_GPIO_NUM_MASK) >= 32 )
    {
        gpio_mask = GPIO_NUM_TO_MASK_HIGH(pahrGpio);
        gpio_reg = &GPIO->GPIOio_high;
    }
#endif

    /* Loop until the reset button is not depressed. */
    for(i = 0; !(*gpio_reg & gpio_mask); i++)
    {
        if (i == nInitPsiDelay)
        {
            reinit = 1;
            if( BpGetBootloaderResetCfgLedGpio( &gpio ) == BP_SUCCESS )
                setLedOn( gpio );
            printf("\n*** Restore to Factory Default Setting ***\n\n");
        }

        if (i == nBreakIntoCfeDelay)
        {
            ret = 1;
            setBreakIntoCfeLed();
            printf("\n*** Break into CFE console ***\n\n");
        }

        //cfe_sleep(CFE_HZ);
        AlertLed_On();//roy
        cfe_sleep(CFE_HZ/2);
        AlertLed_Off();//roy
        cfe_sleep(CFE_HZ/2);
        
        
    }

    if( reinit )
    {
        /* Sometimes the board will reset again when the depressed reset
         * button is released.  Therefore, write a flag to the first word of
         * the PSI flash sector so when the CFE comes back up, it will break
         * into the console.
         */
        kerSysErasePsi(ret);

        /* Reset the default bootline if the board IP address has changed. */
        if( bootLineChanged() )
        {
            cfe_sleep(CFE_HZ);
            setDefaultBootline();
        }
    }

    return( ret );
}

/*  *********************************************************************
    *  setLedOn()
    *
    *  Turns on an LED.
    *
    *  Input parameters:
    *      LED purpose
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void setLedOn( unsigned short led )
{
    int active_low = (led & BP_ACTIVE_LOW) ? 1 : 0;

    unsigned long gpio_mask = GPIO_NUM_TO_MASK(led);
    volatile unsigned long *gpio_dir_reg = &GPIO->GPIODir;
    volatile unsigned long *gpio_io_reg = &GPIO->GPIOio;

#if !defined(_BCM96338_)
    if( (led & BP_GPIO_NUM_MASK) >= 32 )
    {
        gpio_mask = GPIO_NUM_TO_MASK_HIGH(led);
        gpio_dir_reg = &GPIO->GPIODir_high;
        gpio_io_reg = &GPIO->GPIOio_high;
    }
#endif

    if (led & BP_GPIO_SERIAL) {
#if defined(_BCM96358_)
        gpio_io_reg = &GPIO->SerialLed;
        while (GPIO->SerialLedCtrl & SER_LED_BUSY);
#endif
    }
    else {
        *gpio_dir_reg |= gpio_mask;
    }

    if( active_low )
        *gpio_io_reg  &= ~gpio_mask;
    else
        *gpio_io_reg  |= gpio_mask;
}

/*  *********************************************************************
    *  setLedOff()
    *
    *  Turns off an LED.
    *
    *  Input parameters:
    *      LED purpose
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void setLedOff( unsigned short led )
{
    int active_low = (led & BP_ACTIVE_LOW) ? 1 : 0;

    unsigned long gpio_mask = GPIO_NUM_TO_MASK(led);
    volatile unsigned long *gpio_dir_reg = &GPIO->GPIODir;
    volatile unsigned long *gpio_io_reg = &GPIO->GPIOio;

#if !defined(_BCM96338_)
    if( (led & BP_GPIO_NUM_MASK) >= 32 )
    {
        gpio_mask = GPIO_NUM_TO_MASK_HIGH(led);
        gpio_dir_reg = &GPIO->GPIODir_high;
        gpio_io_reg = &GPIO->GPIOio_high;
    }
#endif

    if (led & BP_GPIO_SERIAL) {
#if defined(_BCM96358_)
        gpio_io_reg = &GPIO->SerialLed;
        while (GPIO->SerialLedCtrl & SER_LED_BUSY);
#endif
    }
    else {
        *gpio_dir_reg |= gpio_mask;
    }

    if( active_low )
        *gpio_io_reg  |= gpio_mask;
    else
        *gpio_io_reg  &= ~gpio_mask;
}


/*  *********************************************************************
    *  setPowerOnLedOn()
    *
    *  Turns on the Power LED.
    *
    *  Input parameters:
    *      LED purpose
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void setPowerOnLedOn(void)
{
    unsigned short gpio;
    if( BpGetBootloaderStopLedGpio( &gpio ) == BP_SUCCESS )
        setLedOff( gpio );
    if( BpGetBootloaderPowerOnLedGpio( &gpio ) == BP_SUCCESS )
        setLedOn( gpio );
}


/*  *********************************************************************
    *  setBreakIntoCfeLed()
    *
    *  Turns on the alarm LED.
    *
    *  Input parameters:
    *      LED purpose
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void setBreakIntoCfeLed(void)
{
    unsigned short gpio;
    if( BpGetBootloaderStopLedGpio( &gpio ) == BP_SUCCESS ) {
        setLedOn( gpio );
        if( BpGetBootloaderPowerOnLedGpio( &gpio ) == BP_SUCCESS )
            setLedOff( gpio );
    }
AlertLed_On();//roy
}

/*  *********************************************************************
    *  kerSysMipsSoftReset()
    *
    *  Resets the board.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

void kerSysMipsSoftReset(void)
{
#if defined (_BCM96348_)
    if (PERF->RevID == 0x634800A1) {
        typedef void (*FNPTR) (void);
        FNPTR bootaddr = (FNPTR) FLASH_BASE;
        int i;

        /* Reset all blocks. */
        PERF->BlockSoftReset &= ~BSR_ALL_BLOCKS;
        for( i = 0; i < 1000000; i++ )
            ;
        PERF->BlockSoftReset |= BSR_ALL_BLOCKS;
        /* Jump to the power on address. */
        (*bootaddr) ();
    }
    else
        PERF->pll_control |= SOFT_RESET;    // soft reset mips
#else
    PERF->pll_control |= SOFT_RESET;    // soft reset mips
#endif
}
