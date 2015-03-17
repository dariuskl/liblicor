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
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include <liblicor.h>

#define STS_BASE_DIR	"/var/local/licor"
#define STS_SEQNO	"seqno"

static struct {
	int command;
	char *device;
	uint8_t repetitions;
	struct lc_lamp lamp;
	int verbose;
	struct color color;
} options = {-1, "/dev/spidev0.0", 1,
	{
		{0xF0, 0x58, 0xAD, 0x15, 0xE6, 0x47, 0xA5, 0x0B, 0x11},
		0
	}
};

static int spi;

int spi_init(void)
{
	int ret;
	uint8_t mode, bits;
	uint32_t speed;

	mode = SPI_MODE_0;
	bits = 8;
	speed = 5000000;

	spi = open(options.device, O_RDWR);
	if (spi < 0) {
		fputs("Trying to open the SPI device `", stderr);
		fputs(options.device, stderr);
		perror("`");
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
	tr.delay_usecs = 100;
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
	C_ON = 0, C_OFF = 1, C_SET = 2, C_SCAN = 3
};

static int parse_address(const char *s, uint8_t addr[9])
{
	int ret;

	ret = sscanf(s, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			&addr[0], &addr[1], &addr[2], &addr[3], &addr[4],
			&addr[5], &addr[6], &addr[7], &addr[8]);
	if (ret == 9)
		return 0;

	return -1;
}

static int parse_command(const char *cmnd)
{
	if (strncmp(cmnd, "on", 2) == 0) {
		return C_ON;
	}
	else if (strncmp(cmnd, "off", 3) == 0) {
		return C_OFF;
	}
	else if (strncmp(cmnd, "set", 3) == 0) {
		return C_SET;
	}
	else if (strncmp(cmnd, "scan", 4) == 0) {
		return C_SCAN;
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

const char *argp_program_version = "licor 0.1";
const char *argp_program_bug_address = "<darius.kellermann@gmail.com>";

static struct argp_option argp_options[]  = {
		{"address", 'a', "ADDR", 0, "The 9 byte long address of the "
				"lamp that should be controlled"},
		{"device", 'd', "DEVICE", 0, "The SPI device to use"},
		{"repetitions", 'r', "N", 0, "The number of times the according"
				" command package is sent (hotfix option)"},
		{"sequence", 's', "SEQNUM", 0, "The sequence number to use for "
				"the packet"},
		{"verbose", 'v', NULL, 0, "Be verbose"},
		{0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	int ret;

	switch (key) {
	case 'a':
		ret = parse_address(arg, (uint8_t *)&(options.lamp.addr));
		if (ret != 0) {
			fputs("licor: malformed address string\n", stderr);
			return EINVAL;
		}
		break;
	case 'd':
		options.device = arg;
		break;
	case 'r':
		ret = atoi(arg);
		if (ret > 255 || ret < 1) {
			fputs("licor: number of repetitions out of range\n",
					stderr);
			return EINVAL;
		}
		options.repetitions = (uint8_t)ret;
		break;
	case 's':
		ret = atoi(arg);
		if (ret > 255 || ret < 0) {
			fputs("licor: sequence number out of range\n", stderr);
			return EINVAL;
		}
		options.lamp.seq = (uint8_t)ret;
		break;
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
		else if ((options.command == C_ON || options.command == C_SET)
				&& state->arg_num == 1) {
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
		else if (state->arg_num == 1 && (options.command == C_ON
				|| options.command == C_SET)) {
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
		"\ton <color>\t\tTurn the lamp on\n"
		"\toff\t\t\tTurn the lamp off\n"
		"\tset <color>\t\tSet the color of the lamp\n"
		"\tscan\t\t\tScan for lamp addresses\n"
		"\n"
		"<color> is a color and must be given as\n"
		"\tH,S,V"
};

int main(int argc, char *argv[])
{
	int ret, result;
	FILE *sts_seqno_f;

	result = 1;
	sts_seqno_f = NULL;

	ret = argp_parse(&argp, argc, argv, 0, 0, &options);
	if (ret != 0)
		return 1;

	ret = access(STS_BASE_DIR, F_OK);
	if (ret != 0) {
		perror("error: status directory does not exist");
		goto finish;
	}

	ret = access(STS_BASE_DIR, R_OK);
	if (ret != 0) {
		fputs("error: cannot read from status directory", stderr);
		goto finish;
	}

	sts_seqno_f = fopen(STS_BASE_DIR "/" STS_SEQNO, "r+");
	if (sts_seqno_f == NULL) {
		perror("error: cannot open sequence number file");
		goto finish;
	}

	ret = fread(&options.lamp.seq, sizeof options.lamp.seq, 1, sts_seqno_f);
	if (ret != 1) {
		perror("warning: cannot read from sequence number file, "
				"assuming 0");
		options.lamp.seq = 0;
	}

	ret = lc_init();
	if (ret != 0)
		goto finish;

	*lc_color = options.color;

	if (options.verbose) {
		printf("\tlamp = {\n"
				"\t\taddr = %hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:"
				"%hhx:%hhx\n"
				"\t\tseq = %hhu\n"
				"\t}\n",
				options.lamp.addr[0],
				options.lamp.addr[1],
				options.lamp.addr[2],
				options.lamp.addr[3],
				options.lamp.addr[4],
				options.lamp.addr[5],
				options.lamp.addr[6],
				options.lamp.addr[7],
				options.lamp.addr[8],
				options.lamp.seq);
		printf("\tlc_color: %hhu,%hhu,%hhu\n", lc_color->hue,
				lc_color->saturation, lc_color->value);
	}

	switch (options.command) {
	case C_ON:
		for (register int i = 0; i < options.repetitions; i++)
			lc_on(&options.lamp);
		break;
	case C_OFF:
		for (register int i = 0; i < options.repetitions; i++)
			lc_off(&options.lamp);
		options.lamp.seq = 0;
		break;
	case C_SET:
		for (register int i = 0; i < options.repetitions; i++)
			lc_set_color(&options.lamp, NULL);
		break;
	case C_SCAN:
		puts("licor will now scan for addresses. Use your original "
				"remote intensively for the next few seconds.\n");
		fputs("error: Sorry, this is not yet implemented.\n", stderr);
		break;
	default:
		break;
	}

	ret = fseek(sts_seqno_f, 0, SEEK_SET);
	assert (ret == 0);

	ret = fwrite(&options.lamp.seq, sizeof options.lamp.seq, 1, sts_seqno_f);
	if (ret != 1) {
		perror("warning: cannot write to sequence number file");
	}

	ret = fclose(sts_seqno_f);
	sts_seqno_f = NULL;
	if (ret == EOF) {
		perror("Error closing sequence number file");
		goto finish;
	}

	result = 0;

finish:

	if (sts_seqno_f != NULL) {
		fclose(sts_seqno_f);
	}

	return result;
}
