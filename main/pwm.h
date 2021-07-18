/*
 * pwm.h
 *
 *  Created on: 05.06.2020
 *      Author: hva
 */

#ifndef MAIN_PWM_H_
#define MAIN_PWM_H_

// led pin 9 -> gpio33

#include "driver/ledc.h"

class pwm{
public:
	ledc_channel_config_t channel_conf = {};
	ledc_timer_config_t timer_conf = {};


	pwm(uint8_t pin, uint8_t channel = LEDC_CHANNEL_0, uint32_t freq_hz = 20000);
	void set_duty_cyle(float value);

};

extern pwm* led_red;
extern pwm* led_flash;
extern pwm* pin_1;
extern pwm* pin_2;
extern pwm* pin_3;
extern pwm* pin_4;

void init_pwm();

#endif /* MAIN_PWM_H_ */
