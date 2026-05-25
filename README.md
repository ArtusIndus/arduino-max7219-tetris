
# Arduino MAX7219 Tetris

[![Watch the video](https://img.youtube.com/vi/qoqm8eURhMg/maxresdefault.jpg)](https://youtu.be/qoqm8eURhMg)

A fully playable Tetris clone for Arduino Mega with smooth rendering, animations, sound effects and score display.

## Features

* Smooth delta rendering
* No display flicker
* Fully non-blocking game loop
* Random tetromino spawning
* Row clearing system
* Rotation system
* Multi-row clear animation
* Game over animation
* Sound effects
* TM1637 score display

## Hardware

* Arduino Mega 2560
* 2x MAX7219 8x8 LED matrix
* TM1637 4-digit display
* Passive buzzer
* 3 push buttons

## Pinout

| Function      | Pin |
| ------------- | --- |
| MAX7219 DIN   | 51  |
| MAX7219 CLK   | 52  |
| MAX7219 CS    | 53  |
| TM1637 CLK    | 48  |
| TM1637 DIO    | 49  |
| Buzzer        | 6   |
| Left Button   | 10  |
| Rotate Button | 11  |
| Right Button  | 12  |

Buttons connect to GND.
Internal pullups are enabled.

## Controls

| Action     | Button        |
| ---------- | ------------- |
| Move Left  | Left Button   |
| Rotate     | Middle Button |
| Move Right | Right Button  |

## Animations

### Row Clear Animation

Completed rows flash before being removed.

### Game Over Animation

The entire display flashes before the game resets.

## Sound Effects

* Move sound
* Rotation sound
* Clear sound
* Game over sound

## Upload

1. Install the required libraries
2. Select **Arduino Mega 2560**
3. Upload `tetris2.0.ino`

## Libraries

### LedControl

https://github.com/wayoda/LedControl

### TM1637Display

https://github.com/avishorp/TM1637


## Version History

### v1.0

* Basic Tetris gameplay
* Delta rendering
* MAX7219 support

### v2.0

* TM1637 score display
* Sound system
* Clear animations
* Game over animation
* Rendering fixes
* Improved reset system
