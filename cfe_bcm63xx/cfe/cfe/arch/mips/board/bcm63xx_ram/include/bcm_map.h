/*
<:copyright-broadcom 
 
 Copyright (c) 2004 Broadcom Corporation 
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
#ifndef __BCM_MAP_H
#define __BCM_MAP_H

#if defined (_BCM96338_)
#include "6338_map.h"
#endif
#if defined (_BCM96348_)
#include "6348_map.h"
#endif
#if defined (_BCM96358_)
#include "6358_map.h"
#endif

extern void AlertLed_On(void);//roy
extern void AlertLed_Off(void);//roy

#endif
