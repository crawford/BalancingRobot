## Real-Time and Embedded Systems Research Project

### Topic area:
Balancing Robot

### Team members:
Conlan Wesson
Alex Crawford

### Project description:

The goal of this project is to create a robotic platform which balances on two wheels. The wheels on the platform share an axle, so without any intervention the system would topple to one side or another. The robot will utilize an accelerometer to gauge its current orientation. The controller, running on QNX, will respond to the accelerometer data and adjust the speed off the motors to keep the robot balanced vertically.

Additionally, the robotic system will attempt to keep itself centered over a reference position. The motors onboard will utilize encoders to keep track of their lateral position. The controller will then use that information to move the robot into the desired lateral position.

The form of the robot will be a small table-top platform with two wheels mounted underneath a short body. The body will house the accelerometer, motor controllers, power link, and communication link to the purple box.

Time permitting; the target reference point will be dynamically changed. This could be done through a simple keyboard controller which allows the user to "drive" the robot back and forth.

### Resources needed:
* Power supply
* Two motors (continuous DC)
* Two wheels
* Motor controller
* Frame
* Accelerometer (preferably analog output)
* PurpleBox with data acquisition breakout board

