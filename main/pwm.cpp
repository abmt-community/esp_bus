/*
 * pwm.cpp
 *
 *  Created on: 05.06.2020
 *      Author: hva
 */


#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include <math.h>

#include "pwm.h"


pwm::pwm(uint8_t pin, uint8_t channel, uint32_t freq_hz){

	if(GPIO_IS_VALID_GPIO(pin) == false){
		ESP_LOGE("pwm_error","pin_number (%d) invalid. ", pin);
		return;
	}

	if(channel > 8){
		ESP_LOGW("pwm_error","selected chanel > 8; using 8...");
		channel = 8;
	}

	channel_conf.gpio_num = pin;
	channel_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	channel_conf.channel = (ledc_channel_t) channel;
	channel_conf.intr_type = LEDC_INTR_DISABLE;
	channel_conf.timer_sel = LEDC_TIMER_1;
	channel_conf.duty = 0;

	timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	timer_conf.duty_resolution = ledc_timer_bit_t::LEDC_TIMER_10_BIT;
	timer_conf.timer_num = LEDC_TIMER_1;
	timer_conf.freq_hz = freq_hz;

	ESP_ERROR_CHECK( ledc_channel_config(&channel_conf) );
	ESP_ERROR_CHECK( ledc_timer_config(&timer_conf) );
}

void pwm::set_duty_cyle(float value){
	if(value > 1){
		value = 1;
	}else if(value < 0){
		value = 0;
	}
	uint32_t max_duty = (1 << timer_conf.duty_resolution) - 1;
	uint32_t duty = lroundf(value * (float)max_duty);

	ESP_ERROR_CHECK( ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel_conf.channel, duty) );
	ESP_ERROR_CHECK( ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel_conf.channel) );
}


pwm* led_red = 0;
pwm* led_flash = 0;
pwm* pin_1;
pwm* pin_2;
pwm* pin_3;
pwm* pin_4;


void init_pwm(){
	if(led_red){
		delete led_red;
	}
	led_red = new pwm(33,7);
	led_red->set_duty_cyle(0);

	if(led_flash){
		delete led_flash;
	}
	led_flash = new pwm(4,6);
	led_flash->set_duty_cyle(0);

	if(pin_1){
		delete pin_1;
	}
	pin_1 = new pwm(12,2);
	pin_1->set_duty_cyle(0);

	if(pin_2){
		delete pin_2;
	}
	pin_2 = new pwm(13,3);
	pin_2->set_duty_cyle(0);

	if(pin_3){
		delete pin_3;
	}
	pin_3 = new pwm(14,4);
	pin_3->set_duty_cyle(0);

	if(pin_4){
		delete pin_4;
	}
	pin_4 = new pwm(15,5);
	pin_4->set_duty_cyle(0);
}


