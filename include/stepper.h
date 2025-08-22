#ifndef STEPPER_H
#define STEPPER_H

#include <driver/gpio.h>
#include <driver/gptimer.h>
#include <esp_log.h>

#define NS_TO_T_TICKS(x) (x)
#define TIMER_F 1000000ULL
#define TICK_PER_S TIMER_F

enum motor_status { DISABLED, IDLE, ACC, COAST, DEC };

enum dir { CW, CCW };

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

class Stepper {
public:
  Stepper(uint8_t step_pin, uint8_t dir_pin, uint8_t en_pin,
          micro_stepping_t microsteps, uint16_t steps_per_rotation);
  ~Stepper();

  void write_rad(float relative_rad);
  void write_absolute_rad(float absolute_rad);

  void set_speed(float speed_rad_per_sec, uint16_t acceleration_ms,
                 uint16_t decceleration_ms);

  void disable_motor();
  void enable_motor();

  void set_dir(bool dir_state);

private:
  motor_status m_current_state;

  void set_en(bool en_state);

  static bool xISRwrap(gptimer_t *timer, const gptimer_alarm_event_data_t *data,
                       void *_this) {
    return static_cast<Stepper *>(_this)->xISR(timer, data);
  }

  static void _disableMotor(void *_this) {
    static_cast<Stepper *>(_this)->disable_motor();
  }

  bool xISR(gptimer_t *timer, const gptimer_alarm_event_data_t *data);

  uint8_t m_step_pin = 0;
  uint8_t m_dir_pin = 0;
  uint8_t m_en_pin = 0;
  micro_stepping_t m_microsteps;
  uint16_t m_steps_per_rotation;
  uint32_t m_step_interval = 40000; // interval in ns/25

  bool m_dir = 0;
  uint64_t m_current_pos_steps = 0;
  uint64_t m_step_count = 0;
  uint64_t m_steps_to_go = 0;

  float m_speed_rad_per_sec = 100;

  gptimer_handle_t m_timer_handle;
  gptimer_alarm_config_t m_alarm_config;
};

#endif
