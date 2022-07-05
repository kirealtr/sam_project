
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <delay.h>
#include "asf.h"

/** Reference voltage for AFEC,in mv. */
#define VOLT_REF        (3300)

/** The maximal digital value */
#define MAX_DIGITAL     (4095UL)

#define channel_1 AFEC_CHANNEL_5
#define channel_2 AFEC_CHANNEL_3

#define period 10
#define freq 100

/*
volatile uint16_t data_1;
volatile uint16_t data_2; */

#define data_size 1024
uint16_t data[2][data_size];
volatile uint32_t i = 0;

static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
#endif
		.paritytype = CONF_UART_PARITY,
#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
#endif
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}


static void get_data(void)
{	
	afec_start_software_conversion(AFEC0);
	data[0][i] = afec_channel_get_value(AFEC0, channel_1);
	data[1][i] = afec_channel_get_value(AFEC0, channel_2);
	i++;
	if (i > 1023) {
//		data[0][0] = 0;
		i = 0;
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
	
	tc_start(TC0, 0);
}


int main(void)
{
	
	/* Initialize the SAM system. */
	sysclk_init();
	board_init();

	configure_console();

	afec_enable(AFEC0);

	struct afec_config afec_cfg;

	afec_get_config_defaults(&afec_cfg);

	afec_init(AFEC0, &afec_cfg);
	
	configure_tc();
	
	afec_channel_enable(AFEC0, channel_1);
	afec_channel_enable(AFEC0, channel_2);
	struct afec_ch_config afec_ch_cfg;
	afec_ch_get_config_defaults(&afec_ch_cfg);
	afec_ch_set_config(AFEC0, channel_1, &afec_ch_cfg);
	afec_ch_set_config(AFEC0, channel_2, &afec_ch_cfg);
	afec_channel_set_analog_offset(AFEC0, channel_1, 0x800);
	afec_channel_set_analog_offset(AFEC0, channel_2, 0x800);
	
	REG_PIOD_PER |= PIO_PER_P17;
	REG_PIOD_OER |= PIO_PER_P17;
	REG_PIOD_SODR |= PIO_PER_P17;
	
/*	AFE0_AD0 - PA17
	AFE0_AD1 - PA18
	AFE0_AD2 - PA19
	AFE0_AD3 - PA20
	AFE0_AD4 - PB0
	AFE0_AD5 - PB1  */



	while (1);
}
