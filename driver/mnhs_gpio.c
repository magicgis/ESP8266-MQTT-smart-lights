/*
 * mnhs_gpio.c
 *
 *  Created on: 4. 11. 2015
 *      Author: Tom
 */
#include "c_types.h"
#include <gpio.h>
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include "user_interface.h"

bool io_toggle(uint8 pin, uint8 value);
void initialize_gpio(void);

void initialize_gpio(void) {
	gpio_init();

	//gpio1 id:2
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

	//gpio2 id:4
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);

	//gpio3 id:13
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);

	//gpio4 id:14
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);

	//gpio_output_set(0, 0, (1 << 2), (1 << 0));

	io_toggle(2, 0);
	io_toggle(4, 0);
	io_toggle(13, 0);
	io_toggle(14, 0);

}

bool io_toggle(uint8 pin, uint8 value) {
	if (value < 0 || value > 1)
		return false;

	switch (pin) {
	case 1:
		GPIO_OUTPUT_SET(2, value);
		break;
	case 2:
		GPIO_OUTPUT_SET(4, value);
		break;
	case 3:
		GPIO_OUTPUT_SET(13, value);
		break;
	case 4:
		GPIO_OUTPUT_SET(14, value);
		break;
	default:
		return false;
		break;
	}
	return true;

}
