#include <NewPing.h>
#include <Servo.h>

#define TRIGGER_PIN  7  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     6  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define SERVO_PIN 9   // Arduino pin tied to servo.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

Servo servo;
int val;
int servoPosition;

void setup() {
  Serial.begin(115200);
  servo.attach(SERVO_PIN);
  servoPosition = 90;
}

void loop() {
  val = sonar.ping_cm();
  if(val > 50) {
    val = 0;
  }
  Serial.print(val);
  Serial.println(" cm");

  if(val < 15) {
    servoPosition = 180;
  } else {
    if(val != 0) {
      servoPosition = 90;
    }
  }
  servo.write(servoPosition);
  delay(500);

}
