MEMORY GAME
===
Name is a work in progress.

AUTHOR
---
Nam Nguyen

CONTENTS
---
Mem game is a memory game written in C using ATMEL studios and an ATMEGA1284P. It is a game where a pattern will appear on an LED matrix and the player will have to replicate the pattern. The player uses a joystick to navigate around the LED matrix and will press a button to "draw" the pattern.

COMPONETS
---
Input
  * Joystick
  * On/Off Button
  * Draw button
  * Clear Button

Output
  * Nokia 5110 LCD
  * 8x8 RGB LED Matrix
  * LEDs

TODO
---
- [x] Wire LED matrix
- [x] Get dot to travel around LED matrix
- [x] Wire joystick and test
- [x] Get joystick to move dot around LED matrix
- [x] Be able to draw/drop dots on the screen
- [x] Wire LCD and display relevant information (time, menu, etc)
- [x] Finish game logic
- [ ] Clean up wiring and put in a box to make it presentable
- [ ] Make videos/upload images to youtube/github

BUGS
---
- Nokia LCD5110 LCD causes half second delay when rendering
- User drawn patterns sometimes refresh too slow causing them "blink"
