#define BLYNK_TEMPLATE_ID "TMPL6gObSPJUT"
#define BLYNK_TEMPLATE_NAME "PIR test 1"
#define BLYNK_AUTH_TOKEN "f7gwa21lCMwWInFJPczRC44P8TXcu-Pd"


#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

 
char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "Iphone 11 (3)";  // type your wifi name
char pass[] = "Yurrrrrr";  // type your wifi password
 
const int PIR_SENSOR = 15;
BlynkTimer timer;
//int flag=0;
void notifyOnTheft()
{
  int isTheftAlert = digitalRead(PIR_SENSOR);
  Serial.println(isTheftAlert);
  if (isTheftAlert==1) {
    Serial.println("Theft Alert in Safe");
    Blynk.logEvent("theft_alert","MALICIOUS MOVEMENT DETECTED IN SAFE");
//    flag=1;
  }
  else if (isTheftAlert==0)
  {
   // flag=0;
  }
}

void setup(){
  pinMode(PIR_SENSOR, INPUT);
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  
  timer.setInterval(500L, notifyOnTheft);
}

void loop(){
  Blynk.run();
  timer.run();
}
