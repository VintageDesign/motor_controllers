#ifndef STEPPER_H
#define STEPPER_H

#include "esp_log.h"
#include <driver/gpio.h>
#include <driver/gptimer.h>


enum motor_status
{
    DISABLED,
    IDLE,
    ACC,
    COAST,
    DEC
};

enum dir
{
    CW,
    CCW
};

typedef enum {
    MICROSTEP_1 = 0x1,
    MICROSTEP_2 = 0x2,
    MICROSTEP_4 = 0x4,
    MICROSTEP_8 = 0x8,
    MICROSTEP_16 = 0x10,
    MICROSTEP_32 = 0x20,
    MICROSTEP_64 = 0x40,
    MICROSTEP_128 = 0x80,
    MICROSTEP_256 = 0x100,
} micro_stepping_t;


class Stepper
{
    public:
    Stepper();
    ~Stepper();

    void attach(uint8_t step_pin, uint8_t dir_pin, uint8_t en_pin, micro_stepping_t microsteps, uint16_t steps_per_rotation);
    void detach();

    eps_err_t write_rad(int32_t relative_rad);
    eps_err_t write_absolute_rad(uint32_t absolute_rad);
    
    void set_speed(uitn32_t speed_rad_per_sec, uint16_t acceleration_ms, uint16_t decelleration_ms);

    private:

    void set_en(bool en_state);
    void set_dir(bool dir_state);


	static bool xISRwrap(gptimer_t* timer, const gptimer_alarm_event_data_t* data,void *_this)
	{
		return static_cast<Stepper *>(_this)->xISR(timer,data);
	}

	static void _disableMotor(void *_this)
	{
		static_cast<Stepper *>(_this)->disableMotor();
	}

	bool xISR(gptimer_t* timer, const gptimer_alarm_event_data_t* data);


	uint8_t m_step_pin = 0;
	uint8_t m_dir_pin = 0;
	uint8_t m_en_pin = 0;
	micro_stepping_t m_microstep;
	uint16_t m_steps_per_rotation;
	uint32_t m_step_interval = 40000; // interval in ns/25

	bool m_dir = 0;
    uint64_t m_current_pos_steps = 0;
    uint64_t m_step_count = 0;
    uint64_t m_steps_to_go = 0;

	uint16_t m_speed_rad_per_sec = 100;



    
    gptimer_handle_t m_timer_handle;
    gptimer_alarm_config_t alarm_config = { .reload_count = 0};


}


#endif
