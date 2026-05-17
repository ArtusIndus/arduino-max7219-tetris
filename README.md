# Arduino MAX7219 Tetris

[![Watch the video](https://img.youtube.com/vi/iT9rC8u_At0/maxresdefault.jpg)](https://youtu.be/iT9rC8u_At0)

Tetris for Arduino Mega using:

- 2x MAX7219 LED matrix modules
- 3 buttons
- Non-blocking game loop
- Delta rendering without flicker
- Random tetromino spawning

## Hardware

- Arduino Mega 2560
- 2x MAX7219 8x8 LED matrix
- 3 push buttons

## Pinout

| Function | Pin |
|---|---|
| DIN | 51 |
| CLK | 52 |
| CS | 53 |
| Left Button | 10 |
| Rotate Button | 11 |
| Right Button | 12 |

Buttons connect to GND.
Internal pullups are enabled.

## Features

- Smooth rendering
- No display flicker
- No blocking delays
- Random tetrominos
- Row clearing
- Rotation system

## Upload

1. Install the LedControl library
2. Select Arduino Mega 2560
3. Upload `tetris.ino`

## Library

LedControl:
https://github.com/wayoda/LedControl
