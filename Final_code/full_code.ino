#include "WiFi.h"
#include <FirebaseESP32.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

const char* WIFI_SSID = "Akshad";
const char* WIFI_PASSWORD =  "internet";

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define API_KEY "AIzaSyDx-oWQPIRGNF9ZB15tfXonCTbzQ8ENGG0"
#define DATABASE_URL "https://smart-bottle-2aae7-default-rtdb.firebaseio.com/"
#define USER_EMAIL "akx2404@gmail.com"
#define USER_PASSWORD ""

//Define Firebase Data object
FirebaseData stream;
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

int val_count=0;

void streamCallback(StreamData data)
{
  Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); //see addons/RTDBHelper.h
  Serial.println();
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timeout, resuming...\n");
}



const int trigPin = 5;
const int echoPin = 18;

//cm/microSec
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;
float dist;
float level;
int perc;
float vol_left;
float temp = 0;
float vol_left_day;
int state;
int btn = 15;
float water_temp;


void setup()
{

  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(btn, INPUT);
  mlx.begin();  

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  //Firebase.begin(DATABASE_URL, "wGWTTvn0EKhsf2tNKmHmPmwiurE4XtIZF7g0vsI1");
  

  Firebase.reconnectWiFi(true);

  if (!Firebase.beginStream(stream, "/data"))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.setStreamCallback(stream, streamCallback, streamTimeoutCallback);
}

float measure_distance(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(2);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  
  distanceCm = duration * SOUND_SPEED/2;
  
  distanceInch = distanceCm * CM_TO_INCH;
  
  return distanceCm;
}

void loop()
{
    state = digitalRead(btn);

    if (Firebase.ready())
  {
    sendDataPrevMillis = millis();
    val_count++;
    dist = measure_distance();
    level = 12-dist;
    vol_left = 28.26*level;
    perc = 28.26*level/339.12*100;
    water_temp = mlx.readObjectTempC();

    Serial.println(dist);
    Serial.println(water_temp);
    Serial.println(perc);
    Serial.println(vol_left);
    Serial.println(level);

    if(perc<0 || perc>100){
      perc = -1;
    }

    if(vol_left<0 || vol_left>339.12){
      vol_left = -1;
    }
    
    if (perc !=-1 && vol_left !=-1){
    FirebaseJson json;
    json.add("level", vol_left);
    //json.add("num", val_count);
    json.add("perc", perc);
    json.add("temp", water_temp);
    Serial.printf("Set json... %s\n\n", Firebase.setJSON(fbdo, "/test", json) ? "ok" : fbdo.errorReason().c_str());
    delay(1000);

    }
  }

  delay(500);
    
}
