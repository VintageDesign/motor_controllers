#include "stepper.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <math.h>

#include "driver/gptimer.h"
#include "hal/gpio_ll.h"
#include <esp_log.h>

static const char *TAG = "StepperDriver";

Stepper::Stepper(uint8_t step_pin, uint8_t dir_pin, uint8_t en_pin,
                 micro_stepping_t microsteps, uint16_t steps_per_rotation)
    : m_current_state(motor_status::DISABLED), m_step_pin(step_pin),
      m_dir_pin(dir_pin), m_en_pin(en_pin), m_microsteps(microsteps),
      m_steps_per_rotation(steps_per_rotation) {
  uint64_t gpio_mask =
      (1ULL << m_step_pin | 1ULL << m_dir_pin | 1ULL << m_en_pin);
  gpio_config_t gpio_conf = {
      .pin_bit_mask = gpio_mask,
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };

  ESP_ERROR_CHECK(gpio_config(&gpio_conf));

  gptimer_config_t timer_conf = {
      .clk_src = GPTIMER_CLK_SRC_DEFAULT,
      .direction = GPTIMER_COUNT_UP,
      .resolution_hz = TIMER_F,
      .flags = {0},
  };

  ESP_ERROR_CHECK(gptimer_new_timer(&timer_conf, &m_timer_handle));

  m_alarm_config.alarm_count =
      static_cast<uint64_t>(TIMER_F / m_speed_rad_per_sec);
  m_alarm_config.reload_count = 0;
  m_alarm_config.flags.auto_reload_on_alarm = true;

  gptimer_set_alarm_action(m_timer_handle, &m_alarm_config);

  gptimer_event_callbacks_t cb_group;
  cb_group.on_alarm = xISRwrap;
  gptimer_register_event_callbacks(m_timer_handle, &cb_group, this);
  m_alarm_config.flags.auto_reload_on_alarm = true;
  gptimer_enable(m_timer_handle);
  set_en(false);
}

void Stepper::write_rad(float relative_rad) {
  if (m_current_state == motor_status::DISABLED) {
    enable_motor();
  }

  if (m_current_state != motor_status::IDLE) {
    return;
  }

  ESP_LOGI(TAG, "Setting relative angle");
  if (relative_rad < 0) {
    set_dir(!m_dir);
  }
  m_steps_to_go = (relative_rad / (2 * M_PI)) * m_steps_per_rotation;
  m_step_count = 0;
  m_alarm_config.alarm_count = m_step_interval;

  gptimer_start(m_timer_handle);
  m_current_state = motor_status::ACC;
  ESP_LOGI(TAG, "Exit");
}

void Stepper::write_absolute_rad(float absolute_rad) {
  if (m_current_state == motor_status::DISABLED) {
    enable_motor();
  }

  if (m_current_state != motor_status::IDLE) {
    return;
  }

  ESP_LOGI(TAG, "Setting Abs angle");
  m_steps_to_go = (absolute_rad / 2 * M_PI) * m_steps_per_rotation;
  m_step_count = 0;
  m_alarm_config.alarm_count = m_step_interval;
  gptimer_start(m_timer_handle);
  m_current_state = motor_status::ACC;
}

void Stepper::disable_motor() {
  m_current_state = motor_status::DISABLED;
  set_en(false);
}
void Stepper::enable_motor() {
  m_current_state = motor_status::IDLE;
  set_en(true);
}

void Stepper::set_speed(float speed_rad_per_sec, uint16_t acceleration_ms,
                        uint16_t decceleration_ms) {
  m_speed_rad_per_sec = speed_rad_per_sec;
  m_step_interval = TIMER_F / m_speed_rad_per_sec;
  // TODO
  (void)acceleration_ms;
  (void)decceleration_ms;
}

void Stepper::set_en(bool en_state) {
  gpio_set_level((gpio_num_t)m_en_pin, en_state);
}

void Stepper::set_dir(bool dir_state) {
  gpio_set_level((gpio_num_t)m_dir_pin, dir_state);
}

/* Timer callback, used for generating pulses and calculating speed profile in
 * real time */
bool Stepper::xISR(gptimer_t *timer, const gptimer_alarm_event_data_t *data) {

  gpio_set_level((gpio_num_t)m_step_pin, 1);

  // update current position
  if (m_dir == CW) {
    m_current_pos_steps++;
  } else {
    m_current_pos_steps--;
  }

  // we are done
  if (m_steps_to_go == m_step_count) {
    gptimer_stop(m_timer_handle); // stop the timer
    m_step_count = 0;
    gpio_set_level((gpio_num_t)m_step_pin, 0);
    m_current_state = motor_status::IDLE;
    return 0;
  }

  // TODO add acceleration

  // set alarm to calculated interval and disable pin
  gpio_set_level((gpio_num_t)m_step_pin, 0);

  return 1;
}
