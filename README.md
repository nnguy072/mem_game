MEMORY GAME
===
Name is a work in progress.

AUTHOR
---
Nam Nguyen

CONTENTS
---
Mem game is a memory game written in C using ATMEL studios and an ATMEGA1284P. It is a game where a pattern will appear on an LED matrix and the player will have to replicate the pattern. The player uses a joystick to navigate around the LED matrix and will press a button to "draw" the pattern. The LCD screen displays the remaining time and upon completion, will notify the player if they have lost or won the game. The player wins if/when they correctly replicate all the patterns. Alternatively, the player loses if they run out of time.

Picture(s)
---
Board:

![alt tag](https://drive.google.com/file/d/0B-8QGTy3y8hdQlI3eXhyYTFna3c/view)

User Guide
---
Rules:
  * Draw the patterns that pop up on the screen
  * Draw all 3 and you win the game

Controls:
  * Use joystick to control the dot
  * right most button draws the dot
  * middle button clears the screen
  * left most button starts the game

COMPONETS
---
Pinout:

![alt tag](https://drive.google.com/file/d/0B-8QGTy3y8hddVNnZ1ZQN0NrRjg/view)

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
- [x] Make videos/upload images to youtube/github

BUGS
---
- Nokia LCD5110 LCD causes half second delay when rendering
- User drawn patterns sometimes refresh too slow causing them "blink"
