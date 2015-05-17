/* Copyright (c) 2010, Fabian Greif
 * All rights reserved.
 *
 * The file is part of the CAN bootloader project and is released under the
 * 3-clause BSD license. See the file `LICENSE` for the full license governing
 * this code.
 */
// ----------------------------------------------------------------------------
/**
 * \file
 * \brief	CAN Bootloader
 * 
 * 
 * \author	Fabian Greif <fabian.greif@rwth-aachen.de>
 * \author	Adrian Weiler
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include "at90can.h"
#include "defaults.h"

// Page number of the flash currently being written
static uint16_t flashpage = 0;

// Current read/write position within the flash page
static uint8_t  flashpage_buffer_pos = 0;

// Buffer for the flash page content
static uint8_t  flashpage_buffer[SPM_PAGESIZE];

// -----------------------------------------------------------------------------
// Watchdog Timer als erstes im Programm deaktivieren
// see http://www.nongnu.org/avr-libc/user-manual/group__avr__watchdog.html
void
disable_watchdog(void) \
		__attribute__((section(".init3"), naked, used));

void
disable_watchdog(void)
{
	// Save MCUSR so that the main program can access if later
	GPIOR0 = MCUSR;
	MCUSR = 0;
	wdt_disable();
}

// ----------------------------------------------------------------------------
/**
 * Starts the application program.
 */
static void
boot_jump_to_application(void)
{
	// relocate interrupt vectors
	uint8_t reg = MCUCR & ~((1 << IVCE) | (1 << IVSEL));
	
	MCUCR = reg | (1 << IVCE);
	MCUCR = reg;
	
#if FLASHEND > 0xffff
	__asm__ __volatile__(
			"push __zero_reg__" "\n\t"
			"push __zero_reg__" "\n\t"
			"push __zero_reg__" "\n\t");
#else
	__asm__ __volatile__(
			"push __zero_reg__" "\n\t"
			"push __zero_reg__" "\n\t");
#endif
	
	// when the functions executes the 'ret' command to return to
	// its origin the AVR loads the return address from the stack. Because we
	// pushed zero it instead jumps to address zero which starts the main
	// application.
}

// ----------------------------------------------------------------------------
/**
 * Write a complete page to the flash memory
 * 
 * \param	page	page which should be written
 * \param	*buf	Pointer to the buffer with the data
 *
 * \see		avr-libc Documentation > Modules > Bootloader Support Utilities
 */
static void
boot_program_page(uint16_t page, uint8_t *buf)
{
	uint32_t address = page * SPM_PAGESIZE;
	
	boot_page_erase(address);
	boot_spm_busy_wait();		// Wait until the memory is erased.
	
	for (uint16_t i = 0; i < SPM_PAGESIZE; i += 2)
	{
		// Set up little-endian word.
		uint16_t w = *buf++;
		w |= (*buf++) << 8;
		
		boot_page_fill(address + i, w);
	}
	
	boot_page_write(address);	// Store buffer in flash page.
	boot_spm_busy_wait();		// Wait until the memory is written.
	
	// Reenable RWW-section again. We need this if we want to jump back
	// to the application after loading the application.
	boot_rww_enable();
}

// ----------------------------------------------------------------------------
int
main(void) __attribute__((OS_main));

int
main(void)
{
	enum
	{
		IDLE,
		COLLECT_DATA,
		RECEIVED_PAGE
	} state = IDLE;
	uint8_t next_message_number = -1;
	
	// Do some addition initialization (if required)
	BOOT_INIT;
	
	BOOT_LED_SET_OUTPUT;
	BOOT_LED_ON;
	
	// Relocate interrupt vectors to boot area
	MCUCR = (1 << IVCE);
	MCUCR = (1 << IVSEL);
	
	at90can_init();
	
	// Start timer
	TCNT1 = TIMER_PRELOAD;
	TCCR1A = 0;
	TCCR1B = TIMER_PRESCALER;

	// Clear overflow-flag
	TIFR1 = (1 << TOV1);
	
	sei();

	while (1)
	{
		uint8_t command;
		static uint8_t next_message_data_counter;
		
		// wait until we receive a new message
		while ((command = at90can_get_message()) == NO_MESSAGE)
		{
			if (TIFR1 & (1 << TOV1))
			{
				BOOT_LED_OFF;
				
				// timeout => start application
				boot_jump_to_application();
			}
		}
		
		// stop timer
		TCCR1B = 0;
		
		// check if the message is a request, otherwise reject it
		if ((command & ~COMMAND_MASK) != REQUEST)
		{
			continue;
		}
		command &= COMMAND_MASK;
		
		// check message number
		next_message_number++;
		if (message_number != next_message_number)
		{
			// wrong message number => send NACK
			message_number = next_message_number;
			next_message_number--;
			at90can_send_message(command | WRONG_NUMBER_REPSONSE, 0);
			continue;
		}
		
		BOOT_LED_TOGGLE;
		
		// process command
		switch (command)
		{
		case IDENTIFY:
			// version and command of the bootloader
			message_data[0] = (BOOTLOADER_TYPE << 4) | (BOOTLOADER_VERSION & 0x0f);
			message_data[1] = PAGESIZE_IDENTIFIER;
			
			// number of writeable pages
			message_data[2] = (RWW_PAGES) >> 8;
			message_data[3] = (RWW_PAGES) & 0xFF;
			
			at90can_send_message(IDENTIFY | SUCCESSFULL_RESPONSE, 4);
			break;
		
		// --------------------------------------------------------------------
		// set the current address in the page buffer
		case SET_ADDRESS:
		{
			uint16_t page = (message_data[0] << 8) | message_data[1];
			uint16_t bufferpos = (message_data[2] << 8) | message_data[3];
			
			if ((message_data_length == 4)
				&& (page < RWW_PAGES)
				&& (bufferpos < (SPM_PAGESIZE / 4)))
			{
				flashpage = page;
				flashpage_buffer_pos = bufferpos;
				
				state = COLLECT_DATA;
				
				at90can_send_message(SET_ADDRESS | SUCCESSFULL_RESPONSE, 4);
			}
			else
			{
				goto error_response;
			}
			break;
		}
		// --------------------------------------------------------------------
		// collect data
		case DATA:
		{
			if (message_data_length != 4 ||
				flashpage_buffer_pos >= (SPM_PAGESIZE / 4) ||
				state == IDLE)
			{
				state = IDLE;
				goto error_response;
			}
			
			// check if the message starts a new block
			if (message_data_counter & START_OF_MESSAGE_MASK)
			{
				message_data_counter &= ~START_OF_MESSAGE_MASK;		// clear flag
				next_message_data_counter = message_data_counter;
				state = COLLECT_DATA;
			}
			
			if (message_data_counter != next_message_data_counter)
			{
				state = IDLE;
				goto error_response;
			}
			next_message_data_counter--;
			
			// copy data
			memcpy(flashpage_buffer + flashpage_buffer_pos * 4, &message_data[0], 4);
			flashpage_buffer_pos++;
			
			if (message_data_counter == 0)
			{
				if (flashpage_buffer_pos == (SPM_PAGESIZE / 4))
				{
					message_data[0] = flashpage >> 8;
					message_data[1] = flashpage & 0xff;
					
					if (flashpage >= RWW_PAGES) {
						message_data_length = 2;
						goto error_response;
					}
					
					boot_program_page( flashpage, flashpage_buffer );
					flashpage_buffer_pos = 0;
					flashpage += 1;
					
					// send ACK
					at90can_send_message(DATA | SUCCESSFULL_RESPONSE, 2);
				}
				else {
					at90can_send_message(DATA | SUCCESSFULL_RESPONSE, 0);
				}
			}
			break;
		}
		// start the flashed application program
		case START_APP:
		{
			at90can_send_message(START_APP | SUCCESSFULL_RESPONSE, 0);
			
			// wait for the mcp2515 to send the message
			_delay_ms(50);
			
			// start application
			BOOT_LED_OFF;
			boot_jump_to_application();
			break;
		}

#if BOOTLOADER_TYPE > 0
		case GET_FUSEBITS:
		{
			message_data[0] = boot_lock_fuse_bits_get(GET_LOCK_BITS);
			message_data[1] = boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
			message_data[2] = boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
			message_data[3] = boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS);
			
			at90can_send_message(GET_FUSEBITS | SUCCESSFULL_RESPONSE, 4);
			break;
		}
		case CHIP_ERASE:
		{
			// erase complete flash except the bootloader region
			for (uint16_t page = 0; page < RWW_PAGES; ++page)
			{
				uint32_t address = page * SPM_PAGESIZE;
				
				boot_page_erase(address);
				boot_spm_busy_wait();
			}
			boot_rww_enable();
			
			at90can_send_message(CHIP_ERASE | SUCCESSFULL_RESPONSE, 0);
			break;
		}
		// Read 1..4 Byte from the EEPROM
		case READ_EEPROM:
		{
			uint16_t eeprom_address = (message_data[0] << 8) | message_data[1];
			uint8_t number_of_bytes = message_data[2];

			if ((message_data_length == 3)
				&& (number_of_bytes > 0)
				&& (number_of_bytes <= 4)
				&& (eeprom_address <= E2END))
			{
				eeprom_read_block(&message_data[0], (void *) eeprom_address, number_of_bytes);
				at90can_send_message(READ_EEPROM | SUCCESSFULL_RESPONSE, number_of_bytes);
			}
			else
			{
				goto error_response;
			}
			break;
		}
		// write 1..2 Byte to the EEPROM
		case WRITE_EEPROM:
		{
			uint16_t eeprom_address = (message_data[0] << 8) | message_data[1];

			if ((message_data_length >= 3) && (eeprom_address <= E2END))
			{
				eeprom_write_block(&message_data[2], (void *) eeprom_address, message_data_length - 2);
				at90can_send_message(WRITE_EEPROM | SUCCESSFULL_RESPONSE, 0);
			}
			else
			{
				goto error_response;
			}
			break;
		}
//		// Lese 1..65556 Byte aus dem Flash. Bei mehr als 6 Zeichen
//		// wird das Ergebniss auf mehrere Nachrichten verteilt.
//		case READ_FLASH:
//			if (message_length == 6)
//			{
//				uint16_t flash_ptr = (MESSAGE_DATA[2] << 8) | MESSAGE_DATA[3];
//
//				if (flash_ptr <= FLASHEND)
//				{
//					uint16_t number = (MESSAGE_DATA[4] << 8) | MESSAGE_DATA[5];
//
//					// Anzahl der zu senden Nachrichten bestimmen
//					div_t r = div(number, 6);
//					number = r.quot;
//					if (r.rem > 0)
//						number += 1;
//
//					uint16_t counter = 0;
//					for (uint16_t i=0;i<number;i++)
//					{
//						if (i == r.quot)
//							// das letze Paket ist eventl. kuerzer
//							message_length = r.rem;
//						else
//							message_length = 6;
//
//						// FIXME
//						//memcpy_P( MESSAGE_DATA + 2, (PGM_VOID_P) flash_ptr, message_length );
//						flash_ptr += message_length;
//
//						MESSAGE_DATA[1] = counter;
//						counter = (counter + 1) & 0x3f;
//
//						// Nachricht verschicken
//						at90can_send_message(0);
//					}
//
//					response = ACK;
//					break;
//				}
//			}
//			response = NACK;
//			break;
#endif
		
		error_response:
		default:
			at90can_send_message(command | ERROR_RESPONSE, message_data_length);
			break;
		}
	}
}
