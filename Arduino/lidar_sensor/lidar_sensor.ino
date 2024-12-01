"""
Date: 2024-12-01
Author: Dr. Andre Broekman

Static LiDAR distance measurement device for accurately parking a motor vehicle

Potentiometer sensor: 0-1024 step values
Map this to a 0 - 150 cm range that defines an offset.
The offset measurement is subtracted from the distance measurement of the ultrasonic distance sensor;
This calculated parameter is referred to as the “value”
The transition distance is set to 70 cm that defines the “active” region as you park.

Based on the value:
If the value is less than zero, light up the LED permanently, i.e. correct parking position.
If the values is greater than the transition distance, turn off the LED.
Otherwise, the sensor is within the transition zone. A 300ms window is defined for the LED.
       The off-time period in the window is defined by mapping the value to the 300ms window. 
       Owing to persistence of vision, a minimum function is used to define a minimum off time.
       The on time is defined as the window time subtracted by the off time.

The resulting behaviour is an increasing shorter LED flash until the correct distance is achieved, whereafter
the LED remains lit continuously.
"""

// Arduino Nano Type C

// https://github.com/Segilmez06/DistanceSensor
// HC-SR04 Lidar sensor pinhout
//    Yellow - 5V
//    Black -  GND
//    Red -    ECH (D9)
//    White -  TRIG (D10)
#include <DistanceSensor.h>
const int ECHO = 9;
const int TRIG = 10;
DistanceSensor sensor(TRIG, ECHO);

// Panel indicator LED (with built in resistor) is connected to digital pins directly to power it.
// LED+ - D5; set to LOW or HIGH to source current.
// LED- - D6; set pin permanently to input and low to sink current.
const int LEDP = 5;
const int LEDN = 6;

// Potensiometer
// D2 - set as output to power the potensiometer.
// A6 - input reading for potensiometer sensor over 0-5V range.
const int POT_POWER = 2; 
const int POT_ANALOG = A6; 
int offset = 0;  // global var to store the offset measurement from the potensiometer wiper

// Logic control variables
const int FLASH_WINDOW = 300;  // cycle time in millis
const int TRANSITION = 70;  // length of the transition window [cm] between on and off states for the blink LED

void setup() {
  // Configure the LED pins
  pinMode(LEDN, OUTPUT);
  pinMode(LEDP, OUTPUT);
  digitalWrite(LEDP, LOW);

  // Configure the potensiometer and turn on its power delivery
  pinMode(POT_POWER, OUTPUT);
  digitalWrite(POT_POWER, HIGH);
  delay(10);

  Serial.begin(115200);
}


void loop() {
  // Update the offset measurement
  delay(3);
  offset = analogRead(POT_ANALOG);
  delay(3);
  // Map potensiometer ADC value between 0 cm and 150 cm
  int offset_cm = map(offset, 0, 1023, 0, 150);

  // Get a new reading from the distance sensor
  int distance = sensor.getCM();

  // Calculate the measurement and update the blink LED accordingly
  int val = distance - offset_cm;
  if (val < 0) {  // Too close
    digitalWrite(LEDP, HIGH);  // Turn on permanently
    delay(FLASH_WINDOW);
  }
  else if (val > TRANSITION) {  // Too far
    digitalWrite(LEDP, LOW);  // Turn off permanently
    delay(FLASH_WINDOW);
  } else {  // Linearly interpolate flashing LED behaviour
    int offtime = map(val, 0, TRANSITION, 0, FLASH_WINDOW);
    // Minimum time defined to accomodate persistance of vision effects
    offtime = min(offtime, (FLASH_WINDOW - FLASH_WINDOW*0.35));
    int ontime = FLASH_WINDOW - offtime;
    digitalWrite(LEDP, HIGH);
    delay(offtime);
    digitalWrite(LEDP, LOW);
    delay(ontime);
  }

  // Print debug information to Serial
  Serial.print("Offset [ADC]: ");
  Serial.println(offset);
  Serial.print("Offset [cm]: ");
  Serial.println(offset_cm);
  Serial.print("Distance [cm]: ");
  Serial.println(distance);
  Serial.print("value: ");
  Serial.println(val);
  Serial.println("");
}
