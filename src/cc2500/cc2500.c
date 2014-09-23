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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cc2500.h"
#include "cc2500_regmap.h"

#include "../liblicor.h"

enum CC2K5_STATES {
	CC2K5_IDLE = 0,			/** Idle */
	CC2K5_RX = 1,			/** Receiving */
	CC2K5_TX = 2,			/** Transmitting */
	CC2K5_FSTXON = 3,		/** Freq. synth. on */
	CC2K5_CALIBRATE = 4,		/** Freq. synth. calibrating */
	CC2K5_SETTLING = 5,		/** PLL is settling */
	CC2K5_RXFIFOOVERFLOW = 6,	/** RX overflow, flush with SFRX */
	CC2K5_TXFIFOUNDERFLOW = 7	/** TX underflowed, ack. with SFTX */
};

enum CC2K5_ACCESS_MODES {
	READ = 0x80,	/** Read mode */
	WRITE = 0x00,	/** Write mode */
	BURST = 0x40,	/** Burst read/write */
	SINGLE = 0x00	/** Single read/write */
};

enum CC2K5_STATUS_BYTE {
	/** Stays high until power and crystal have stabilized.  Should always
	 * be low when using the SPI interface. */
	CHIP_RDYn = 0x80,
	/** One of `CC2K5_STATES`. */
	STATE = 0x70,
	/** The number of bytes available in the RX FIFO or free bytes in the TX
	 * FIFO. */
	FIFO_BYTES_AVAILABLE = 0x0F
};

/**
 * The part number of the cc2500, i.e. the contents of the partnum register.
 * Used to check that the cc2500 is present on the bus.
 */
#define CC2K5_PARTNUM		0x80

int cc2k5_init(void)
{
	int ret;
	uint8_t tx[2];
	uint8_t rx[2];

	ret = spi_init();
	if (ret != 0)
		return -1;

	tx[0] = SINGLE | WRITE | SRES;
	do {
		spi_transfer(tx, rx, 1);
	} while((rx[0] & CHIP_RDYn) != 0);

	tx[0] = BURST | READ | PARTNUM;
	tx[1] = 0x00;
	do {
		spi_transfer(tx, rx, 2);
	} while ((rx[0] & CHIP_RDYn) != 0);

	if (rx[1] != CC2K5_PARTNUM) {
		errno = ENODEV;
		return -1;
	}

	return 0;
}

void cc2k5_set_register(uint8_t addr, uint8_t val)
{
	uint8_t tx[2];

	tx[0] = SINGLE | WRITE | addr;
	tx[1] = val;

	spi_transfer(tx, NULL, 2);
}

uint8_t cc2k5_get_register(uint8_t addr)
{
	uint8_t tx[2];
	uint8_t rx[2];

	tx[0] = SINGLE | READ | addr;
	tx[1] = 0x00;

	spi_transfer(tx, rx, 2);

	return rx[1];
}

void cc2k5_send_cmnd(uint8_t command)
{
	spi_transfer(&command, NULL, 1);
}

void cc2k5_send(void *buf, uint8_t n_bytes)
{
	uint8_t *tx;

	tx = malloc(n_bytes + 1);
	if (tx == NULL)
		return;

	tx[0] = BURST | WRITE | FIFO;
	memcpy(tx + 1, buf, n_bytes);

	spi_transfer(tx, NULL, n_bytes + 1);

	tx[0] = SINGLE | WRITE | STX;
	spi_transfer(tx, NULL, 1);

	free(tx);
}

void cc2k5_recv(void *buf, uint8_t *n_bytes)
{
	uint8_t status, *tx, *rx;

	spi_transfer(NULL, &status, 1);
	*n_bytes = status & 0x0F;

	tx = malloc(*n_bytes + 1);
	if (tx == NULL)
		return;

	rx = malloc(*n_bytes + 1);
	if (rx == NULL) {
		free(tx);
		return;
	}

	tx[0] = BURST | READ | FIFO;

	spi_transfer(tx, rx, *n_bytes  + 1);

	memcpy(buf, rx + 1, *n_bytes);

	free(rx);
	free(tx);
}

#ifdef __cplusplus
}
#endif	/* __cplusplus */

