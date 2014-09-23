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

#ifndef CC2500_H_
#define CC2500_H_

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#include <stdint.h>

/**
 * \brief	Initializes the CC2500 driver.
 *
 * \return	Returns 0 on success, -1 otherwise. The `errno` will be set
 *		in case of an error.
 *
 * \exception	ENODEV	The SPI device could not be opened.
 * \exception	EIO	The CC2500 did not respond as expected. This could be
 *			caused by a faulty SPI driver implementation, or a
 *			wrongly connected CC2500.
 */
int cc2k5_init(void);

/**
 * \brief	Sets one of the CC2500's configuration registers.
 *
 * \param[in]	reg	The address of the register. You should use the
 * 			constants in \f cc2500_regmap.h.
 *
 * \param[in]	val	The new value for the register.
 */
void cc2k5_set_register(uint8_t reg, uint8_t val);

/**
 * \brief	Reads a value from one of the CC2500's configuration or status
 *              registers.
 *
 * \param[in]	addr	The address of the register. You should use the
 * 			constants in \f cc2500_regmap.h.
 *
 * \return	The current value of the register.
 */
uint8_t cc2k5_get_register(uint8_t addr);

/**
 * \brief	Sends a command with the command code in `command` to the CC2500.
 *
 * \param[in]	command	One of the `CC2K5_COMMAND_STROBES`.
 */
void cc2k5_send_cmnd(uint8_t command);

/**
 * \brief	Sends out data via the CC2500 RF link.
 *
 * \param[in]	buf	The data that should be sent.
 * \param[in]	n_bytes	The number of bytes in `buf`.
 */
void cc2k5_send(void *buf, uint8_t n_bytes);

/**
 * \brief	Receives data via the CC2500 RF link.
 *
 * \param[in]	buf	Allocated memory of `n_bytes` bytes. Will be filled with
 *			the received data.
 * \param[in]	n_bytes	The number of bytes to receive resp. the number of bytes
 *			available in `buf`.
 */
void cc2k5_recv(void *buf, uint8_t *n_bytes);

 #ifdef __cplusplus
 }
 #endif	/* __cplusplus */

#endif	/* CC2500_H_ */
