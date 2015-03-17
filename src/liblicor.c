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

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#include <stddef.h>
#include <string.h>

#include "cc2500/cc2500.h"
#include "cc2500/cc2500_regmap.h"
#include "liblicor.h"

enum LIVING_COLORS_COMMANDS {
	LC_SET_COLOR = 3,
	LC_ON = 5,
	LC_OFF = 7
};

/**
 * This is the structure of the packets that are sent from the remote control to
 * the lamp.
 */
#pragma pack(1)
struct packet {
	uint8_t preamble;		/**< This must always be 0x0E. */
	uint8_t address[9];		/**< The address of the lamp. */
	uint8_t command;		/**< The command to execute. */
	uint8_t sequence_number;	/**< The packets sequence number. */
	struct color color;		/**< The color of the lamp's light. */
};
static struct packet p_buf = {0x0E, {0}, 0, 0, {0}};

struct color *lc_color = &(p_buf.color);

int lc_init(void)
{
	int ret;

	ret = cc2k5_init();
	if (ret < 0)
		return ret;

	/*
	 * Configure the CC2500 for usage with the Living Colors lamps.
	 */
	cc2k5_set_register(IOCFG2,	0x06);
	cc2k5_set_register(IOCFG0,	0x01);
	cc2k5_set_register(FIFOTHR,	0x0D);
	cc2k5_set_register(PKTLEN,	0xFF);
	cc2k5_set_register(PKTCTRL1,	0x04);
	cc2k5_set_register(PKTCTRL0,	0x45);
	cc2k5_set_register(ADDR,	0x00);
	cc2k5_set_register(CHANNR,	0x03);
	cc2k5_set_register(FSCTRL1,	0x09);
	cc2k5_set_register(FSCTRL0,	0x00);
	cc2k5_set_register(FREQ2,	0x5D);
	cc2k5_set_register(FREQ1,	0x93);
	cc2k5_set_register(FREQ0,	0xB1);
	cc2k5_set_register(MDMCFG4,	0x2D);
	cc2k5_set_register(MDMCFG3,	0x3B);
	cc2k5_set_register(MDMCFG2,	0x73);
	cc2k5_set_register(MDMCFG1,	0x22);
	cc2k5_set_register(MDMCFG0,	0xF8);
	cc2k5_set_register(DEVIATN,	0x00);
	cc2k5_set_register(MCSM0,	0x18);
	cc2k5_set_register(FOCCFG,	0x1D);
	cc2k5_set_register(BSCFG,	0x1C);
	cc2k5_set_register(AGCTRL2,	0xC7);
	cc2k5_set_register(AGCTRL1,	0x00);
	cc2k5_set_register(AGCTRL0,	0xB2);
	cc2k5_set_register(FREND1,	0xB6);
	cc2k5_set_register(FREND0,	0x10);
	cc2k5_set_register(FSCAL3,	0xEA);
	cc2k5_set_register(FSCAL2,	0x0A);
	cc2k5_set_register(FSCAL1,	0x00);
	cc2k5_set_register(FSCAL0,	0x11);
	cc2k5_set_register(FSTEST,	0x59);
	cc2k5_set_register(TEST2,	0x88);
	cc2k5_set_register(TEST1,	0x31);
	cc2k5_set_register(TEST0,	0x0B);
	cc2k5_set_register(PATABLE,	0xFF);

	cc2k5_send_cmnd(SIDLE);
	cc2k5_send_cmnd(SIDLE);
	cc2k5_send_cmnd(SPWD);
	cc2k5_send_cmnd(SIDLE);

	return 0;
}

int lc_learn(struct lc_lamp *lamp, int max, uint8_t t)
{
	int n_lamps;

	n_lamps = 0;

	for (; max > 0; max--) {
		/* TODO */
	}

	return n_lamps;
}

void lc_on(struct lc_lamp *lamp)
{
	memcpy(&(p_buf.address), &(lamp->addr), 9);
	p_buf.command = LC_ON;
	p_buf.sequence_number = lamp->seq;

	cc2k5_send(&p_buf, sizeof(p_buf));

	lamp->seq++;
}

void lc_off(struct lc_lamp *lamp)
{
	memcpy(&(p_buf.address), &(lamp->addr), 9);
	p_buf.command = LC_OFF;
	p_buf.sequence_number = lamp->seq;

	cc2k5_send(&p_buf, sizeof(p_buf));

	lamp->seq++;
}

void lc_set_color(struct lc_lamp *lamp, struct color *new_color)
{
	memcpy(&(p_buf.address), &(lamp->addr), 9);
	p_buf.command = LC_SET_COLOR;
	p_buf.sequence_number = lamp->seq;
	if (new_color != NULL) {
		p_buf.color.hue = new_color->hue;
		p_buf.color.saturation = new_color->saturation;
		p_buf.color.value = new_color->value;
	}

	cc2k5_send(&p_buf, sizeof(p_buf));

	lamp->seq++;
}

#ifdef __cplusplus
}
#endif	/* __cplusplus */
