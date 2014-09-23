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

#ifndef CC2500_REGMAP_H_
#define CC2500_REGMAP_H_

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#define BIT(n)	(1 << n)

enum CC2K_REGISTERS_CONFIGURATION {
	IOCFG2	= 0x00,		/**< GDO2 Output pin configuration */
	IOCFG1	= 0x01,		/**< GDO1 Output pin configuration */
	IOCFG0	= 0x02,		/**< GDO0 Output pin configuration */
	FIFOTHR = 0x03,		/**< RX FIFO and TX FIFO thresholds */
	/**
	 * The 8 most significant bits of the 16-bit sync word.
	 *
	 * \reset	0xD3
	 */
	SYNC1	= 0x04,		/**< Sync word, high byte */
	/**
	 * The 8 least significant bits of the 16-bit sync word.
	 *
	 * \reset	0x91
	 */
	SYNC0	= 0x05,		/**< Sync word, low byte */
	/**
	 * Indicates the packet length when fixed packet length is enabled. If
	 * variable length packets are used this value indicates the maximum
	 * length packets allowed.
	 *
	 * \reset
	 */
	PKTLEN	= 0x06,		/**< Packet length */
	PKTCTRL1 = 0x07,	/**< Packet automation control */
	PKTCTRL0 = 0x08,	/**< Packet automation control */
	/**
	 * Address used for packet filtration. Optional broadcast addresses are
	 * 0 (0x00) and 255 (0xFF).
	 *
	 * \reset	0x00
	 */
	ADDR	= 0x09,		/**< Device address */
	CHANNR	= 0x0A,		/**< Channel number */
	FSCTRL1	= 0x0B,		/**< Frequency synthesizer control */
	FSCTRL0	= 0x0C,		/**< Frequency synthesizer control */
	FREQ2	= 0x0D,		/**< Frequency control world, high byte */
	FREQ1	= 0x0E,		/**< Frequency control world, middle byte */
	FREQ0	= 0x0F,		/**< Frequency control world, low byte */
	MDMCFG4 = 0x10,		/**< Modem configuration */
	MDMCFG3 = 0x11,		/**< Modem configuration */
	MDMCFG2 = 0x12,		/**< Modem configuration */
	MDMCFG1 = 0x13,		/**< Modem configuration */
	MDMCFG0 = 0x14,		/**< Modem configuration */
	DEVIATN = 0x15,		/**< Modem deviation setting */
	MCSM2 = 0x16,	/**< Main Radio Control State Machine configuration */
	MCSM1 = 0x17,	/**< Main Radio Control State Machine configuration */
	MCSM0 = 0x18,	/**< Main Radio Control State Machine configuration */
	FOCCFG = 0x19,	/**< Frequency Offset Compensation configuration */
	BSCFG = 0x1A,		/**< Bit Synchronization configuration */
	AGCTRL2 = 0x1B,		/**< AGC control */
	AGCTRL1 = 0x1C,		/**< AGC control */
	AGCTRL0 = 0x1D,		/**< AGC control */
	WOREVT1 = 0x1E,		/**< High byte Event 0 timeout */
	WOREVT0 = 0x1F,		/**< Low byte Event 0 timeout */
	WORCTRL = 0x20,		/**< Wake On Radio Control */
	FREND1 = 0x21,		/**< Front end RX configuration */
	FREND0 = 0x22,		/**< Front end TX configuration */
	FSCAL3 = 0x23,		/**< Frequency synthesizer calibration */
	FSCAL2 = 0x24,		/**< Frequency synthesizer calibration */
	FSCAL1 = 0x25,		/**< Frequency synthesizer calibration */
	FSCAL0 = 0x26,		/**< Frequency synthesizer calibration */
	RCCTRL1 = 0x27,		/**< RC oscillator configuration */
	RCCTRL0 = 0x28,		/**< RC oscillator configuration */
	FSTEST = 0x29,	/**< Frequency synthesizer calibration control */
	PTEST = 0x2A,		/**< Production test */
	AGCTEST = 0x2B,		/**< AGC test */
	TEST2 = 0x2C,		/**< Various test settings */
	TEST1 = 0x2D,		/**< Various test settings */
	TEST0 = 0x2E		/**< Various test settings */
};

enum CC2K5_REGISTER_CONFIGURATION_IOCFG2 {
	/** Invert output, i.e. select active low (1) / high (0). */
	GDO2_INV	= BIT(6)
};

enum CC2K5_REGISTERS_STATUS {
	PARTNUM		= 0x30,	/**< CC2500 part number (0x81) */
	VERSION		= 0x31,	/**< Current version number */
	FREQEST		= 0x32,	/**< Frequency offset estimate */
	LQI		= 0x33,	/**< Domodulator estimate for Link Quality */
	RSSI		= 0x34,	/**< Received signal strength inidcation */
	MARCSTATE	= 0x35,	/**< Control state machine state */
	WORTIME1	= 0x36,	/**< High byte of WOR timer */
	WORTIME0	= 0x37,	/**< Low byte of WOR timer */
	PKTSTATUS	= 0x38,	/**< Current GDOx status and packet status */
	VCO_VC_DAC	= 0x39,	/**< Current settings from PLL calibration module */
	TXBYTES		= 0x3A,	/**< Underflow and number of bytes in the TX FIFO */
	RXBYTES		= 0x3B,	/**< Overflow and number of bytes in the RX FIFO */
	RCCTRL1_STATUS	= 0x3C,	/**< Last RC calibrator calibration result */
	RCCTRL0_STATUS	= 0x3D	/**< Last RC calibrator calibration result */
};

#define PATABLE		0x3E
#define FIFO		0x3F

/*
 * The thirteen command strobe registers of the CC2500.
 */
enum CC2K5_COMMAND_STROBES {
	SRES	= 0x30,	/**< Reset chip. */
	/**
	 * Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1).
	 * If in RX (with CCA): Go to a wait state where only the synthesizer is
	 * running (for quick RX / TX turnaround).
	 */
	SFSTXON	= 0x31,	/**< */
	SXOFF	= 0x32,	/**< Turn off crystal oscillator. */
	SCAL	= 0x33,	/**< */
	SRX	= 0x34,	/**< Enable RX */
	STX	= 0x35,	/**< Enable TX */
	/**
	 * Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio
	 * mode if applicable.
	 */
	SIDLE	= 0x36,	/**< */
	SPWD	= 0x39	/**< Enter power down mode when CSn goes high. */
};

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* CC2500_REGMAP_H_ */
