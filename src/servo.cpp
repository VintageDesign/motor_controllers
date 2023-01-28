//
// Created by riley on 11/5/20.
//

#include "servo.h"

Servo::Servo() {
    this->m_current_angle = 0;
    this->m_max_angle = 180;
    this->m_min_angle = 0;
    this->m_freqHz = 50;

}

Servo::~Servo() {
    // Do nothing
}

void Servo::attach(uint8_t pin) {
    ledc_timer_config_t timer_conf;
    timer_conf.duty_resolution 	= LEDC_TIMER_15_BIT;
    timer_conf.freq_hz    		= m_freqHz;
    timer_conf.speed_mode 		= LEDC_HIGH_SPEED_MODE;
    timer_conf.timer_num  		= LEDC_TIMER_0;
    timer_conf.clk_cfg = LEDC_USE_APB_CLK;
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t ledc_conf;
    ledc_conf.channel		= LEDC_CHANNEL_0;
    ledc_conf.duty			= 0;
    ledc_conf.hpoint        = 0;
    ledc_conf.gpio_num		= (gpio_num_t) pin;
    ledc_conf.intr_type		= LEDC_INTR_DISABLE;
    ledc_conf.speed_mode	= LEDC_HIGH_SPEED_MODE;
    ledc_conf.timer_sel		= LEDC_TIMER_0;
    ledc_channel_config(&ledc_conf);
}

void Servo::detach() {
   // ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, get_duty_by_uS(this->map(m_current_angle)));
   ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
}

void Servo::write(uint8_t angle) {
    uint32_t write_val = angle;
    if(this->m_min_angle <= write_val &&  write_val <= m_max_angle)
    {
        // Map the values to the correct micro-seconds val
        write_micro_seconds(this->map(write_val));
        this->m_current_angle = write_val;
    }
}

uint32_t Servo::map(uint32_t angle)
{
    uint32_t retval = 0;
    retval = (angle - 0) * (2500 - 500) / (180 - 0) + 400;
    return retval;
}

void Servo::write_micro_seconds(uint32_t value) {
    uint32_t current_angle = this->map(this->m_current_angle);
    uint8_t delay_time = 3;

    // If we are larger than the target then we need to decrement
    if(value < current_angle)
    {
        for(uint32_t angle = current_angle; angle >= value; angle--)
        {
            this->set_clock(angle);
            vTaskDelay(delay_time / portTICK_PERIOD_MS);
        }
    }
    else{
        for(uint32_t angle = current_angle; angle <= value; angle++)
        {
            this->set_clock(angle);
            vTaskDelay(delay_time / portTICK_PERIOD_MS);
        }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);


}

void Servo::set_clock(uint32_t value)
{
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, get_duty_by_uS(value));
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}

double Servo::get_duty_by_percentage(double percentage)
{
    if (percentage <= 0){
        return 0;
    }
    if (percentage > 100){
        percentage = 100;
    }
    return (percentage / 100.0) * ((2<<14)-1); // LEDC_TIMER_15_BIT
}

double Servo::get_duty_by_uS(double uS)
{
    return this->get_duty_by_percentage(((uS * 100.0)/(1000000/m_freqHz)));
}


uint8_t Servo::get_current_angle() {
    return this->m_current_angle;
}