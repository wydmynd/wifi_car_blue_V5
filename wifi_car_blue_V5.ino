#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const int JOYSTICK_RANGE = 100;
const char *ssid = "WIFI_CAR_24";
const char *password = "2456456456";

// Motor pins
const int PWM_A = 5;  // Right motor PWM
const int PWM_B = 4;  // Left motor PWM
const int DIR_A = 0;  // Right motor direction
const int DIR_B = 2;  // Left motor direction

// Control parameters
bool high_speed_mode = false;
bool repeat_command = false;
float SPEED = 500;  // Default speed (0-1023)
int speed_Coeff = 3;
int max_linear_speed = 1000;
int max_angular_speed = 500;

// State tracking
String command;
String prev_command;
int joystick_x = 0;
int joystick_y = 0;
int prev_joystick_speed = 0;

// Network configuration
ESP8266WebServer server(80);
IPAddress ip(192, 168, 4, 1);
IPAddress netmask(255, 255, 255, 0);

void run_motors(int right_speed, int left_speed) {
    // Constrain speeds to valid range
    right_speed = constrain(right_speed, -1023, 1023);
    left_speed = constrain(left_speed, -1023, 1023);
    
    // Set right motor direction and speed
    digitalWrite(DIR_A, right_speed >= 0 ? HIGH : LOW);
    analogWrite(PWM_A, abs(right_speed));
    
    // Set left motor direction and speed
    digitalWrite(DIR_B, left_speed >= 0 ? LOW : HIGH);
    analogWrite(PWM_B, abs(left_speed));
    
    // Debug output
    Serial.print("Right Motor - Speed: ");
    Serial.print(right_speed);
    Serial.print(", Left Motor - Speed: ");
    Serial.println(left_speed);
}

void setup() {
    Serial.begin(115200);
    Serial.println("*WiFi Robot Remote Control Mode*");
    
    // Initialize motor pins
    pinMode(PWM_A, OUTPUT);
    pinMode(PWM_B, OUTPUT);
    pinMode(DIR_A, OUTPUT);
    pinMode(DIR_B, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    
    // Initial motor state
    run_motors(0, 0);
    
    // Setup WiFi Access Point
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(ip, ip, netmask);
    WiFi.softAP(ssid, password);
    
    // Setup web server routes
    server.on("/", HTTP_handleRoot);
    server.onNotFound(handleNotFound);
    server.begin();
}

void handleState() {
    if (command == prev_command && command != "") {
        Serial.println("DEBUG: Repeat Command");
        return;
    }
    
    prev_command = command;
    Serial.print("DEBUG: State Command= ");
    Serial.println(command);
    
    // Convert speed commands to integer
    if (command.length() == 1 && isdigit(command[0])) {
        SPEED = command.toInt() * 100;
        return;
    }
    
    // Handle movement commands
    switch (command[0]) {
        case 'F': run_motors(SPEED, SPEED); break;  // Forward
        case 'B': run_motors(-SPEED, -SPEED); break;  // Backward
        case 'L': run_motors(-SPEED, SPEED); break;  // Turn Left
        case 'R': run_motors(SPEED, -SPEED); break;  // Turn Right
        case 'I': run_motors(SPEED, SPEED/speed_Coeff); break;  // Forward Left
        case 'J': run_motors(-SPEED, -SPEED/speed_Coeff); break;  // Backward Left
        case 'G': run_motors(SPEED/speed_Coeff, SPEED); break;  // Forward Right
        case 'H': run_motors(-SPEED/speed_Coeff, -SPEED); break;  // Backward Right
        case 'S': run_motors(0, 0); break;  // Stop
        case 'W': digitalWrite(LED_BUILTIN, LOW); break;  // Light On
        case 'w': digitalWrite(LED_BUILTIN, HIGH); break;  // Light Off
        case 'V': Serial.println("horn"); break;  // Horn On
        case 'v': Serial.println("stop horn"); break;  // Horn Off
    }
}

void move_with_joystick(int joy_x, int joy_y) {
    // Convert joystick input to motion commands
    int x_offset = joy_x - JOYSTICK_RANGE;  // Range: -100 to 100
    int y_offset = joy_y - JOYSTICK_RANGE;  // Range: -100 to 100
    
    // Calculate linear and angular velocity components
    int linear_vel = y_offset * max_linear_speed / 100;
    int angular_vel = x_offset * max_angular_speed / 100;
    
    // Convert to left and right motor speeds
    int right_speed = linear_vel - angular_vel;
    int left_speed = linear_vel + angular_vel;
    
    // Send commands to motors
    run_motors(right_speed, -left_speed);  // Left motor inverted for correct motion
    
    // Debug output
    Serial.print("Joystick - X: ");
    Serial.print(x_offset);
    Serial.print(", Y: ");
    Serial.print(y_offset);
    Serial.print(", Linear: ");
    Serial.print(linear_vel);
    Serial.print(", Angular: ");
    Serial.println(angular_vel);
}

void handleJoystick() {
    int joystick_speed = sqrt(pow(joystick_x - JOYSTICK_RANGE, 2) + 
                             pow(joystick_y - JOYSTICK_RANGE, 2));
                             
    if (joystick_speed != prev_joystick_speed) {
        prev_joystick_speed = joystick_speed;
        move_with_joystick(joystick_x, joystick_y);
    }
}

void HTTP_handleRoot() {
    if (server.hasArg("State")) {
        command = server.arg("State");
        server.send(200, "text/html", "S");
        handleState();
    } 
    else if (server.hasArg("JX")) {
        command = "";
        joystick_x = server.arg("JX").toInt();
        joystick_y = server.arg("JY").toInt();
        server.send(200, "text/html", "J");
        handleJoystick();
    } 
    else {
        handleNotFound();
    }
}

void handleNotFound() {
    server.send(404, "text/plain", "404: Not Found");
}

void loop() {
    server.handleClient();
}