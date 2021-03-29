## requirements

- data collection is hard to trigger
    - requires button to be held for 3 seconds and released before 5 seconds
    - countdown timer
- LEDs signal the current state:
    - default state (PWM oscillation T = 4)
    - recovering data (off)
    - data collection button held (solid on)
    - data collection countdown (blinking speeds up)
    - collecting data (solid on)

## states

### default state

- serial byte sent → recovering data
- button pressed → data collection button held

### recovering data

- finished → default state

### data collection button held

- released before 3 seconds or after 5 → default state
- released between 3 and 5 seconds → data collection countdown

### data collection countdown

- button pressed → default state
- finished → collecting data

### collecting data

- finished → default state
