/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  bcm63xx_board.c   utility functions for bcm63xx board
    *  
    *  Created on :  09/25/2002  seanl
    *
    *********************************************************************

<:copyright-broadcom 
 
 Copyright (c) 2002 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
    
#include "bcm63xx_util.h"

extern NVRAM_DATA nvramData;   
extern int readNvramData(void);

#define MAX_BOARD_ID_NAMES          16
static char g_boardIdNames[BP_BOARD_ID_LEN * MAX_BOARD_ID_NAMES];
static int g_numBoardIdNames = 0;


static PARAMETER_SETTING gBoardParam[] =
{
    // prompt name                      Error Prompt Define Param  Validation function
    {"Board Id Name (0-    \n\
                 -------  0\n\
                 -------  1\n\
                 -------  2\n\
                 -------  3\n\
                 -------  4\n\
                 -------  5\n\
                 -------  6\n\
                 -------  7\n\
                 -------  8\n\
                 -------  9\n\
                 ------- 10\n\
                 ------- 11\n\
                 ------- 12\n\
                 ------- 13\n\
                 ------- 14\n\
                 ------- 15       :", BOARDID_STR_PROMPT, "", "", 2,
        parseBoardIdStr, TRUE},
    {"Number of MAC Addresses (1-32)    :", MAC_CT_PROMPT, "", "", 2,
        parseMacAddrCount, TRUE},
    {"Base MAC Address                  :", MAC_ADDR_PROMPT, "", "", 17,
        parseMacAddr, TRUE},
    {NULL}
};


// getBoardParam:  convert the board param data and put them in the gBoardParam struct
//
void getBoardParam(void)
{
    const int Interval = 28;
    int i;
    char *dest;
    char *src;
    char tmp[10];
    
    if( g_numBoardIdNames == 0 )
        g_numBoardIdNames = BpGetBoardIds(g_boardIdNames, MAX_BOARD_ID_NAMES);

    memset(gBoardParam[0].parameter, 0, NVRAM_BOARD_ID_STRING_LEN);

    for( i = 0, dest = gBoardParam[0].promptName + strlen("Board Id Name (0-    \n"),
        src = g_boardIdNames;
        i < g_numBoardIdNames;
        i++, dest += Interval, src += BP_BOARD_ID_LEN )
    {
        memcpy( dest, src, strlen(src) );
        if( !strcmp( nvramData.szBoardId, src ) )
            sprintf(gBoardParam[0].parameter, "%d", i);
    }
    dest--;
    strcpy( dest, "       :" );
    sprintf(tmp, "%d)", g_numBoardIdNames - 1);
    memcpy(&gBoardParam[0].promptName[strlen("Board Id Name (0-")], tmp, strlen(tmp));
    
    if (nvramData.ulNumMacAddrs == 0xffffffff)
        nvramData.ulNumMacAddrs = NVRAM_MAC_COUNT_DEFAULT;
    sprintf(gBoardParam[1].parameter, "%d", nvramData.ulNumMacAddrs);

    if (nvramData.ucaBaseMacAddr[0] == 0xff && nvramData.ucaBaseMacAddr[1] == 0xff &&
        nvramData.ucaBaseMacAddr[2] == 0xff && nvramData.ucaBaseMacAddr[3] == 0xff)
        memset(gBoardParam[2].parameter, 0, NVRAM_MAC_ADDRESS_LEN);
    else
        macNumToStr(nvramData.ucaBaseMacAddr, gBoardParam[2].parameter);  
}


// setBoardParam: Set the board Id string, mac addresses, psi size, etc...
//
int setBoardParam(void)
{
    int count = 0;
    PPARAMETER_SETTING tmpPtr = gBoardParam;    

    while (tmpPtr->promptName !=  NULL)
    {
        count++;
        tmpPtr++;
    }

    tmpPtr = gBoardParam;
    getBoardParam();

#ifdef USE_SINGLE_BOARD_PARAM
    tmpPtr++;      // if use single board parameter, skip the board id display and selection
    count--;
#endif

    if( processPrompt(tmpPtr, count) )
    {
        /* At least one field was changed. */
        unsigned long memType, thread;
        int i = atoi(gBoardParam[0].parameter);
        strcpy( nvramData.szBoardId, &g_boardIdNames[i * BP_BOARD_ID_LEN] );
        nvramData.ulVersion = NVRAM_VERSION_NUMBER;
        nvramData.ulNumMacAddrs = atoi(gBoardParam[1].parameter);
        parsehwaddr(gBoardParam[2].parameter, nvramData.ucaBaseMacAddr);

        BpSetBoardId( nvramData.szBoardId );
        BpGetSdramSize( &memType );
        kerSysMemoryTypeSet(memType);
        BpGetCMTThread( &thread );
        kerSysThreadNumSet(thread);

        // save the buf to nvram
        writeNvramData();

        printf("Press any key to reset the board: \n");
        while (1)
            if (console_status())
                kerSysMipsSoftReset();
    }

    return 0;
}  // setBoardParam


void displayBoardParam(void)
{
    char str[] = "Board Id Name                     :";
    unsigned long value;
    ETHERNET_MAC_INFO EnetInfo;

    getBoardParam();

    printf("%s %s  \n", str, nvramData.szBoardId);

    if( BpGetPsiSize( &value ) == BP_SUCCESS )
        printf("Psi size in KB                    : %d\n", value);

    printf("%s %s  \n", gBoardParam[1].promptName, gBoardParam[1].parameter);
    printf("%s %s  \n", gBoardParam[2].promptName, gBoardParam[2].parameter);

    if( BpGetEthernetMacInfo( &EnetInfo, 1 ) == BP_SUCCESS )
    {
        printf("Ethernet PHY Type                 : ");
        switch (EnetInfo.ucPhyType)
        {
        case BP_ENET_EXTERNAL_SWITCH:
            if (EnetInfo.usReverseMii == BP_ENET_NO_REVERSE_MII)
                printf("External Switch Not Using Reverse MII\n");
            else
                printf("External Switch Using Reverse MII\n");
            break;
        case BP_ENET_EXTERNAL_PHY:
            printf("External\n");
            break;
        case BP_ENET_INTERNAL_PHY:
        default:
            printf("Internal\n");
            break;
        }
    }

    if( BpGetSdramSize( &value ) == BP_SUCCESS )
    {
        printf("Memory size in MB                 : ");
        switch (value)
        {
        case BP_MEMORY_8MB_1_CHIP:
            printf("8\n");
            break;
        case BP_MEMORY_16MB_1_CHIP:
        case BP_MEMORY_16MB_2_CHIP:
            printf("16\n");
            break;
        case BP_MEMORY_32MB_1_CHIP:
        case BP_MEMORY_32MB_2_CHIP:
            printf("32\n");
            break;
        case BP_MEMORY_64MB_2_CHIP:
            printf("64\n");
            break;
        default:
            printf("*** Not defined ***\n");
            break;
        }
    }

    if( BpGetCMTThread( &value ) == BP_SUCCESS )
    {
        printf("CMT Thread Number                 : %d\n", value);
    }

    printf("\n");
}


void setDefaultBootline(void)
{
    memset(nvramData.szBootline, 0, NVRAM_BOOTLINE_LEN);
    strncpy(nvramData.szBootline, DEFAULT_BOOTLINE, strlen(DEFAULT_BOOTLINE));
    printf("Use default boot line parameters: %s\n", DEFAULT_BOOTLINE);
    writeNvramData();
    readNvramData();
    convertBootInfo();
}


void setBoardIdName(void)
{
    if( readNvramData() == 0 )
        BpSetBoardId( nvramData.szBoardId );
} 

int getNumberBoardIdNames(void)
{
    return( g_numBoardIdNames );
} 

