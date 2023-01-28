//
// Created by riley on 11/5/20.
//

#ifndef SERVO_H
#define SERVO_H
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/ledc.h>

class Servo {
public:
    Servo();
    ~Servo();

    /**
     * Attaches the clock to the given pin
     * @param pin uint8_t the GPIO pin to connect to
     */
    void attach(uint8_t pin);

    /**
     * Detaches the clock from the pin
     */
    void detach();

    /**
     * Sets the angle of the servo in a gradual manner to avoid brownouts
     * @param angle the desired angle
     */
    void write(uint8_t angle);

    /**
     * Getter and setter for max angle
     * @param value
     */
    void set_max_angle(uint8_t value);
    uint8_t get_max_angle();

    /**
     * Getter and setter for min angle
     * @param value
     */
    void set_min_angle(uint8_t value);
    uint8_t get_min_angle();

    /**
     * Getter for current angle
     * @param value
     */
    uint8_t get_current_angle();

private:
    uint8_t m_current_angle;
    uint8_t m_max_angle;
    uint8_t m_min_angle;
    uint8_t m_freqHz;

    void write_micro_seconds(uint32_t value);
    uint32_t map(uint32_t angle);

    void set_clock(uint32_t value);

    double get_duty_by_percentage(double percentage);

    double get_duty_by_uS(double uS);
};


#endif //FIRMWAREDEVELOPMENTKIT_SERVO_H
