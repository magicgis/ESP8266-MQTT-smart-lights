/* main.c -- MQTT client example
*
* Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"
#include "driver/mnhs_gpio.h"

MQTT_Client mqttClient;
LOCAL os_timer_t heartbeat_timer;
uint8 hbnum = 0;
int qos =1;

#define HEARTBEAT_DELAY 2000

void wifiConnectCb(uint8_t status)
{
	if(status == STATION_GOT_IP){
		MQTT_Connect(&mqttClient);
	} else {
		MQTT_Disconnect(&mqttClient);
	}
}

void heartbeat_packet(void *arg) {
	char topic[40];
	os_sprintf(topic, "/ESP8266/%X/HEARTBEAT", system_get_chip_id());

	char msg[100];
	os_sprintf(msg, "%X:%d:%d", system_get_chip_id(), hbnum++,wifi_station_get_rssi());
	ets_uart_printf(msg);


	MQTT_Publish(&mqttClient, topic, msg, strlen(msg), qos, 0);
	MQTT_Publish(&mqttClient, "/ESP8266/ALL/HEARTBEAT", msg,strlen(msg), qos, 0);
}


void mqttConnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Connected\r\n");

	char topic[40];
	os_sprintf(topic, "/ESP8266/%X/GPIO", system_get_chip_id());

	MQTT_Subscribe(client, topic, qos);
	MQTT_Subscribe(client, "/ESP8266/ALL/GPIO", qos);

	os_timer_disarm(&heartbeat_timer);
	os_timer_setfn(&heartbeat_timer, (os_timer_func_t *) heartbeat_packet,
					(void *) 0);
			os_timer_arm(&heartbeat_timer, HEARTBEAT_DELAY, 2);

}

void mqttDisconnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Disconnected\r\n");
	os_timer_disarm(&heartbeat_timer);
}

void mqttPublishedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Published\r\n");
}


int8 count_char(char* str, char c) {
	uint8 index = 0;
	uint8 count = 0;
	while (str[index] != '\0') {
		if (str[index++] == c) {
			count++;
		}
	}
	return count;
}

bool process_data(char* topic,char* data, int token_count) {
	//INFO("%s %s %u\r\n",topic,data,token_count);
	if (!strcmp(topic, "ESP8266")&&token_count==3) {
		topic = strtok(NULL, "/");
		topic = strtok(NULL, "/");

		if (!strcmp(topic, "GPIO")) {
			if(strlen(data)==2){
				int pin = data[0]-48;
				int value = data[1]-48;
				if (io_toggle(pin, value)) {
					return true;
				}
				/*
				if(pin>0&&pin<5){
					if(value>=0&&value<=1){
						INFO("=> set pin %d to %d\r\n",pin,value);
						return true;
					}
				}
				*/
			}
		}

	}
	return false;
}


void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
	char *topicBuf = (char*)os_zalloc(topic_len+1),
			*dataBuf = (char*)os_zalloc(data_len+1);

	MQTT_Client* client = (MQTT_Client*)args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

	//INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);


	//GPIO
	uint8 count = count_char(topicBuf, '/');

	process_data(strtok(topicBuf, "/"),dataBuf, count);



	os_free(topicBuf);
	os_free(dataBuf);
}

void user_rf_pre_init(void)
{
}

void user_init(void)
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_delay_us(1000000);

	CFG_Load();
	system_update_cpu_freq(160);
	initialize_gpio();

	MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.security);
	//MQTT_InitConnection(&mqttClient, "192.168.11.122", 1880, 0);

	MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user, sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);
	//MQTT_InitClient(&mqttClient, "client_id", "user", "pass", 120, 1);

	MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);

	WIFI_Connect("CC-P","lasi4ka-4esk8", wifiConnectCb);

	INFO("\r\nSystem started ...\r\n");
}
