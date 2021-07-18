/*
 * cam2udp.cpp
 *
 *  Created on: 24.05.2020
 *      Author: hva
 */

#include "cam2udp.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include <lwip/udp.h>
#include <esp_camera.h>
#include "udp_receive.h"

#include "mdns.h"


static const char* TAG = "cam:udp";
udp_pcb* echo_pcb;


// Wichtig: von einem Thread aus initialisieren
struct udp_sender{
	udp_pcb* pcb; // control block
	ip_addr_t target;
	uint16_t port = 3333;

	udp_sender(){
		pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
		ipaddr_aton("192.168.1.10",&target);
		udp_bind(pcb, IP_ADDR_ANY, 3333);
	}

	void send(const void* d, uint16_t size){
		char* data = (char*)d;
		pbuf* p = pbuf_alloc(PBUF_TRANSPORT,size,PBUF_POOL);
		if(p == 0){
			ESP_LOGI("udp_sender","Error allocatin pbuf");
			return;
		}
		size_t bytes_to_send = size;
		size_t pos = 0;
		pbuf* p_work = p;
		while(p_work != 0 && bytes_to_send > 0){
			size_t bytes_to_copy = p_work->len;
			if(bytes_to_send < bytes_to_copy){
				bytes_to_copy = bytes_to_send;
			}
			memcpy(p_work->payload, data + pos, bytes_to_copy);
			bytes_to_send -= bytes_to_copy;
			pos += bytes_to_copy;
			p_work = p_work->next;
		}
		udp_sendto(pcb,p,&target,port);
		pbuf_free(p);
	}
};


void cam2udp_task(void* param){
	const TickType_t xDelay = 5000 / portTICK_PERIOD_MS;
	udp_sender s;
	udp_receive_init();

	mdns_init();
	mdns_hostname_set("esp_bus");


	static int cnt = 0;

	vTaskDelay( xDelay );
	while(true){
		vTaskDelay(5 / portTICK_PERIOD_MS);

		auto fb = esp_camera_fb_get();
		if(!fb){
			ESP_LOGE(TAG, "Camera capture failed");
			continue;
		}
		s.send(fb->buf, fb->len);
		cnt++;
		if(cnt > 100){
			cnt = 0;
		}
		esp_camera_fb_return(fb);

	}
}


