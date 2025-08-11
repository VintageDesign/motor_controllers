#include "stepper.h"

#include <math.h>

#include <esp_log.h>

Stepper::Stepper(uint8_t step_pin, uint8_t dir_pin, uint8_t en_pin, micro_stepping_t microsteps, uint16_t steps_per_rotation):
m_step_pin(step_pin),
    m_dir_pin(dir_pin),
    m_en_pin(en_pin),
    m_microsteps(microsteps),
    m_steps_per_rotation(steps_per_rotation)
{
    uint64_t gpio_mask = ( 1ULL << m_step_pin 
                         | 1ULL << m_dir_pin
                         | 1ULL << m_en_pin);
    gpio_config_t gpio_conf = {
        .pin_bit_mask = gpio_mask,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&gpio_config));

    gptimer_config_t timer_conf =
    {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_F,
    };

    timer_conf.intr_shared = false;

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_conf, &m_timer_handle)


    gptimer_event_callbacks_t cb_group;
    cb_group.on_alarm = xISRwrap;
    alarm_cfg.flags.auto_reload_on_alarm = 1;
    gptimer_register_event_callbacks(m_timer_handle, &cb_group, this);

}

/* Timer callback, used for generating pulses and calculating speed profile in real time */
bool Stepper::xISR(gptimer_t *timer, const gptimer_alarm_event_data_t *data)
{
    GPIO.out_w1ts = (1ULL << m_step_pin);


    // update current position
    if (m_dir == CW)
    {
        m_current_pos_steps++;
    }
    else
    {
        m_current_pos_steps--;
    }

    // we are done
    if (m_steps_to_go == m_step_count)
    {
        gptimer_stop(m_timer_handle); // stop the timer
        m_step_count = 0;
        gptimer_disable(m_timer_handle);
        GPIO.out_w1tc = (1ULL << m_step_pin);
        return 0;
    }

    // TODO add acceleration

    m_step_interval = TIMER_F / m_speed_rad_per_sec;

    // set alarm to calculated interval and disable pin
    GPIO.out_w1tc = (1ULL << m_step_pin);
    m_alarm_cfg.alarm_count = m_step_interval;

    gptimer_set_alarm_action(m_timer_handle, &alarm_cfg);
    return 1;
}

