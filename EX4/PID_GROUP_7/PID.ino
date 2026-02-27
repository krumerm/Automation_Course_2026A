#include <PIDController.h>
#include <Encoder.h>

// Variable and parameter definitions
PIDController pid;
#define ENCODER_A 2
#define ENCODER_B 3
#define MOTOR_PWM 9
#define MOTOR_CCW 11
#define MOTOR_CW 10


float encoder_count;
float enc_deg;
int motor_pwm_value;
int goal_degree = 0; // Initial state: zero degrees
Encoder myEnc(ENCODER_B, ENCODER_A);
float Kp = 20; // Proportional parameter
float Ki = 0.8; // Integral parameter
float Kd = 15; // Derivative parameter

// Conversion from ticks to degrees
float tick_to_deg(long tick) {
return (tick % 440) * 360.0 / 440.0; // Conversion of ticks to degrees
}

// Run the motor clockwise
void forward() {
digitalWrite(MOTOR_CW, HIGH);
digitalWrite(MOTOR_CCW, LOW);
}

// Run the motor counterclockwise
void reverse() {
digitalWrite(MOTOR_CW, LOW);
digitalWrite(MOTOR_CCW, HIGH);
}

// Get input from the user
void get_user_input() {
goal_degree = Serial.parseFloat();
Serial.read();

// Keep the range between -180 and 180
while (goal_degree > 180) goal_degree -= 360;
while (goal_degree < -180) goal_degree += 360;
}

void setup() {
Serial.begin(9600);
pinMode(MOTOR_PWM, OUTPUT);
pinMode(MOTOR_CW, OUTPUT);
pinMode(MOTOR_CCW, OUTPUT);
digitalWrite(MOTOR_CW, HIGH);
digitalWrite(MOTOR_CCW, LOW);
pid.begin(); // PID initialization
pid.limit(-255, 255);// Limit the PID output between -255 and 255
pid.tune(Kp, Ki, Kd); // Set PID parameters
Serial.println("Please write the target angle between -180 and 180: ");
}

// Variables for tracking the angle state
bool isMoved = false; 

void loop() {
// If there is input from the user (target angle)

if (Serial.available() > 0) {
   get_user_input();
   Serial.print("Goal angle set to: ");
   Serial.println(goal_degree);
}

encoder_count = myEnc.read();
enc_deg = tick_to_deg(encoder_count); // Convert ticks to degrees

// Correct the current angle so it is between -180 and 180 degrees
if (enc_deg > 180) enc_deg -= 360;
if (enc_deg < -180) enc_deg += 360;


// If the angle specified by the code is 0, the motor will not move
if (abs(enc_deg) < 45 && !isMoved) {
// If the system has not been manually moved up to 45 degrees, do not start motion
   Serial.println("Move manually until the system reaches 45 degrees.");
return;
}

if (!isMoved && abs(enc_deg) >= 45) {
// Once the angle exceeds 45 degrees, mark the system as manually moved
isMoved = true;
Serial.println("System has been moved. Now the motor will start moving to the target.");
}

// Calculate the angle error
float angle_error = goal_degree - enc_deg;

// Normalize the error so it is always between -180 and 180
if (angle_error > 180) angle_error -= 360;
if (angle_error < -180) angle_error += 360;

Serial.println(enc_deg);
Serial.print('\t');
Serial.println(angle_error);

// If the error is less than -15 degrees, stop the motor
if (abs(angle_error) < 15) {
   analogWrite(MOTOR_PWM, 0); // כיבוי המנו ע
   Serial.println("Target reached");
return;
}

// Calculate PID output
motor_pwm_value = pid.compute(angle_error);

// Set the direction of motion
if (motor_pwm_value > 0) {
   forward();
} else {
   reverse();
}

analogWrite(MOTOR_PWM, abs(motor_pwm_value)); 
delay(10);// Delay to prevent system overload

}
