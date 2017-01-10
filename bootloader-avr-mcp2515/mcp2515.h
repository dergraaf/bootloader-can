/*
 * Copyright (c) 2010, 2015-2017 Fabian Greif.
 * All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MCP2515_H
#define MCP2515_H

#include "defaults.h"
#include "mcp2515_defs.h"

#include <inttypes.h>

extern uint8_t message_number;          //!< Running number of the messages
extern uint8_t message_data_counter;
extern uint8_t message_data_length;     //!< Length of the data-field
extern uint8_t message_data[4];

typedef enum {
    // every bootloader type has this commands
    IDENTIFY        = 1,
    SET_ADDRESS     = 2,
    DATA            = 3,
    START_APP       = 4,

    // only avilable in the "bigger" versions
    GET_FUSEBITS    = 5,
    CHIP_ERASE      = 6,

    REQUEST                 = 0x00,
    SUCCESSFULL_RESPONSE    = 0x40,
    ERROR_RESPONSE          = 0x80,
    WRONG_NUMBER_REPSONSE   = 0xC0,

    NO_MESSAGE      = 0x3f
} command_t;

#define COMMAND_MASK            0x3F
#define START_OF_MESSAGE_MASK   0x80


void
mcp2515_send_message(uint8_t type, uint8_t length);

uint8_t
mcp2515_get_message(void);

#endif  // MCP2515_H
