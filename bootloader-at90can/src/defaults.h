/* Copyright (c) 2010, Fabian Greif
 * All rights reserved.
 *
 * The file is part of the CAN bootloader project and is released under the
 * 3-clause BSD license. See the file `LICENSE` for the full license governing
 * this code.
 */
// ----------------------------------------------------------------------------

#ifndef	DEFAULTS_H
#define	DEFAULTS_H

#include "config.h"

// ----------------------------------------------------------------------------
// create pagesize identifier
#if	SPM_PAGESIZE == 32
	#define	PAGESIZE_IDENTIFIER		(0)
#elif SPM_PAGESIZE == 64
	#define	PAGESIZE_IDENTIFIER		(1)
#elif SPM_PAGESIZE == 128
	#define	PAGESIZE_IDENTIFIER		(2)
#elif SPM_PAGESIZE == 256
	#define	PAGESIZE_IDENTIFIER		(3)
#else
	#error	Strange value for SPM_PAGESIZE. Check the define!
#endif

// -----------------------------------------------------------------------------
// check defines for the bootloader led
#ifdef BOOT_LED

#define	RESET(x)			RESET2(x)
#define	SET(x)				SET2(x)
#define	TOGGLE(x)			TOGGLE2(x)
#define	SET_OUTPUT(x)		SET_OUTPUT2(x)

#define	RESET2(x,y)			PORT(x) &= ~(1<<y)
#define	SET2(x,y)			PORT(x) |= (1<<y)
#define	TOGGLE2(x,y)		PORT(x) ^= (1<<y)
#define	SET_OUTPUT2(x,y)	DDR(x) |= (1<<y)

#define	PORT(x)				_port2(x)
#define	DDR(x)				_ddr2(x)

#define	_port2(x)			PORT ## x
#define	_ddr2(x)			DDR ## x

	#ifndef	BOOT_LED_SET_OUTPUT	
		#define	BOOT_LED_SET_OUTPUT		SET_OUTPUT(BOOT_LED)
	#endif
	
	#ifndef	BOOT_LED_ON
		#define	BOOT_LED_ON				SET(BOOT_LED)
	#endif
	
	#ifndef	BOOT_LED_OFF
		#define	BOOT_LED_OFF			RESET(BOOT_LED)
	#endif
	
	#ifndef	BOOT_LED_TOGGLE
		#define	BOOT_LED_TOGGLE			TOGGLE(BOOT_LED)
	#endif
#else
	#define	BOOT_LED_SET_OUTPUT			
	#define	BOOT_LED_ON
	#define	BOOT_LED_OFF
	#define	BOOT_LED_TOGGLE
	
	#warning	compiling bootloader without LED support
#endif

// -----------------------------------------------------------------------------
#ifndef	BOOT_INIT
	#define	BOOT_INIT
#endif

#ifndef	BOOTLOADER_TYPE
	#define	BOOTLOADER_TYPE		0
#endif

// -----------------------------------------------------------------------------
// set current version of the bootloader
#define	BOOTLOADER_VERSION		3

// -----------------------------------------------------------------------------
// CAN settings
#ifndef	CAN_BITRATE
	#define	CAN_BITRATE		125
#endif

// ----------------------------------------------------------------------------
// Set a few AVR specific defines

#if defined(__AVR_AT90CAN32__)
	
	#define	RWW_PAGES	96
//	#define	RAMSTART	0x0100
	#define	SIG_FAMILY	0x95
	#define	SIG_DEVICE	0x81
	
#elif defined(__AVR_AT90CAN64__)
	
	#define	RWW_PAGES	224
//	#define	RAMSTART	0x0100
	#define	SIG_FAMILY	0x96
	#define	SIG_DEVICE	0x81
	
#elif defined(__AVR_AT90CAN128__)
	
	#define	RWW_PAGES	480
//	#define	RAMSTART	0x0100
	#define	SIG_FAMILY	0x97
	#define	SIG_DEVICE	0x81
	
#else
	#error	chosen AVR command is not supported yet!
#endif

// ----------------------------------------------------------------------------
// Timereinstellung fuer aktuelle Taktfrequenz auswaehlen (500ms)
//
// TIMER_PRELOAD = 65536 - (0.5s * F_CPU) / 1024

#if F_CPU == 16000000UL
	// Prescaler = 1024
	#define	TIMER_PRESCALER		((1 << CS12) | (1 << CS10))
	#define	TIMER_PRELOAD		57724
#else
	#error	choosen F_CPU not supported yet!
#endif


#define CAN_IDENTIFIER_SEND			0x7FE
#define CAN_IDENTIFIER_RECEIVE		0x7FF

#endif	// DEFAULTS_H
