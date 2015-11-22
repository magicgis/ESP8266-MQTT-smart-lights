/*
 * mnhs_sensor_board.c
 *
 *  Created on: 23. 11. 2015
 *      Author: Tom
 */


#include "c_types.h"
#include <gpio.h>
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include "user_interface.h"




void sensors_init(){
	i2c_master_gpio_init();
	configure_OPT3001();
	configure_MPL3115();
}
