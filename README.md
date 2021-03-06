Living Colors Remote Control Library (licor)
============================================

An easy-to-use high level API to control the first generation of Philips Living
Colors lamps.

It relies on the CC2500 2.4 GHz transceiver by Texas Instruments and thus needs
an SPI device or four GPIO pins to make connection to it. There are a couple of
driver implementations for the SPI cores of various MCUs already available
(MSP430, CC3200, Linux' `spidev` driver) and a soft SPI implementation, that
only needs to be adjusted to the GPIO at hand, as well. (Coming soon – after a
code cleanup – actually.)

**Please note** that this is still a work in progress and e.g. the address
learning functionality is not yet implemented.


To Do
-------------

  - [ ] Implement a device reset
        Sometimes the CC2500 will become unresponsive and needs to be resetted.
        This is not unproblematic (at least with Linux), because the `spidev`
        driver has full and exclusive control over the `CSn` line.

