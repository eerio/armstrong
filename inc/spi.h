/* Prorotypes for SPI init procedures, communication abstractions etc.
 *
 * author: Paweł Balawender
 * github.com@eerio
 */

/* Send byte by the SPI interface */
void SPI_send(uint8_t);

/* Initialize SPI peripheral */
void setup_spi(void);

