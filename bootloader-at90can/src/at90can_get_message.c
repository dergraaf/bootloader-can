/* Copyright (c) 2010, Fabian Greif
 * All rights reserved.
 *
 * The file is part of the CAN bootloader project and is released under the
 * 3-clause BSD license. See the file `LICENSE` for the full license governing
 * this code.
 */
// ----------------------------------------------------------------------------

#include <util/atomic.h>

#include "at90can.h"

static bool
at90can_check_message(void)
{
	if (at90can_messages_waiting > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

static command_t
at90can_read_message(uint8_t mob)
{
	// clear flags (read-write-cycle required)
	CANSTMOB &= 0;

	// read status
	message_data_length = CANCDMOB & 0x0f;

	uint8_t type = NO_MESSAGE;
	uint8_t board_id = CANMSG;

	if ((message_data_length >= 4) &&
		(board_id == message_board_id))
	{
		// Only process data if the board number matches. Otherwise
		// the received message is not reported.
		type = CANMSG;
		message_number = CANMSG;
		message_data_counter = CANMSG;
		message_data_length -= 4;

		// read data
		for (uint8_t i = 0; i < message_data_length; i++)
		{
			message_data[i] = CANMSG;
		}
	}

	// mark message as processed
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		at90can_messages_waiting--;
	}

	// re-enable interrupts
	CANIE2 |= (1 << mob);

	// clear flags
	CANCDMOB = (1 << CONMOB1);

	return type;
}

command_t
at90can_get_message(void)
{
	// check if there is any waiting message
	if (at90can_check_message())
	{
		// find the MOb with the received message
		for (uint8_t mob = 0; mob < 8; mob++)
		{
			CANPAGE = mob << 4;
			
			if (CANSTMOB & (1 << RXOK))
			{
				return at90can_read_message(mob);
			}
		}
	}
	
	return NO_MESSAGE;
}
