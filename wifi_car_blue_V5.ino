/******************* WiFi Robot Remote Control Mode ********************/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
// #include <ArduinoOTA.h>
// #include <Servo.h>

// Servo myservo;  // create servo object to control a servo

const int JOYSTICK_RANGE = 100;

const char *ssid = "WIFI_CAR_24";
const char *password = "2456456456";

bool high_speed_mode = false;
bool repeat_command = false;
// connections for drive Motors
int PWM_A = 5;
int PWM_B = 4;
int DIR_A = 0;
int DIR_B = 2;

String command;       // String to store app command state.
String prev_command;  // String to store app command state.

int wheelsD = 0.1;  //we dont yet know the exact length
float SPEED = 130;  // 330 - 1023.
int speed_Coeff = 3;
int joystick_x = 0;
int joystick_y = 0;
int prev_joystick_speed = 0;
int max_linear_speed = 1000;
int max_angular_speed = 500;

ESP8266WebServer server(80);  // Create a webserver object that listens for HTTP request on port 80

unsigned long previousMillis = 0;

String sta_ssid = "";      // set Wifi networks you want to connect to
String sta_password = "";  // set password for Wifi networks
IPAddress ip(192, 168, 4, 1);
IPAddress netmask(255, 255, 255, 0);

void setup() {
  Serial.begin(115200);  // set up Serial library at 115200 bps
  Serial.println();
  Serial.println("*WiFi Robot Remote Control Mode*");
  Serial.println("--------------------------------------");

  // myservo.attach(2, 600, 2000);  // attaches the servo on GIO2 to the servo object

  pinMode(PWM_A, OUTPUT);
  pinMode(PWM_B, OUTPUT);
  pinMode(DIR_A, OUTPUT);
  pinMode(DIR_B, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, LOW);
  analogWrite(PWM_A, 0);
  analogWrite(PWM_B, 0);

  WiFi.mode(WIFI_AP);

  WiFi.softAPConfig(ip, ip, netmask);

  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();

  Serial.println("*WiFi-AP-Mode*");
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  delay(3000);



  server.on("/", HTTP_handleRoot);    // call the 'handleRoot' function when a client requests URI "/"
  server.onNotFound(handleNotFound);  // when a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  server.begin();                     // actually start the server

  // ArduinoOTA.begin();                       // enable to receive update/uploade firmware via Wifi OTA
}


void loop() {
  server.handleClient();  // listen for HTTP requests from clients
}

void handleState() {
  if (command != prev_command) {
    prev_command = command;
    Serial.print("DEBUG: State Command= ");
    Serial.println(command);



    if (command == "F") Forward();  // check string then call a function or set a value
    else if (command == "B") Backward();
    else if (command == "R") TurnRight();
    else if (command == "L") TurnLeft();
    else if (command == "G") ForwardLeft();
    else if (command == "H") BackwardLeft();
    else if (command == "I") ForwardRight();
    else if (command == "J") BackwardRight();
    else if (command == "S") Stop();
    else if (command == "V") BeepHorn();       //TODO add code to beep horn
    else if (command == "v") BeepHorn_stop();  //TODO add code to stop beeping horn
    else if (command == "W") TurnLightOn();
    else if (command == "w") TurnLightOff();
    else if (command == "0") SPEED = command.toInt() * 100;
    else if (command == "1") SPEED = command.toInt() * 100 + 30;
    else if (command == "2") SPEED = command.toInt() * 100;
    else if (command == "3") SPEED = command.toInt() * 100;
    else if (command == "4") SPEED = command.toInt() * 100;
    else if (command == "5") SPEED = command.toInt() * 100;
    else if (command == "6") SPEED = command.toInt() * 100;
    else if (command == "7") SPEED = command.toInt() * 100;
    else if (command == "8") SPEED = command.toInt() * 100;
    else if (command == "9") SPEED = command.toInt() * 100;
    else if (command == "10") SPEED = command.toInt() * 100;
  } else if (command == "") {
    Serial.println("DEBUG: Error - Empty Command");
    return;
  } else {
    Serial.println("DEBUG: Repeat Command");
  }
}

void handleJoystick() {
  int joystick_speed = (sqrt(pow(joystick_x - JOYSTICK_RANGE, 2) + pow(joystick_y - JOYSTICK_RANGE, 2)));
  if (joystick_speed != prev_joystick_speed) {
    prev_joystick_speed = joystick_speed;
    Serial.print("DEBUG: Joystick_x=");
    Serial.print(joystick_x);
    Serial.print(",Joystick_y=");
    Serial.print(joystick_y);
    Serial.print(",Joystick Speed=");
    Serial.println(joystick_speed);

    move_with_joystick(joystick_x, joystick_y);  //move robot
  }

  else {
    Serial.println("DEBUG: Repeat Command");
  }
}

// function prototypes for HTTP handlers
void HTTP_handleRoot(void) {
  if (server.hasArg("State")) {
    if (server.arg("State") != "") {
      command = server.arg("State");
      server.send(200, "text/html", "S");  // Send HTTP status 200 (Ok) and send some text to the browser/client
      handleState();
    }
  } else if (server.hasArg("JX")) {  // check if the client sent a value for the JX parameter
    if (server.arg("JX") != "") {
      command = "";
      joystick_x = server.arg("JX").toInt();
      joystick_y = server.arg("JY").toInt();
      server.send(200, "text/html", "J");  // Send HTTP status 200 (Ok) and send some text to the browser/client
      handleJoystick();
    }
  } else {
    handleNotFound();
  }
}

void handleNotFound() {
  server.send(404, "text/plain", "404: error");
}

void StartMotor() {
  analogWrite(PWM_A, 1023);
  analogWrite(PWM_B, 1023);
  delay(50);
}

// function to move forward
void Forward() {
  digitalWrite(DIR_A, HIGH);
  digitalWrite(DIR_B, LOW);
  if (!high_speed_mode && !repeat_command) { StartMotor(); }
  analogWrite(PWM_A, SPEED);
  analogWrite(PWM_B, SPEED);

  // Serial.println("Forward");
}

// function to move backward
void Backward() {
  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, HIGH);
  if (!high_speed_mode && !repeat_command) { StartMotor(); }
  analogWrite(PWM_A, SPEED);
  analogWrite(PWM_B, SPEED);

  // Serial.println("Backward");
}

// function to turn right
void TurnRight() {
  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, LOW);
  if (!high_speed_mode && !repeat_command) { StartMotor(); }
  analogWrite(PWM_A, SPEED);
  analogWrite(PWM_B, SPEED);

  // Serial.println("Right");
}

// function to turn left
void TurnLeft() {
  digitalWrite(DIR_A, HIGH);
  digitalWrite(DIR_B, HIGH);
  if (!high_speed_mode && !repeat_command) { StartMotor(); }

  analogWrite(PWM_A, SPEED);
  analogWrite(PWM_B, SPEED);
}

// function to move forward left
void ForwardLeft() {
  digitalWrite(DIR_A, HIGH);
  digitalWrite(DIR_B, LOW);
  if (!high_speed_mode && !repeat_command) { StartMotor(); }

  analogWrite(PWM_A, SPEED);
  analogWrite(PWM_B, SPEED / speed_Coeff);
}

// function to move backward left
void BackwardLeft() {
  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, HIGH);
  if (!high_speed_mode && !repeat_command) { StartMotor(); }

  analogWrite(PWM_A, SPEED);
  analogWrite(PWM_B, SPEED / speed_Coeff);
}

// function to move forward right
void ForwardRight() {
  digitalWrite(DIR_A, HIGH);
  digitalWrite(DIR_B, LOW);
  if (!high_speed_mode && !repeat_command) { StartMotor(); }



  analogWrite(PWM_A, SPEED / speed_Coeff);
  analogWrite(PWM_B, SPEED);
}

// function to move backward left
void BackwardRight() {
  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, HIGH);
  if (!high_speed_mode && !repeat_command) { StartMotor(); }

  analogWrite(PWM_A, SPEED / speed_Coeff);
  analogWrite(PWM_B, SPEED);
}

// function to stop motors
void Stop() {
  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, LOW);
  analogWrite(PWM_A, 0);
  analogWrite(PWM_B, 0);

  // Serial.println("Stop");
}

// function to beep a buzzer
void BeepHorn() {
  Serial.println("horn");
}

// function to beep a buzzer
void BeepHorn_stop() {
  Serial.println("stop horn");
}


// function to turn on LED
void TurnLightOn() {
  Serial.println("HS_on");

  digitalWrite(LED_BUILTIN, LOW);
}

// function to turn off LED
void TurnLightOff() {
  Serial.println("HS_off");
  digitalWrite(LED_BUILTIN, HIGH);
}


void motor_dir(bool bIsForward, bool aIsForward)  //b is the left motor, a is right motor
{
  Serial.print(",Left_Motor_Dir,");
  Serial.print(bIsForward);

  Serial.print(",Right_Motor_Dir,");
  Serial.println(aIsForward);

  if (bIsForward)
    digitalWrite(DIR_B, LOW);
  else
    digitalWrite(DIR_B, HIGH);
  if (aIsForward)
    digitalWrite(DIR_A, HIGH);
  else
    digitalWrite(DIR_A, LOW);
}

void run_motors(int a_speed, int b_speed)  //b is the left motor, a is right motor
{
  //speed can be 0-1000 for each motor
  //if speed is positive, direction pin is set to LOW
  //if speed is negative, direction pin is set to HIGH

  Serial.print("R_Speed:");
  Serial.print(a_speed);

  if (a_speed < 0)
  {
    a_speed = -a_speed;
    digitalWrite(DIR_A, HIGH);
    Serial.print(",R_dir:1,");
  }
  else
    digitalWrite(DIR_A, LOW);
    Serial.print(",R_dir:0,");

  Serial.print("L_Speed:");
  Serial.print(b_speed);
  if (b_speed < 0)
  {
    b_speed = -b_speed;
    digitalWrite(DIR_B, HIGH);
    Serial.print(",L_dir:1,");
  }
  else
    digitalWrite(DIR_B, LOW);
    Serial.println(",L_dir:0");
 
  analogWrite(PWM_A, a_speed);
  analogWrite(PWM_B, b_speed);

  
  

}

void move_with_joystick(int joy_x, int joy_y) {

  int xMove = joy_x - JOYSTICK_RANGE;  //joystick range is from 0-200 where 100,100 is center. we want to know the offset from center. here we get a range of -100 to 100
  int yMove = joy_y - JOYSTICK_RANGE;  //y is the forward / backward component. here we get a range of -100 to 100

  int v = yMove * max_linear_speed / 100;
  int omega = xMove * max_angular_speed / 100;

  Serial.print("xm:");
  Serial.print(xMove);
  Serial.print(",ym:");
  Serial.print(xMove);
  Serial.print(",v:");
  Serial.print(v);
  Serial.print(",omega:");
  Serial.print(omega);

  run_motors(-1*(v + omega), v - omega); //invert left motor

  Serial.print(",Right Speed:");
  Serial.print(v - omega);
  Serial.print(",Left Speed:");
  Serial.println(v + omega);
}
