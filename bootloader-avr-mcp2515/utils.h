/*
 * Copyright (c) 2010, 2015-2017 Fabian Greif.
 * All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_H
#define UTILS_H

/**
 * \ingroup utils_h
 * \name    Nützliches
 */
//@{

#define LOW_BYTE(x)     ((uint8_t) (x & 0xff))
#define HIGH_BYTE(x)    ((uint8_t) (x >> 8))
#define LOW_WORD(x)     ((uint16_t) (x & 0xffff))
#define HIGH_WORD(x)    ((uint16_t) (x >> 16))

//@}

/**
 * \ingroup utils_h
 * \name    Port-Makros
 *
 * Die Makros RESET(), SET(), SET_OUTPUT(), SET_INPUT() und IS_SET()
 * beziehen sich immer auf ein bestimmtes Bit eines Ports und helfen somit
 * den Code sehr portabel zu gestalten.
 *
 * Beispiel:
 * \code
 * #define LED   D,5        // PORTD, Pin 5
 *
 * SET_OUTPUT(LED);     // Pin als Ausgang schalten (wird z.B. zu DDRD |= (1<<5);)
 *
 * SET(LED);                // LED aktivieren
 * \endcode
 *
 * oder auch:
 *
 * \code
 * #define SCHALTER   B,1       // PORTB, Pin 1
 *
 * SET_INPUT_WITH_PULLUP(SCHALTER);
 *
 * if (IS_SET(SCHALTER)) {
 *      ...
 * }
 * \endcode
 *
 * Somit muss nur ein Define geändert werden sobald ein anderer Pin verwendet
 * werden soll. Außerdem muss nicht immer noch ein extra Define für den
 * entsprechenden Port angelegt werden wie es bisher immer der Fall war.
 */
//@{
#if defined(__DOXYGEN__)

#define RESET(x)        //!< Einzelnes Bit eines bestimmten Ports setzen
#define SET(x)          //!< Bit löschen
#define TOGGLE(x)       //!< Bit umschalten

#define SET_OUTPUT(x)   //!< Einzeles Bit ein Port als Ausgang schalten
#define SET_INPUT(x)    //!< Bit als Eingang schalten
#define SET_PULLUP(x)   //!< aktiviert den Pullup eines Pins (nur falls dieser als Eingang geschaltet ist)

#define SET_INPUT_WITH_PULLUP(x)    //!< Set den Pin als Eingang mit akiviertem Pullup

#define IS_SET(x)       //!< Zustand eines Eingangs abfragen

#else /* !DOXYGEN */

/* Warum hier zum Teil so seltsame Konstrukte notwendig sind wird zum Beispiel
 * in http://www.mikrocontroller.net/forum/read-1-324854.html#324980 erklärt.
 */
#define PORT(x)         _port2(x)
#define DDR(x)          _ddr2(x)
#define PIN(x)          _pin2(x)
#define REG(x)          _reg(x)
#define PIN_NUM(x)      _pin_num(x)

#define RESET(x)        RESET2(x)
#define SET(x)          SET2(x)
#define TOGGLE(x)       TOGGLE2(x)
#define SET_OUTPUT(x)   SET_OUTPUT2(x)
#define SET_INPUT(x)    SET_INPUT2(x)
#define SET_PULLUP(x)   SET2(x)
#define IS_SET(x)       IS_SET2(x)

#define SET_INPUT_WITH_PULLUP(x)    SET_INPUT_WITH_PULLUP2(x)

#define _port2(x)   PORT ## x
#define _ddr2(x)    DDR ## x
#define _pin2(x)    PIN ## x

#define _reg(x,y)       x
#define _pin_num(x,y)   y

#define RESET2(x,y)     PORT(x) &= ~(1<<y)
#define SET2(x,y)       PORT(x) |= (1<<y)
#define TOGGLE2(x,y)    PORT(x) ^= (1<<y)

#define SET_OUTPUT2(x,y)    DDR(x) |= (1<<y)
#define SET_INPUT2(x,y)     DDR(x) &= ~(1<<y)
#define SET_INPUT_WITH_PULLUP2(x,y) SET_INPUT2(x,y);SET2(x,y)

#define IS_SET2(x,y)    ((PIN(x) & (1<<y)) != 0)

#endif /* DOXYGEN */
//@}

#endif  // UTILS_H
