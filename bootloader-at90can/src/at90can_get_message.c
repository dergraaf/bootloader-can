/* Copyright (c) 2010, Fabian Greif
 * All rights reserved.
 *
 * The file is part of the CAN bootloader project and is released under the
 * 3-clause BSD license. See the file `LICENSE` for the full license governing
 * this code.
 */
// ----------------------------------------------------------------------------

#include <util/atomic.h>

#include "../src/at90can.h"

// ----------------------------------------------------------------------------
static bool
check_message(void)
{
	if (at90can_messages_waiting > 0)
		return true;
	else
		return false;
}

// ----------------------------------------------------------------------------
command_t
at90can_get_message(void)
{
	// check if there is any waiting message
	if (!check_message())
		return NO_MESSAGE;
	
	// find the MOb with the received message
	for (uint8_t mob = 0; mob < 8; mob++)
	{
		CANPAGE = mob << 4;
		
		if (CANSTMOB & (1 << RXOK))
		{
			// clear flags (read-write-cycle required)
			CANSTMOB &= 0;
			
			// read status
			message_data_length = CANCDMOB & 0x0f;
			
			uint8_t type = NO_MESSAGE;
			uint8_t board_id = CANMSG;
			if (message_data_length >= 4 && board_id == BOOTLOADER_BOARD_ID)
			{
				type = CANMSG;
				message_number = CANMSG;
				message_data_counter = CANMSG;
				message_data_length -= 4;
				
				// read data
				for (uint8_t i = 0; i < message_data_length; i++) {
					message_data[i] = CANMSG;
				}
			}
			
			// mark message as processed
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
				at90can_messages_waiting--;
			}
			
			// re-enable interrupts
			CANIE2 |= (1 << mob);
			
			// clear flags
			CANCDMOB = (1 << CONMOB1);
			
			return type;
		}
	}
	
	return NO_MESSAGE;		// should never happen
}
