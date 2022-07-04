
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

volatile uint16_t data_1;
volatile uint16_t data_2;

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
	
	afec_channel_enable(AFEC0, channel_1);
	afec_channel_enable(AFEC0, channel_2);
	struct afec_ch_config afec_ch_cfg;
	afec_ch_get_config_defaults(&afec_ch_cfg);
	afec_ch_set_config(AFEC0, channel_1, &afec_ch_cfg);
	afec_ch_set_config(AFEC0, channel_2, &afec_ch_cfg);
	afec_channel_set_analog_offset(AFEC0, channel_1, 0x800);
	afec_channel_set_analog_offset(AFEC0, channel_2, 0x800);
	
	/*ioport_set_pin_dir(PIO_PD17, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIO_PD17, IOPORT_PIN_LEVEL_HIGH); */
	
	REG_PIOD_PER |= PIO_PER_P17;
	REG_PIOD_OER |= PIO_PER_P17;
	REG_PIOD_SODR |= PIO_PER_P17;
	
//	REG_PIOB_PER |= PIO_PER_P1;
//	REG_PIOB_ODR |= PIO_PER_P1;
	
	
//	ioport_set_pin_dir(PIO_PB1, IOPORT_DIR_INPUT);
	
/*	AFE0_AD0 - PA17
	AFE0_AD1 - PA18
	AFE0_AD2 - PA19
	AFE0_AD3 - PA20
	AFE0_AD4 - PB0
	AFE0_AD5 - PB1  */

	while (1) {
			afec_start_software_conversion(AFEC0);
			data_1 = afec_channel_get_value(AFEC0, channel_1);
			data_2 = afec_channel_get_value(AFEC0, channel_2);
			delay_ms(500);
			
	}
}
