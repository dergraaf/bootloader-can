/*
 * Copyright (c) 2010, 2015-2017 Fabian Greif.
 * All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef AT90CAN_H
#define AT90CAN_H

#include <stdint.h>
#include <stdbool.h>

#include <avr/pgmspace.h>

#include "defaults.h"

extern volatile uint8_t at90can_messages_waiting;
extern volatile uint8_t at90can_free_buffer;

extern uint8_t message_board_id;

extern uint8_t message_number;          //!< Running number of the messages
extern uint8_t message_data_counter;
extern uint8_t message_data_length;     //!< Length of the data-field
extern uint8_t message_data[4];

typedef enum
{
    NO_OPERATION    = 0,

    // Every bootloader type has the following commands
    IDENTIFY        = 1,
    SET_ADDRESS     = 2,
    DATA            = 3,
    START_APP       = 4,

    // only available in type >= 1
    READ_FLASH      = 5,
    GET_FUSEBITS    = 6,
    CHIP_ERASE      = 7,

    READ_EEPROM     = 8,
    WRITE_EEPROM    = 9,

    // only available in type >= 2
    SET_BOARD_ID    = 10,
    SET_BITRATE     = 11,


    // Message Type
    REQUEST                 = 0x00,
    SUCCESSFULL_RESPONSE    = 0x40,
    ERROR_RESPONSE          = 0x80,
    WRONG_NUMBER_REPSONSE   = 0xC0,

    NO_MESSAGE      = 0x3F
} command_t;

typedef enum
{
    BITRATE_10_KBPS = 0,
    BITRATE_20_KBPS = 1,
    BITRATE_50_KBPS = 2,
    BITRATE_100_KBPS = 3,
    BITRATE_125_KBPS = 4,
    BITRATE_250_KBPS = 5,
    BITRATE_500_KBPS = 6,
    BITRATE_1_MBPS = 7
} bitrate_t;

#define COMMAND_MASK            0x3F
#define START_OF_MESSAGE_MASK   0x80

/**
 * The lower eight MObs are used for receiption, the upper seven for
 * transmission. This separation simplifies the access to the registers
 * and leads to smaller code size.
 */
void
at90can_init(uint8_t bitrate);

/**
 * Send a message.
 *
 * If all seven send buffers are used this method waits until one gets free.
 *
 * \param   length  Length of the data segment (0-4)
 */
void
at90can_send_message(command_t type, uint8_t length);

/**
 * Receive new messages.
 *
 * \return  Type of the message, NO_MESSAGE if no or an invalid message was
 *          received.
 */
command_t
at90can_get_message(void);

#endif // AT90CAN_H
