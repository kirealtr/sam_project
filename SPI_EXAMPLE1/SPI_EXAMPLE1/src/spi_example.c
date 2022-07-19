/*
 * \mainpage SPI Example
 *
 * \par Purpose
 *
 * This example uses Serial Peripheral Interface (SPI) of one EK board in
 * slave mode to communicate with another EK board's SPI in master mode.
 *
 * \par Requirements
 *
 * This package can be used with two SAM evaluation kits boards.
 * Please connect the SPI pins from one board to another.
 * \copydoc spi_example_pin_defs
 *
 * \par Descriptions
 *
 * This example shows control of the SPI, and how to configure and
 * transfer data with SPI. By default, example runs in SPI slave mode,
 * waiting SPI slave & UART inputs.
 *
 * The code can be roughly broken down as follows:
 * <ul>
 * <li> 't' will start SPI transfer test.
 * <ol>
 * <li>Configure SPI as master, and set up SPI clock.
 * <li>Send 4-byte CMD_TEST to indicate the start of test.
 * <li>Send several 64-byte blocks, and after transmitting the next block, the
 * content of the last block is returned and checked.
 * <li>Send CMD_STATUS command and wait for the status reports from slave.
 * <li>Send CMD_END command to indicate the end of test.
 * </ol>
 * <li>Setup SPI clock for master.
 * </ul> */

#include "asf.h"
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "conf_spi_example.h"

/*  GPIO11 - CLK (red) - PA14
	GPIO10 - MOSI (not used)
	GPIO9 - MISO (brown) - PA12
	GPIO8 - NSS (yellow) - PA11
	  */

/* Chip select. */
#define SPI_CHIP_SEL 0
#define SPI_CHIP_PCS spi_get_pcs(SPI_CHIP_SEL)

/* Clock polarity. */
#define SPI_CLK_POLARITY 0

/* Clock phase. */
#define SPI_CLK_PHASE 0

/* Number of commands logged in status. */
#define NB_STATUS_CMD   20

/* UART baudrate. */
#define UART_BAUDRATE      115200

#define bits_per_transfer SPI_CSR_BITS_16_BIT

volatile uint32_t i = 0;
static uint16_t buffer[256];
volatile uint8_t counter = 0;

/**
 * \brief Initialize SPI as slave.
 */
static void spi_slave_initialize(void)
{	
	/* Configure an SPI peripheral. */
	spi_enable_clock(SPI_SLAVE_BASE);

	spi_disable(SPI_SLAVE_BASE);
	spi_reset(SPI_SLAVE_BASE);
	spi_set_slave_mode(SPI_SLAVE_BASE);
	spi_disable_mode_fault_detect(SPI_SLAVE_BASE);
	spi_set_peripheral_chip_select_value(SPI_SLAVE_BASE, SPI_CHIP_PCS);
	spi_set_clock_polarity(SPI_SLAVE_BASE, SPI_CHIP_SEL, SPI_CLK_POLARITY);
	spi_set_clock_phase(SPI_SLAVE_BASE, SPI_CHIP_SEL, SPI_CLK_PHASE);
	spi_set_bits_per_transfer(SPI_SLAVE_BASE, SPI_CHIP_SEL, bits_per_transfer);
	spi_enable_interrupt(SPI_SLAVE_BASE, SPI_IER_NSSR);
	spi_enable(SPI_SLAVE_BASE);
	
}


static void spi_slave_transfer(void)
{
	if(i <= 4) {
		spi_write(SPI_SLAVE_BASE, buffer[i], 0, 0);
		i++; 
	}
	else {
		i = 0;
	}
}


void SPI_Handler(void)
{	
	spi_slave_transfer();
}


static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.paritytype = CONF_UART_PARITY,
	};

	/* Configure console UART. */
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

/**
 * \brief Application entry point for SPI example.
 *
 * \return Unused (ANSI-C compatibility).
 */
int main(void)
{
	/* Initialize the SAM system. */
	sysclk_init();
	board_init();

	/* Initialize the console UART. */
	configure_console();
	
	
	for(int j = 0; j <= 4; j++)
	{
		buffer[j] = j * 1000;
	}
	
	spi_slave_initialize();
	
	NVIC_DisableIRQ((IRQn_Type) ID_TC0);

	NVIC_SetPriority(SPI_IRQn, 0);
	NVIC_EnableIRQ(SPI_IRQn);

	NVIC_DisableIRQ(SPI_IRQn);
	NVIC_EnableIRQ(SPI_IRQn);
	while (1);
}

