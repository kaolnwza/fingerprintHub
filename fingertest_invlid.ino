#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <Adafruit_Fingerprint.h>
#include <Servo.h>


volatile int finger_status = -1;

SoftwareSerial mySerial(5, 4); // TX/RX on fingerprint sensor
Servo servo;
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

#define WIFI_SSID "HOME197"
#define WIFI_PASSWORD "Rubber197"
#define FIREBASE_HOST "com-pro-a93c8.firebaseio.com"
#define FIREBASE_AUTH "k4f674Bntkl9cJ2KNOecHnBGNfXveAPzVHuplYlZ"
void setup()  
{
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
  Serial.print(".");
  delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  
  servo.attach(16);
  servo.write(0);
  pinMode(D8,OUTPUT);
  pinMode(D7,OUTPUT);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
}

void loop()                     // run over and over again
{
  servo.write(90);
  delay(2000);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& timeStampObject = jsonBuffer.createObject();
  timeStampObject[".sv"] = "timestamp";
  finger_status = getFingerprintIDez();
  if (finger_status!=-1 and finger_status!=-2){
    Serial.println("Open");
    digitalWrite(D7, HIGH);
    servo.write(90);
    delay(100);
    servo.write(0);
    delay(3000);
    digitalWrite(D7, LOW);
    JsonObject& temperatureObject = jsonBuffer.createObject();
    JsonObject& tempTime = temperatureObject.createNestedObject("timestamp");
    temperatureObject["ID"] = finger.fingerID;
    tempTime[".sv"] = "timestamp";
    Firebase.push("ID", temperatureObject);
  } else{
    if (finger_status==-2){
      Serial.println("Access Denied");
      digitalWrite(D8, HIGH);
      delay(2000);
      digitalWrite(D8, LOW);
      JsonObject& temperatureObject = jsonBuffer.createObject();
      JsonObject& tempTime = temperatureObject.createNestedObject("timestamp");
      temperatureObject["ID"] = "Unknow";
      tempTime[".sv"] = "timestamp";
      Firebase.push("ID", temperatureObject);
    }
  }
  

  // Push to Firebase
  
  delay(50);            //don't ned to run this at full speed.
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p!=2){
    Serial.println(p);
  }
  if (p != FINGERPRINT_OK)  return -1;
  
  p = finger.image2Tz();
  if (p!=2){
    Serial.println(p);
  }
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -2;
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}
