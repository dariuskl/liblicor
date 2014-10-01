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

#include <argp.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include <liblicor.h>

static struct {
	int command;
	int verbose;
	struct color color;
} options = {-1};

static int spi;

int spi_init(void)
{
	int ret;
	uint8_t mode, bits;
	uint32_t speed;

	mode = SPI_MODE_0;
	bits = 8;
	speed = 5000000;

	spi = open("/dev/spidev0.0", O_RDWR);
	if (spi < 0) {
		perror("Trying to open the SPI device");
		return -1;
	}

	ret = ioctl(spi, SPI_IOC_WR_MODE, &mode);
	ret |= ioctl(spi, SPI_IOC_WR_BITS_PER_WORD, &bits);
	ret |= ioctl(spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret != 0) {
		perror("Trying to configure spidev");
		return -1;
	}

	return 0;
}

int spi_transfer(void *tx_buf, void *rx_buf, uint8_t n_bytes)
{
	int ret;
	struct spi_ioc_transfer tr = {0};

	if (options.verbose) {
		if (tx_buf != NULL) {
			puts("tx_buf:");
			for (ret = 0; ret < n_bytes; ret++) {
				printf("%#02hhx ", ((uint8_t *)tx_buf)[ret]);
				if (ret % 26 == 0)
					puts("\n");
			}
			puts("\n");
		}
		if (rx_buf != NULL) {
			puts("rx_buf:");
			for (ret = 0; ret < n_bytes; ret++) {
				printf("%#02hhx ", ((uint8_t *)rx_buf)[ret]);
				if (ret % 26 == 0)
					puts("\n");
			}
			puts("\n");
		}
	}

	tr.tx_buf = (unsigned long)tx_buf;
	tr.rx_buf = (unsigned long)rx_buf;
	tr.len = n_bytes;
	tr.delay_usecs = 0;
	tr.speed_hz = 500000;
	tr.bits_per_word = 8;
	tr.cs_change = 0;

	ret = ioctl(spi, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		perror("can't send spi message");
		return -1;
	}

	return 0;
}

enum COMMANDS {
	C_ON = 0, C_OFF = 1
};

static int parse_command(const char *cmnd)
{
	if (strncmp(cmnd, "on", 2) == 0) {
		return C_ON;
	}
	else if (strncmp(cmnd, "off", 3) == 0) {
		return C_OFF;
	}
	else {
		return -1;
	}
}

static int parse_color(char *s, struct color *c)
{
	int ret;

	ret = sscanf(s, "%hhu,%hhu,%hhu",
			&(c->hue),
			&(c->saturation),
			&(c->value));
	if (ret == 3)
		return 0;

	return -1;
}

static struct argp_option argp_options[]  = {
		{"verbose", 'v', NULL, 0, "Be verbose"},
		{0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	int ret;

	switch (key) {
	case 'v':
		options.verbose = 1;
		break;

	case ARGP_KEY_ARG:
		if (state->arg_num == 0) {
			options.command = parse_command(arg);
			if (options.command < 0) {
				fputs("licor: invalid command given\n", stderr);
				return EINVAL;
			}
		}
		else if (options.command == C_ON && state->arg_num == 1) {
			ret = parse_color(arg, &options.color);
			if (ret != 0) {
				fputs("licor: invalid color given\n", stderr);
				return EINVAL;
			}
		}
		else {
			fprintf(stderr, "licor: unexpected argument `%s`\n",
					arg);
			return EINVAL;
		}
		break;

	case ARGP_KEY_END:
		if (state->arg_num < 1) {
			argp_usage(state);
		}
		else if (state->arg_num == 1 && options.command == C_ON) {
			fputs("licor: missing argument <color>\n", stderr);
			return EINVAL;
		}
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = {
		argp_options, &parse_opt,
		"<command> [<color>]",
		"A simple command-line interface for liblicor. You can use this"
		" to control Philips Living Colors lamps.\n\n"
		"<command> can be one of\n"
		"\ton\t\tTurn the lamp on\n"
		"\toff\t\tTurn the lamp off\n"
		"\n"
		"<color> is a color and must be given as\n"
		"\tH,S,V"
};

static struct lc_lamp lamp = {
		{0xF0, 0x58, 0xAD, 0x15, 0xE6, 0x47, 0xA5, 0x0B, 0x11},
		0
};

int main(int argc, char *argv[])
{
	int ret;

	ret = argp_parse(&argp, argc, argv, 0, 0, &options);
	if (ret != 0)
		return 1;

	ret = lc_init();
	if (ret != 0) {
		perror("lc_init()");
		return 1;
	}

	*lc_color = options.color;

	switch (options.command) {
	case C_ON:
		lc_on(&lamp);
		break;
	case C_OFF:
		lc_off(&lamp);
		break;
	}
}

