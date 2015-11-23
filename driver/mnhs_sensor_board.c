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
#include "../include/driver/mnhs_sensor_board.h"


#define DELAY 1000
#define OPT3001_address 0x44
#define MPL3115_address 0x60

LOCAL os_timer_t collect_timer;

int Illumination=0;
int Pressure=0;
float Temperature=0;

bool i2c_address(uint8 address,uint8 dir){
	//direction 0 transmitter 1 receiver
	i2c_master_writeByte((uint8_t) ((address << 1) | dir ));
	return !i2c_master_getAck();
}

bool i2c_write(uint8 byte){
	i2c_master_writeByte(byte);
	return !i2c_master_getAck();
}

uint8 i2c_read(bool ack){
	uint8 value=i2c_master_readByte();
	if(ack){
		i2c_master_send_ack();
	} else {
		i2c_master_send_nack();
	}
	return value;
}

void read_OPT3001(){
	uint16_t exponent = 0;
	uint32_t result = 0;
	int16_t raw;


	i2c_master_start();
	i2c_address(OPT3001_address,0);
	i2c_write(0x00);
		//i2c_master_stop();

	i2c_master_start();
	i2c_address(OPT3001_address,1);
	int msb=i2c_read(true);
	int lsb=i2c_read(false);
	i2c_master_stop();

	raw = ((msb << 8) | lsb);

	result = raw&0x0FFF;
	exponent = (raw>>12)&0x000F;

	switch(exponent){
		case 0: //*0.015625
			Illumination = result>>6;
			break;
		case 1: //*0.03125
			Illumination = result>>5;
			break;
		case 2: //*0.0625
			Illumination = result>>4;
			break;
		case 3: //*0.125
			Illumination = result>>3;
			break;
		case 4: //*0.25
			Illumination = result>>2;
			break;
		case 5: //*0.5
			Illumination = result>>1;
			break;
		case 6:
			Illumination = result;
			break;
		case 7: //*2
			Illumination = result<<1;
			break;
		case 8: //*4
			Illumination = result<<2;
			break;
		case 9: //*8
			Illumination = result<<3;
			break;
		case 10: //*16
			Illumination = result<<4;
			break;
		case 11: //*32
			Illumination = result<<5;
			break;
	}
}

void read_MPL3115(){

	i2c_master_start();
	i2c_address(MPL3115_address,0);
	i2c_write(0x26);

	i2c_master_start();
	i2c_address(MPL3115_address,1);
	uint8 tm=i2c_read(false);
	i2c_master_stop();

	i2c_master_start();
	i2c_address(MPL3115_address,0);
	i2c_write(0x06);

	i2c_master_start();
	i2c_address(MPL3115_address,1);
	uint8 a6=i2c_read(false);
	i2c_master_stop();

	i2c_master_start();
	i2c_address(MPL3115_address,0);
	i2c_write(0x01);

	i2c_master_start();
	i2c_address(MPL3115_address,1);
	uint8 pmsb=i2c_read(true);
	uint8 pcsb=i2c_read(true);
	uint8 plsb=i2c_read(true);


	uint8 msb=i2c_read(true);
	uint8 lsb=i2c_read(false);
	i2c_master_stop();

	Pressure = (pmsb<<16|pcsb<<8|plsb)/64;
	Temperature = ((float)(msb<<8|lsb)/256);

	i2c_master_start();
	i2c_address(MPL3115_address,0);
	i2c_write(0x26);
	i2c_write(0b00100001);
	i2c_master_stop();


}

void configure_OPT3001(){
	i2c_master_start();
	i2c_address(OPT3001_address,0);
	i2c_write(0x01);
	i2c_write(0xC4);
	i2c_write(0x10);
	i2c_master_stop();
}

void configure_MPL3115(){
	i2c_master_start();
	i2c_address(MPL3115_address,0);
	i2c_write(0x05);
	i2c_write(0x00);
	i2c_master_stop();

	i2c_master_start();
	i2c_address(MPL3115_address,0);
	i2c_write(0x26);
	i2c_write(0b00100011);
	i2c_master_stop();

	i2c_master_start();
	i2c_address(MPL3115_address,0);
	i2c_write(0x26);
	i2c_write(0b00100001);
	i2c_master_stop();
}

void collect_data(){
	read_OPT3001();
	read_MPL3115();
}

int getPressure(){
	return Pressure;
}

int getIllumination(){
	return Illumination;
}

float getTemperature(){
	return Temperature;
}

void sensors_init(){
	i2c_master_gpio_init();
	configure_OPT3001();
	configure_MPL3115();

	//run timer
	os_timer_disarm(&collect_timer);
	os_timer_setfn(&collect_timer, (os_timer_func_t *) collect_data, (void *) 0);
	os_timer_arm(&collect_timer, DELAY, 1);
}
