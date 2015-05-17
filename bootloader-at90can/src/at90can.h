/* Copyright (c) 2010, Fabian Greif
 * All rights reserved.
 *
 * The file is part of the CAN bootloader project and is released under the
 * 3-clause BSD license. See the file `LICENSE` for the full license governing
 * this code.
 */
// ----------------------------------------------------------------------------

#ifndef AT90CAN_H
#define AT90CAN_H

#include <stdint.h>
#include <stdbool.h>

#include <avr/pgmspace.h>

extern volatile uint8_t at90can_messages_waiting;
extern volatile uint8_t at90can_free_buffer;

extern uint8_t message_number;			//!< Running number of the messages
extern uint8_t message_data_counter;	
extern uint8_t message_data_length;		//!< Length of the data-field
extern uint8_t message_data[4];

typedef enum
{
	// every bootloader type has this commands
	IDENTIFY		= 1,
	SET_ADDRESS		= 2,
	DATA			= 3,
	START_APP		= 4,
	
	// only avilable in the "bigger" versions
	READ_FLASH		= 5,
	GET_FUSEBITS	= 6,
	CHIP_ERASE		= 7,
	
	READ_EEPROM		= 8,
	WRITE_EEPROM	= 9,

	REQUEST					= 0x00,
	SUCCESSFULL_RESPONSE	= 0x40,
	ERROR_RESPONSE			= 0x80,
	WRONG_NUMBER_REPSONSE	= 0xC0,
	
	NO_MESSAGE		= 0x3f
} command_t;

#define	COMMAND_MASK			0x3F
#define	START_OF_MESSAGE_MASK	0x80

/**
 * The lower eight MObs are used for receiption, the upper seven for
 * transmission. This separation simplifies the access to the registers 
 * and leads to smaller code size.
 */
void
at90can_init(void);

/**
 * Send a message.
 * 
 * If all seven send buffers are used this method waits until one gets free.
 * 
 * \param	length	Length of the data segment (0-4)
 */
void
at90can_send_message(command_t type, uint8_t length);

/**
 * Receive new messages.
 * 
 * \return	Type of the message, NO_MESSAGE if no or an invalid message was
 * 			received.
 */
command_t
at90can_get_message(void);

#endif // AT90CAN_H
