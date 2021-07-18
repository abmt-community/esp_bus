/*
 * udp_receive.cpp
 *
 *  Created on: 12.06.2020
 *      Author: hva
 */

#include "udp_receive.h"
#include <lwip/udp.h>

#include "pwm.h"

#include "esp_log.h"

int time_out_counter = 0;
udp_pcb* pcb; // control block
uint16_t port = 3334;

void udp_recv_helper(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port){
	//
	if(p->len == 4*6){
		float* data = (float*) p->payload;
		pin_1->set_duty_cyle(data[0]);
		pin_2->set_duty_cyle(data[1]);
		pin_3->set_duty_cyle(data[2]);
		pin_4->set_duty_cyle(data[3]);
		led_red->set_duty_cyle(data[4]);
		led_flash->set_duty_cyle(data[5]);
		time_out_counter = 3;
	}else{
		ESP_LOGI("rcv","%s", (char*)p->payload);
	}
	pbuf_free(p);
}

void time_out_task(void *){
	while(true){
		vTaskDelay(50);
		if(time_out_counter > 0){
			--time_out_counter;
		}else{
			pin_1->set_duty_cyle(0);
			pin_2->set_duty_cyle(0);
			pin_3->set_duty_cyle(0);
			pin_4->set_duty_cyle(0);
			led_red->set_duty_cyle(0);
			led_flash->set_duty_cyle(0);
		}
	}
}

void udp_receive_init(){
	pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
	udp_bind(pcb, IP_ADDR_ANY, port);
	udp_recv(pcb, udp_recv_helper, NULL);
	TaskHandle_t xHandle = NULL;
	xTaskCreate(time_out_task, "time_out_task", 2000, ( void * ) 1,tskIDLE_PRIORITY, &xHandle);
}

