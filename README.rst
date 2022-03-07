Assignment 2:

Name - Ravi Shankar
ASU ID - 1220325912

Overview : 

We have implemented an application program with 3 shell commands and functions to control an RGB LED along with brightness control using the PWM blocks, and developed display driver for MAX7219-controlled led matrix. Interfaced the LED matrix with the driver using hardware SPI lpspi1. 

The commands are listed here:
 p2 rgb x y z (pwm duty cycle setup)
 p2 ledm r x0 x1 x2 .... (pattern values)
 p2 ledb n (blinking on/off)


******************************************************************************

Requirements : 

1. RGB led 
2. LED Matrix
3. MAX7219 driver IC 

*****************Steps for build and flash*****************


1. Unzip the RTES-Shankar-R_02.zip in the zephyrproject directory, copy display_max7219.c to /{zephyr tree}/drivers/display and add the necessary Kconfig, Kconfig.max7219, CMakeLists.txt changes.

2. To build, run west build -b mimxrt1050_evk project_2.

3. Run west flash.
	
4. Open putty and select port /dev/ttyACM0 and enter baud rate 115200

5. Enter the root command (p2) and then sub commands as explained below.

6. For controlling the intensity of the LED(p2 rgb):
	Enter the subcommand rgb and then 3 input values to set the duty cycle of each PWM signal to the LED

7. For displaying the row pattern(p2 ledm)
	Enter the subcommand ledm and input the starting row number and the corresponding hex values to display the row patterns
	
8. For blinking mode (p2 ledb)
	Enter argument n, if n is 1 then blink the pattern on and off with a period of 1 second, if n is 0 stop blinking.

  	



	



