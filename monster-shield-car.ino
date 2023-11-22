#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "home.h"
#include "car.h"

/* === ARDUINO + MOTORSHIELD

  ORIGINAL PINOUT FOR DUMMIES:
  int MOTOR_A1_PIN = 7;
  int MOTOR_B1_PIN = 8;

  int MOTOR_A2_PIN = 4;
  int MOTOR_B2_PIN = 9;

  int PWM_MOTOR_1 = 5;
  int PWM_MOTOR_2 = 6;

  int MOTORS_PIN[2][2] = {{7, 8}, {4, 9}}; // MOTOR_1 (A, B), MOTOR_2 (A, B)
  int MOTORS_PWM[2] = {5, 6};

  int CURRENT_SEN_1 = A2;
  int CURRENT_SEN_2 = A3;

  int EN_PIN_1 = A0;
  int EN_PIN_2 = A1;
*/

/* === NODEMCU + MOTORSHIELD (OR L298N) === */
char MOTORS_PIN[2][2] = {{D2, D3}, {D1, D4}}; // MOTOR_1 (A, B), MOTOR_2 (A, B)
char MOTORS_PWM[2] = {D6, D8};

int speedStep = 30;
int curSpeed = 150;
int maxSpeed = 255;
int minSpeed = 100;

/* === DISTANCE CHECKER === */
char ECHO_PIN = D5;
char TRIG_PIN = D7;
float minDistance = 15.0; // critical distnce (cm)
unsigned long duration;
float distance;
bool isForward = false;

/* === CREATING WEB SERVER === */
ESP8266WebServer server(80);    //Webserver Object
// const char* ssid = "forg";
// const char* password =  "amdistrash";

const char* ssid = "212E";
const char* password = "Cneltyn_212";

void setup() {
  // initialize car pins
  for (int i = 0; i < 2; i++) {
    pinMode(MOTORS_PIN[i][0], OUTPUT);
    pinMode(MOTORS_PIN[i][1], OUTPUT);
    pinMode(MOTORS_PWM[i], OUTPUT);
  }

  // initialize sensor pins
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);

  Serial.begin(115200);

  // initialize wifi connection
  Serial.println();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }
  Serial.println();
  String ip = WiFi.localIP().toString();
  Serial.print("This is my ip: ");
  Serial.println(ip);
  
  server.on("/", handle_home);
  server.on("/car", handle_car);
  // server.on("/button", handle_button);
  
  server.begin();
  Serial.println("HTTP server started");

  // Serial.begin(9600);

  // Serial.println("Begin motor control");
  // Serial.println("=======");
  // Serial.println("Enter number for control option:");
  // Serial.println("1. STOP");
  // Serial.println("W. MOVE FORWARD");
  // Serial.println("A. ROTATE LEFT");
  // Serial.println("D. ROTATE RIGHT");
  // Serial.println("S. MOVE BACKWARD");
  // Serial.println("=======");
}

void handle_home() {
  server.send(200, "text/html", home);
}

// void handle_button() {
//   server.send(200, "text/html", button);
//   isLOW = !isLOW;
//   digitalWrite(BUILTIN_LED, isLOW);
// }

void handle_car() {
  if (server.arg("state") == "") { 
    server.send(200, "text/html", car);
  } else {
    String arg = server.arg("state");
    input_manager(arg);
    server.send(200, "text/html", car);
  }
}

void loop() {
  server.handleClient();
  // char command;
  
  /* while (Serial.available()) {
    command = Serial.read();
    // digitalWrite(EN_PIN_1, HIGH);
    // digitalWrite(EN_PIN_2, HIGH);
    
    input_manager(command);
  } */

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;

  if (distance <= minDistance && isForward) {
    isForward = false;
    stop();
  }

  Serial.print("Distance: ");
  Serial.println(distance);
}

void input_manager(String command) {
  if (command == "stop") stop();
  if (command == "forward") move_forward();
  if (command == "left") rotate_left();
  if (command == "backward") move_backward();
  if (command == "right") rotate_right();
  if (command == "add") increase_speed();
  if (command == "sub") decrease_speed();
}

void set_CW(int motorNum) {
  digitalWrite(MOTORS_PIN[motorNum][0], HIGH);
  digitalWrite(MOTORS_PIN[motorNum][1], LOW);
}

void set_CCW(int motorNum) {
  digitalWrite(MOTORS_PIN[motorNum][0], LOW);
  digitalWrite(MOTORS_PIN[motorNum][1], HIGH);
}

void stop() {
  // Serial.println("stop");
  isForward = false;
  analogWrite(MOTORS_PWM[0], 0);
  analogWrite(MOTORS_PWM[1], 0);
}

void move_forward() {
  // COUNTER CLOCKWISE MOVEMENT
  Serial.println("backward " + String(curSpeed));
  isForward = true;

  for (int i = 0; i < 2; i++) {
    set_CCW(i);
    analogWrite(MOTORS_PWM[i], curSpeed);
  }
}

void move_backward() {
  // CLOCKWISE MOVEMENT
  Serial.println("forward " + String(curSpeed));
  isForward = false;

  for (int i = 0; i < 2; i++) {
    set_CW(i);
    analogWrite(MOTORS_PWM[i], curSpeed);
  }
}

void rotate_left() {
  Serial.println("left " + String(curSpeed));
  isForward = false;

  set_CW(1);
  set_CCW(0);

  for (int i = 0; i < 2; i++) {
    analogWrite(MOTORS_PWM[i], curSpeed);
  }
}

void rotate_right() {
  Serial.println("right " + String(curSpeed));
  isForward = false;

  set_CW(0);
  set_CCW(1);

  for (int i = 0; i < 2; i++) {
    analogWrite(MOTORS_PWM[i], curSpeed);
  }
}

void increase_speed() {
  curSpeed += speedStep;
  if (curSpeed > maxSpeed) curSpeed = maxSpeed;

  Serial.println("add " + String(curSpeed));

  for (int i = 0; i < 2; i++) {
    analogWrite(MOTORS_PWM[i], curSpeed);
  }
}

void decrease_speed() {
  curSpeed -= speedStep;
  if (curSpeed < minSpeed) curSpeed = minSpeed;

  Serial.println("sub " + String(curSpeed));

  for (int i = 0; i < 2; i++) {
    analogWrite(MOTORS_PWM[i], curSpeed);
  }
}