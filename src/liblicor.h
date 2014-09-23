/* Copyright (c) 2014 Darius Kellermann <darius.kellermann@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef LIBLICOR_H_
#define LIBLICOR_H_

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#include <stdint.h>

/**
 * Represents a color, which is defined by its hue, saturation and whiteness
 * (value).
 */
#pragma pack(1)
struct color {
	/**
	 * Hue is conventionally measured in degrees, but Philips expects only
	 * an eight-bit octet, so the full color range is scaled to the interval
	 * [0, 255]. The scaling is performed non-linear and a look-up table is
	 * provided by the implementation.
	 */
	uint8_t hue;		/**< Color */
	uint8_t saturation;	/**< Saturation */
	uint8_t value;		/**< Whiteness */
};

extern struct color *lc_color;

struct lc_lamp {
	uint8_t addr[9];	/**< The 9 byte address of the lamp. */
	uint8_t seq;		/**< The sequence number of the lamp. */
};

/**
 * Initializes the Living Colors API.
 *
 * Will perform initialization of the SPI driver, then power up and configure
 * the CC2500. It can also be used to reset the library.
 *
 * \note	This function will wait for the CC2500 to become ready. It does
 *		not time out and thus might lockup the application if the CC2500
 *		misbehaves or the SPI driver implementation is faulty.
 *
 * \return	Returns 0 on success, -1 otherwise. The `errno` will be set
 *		in case of an error.
 *
 * \exception	ENODEV	The SPI device could not be opened.
 * \exception	EIO	The CC2500 did not respond as expected. This could be
 *			caused by a faulty SPI driver implementation, or a
 *			wrongly connected CC2500.
 */
int lc_init(void);

/**
 * Starts a learning phase of `t` seconds in which at most `max` addresses will
 * be learned and stored in `lamp`.
 *
 * The CC2500 will listen for ongoing communication and infer the addresses of
 * lamps from the traffic. To ensure that this works, the user has to use the
 * original remote to control the lamp during the learning phase.
 *
 * @param[in,out]	lamp	The addresses learned during the phase.
 * @param[in]		max	The maximum number of addresses to store in
 *				`addr`.
 * @param[in]		t	The number of seconds that the learning phase
 *				will last.
 *
 * @return	On success, the number of addresses that were learned.
 */
int lc_learn(struct lc_lamp *lamp, int max, uint8_t t);

/**
 * Turns a Living Colors lamp on.
 *
 * The Living Colors lamp expects a color together with the turn on command
 * code, this function will just send an default color resp. the one that has
 * been set most recently.
 */
void lc_on(struct lc_lamp *lamp);

/**
* Turns a Living Colors lamp off.
*/
void lc_off(struct lc_lamp *lamp);

/**
 * Sets the current color of the lamp.
 *
 * \param[in]	new_color	If this is NULL, the color in lc_color will be
 * 				used.
 */
void lc_set_color(struct lc_lamp *lamp, struct color *new_color);

enum SPI_TRANSFER_FLAGS {
	SPI_NONE = 0,
	/**
	 * When this flag is set, CS will not be de-asserted after the transfer
	 * is complete. Thus further calls to spi_transfer() may follow that
	 * write to the same transfer.
	 * If you wish to end the transfer, i.e. de-assert CS, just issue
	 * another call to spi_transfer without providing this flag.
	 */
	SPI_BURST = 1
};

/**
 * Initializes the SPI implementation as given by the configuration flags. If
 * no flags are given, the default configuration (CS active high, SCLK idles
 * low, data is sampled on the positive edge) is applied.
 *
 * \return	Returns 0 on success, -1 otherwise. The `errno` will be set
 *		in case of an error.
 *
 * \exception	ENODEV
 *
 * \sa	SPI_CFG_FLAGS
 */
int spi_init(void);

/**
 * Transfers a number of bytes, i.e. transmits n_bytes bytes in tx_buf and
 * receives n_bytes into rx_buf simultaneously.
 *
 * If just one or neither transmit direction is desired, tx_buf reps. rx_buf may
 * be NULL.
 *
 * \param[in]	tx_buf	The buffer that contains the data that should be sent
 *			during the transfer. If this is NULL, 0 will be sent.
 * \param[out]	rx_buf	The buffer that will be filled with the data that is
 *			received during the transfer. If this NULL, no data will
 *			be kept. If this is the same buffer as \p tx_buf it will
 *			safely be overwritten.
 * \param[in]	n_bytes	The number of bytes that should be transferred.
 *
 * \return	Returns 0 on success, -1 otherwise. The `errno` will be set
 *		in case of an error.
 */
int spi_transfer(void *tx_buf, void *rx_buf, uint8_t n_bytes);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* LIBLICOR_H_ */
