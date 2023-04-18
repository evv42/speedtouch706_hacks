This is a collection of software/information for hacking the Thomson Speedtouch 706 WL router.
Some stuff may apply to similar routers.

# About

The Speedtouch 706 WL (a.k.a st706wl, BANT-S) is a ADSL router released circa 2006.
It is a cost-reduced version of the Speedtouch 780 WL (PCB scan in this repo).

# Specs
- Broadcom BCM6348 (MIPS32) at 256 MHz, overclock to 300 MHz possible
- 16 MiB of SDRAM (K4S281632F)
- 4 MiB of NOR Flash (S29GL032)
- Custom Bootloader and Non-Linux Firmware

# Original firmware TCRF

Found the following messages in the bootloader section:

The dev names ? The FCC ID (RSE-ST780) indicates that this router was registered by Thomson Belgium:
```
Hello world Geert en Rudi en Jan en Peter
```

The bootloader name:
```
KH1.1.3
```

Error messages:
```
FAILURE :o( -> resetting
incorrect header check
invalid window size
unknown compression method
need dictionary
incorrect data check
```
# Running Linux

DISCLAIMER: Old OpenWRT is insecure, but the original firmware is probably worse (telnet can't be disabled, old https)

## What does not work
- Second Ethernet port (unusual BCM5241 PHY configuration)
- USB headers
- ADSL
- Phone stuff
- CFE recovery mode

## Install

OpenWRT does not support this router, but you can use the images builds for the Telsey CPVA502+ / CPVA502+W.  
You need a working JTAG setup e.g. a Raspberry Pi with openocd. (See openocd-config folder)

- Solder pins or wires to the 14-pin JTAG header on the PCB (J201, see jtag-header.png).
- Build a custom CFE with the provided cfe_bcm63xx folder by editing the MAC address in build.sh and then running it.
- Start openocd
```
openocd -f raspberrypi2-native.cfg -f bcm6348.cfg -c "init;halt"
```
- Backup the installed firmware ! (It will take ~ 1 hour)
```
reset init
dump_image firm 0xBFC00000 0x400000
```
- Check and erase flash
```
flash probe 0
flash info 0
flash erase_address 0xBFC00000 0x400000
```
- Flash cfe6348-nvr.bin from the CFE build (~ 5 minutes)
```
flash write_image cfe6348-nvr.bin 0xBFC00000
```
- Flash included 14.07 openwrt.bin or any CPVA502+ OpenWRT image at 0xBFC*1*0000. (Takes ~ 4 hours)
```
flash write_image openwrt.bin 0xBFC10000
```
- Reset the board.

You should have a working OpenWRT installation.
With the serial console, disable telnet.

# Hardware Hacks

## Overclock

Remove R221 and R225 (both 472), put one of those at R223.

## Serial Console

3.3V TTL, marked J303 on PCB:  
1-VCC  
2-GND  
3-TX  
4-RX  
