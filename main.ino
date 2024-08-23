// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>

const char* ssid = "ini wifii";
const char* password = "12345678";
Servo servo;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            background-color: #121212;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            margin: 0;
            color: white;
            font-family: 'Montserrat', sans-serif;
        }
        .container {
            width: 80%;
            max-width: 400px;
            display: flex;
            flex-direction: column;
            gap: 10px;
            align-items: center;
        }
        .slider {
            width: 100%;
            height: 50px;
            margin-bottom: 20px;
        }
        .switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
            margin-right: 10px;
            border: 2px solid #2196F3; /* Added border for visibility */
            border-radius: 34px; /* Rounded border */
        }
        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        .slider, .slider:before {
            box-sizing: border-box;
            transition: .4s;
        }
        .slider:before {
            content: "";
            position: absolute;
            top: 0;
            left: 0;
            width: 26px;
            height: 26px;
            bottom: 4px;
            background-color: white;
            border-radius: 50%;
        }
        input:checked + .slider {
            background-color: #2196F3;
        }
        input:checked + .slider:before {
            transform: translateX(26px);
        }
        .value {
            font-size: 2em;
            font-weight: bold;
            text-align: center;
            padding: 10px;
        }
        .send-button {
    background-color: #2196F3;
    color: white;
    border: none;
    padding: 15px 32px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: 16px;
    margin: 4px 2px;
    cursor: pointer;
    border-radius: 4px;
}
        .counters-container {
            display: flex;
            align-items: center;
            justify-content: center;
            flex-wrap: nowrap;
            overflow-x: auto;
            width: 100%;
            margin-top: 20px;
        }
        .add-counter {
            cursor: pointer;
            user-select: none;
            padding: 10px;
            margin-right: 10px;
            font-size: 2em;
            line-height: 1;
        }
        .counter {
            cursor: pointer;
            user-select: none;
            padding: 10px;
            margin-right: 10px;
            background: #333;
            border-radius: 5px;
            font-size: 2em;
            font-weight: bold;
        }
        .button-text-container {
            display: flex;
            align-items: center;
            justify-content: center;
            width: 100%;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="button-text-container">
            <label class="switch"><input type="checkbox" onchange="clickButton(this)" id="button"><span class="slider"></span></label>
            <label class="value" id="demo"></label>
        </div>
        <input type="range" min="0" max="100" value="50" class="slider" id="myRange">
        <div class="counters-container" id="counters">
            <div class="add-counter" onclick="sendData()">Save</div>
            <div class="counter" id="counter-1" value="" onclick="editCounter(this.id);">0</div>
            <div class="counter" id="counter-2" value="" onclick="editCounter(this.id);">0</div>
            <div class="counter" id="counter-3" value="" onclick="editCounter(this.id);">0</div>

        </div>
    </div>
    <script>
        let currentCounterId = null;
        let counterId = 0;
        const slider = document.getElementById("myRange");
        const output = document.getElementById("demo");
        output.innerHTML = slider.value;
        // addCounter();
        slider.oninput = function() {
            output.innerHTML = this.value;
            if (currentCounterId !== null) {
                document.getElementById(currentCounterId).textContent = this.value;
            }
        }

        function addCounter() {
    for(let i = 1; i <= 5; i++){
        counterId++;
        const counterElement = document.createElement('div');
        counterElement.classList.add('counter');
        counterElement.id = 'counter-' + counterId;
        counterElement.textContent = '0';
        counterElement.onclick = (function(id) {return function() { editCounter(id); };})(counterId);
        document.getElementById('counters').appendChild(counterElement);
    }
}

function editCounter(id) {
    currentCounterId = id;
    const counterElement = 
    slider.value = document.getElementById(id).textContent;
    output.innerHTML = slider.value;
}

function clickButton(){
    let state = document.getElementById("button").checked? 1 : 0;
    var xhr = new XMLHttpRequest();
    xhr.open("get", "/set", true);
    xhr.send();
    xhr.open("get", "/setServo", true);
    xhr.send();
}

function sendData() {
  for (let i = 1; i <= 5; i++) {
    let interval = document.getElementById("interval-" + i).value;
    if (interval > 0) {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/update?timer=" + i + "&interval=" + interval, true);
      xhr.send();
    }
  }
}</script></body></html>)rawliteral";


// Define a structure for the timers
struct Timer {
  unsigned long previousMillis;
  unsigned long interval;
  void (*callback)();
};
void task(){
  servoWrite(1);
  servoWrite(0);
}
Timer timers[] = {
  {0, 1000, task},
  {0, 2000, task},
  // Add more timers as needed
};
const int numTimers = sizeof(timers) / sizeof(Timer);

void scheduler(Timer &t) {
  unsigned long currentMillis = millis();
  
  // Check if the timer has elapsed
  if (currentMillis - t.previousMillis >= t.interval) {
    // Save the last time the function was called
    t.previousMillis = currentMillis;
    
    // Call the function associated with the timer
    t.callback();
  }
}
void updateTimerInterval(Timer &t, unsigned long newInterval) {
  t.interval = newInterval;
}
void servoWrite(int a){
  servo.write(0);
  int start = a ? 1 : 179;
  int end = a ? 180 : 0;
  int step = a ? 3 : -3;

  for(int i = start; i != end; i += step){
    servo.write(i);
    if(!a) Serial.println(i);
  }
}

void setup() {
  Serial.begin(115200);
  servo.attach(5);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  
  server.on("/", HTTP_GET,[](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
});

server.on("/setServo", HTTP_GET,[](AsyncWebServerRequest *request)  {
    Serial.print("works set servo");
    servoWrite(1);
    request->send(200, "text/plain", "OK");
    servoWrite(0);
});

server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("timer") && request->hasParam("interval")) {
      int timerId = request->getParam("timer")->value().toInt() - 1;
      unsigned long newInterval = request->getParam("interval")->value().toInt();
      if (timerId >= 0 && timerId < numTimers) {
        updateTimerInterval(timers[timerId], newInterval);
        Serial.println("Timer " + String(timerId + 1) + " interval updated to: " + newInterval);
      }
    }
    request->send(200, "text/plain", "Interval updated");
});
}

void loop(){
  for (int i = 0; i < numTimers; i++) {
    scheduler(timers[i]);
  }
}
