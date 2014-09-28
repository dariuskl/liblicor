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

#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include <liblicor.h>

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

#ifndef min
#define min(a, b)	(a < b?a:b)
#endif

static struct {
	int command;
	int verbose;
} options = {-1, 0};

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

static void rgb2hsi(uint8_t R, uint8_t G, uint8_t B, uint8_t *H, uint8_t *S,
		uint8_t *I)
{
	float r, g, b, h, s, i;

	if (R + G + B == 0) {
		*H = 0;
		*S = 0;
		*I = 0;
	}
	else if (R == G && R == B) {
		*H = 0;
		*S = 0;
		i = (R + G + B) / (3.0f * 255.0f);

		*I = i * 255;
	}
	else {
		/* normalize RGB values */
		r = R / (float)(R + G + B);
		g = G / (float)(R + G + B);
		b = B / (float)(R + G + B);

		/* calculate normalized H */
		h = acos (
			( 0.5f * ((r-g)+(r-b)) )
			/ sqrt( pow (r-g,2) + ((r-b)*(g-b)) )
		);
		if (b > g) {
			h = 2 * M_PI - h;
		}

		// calculate normalized s
		s = 1.0f - ( 3.0f * min(min(r, g), b) );

		// calculate normalized i
		i = (R + G + B) / (float)(3 * 255);

		// normieren
		*H = (h * 180.0f / M_PI) * 255.f/360.f;
		*S = s * 255;
		*I = i * 255;
	}
}

static int parse_color(char *s, struct color *c)
{
	int ret;
	uint8_t r, g, b;

	ret = sscanf(s, "%hhu,%hhu,%hhu",
			&(c->hue),
			&(c->saturation),
			&(c->value));
	if (ret == 3)
		return 0;

	ret = sscanf(s, "%2hhx%2hhx%2hhx", &r, &g, &b);
	if (ret == 3) {
		rgb2hsi(r, g, b, &(c->hue), &(c->saturation), &(c->value));
		return 0;
	}

	return -1;
}

static void print_usage(void)
{
	fputs("licor [<options>] <command> [<color>]\n"
			"<options> can be one of\n"
			"\t-v\t\tbe verbose\n"
			"\n"
			"<command> can be one of\n"
			"\ton\t\tturn the lamp on\n"
			"\toff\t\tturn the lamp off\n"
			"\n"
			"<color> is a color and can be given in HSI or RGB:\n"
			"\tH,S,I\n"
			"\tRGB\n",
			stderr);
	exit(1);
}

enum COMMANDS {
	C_ON, C_OFF
};

static void parse_args(int argc, char **argv)
{
	int ret;

	if (argc == 1)
		print_usage();

	if (argc < 2) {
		fputs("Insufficiently many arguments provided\n", stderr);
	}

	for (; argc > 0; argc--) {
		if (argv[argc][0] == '-') {
			/* option */
			switch (argv[argc][1]) {
			case 'v':
				options.verbose = 1;
				break;
			default:
				fprintf(stderr, "licor: invalid option `%c`",
						argv[argc][1]);
				exit(1);
				break;
			}
		}
		else if (options.command == -1) {
			/* command */
			if (strncmp(argv[1], "on", 2) == 0) {
				options.command = C_ON;
			}
			else if (strncmp(argv[1], "off", 3) == 0) {
				options.command = C_OFF;
			}
			else {
				fprintf(stderr, "licor: invalid command `%s`\n",
						argv[argc]);
				exit(1);
			}
		}
		else {
			/* color */
			ret = parse_color(argv[argc], lc_color);
			if (ret != 0) {
				fputs("licor: invalid color format\n", stderr);
				exit(1);
			}
		}
	}
}

static struct lc_lamp lamp = {
		{0xF0, 0x58, 0xAD, 0x15, 0xE6, 0x47, 0xA5, 0x0B, 0x11},
		0
};

int main(int argc, char *argv[])
{
	int ret;

	ret = lc_init();
	if (ret != 0) {
		perror("lc_init()");
		return 1;
	}

	parse_args(argc, argv);

	switch (options.command) {
	case C_ON:
		lc_on(&lamp);
		break;
	case C_OFF:
		lc_off(&lamp);
		break;
	}
}
