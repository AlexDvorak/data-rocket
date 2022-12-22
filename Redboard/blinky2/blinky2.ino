#include "config.h"
#include "periph.h"

enum RocketState {
    IDLE,
    INIT_COUNTDOWN,
    COUNTDOWN,
    COLLECTING,
    HALT,
    RECOVER,
} rocket_state = IDLE;

void setup() {

}

void loop() {
    switch (rocket_state) {
        break; case IDLE:
            led_pulse(8);
            if (Serial.available() > 0)
                rocket_state = RECOVER;
            if (button_pressed())
                rocket_state = INIT_COUNTDOWN;

        break; case INIT_COUNTDOWN:
            rocket_state = init_countdown()
                ? COUNTDOWN
                : IDLE;

        break; case COUNTDOWN:
            rocket_state = countdown()
                ? COLLECTING
                : IDLE;

        break; case COLLECTING:
            if (!collect_data())
                rocket_state = HALT;

        break; case HALT:
            led_pulse(8);

        break; case RECOVER:
            recover_data();
            rocket_state = IDLE;
    }
}

bool init_countdown() {
    led_set(false);

    if (released_before(3000)) return false;

    led_set(true);

    if (released_before(2000)) return true;
    else
        wait_until_released();
        return false;
}

bool countdown() {
    for (int i = 0; i < COUNTDOWN_STAGE_ONE; i++) {
        led_blink(1000);
        if (button_pressed()) {
            wait_until_released();
            return false;
        }
    }

    for (int i = 0; i < COUNTDOWN_STAGE_ONE; i++) {
        led_blink(1000);
        if (button_pressed()) {
            wait_until_released();
            return false;
        }
    }

    return true;
}

bool collect_data() {}

void recover_data() {}