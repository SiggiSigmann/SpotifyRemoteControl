#include "Arduino.h"
#include "heltec.h"
#include <WiFi.h>
#include "secrets.h"
#include "src/ApiClient.h"

//counter for connectiong animation
int loadingstep = 0;

ApiClient* apiClient;

//stick calc
int x_analog = 0, y_analog = 0;
bool x_direct = 0, y_direct = 0;
bool stickOutput[5] = {false, false, false, false, false};
unsigned long antiBound=0;

//update
unsigned long now = 0;
unsigned long updateTime = 1000;
bool updatetoggle = false;

//display connection animation
//display task and comment below
void loadingAnimation(String task, String comment) {
  Heltec.display->clear();

  //create animation step
  switch (loadingstep % 4) {
    case 0:
      Heltec.display->drawString(0, 0, task);
      Heltec.display->drawString(0, 10, comment);
      break;
    case 1:
      Heltec.display->drawString(0, 0, task + ".");
      Heltec.display->drawString(0, 10, comment);
      break;
    case 2:
      Heltec.display->drawString(0, 0, task + "..");
      Heltec.display->drawString(0, 10, comment);
      break;
    case 3:
      Heltec.display->drawString(0, 0, task + "...");
      Heltec.display->drawString(0, 10, comment);
      break;
  }

  Heltec.display->display();
  loadingstep++;
  delay(100);
}

//connect to serial
void connectToSerial() {
  while (!Serial) { // wait for serial port to connect. Needed for native USB
    loadingAnimation("Connecting", "to Serial");
  }
}

//connect to wifi
void connectToWifi(String ssid, String password) {
  //configure
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  //start connection
  WiFi.begin(SSID, PASSWORD);

  //wait for connectiom
  while (WiFi.status() != WL_CONNECTED) {
    loadingAnimation("Connecting", "to " + ssid);
  }

}

void processStick(){
  for(int i =0; i<4; i++){
    stickOutput[i] = false;
  }
  
  //smothing
  x_analog = (analogRead(36) * 0.3 + (0.7 * (x_analog + 2000))-2000);
  y_analog = (analogRead(37) * 0.3 + (0.7 * (y_analog + 2000))-2000);

  //process y
  if (y_analog > 1500 and !x_direct) {
    y_direct = true;
    stickOutput[0] = true;
  } else if (y_analog < -1500 and !x_direct) {
    y_direct = true;
    stickOutput[1] = true;
  }else{
    y_direct = false;
  }

  //process x
  if (x_analog > 1500 and !y_direct) {
    stickOutput[2] = true;
    x_direct = true;
  } else if (x_analog < -1500 and !y_direct) {
    stickOutput[3] = true;
    x_direct = true;
  }else{
    x_direct = false;
  }
  
  if (!digitalRead(38)) {
    if(antiBound+10<millis() or stickOutput[4]){
      stickOutput[4] = true;
      antiBound = millis();
    } 
  }else if(stickOutput[4]){
    if(antiBound+10<millis()){
      stickOutput[4] = false;
      antiBound = millis();
    }
  }else{
    antiBound = millis();
  }

}

void handleStick(){
  processStick();
  
  if (stickOutput[0]) {
    Heltec.display->drawString(0, 0, String("up"));
  }else if (stickOutput[1]) {
    Heltec.display->drawString(0, 0, String("down"));
  }else if (stickOutput[2]) {
    Heltec.display->drawString(0, 0, String("right"));
  } else if (stickOutput[3]) {
    Heltec.display->drawString(0, 0, String("left"));
  }

  if (stickOutput[4]) {
    Heltec.display->drawString(0, 10, String("button"));
  }
}

void handelUpdate(){
  if(millis()>now ){
    now = millis()+updateTime;
    updatetoggle = !updatetoggle;
  }
}

void setup() {
  adcAttachPin(36);
  adcAttachPin(37);
  pinMode(38, INPUT);
  
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);

  //init disaly
  Heltec.display->setContrast(255);
  Heltec.display->clear();

  connectToSerial();
  //connectToWifi(SSID, PASSWORD);

  Heltec.display->clear();
  Heltec.display->display();

  WiFiClient wifiClient;
  const char* host = "api.spotify.com";
  apiClient = new ApiClient(&wifiClient);
  Serial.println(apiClient->connect(host));

  DynamicJsonDocument doc(2048);
  doc["access_token"]=
  
  apiClient->GET("/api/token");
  apiClient->setAuthentication("12345");
  apiClient->GET("https://api.spotify.com/v1/me/player");

}

void loop() {
  Heltec.display->clear();
  
  handleStick();
  handelUpdate();

  //Serial.println(String(stickOutput[0])+" "+String(stickOutput[1])+" "+String(stickOutput[2])+" "+String(stickOutput[3])+" "+String(stickOutput[4]));

  if(updatetoggle){
    Heltec.display->drawString(0, 30, String("#"));
  }
  Heltec.display->display();
}
