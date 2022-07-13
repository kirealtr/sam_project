
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <delay.h>
#include "asf.h"
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "spi.h"

/** Reference voltage for AFEC,in mv. */
#define VOLT_REF        (3300)

/** The maximal digital value */
#define MAX_DIGITAL     (4095UL)

#define channel_1 AFEC_CHANNEL_5
#define channel_2 AFEC_CHANNEL_4

#define period 0.01
#define freq 100000

/* Creating PIO pins */
#define V_pin IOPORT_CREATE_PIN(PIOD, 17)
#define sound_pin IOPORT_CREATE_PIN(PIOD, 28)

/* Pins to communicate with RPi, black wire connects GND */
#define GO_pin IOPORT_CREATE_PIN(PIOC, 31) // red wire
#define resp_pin IOPORT_CREATE_PIN(PIOC, 30) // yellow wire

/*	AFE0_AD0 - PA17
	AFE0_AD1 - PA18
	AFE0_AD2 - PA19
	AFE0_AD3 - PA20
	AFE0_AD4 - PB0
	AFE0_AD5 - PB1  */

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

#define SPI_SLAVE_BASE SPI

#define data_size 25000
uint16_t data[2][data_size];
volatile uint32_t i = 0;

volatile bool buffer_full = false;
volatile bool GO_status = false;

typedef enum{
	SL_READY = 0,
	SL_SAMPLING,
	SL_WRITING,
	} sl_state_t;
	
static sl_state_t state;

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
	spi_enable_interrupt(SPI_SLAVE_BASE, SPI_IER_TDRE);
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
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

static void get_data(void)
{	
	if (i < data_size) {
		afec_start_software_conversion(AFEC0);
		data[0][i] = afec_channel_get_value(AFEC0, channel_1);
		data[1][i] = afec_channel_get_value(AFEC0, channel_2);
		i++;
	}
	else {
		buffer_full = true;
	}
}

void TC0_Handler(void)
{
	volatile uint32_t ul_dummy;

	/* Clear status bit to acknowledge interrupt */
	ul_dummy = tc_get_status(TC0, 0);

	/* Avoid compiler warning */
	UNUSED(ul_dummy);
	
	/** Measure voltage. */
	get_data();
}

/* Configure Timer Counter 0 to generate an interrupt every (period) ms. */
static void configure_tc(void)
{
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	/* Configure PMC */
	pmc_enable_periph_clk(ID_TC0);

	/** Configure TC for a (freq) Hz frequency and trigger on RC compare. */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC0, 0, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC0, 0, (ul_sysclk / ul_div) / freq);

	/* Configure and enable interrupt on RC compare */
	NVIC_EnableIRQ((IRQn_Type) ID_TC0);
	tc_enable_interrupt(TC0, 0, TC_IER_CPCS);
}

static void configure_channel(int chan)
{
	afec_channel_enable(AFEC0, chan);
	struct afec_ch_config afec_ch_cfg;
	afec_ch_get_config_defaults(&afec_ch_cfg);
	afec_ch_set_config(AFEC0, chan, &afec_ch_cfg);
	afec_channel_set_analog_offset(AFEC0, chan, 0x800);
}
/*
static void mk_sound(void)
{	
	// Dynamic is connected to PC17 pin
	REG_PIOC_PER |= PIO_PER_P17;
	REG_PIOC_OER |= PIO_PER_P17;
	REG_PIOC_SODR |= PIO_PER_P17;
	delay_us(20);
	REG_PIOC_CODR |= PIO_PER_P17;
} */

int main(void)
{
	/* Initialize the SAM system. */
	sysclk_init();
	board_init();
	ioport_init();

	configure_console();

	afec_enable(AFEC0);

	struct afec_config afec_cfg;

	afec_get_config_defaults(&afec_cfg);

	afec_init(AFEC0, &afec_cfg);
	
	configure_tc();
	
	configure_channel(channel_1);
	configure_channel(channel_2);
	
	/* Applying voltage from PD17 pin to V contact of potentiometer */
	ioport_set_pin_dir(V_pin, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(V_pin, true);
	
	/* Configuring PIO */
	ioport_set_pin_dir(GO_pin, IOPORT_DIR_INPUT);
	ioport_set_pin_dir(sound_pin, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(resp_pin, IOPORT_DIR_OUTPUT);
	
	/* Response pin will be on high level when sampling is done */
	ioport_set_pin_level(resp_pin, false);

	state = SL_READY;

	while (1) {
		switch (state){
			case SL_READY:
				GO_status = ioport_get_pin_level(GO_pin);
				if (GO_status)
					state = SL_SAMPLING;
				break;
			case SL_SAMPLING:
				tc_start(TC0, 0);
				//mk_sound();
				if (buffer_full) {
					ioport_set_pin_level(resp_pin, 1);
					state = SL_WRITING;
				}
				break;
			case SL_WRITING:
				
				
				break;
		}
	}
}
