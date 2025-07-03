#include <WiFi.h>            // Use ESP32 WiFi library
#include <WebServer.h>        // Use ESP32 WebServer library

#define AIN1 19   // Front-left wheel direction pin 1
#define AIN2 18  // Front-left wheel direction pin 2
#define PWMA 0   // Front-left wheel speed

#define BIN1 17   // Front-right wheel direction pin 1
#define BIN2 5  // Front-right wheel direction pin 2
#define PWMB 2  // Front-right wheel speed

// Second Motor Driver (Rear Wheel)
#define CIN1 12  // Rear wheel direction pin 1
#define CIN2 14  // Rear wheel direction pin 2
#define PWMC 4  // Rear wheel speed

#define solenoid 13
#define roller 15
// Speed setting (0 to 255)
int speedValue = 240;  // Adjust as needed

// Replace with your network credentials
const char *ssid = "realme GT 6T";
const char *password = "j5u2284c";

// Create a WebServer object on port 80
WebServer server(80);

// HTML, CSS, and JavaScript for the updated interface
const char* html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RoboSoccer Control</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            background-color: #f0f0f0;
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }
        h1 {
            color: #333;
            margin-bottom: 20px;
        }
        .joystick-container {
            display: grid;
            grid-template-columns: repeat(3, 100px);
            grid-template-rows: repeat(3, 100px);
            gap: 10px;
            margin-bottom: 20px;
        }
        .joystick-button {
            width: 100px;
            height: 100px;
            background-color: #4CAF50;
            border: none;
            border-radius: 10px;
            color: white;
            font-size: 24px;
            cursor: pointer;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.2);
            user-select: none;
        -webkit-user-select: none; /* Safari */
        -moz-user-select: none; /* Firefox */
          -ms-user-select: none; /* IE/Edge */
        }
        .joystick-button:active {
            background-color: #45a049;
        }
        .joystick-button.up {
            grid-column: 2;
            grid-row: 1;
        }
        .joystick-button.down {
            grid-column: 2;
            grid-row: 3;
        }
        .joystick-button.left {
            grid-column: 1;
            grid-row: 2;
        }
        .joystick-button.right {
            grid-column: 3;
            grid-row: 2;
        }
        .action-buttons {
            display: flex;
            gap: 10px;
            margin-bottom: 20px;
        }
        .action-buttons button {
            width: 100px;
            height: 50px;
            background-color: #008CBA;
            border: none;
            border-radius: 5px;
            color: white;
            font-size: 16px;
            cursor: pointer;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.2);
             user-select: none;
        -webkit-user-select: none; /* Safari */
        -moz-user-select: none; /* Firefox */
          -ms-user-select: none; /* IE/Edge */

        }
        .action-buttons button:active {
            background-color: #007B9E;
        }
        .rotate-buttons {
            display: flex;
            gap: 10px;
        }
        .rotate-buttons button {
            width: 120px;
            height: 50px;
            background-color: #f44336;
            border: none;
            border-radius: 5px;
            color: white;
            font-size: 16px;
            cursor: pointer;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.2);
             user-select: none;
        -webkit-user-select: none; /* Safari */
        -moz-user-select: none; /* Firefox */
          -ms-user-select: none; /* IE/Edge */
        }
        .rotate-buttons button:active {
            background-color: #D32F2F;
        }
    </style>
</head>
<body>
    <h1>RoboSoccer</h1>
    <div class="joystick-container">
        <button class="joystick-button up" ontouchstart="sendCommand('up')" 
        ontouchend="sendCommand('stop')">▲</button>
        <button class="joystick-button down" ontouchstart="sendCommand('down')" ontouchend="sendCommand('stop')">▼</button>
        <button class="joystick-button left" ontouchstart="sendCommand('left')" ontouchend="sendCommand('stop')" >◀</button>
        <button class="joystick-button right" ontouchstart="sendCommand('right')" ontouchend="sendCommand('stop')">▶</button>
    </div>
    <div class="action-buttons">
        <button ontouchstart="sendCommand('hit')">Hit</button>
        <button ontouchstart ="sendCommand('roller_on')">Roller On</button>
        <button ontouchstart ="sendCommand('roller_off')">Roller Off</button>
        <button ontouchstart ="sendCommand('stop')">Stop</button>
    </div>
    <div class="rotate-buttons">
        <button ontouchstart ="sendCommand('rotate_cw')"  ontouchend="sendCommand('stop')">Rotate CW</button>
        <button ontouchstart ="sendCommand('rotate_acw')" ontouchend="sendCommand('stop')">Rotate ACW</button>
    </div>
    <script>
        function sendCommand(command) {
            fetch(`/control?cmd=${command}`)
                .then(response => response.text())
                .then(data => console.log(data))
                .catch(error => console.error('Error:', error));
        }
    </script>
</body>
</html>
)rawliteral";

void setup() {
    // Start serial communication
    Serial.begin(921600);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);

    // Wait until connected to Wi-Fi
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");

        // Check if the connection attempt takes too long
        if (millis() - startTime > 10000) {
            Serial.println("Failed to connect to Wi-Fi within 10 seconds!");
            break;
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        String ipAddress = WiFi.localIP().toString();
        Serial.print("Connected to Wi-Fi. ESP32 IP Address: ");
        Serial.println(ipAddress);
    } else {
        Serial.println("Failed to connect to Wi-Fi.");
    }

    // Set up the web server routes
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", html);
    });

    server.on("/control", HTTP_GET, []() {
        String command = server.arg("cmd");
        Serial.print("Command received: ");
        Serial.println(command);

        // Add your motor control logic here based on the command
        if (command == "up") {
          unsigned int start = millis();
            Forward();
        } else if (command == "down") {
            Backward();
        } else if (command == "left") {
            Left();
        } else if (command == "right") {
            Right();
        } else if (command == "hit") {
          digitalWrite(solenoid,HIGH);
          delay(1000);
          digitalWrite(solenoid,LOW);
        } else if (command == "roller_on") {
            analogWrite(roller,100);
        } 
        else if(command == "stop"){
          stopBot();
        }
        else if (command == "roller_off") {
            analogWrite(roller,0);
        } else if (command == "rotate_cw") {
            Clk_Rot();
        } else if (command == "rotate_acw") {
             Aclk_Rot();
        }

        server.send(200, "text/plain", "OK");
    });

    server.begin();
    pinMode(solenoid,OUTPUT);
     pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(PWMA, OUTPUT);
    
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(PWMB, OUTPUT);

    // Rear wheel motor driver
    pinMode(CIN1, OUTPUT);
    pinMode(CIN2, OUTPUT);
    pinMode(PWMC, OUTPUT);
}

void loop() {
    server.handleClient();
}

void driveMotor(int in1, int in2, int pwmPin, int speed) {
    if (speed > 0) {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
    } else if (speed < 0) {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
    } else {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
    }
    analogWrite(pwmPin, abs(speed));
}

// Omni-wheel movement functions
void Forward() {  // Forward
    driveMotor(AIN1, AIN2, PWMA, 0);
    driveMotor(BIN1, BIN2, PWMB, -speedValue);
    driveMotor(CIN1, CIN2, PWMC, speedValue);
}

void Backward() {  // Backward
    driveMotor(AIN1, AIN2, PWMA, 0);
    driveMotor(CIN1, CIN2, PWMC, -speedValue);
    driveMotor(BIN1, BIN2, PWMB, speedValue);
}

void Left() {  // Right
    driveMotor(AIN1, AIN2, PWMA, speedValue);
    driveMotor(BIN1, BIN2, PWMB, speedValue/2);
    driveMotor(CIN1, CIN2, PWMC, speedValue/2);
}

void Right() {  // Left
    driveMotor(AIN1, AIN2, PWMA, -speedValue-15);
    driveMotor(BIN1, BIN2, PWMB, -speedValue/2);
    driveMotor(CIN1, CIN2, PWMC, -speedValue/2);
}

void Clk_Rot(){
    driveMotor(AIN1, AIN2, PWMA, -speedValue/2.55);
    driveMotor(BIN1, BIN2, PWMB, speedValue/2.55);
    driveMotor(CIN1, CIN2, PWMC, speedValue/2.55);
}

void Aclk_Rot(){
    driveMotor(AIN1, AIN2, PWMA, speedValue/2.55);
    driveMotor(BIN1, BIN2, PWMB, -speedValue/2.55);
    driveMotor(CIN1, CIN2, PWMC, -speedValue/2.55);
}

void stopBot() {
    driveMotor(AIN1, AIN2, PWMA, 20);
    driveMotor(BIN1, BIN2, PWMB, 20);
    driveMotor(CIN1, CIN2, PWMC, 20);
}

