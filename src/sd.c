/* SDHC card initialization procedure and communication abstractions
 *
 * author: Paweł Balawender
 * github.com@eerio
 */
#include<sd.h>
#include<spi.h>
#include<common.h>

#define R1_PARAMETER_ERROR (1U << 6)
#define R1_ADDRESS_ERROR (1U << 5)
#define R1_ERASE_SEQUENCE_ERROR (1U << 4)
#define R1_COM_CRC_ERROR (1U << 3)
#define R1_ILLEGAL_COMMAND (1U << 2)
#define R1_ERASE_RESET (1U << 1)
#define R1_IN_IDLE_STATE (1U)

#define R2_OUT_OF_RANGE (1U << 7)
#define R2_CSD_OVERWRITE (R2_OUT_OF_RANGE)
#define R2_ERASE_PARAM (1U << 6)
#define R2_WP_VIOLATION (1U << 5)
#define R2_CARD_ECC_FAILED (1U << 4)
#define R2_CC_ERROR (1U << 3)
#define R2_ERROR (1U << 2)
#define R2_WP_ERASE_SKIP (1U << 1)
#define R2_LOCK_CMD_FAILED (R2_WP_ERASE_SKIP)
#define R2_UNLOCK_CMD_FAILED (R2_WP_ERASE_SKIP)
#define R2_CARD_IS_LOCKED (1U)

#define SD_SPI (SPI1)
#define SD_GPIO (GPIOA) 

uint8_t blank[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
/* resp: r7, arg: bits [11:8] of cmd: supply voltage, [7:0]: check pattern
 p.237*/
uint8_t cmd8[] = {0b01001000, 0x00, 0x00, 0x01, 0xAA, 0b00001111};
uint8_t cmd58[] = {0b01111010, 0, 0, 0, 0, 0b01110101};
uint8_t cmd55[] = {0b01110111, 0, 0, 0, 0, 1};
/* hcs = 1 to control sdhc */
uint8_t acmd41[] = {0b01101001, 0x40, 0, 0, 0, 1};

typedef uint8_t bool;
typedef uint8_t uint4_t;
typedef uint8_t ResponseR1;

typedef struct {
    uint8_t voltage_range;
    bool S18A;
    bool UHSII_CardStatus;
    bool CCS;
    bool BSY;
} OCR;

typedef struct {
    ResponseR1 R1;
    OCR OCRValue;
} ResponseR3;

typedef struct {
    ResponseR1 R1;
    uint4_t command_version;
    uint4_t voltage_accepted;
    uint8_t check_pattern;
} ResponseR7;


void init_sd(void) {
    volatile uint8_t *resp;

    /* Toggle CLK for 96 cycles (at least 74) */
    spi_send(blank);
    spi_send(blank);

    /* Spam CMD0 until there's a response */
    do {
        spi_send(cmd0);
        spi_send(blank);
        resp = spi_read();
    } while (*resp == 0xFF);

    /* Send CMD8, check if Illegal Command in responce */
    spi_send(cmd8);
    spi_send(blank);
    resp = spi_read();

    if (*resp & (1 << 2)) {
        /* Illegal command -> Ver 1.X SD card or not a SD card */
        spi_send(cmd58);
        spi_send(blank);
        resp = spi_read();
        if ((*(resp + 3) & 0xF) != 0b0001) while (1) { /* Voltage wrong */}
        if (*resp & (1 << 2)) {
            /* Illegal command again -> Not SD card */
            while (1) { /* Not a SD Memory card */ }
        }
    } else {
        /* No Illegal command in reponce -> Ver >= 2.0 SD card
         * Check if CRC error or non-compatible voltage range
         */
        if (*resp & (1 << 3)) while (1) { /* Card unusable */ }
        if ((*(resp + 3) & 0xF) != 0b0001) while (1) { /* Voltage wrong */}
    }

    /* Spam ACMD41 until SD card's not busy (so it's ready)
     * preceed ACMD41 with CMD55 to inform SD that the next
     * command is special; ACMD41 starts SD initialization process
     *
     * ref: p.227 PhysicalLayerSimplifiedSpecification, p.7 lec12
     */
    do {
        spi_send(cmd55);
        spi_send(acmd41);
        spi_send(blank);
        resp = spi_read();
    } while ((*resp & 1) == 0);

    /* Get CCS */
    spi_send(cmd58);
    spi_send(blank);
    resp = spi_read();

    /* ccs=0: ver 2.0 sd standard capacity, ccs=1: ver 2.0, sdhc/sdxc */
    if (*(resp + 1) & 0b01000000) {
        /* Version >= 2.00 SDHC / SDXC memory card */
    } else {
        /* Version >= 2.00 SD Standard Capacity card */
    }
}

