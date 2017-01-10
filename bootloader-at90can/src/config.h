/*
 * Copyright (c) 2010, 2015-2017 Fabian Greif.
 * All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define BOOTLOADER_TYPE         2
//#define   BOOT_LED                E,4

//#define   BOOT_LED                A,5
#define BOOT_INIT               PORTG &= ~(1<<PG0); \
                                DDRG |= (1<<PG0)
