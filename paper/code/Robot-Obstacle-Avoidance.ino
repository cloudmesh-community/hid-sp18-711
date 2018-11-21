/* include library */
#include <ESP8266WiFi.h>
#include <Servo.h> 
#include <NewPing.h>

/* define port */
WiFiClient client;
WiFiServer server(80);

#define SONAR_SERVO_PIN 16
#define TRIGGER_PIN     5
#define ECHO_PIN        4

#define MAX_DISTANCE    200
/* WIFI settings */
const char* ssid = "sid_username";
const char* password = "sid_password";

/* data received from application */
String  data =""; 

/* define L298N motor control pins */
int rightMotorForward = 12;  /*GPIO14(D5) -- IN1*/
int rightMotorBackward = 14; /*GPIO12(D6) -- IN2*/
int leftMotorForward = 15;    /*GPIO13(D7) -- IN3*/
int leftMotorBackward = 13;   /*GPIO15(D8) -- IN4*/



/* define L298N enable pins */
int rightMotorENB = 0; /*GPIO0(D3)->Motor-A Enable*/
int leftMotorENB = 2;  /*GPIO2(D4)->Motor-B Enable*/

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
Servo myServo;

const int triggerDistance = 25;

// Variables
unsigned int time1; // to store how long it 
                    // takes for the ultrasonic 
                    // wave to come back
int distance;       // to store the distance calculated 
                    // from the sensor
int fDistance;      // to store the distance 
                    // in front of the robot
int lDistance;      // to store the distance on 
                    // the left side of the robot
int rDistance;      // to store the distance on 
                    // the right side of the robot


char dist[3];
char rot[3];
int rotation = 0;
String output = "";

void connectWiFi() {
  Serial.println("Connecting to WIFI");
  WiFi.begin(ssid, password);
  while ((!(WiFi.status() == WL_CONNECTED))) {
    delay(300);
    Serial.print("..");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("NodeMCU Local IP is : ");
  Serial.print((WiFi.localIP()));
}

void scan(int deg) {
  myServo.write(deg);
  delay(10);

  int time1 = sonar.ping();
  distance = time1 / US_ROUNDTRIP_CM;
  delay(10);
  if(distance <= 0){
    distance = triggerDistance;
  }

  delay(30);
} 

/*** FORWARD ***/
void MotorForward(void) {
  digitalWrite(leftMotorENB,HIGH);
  digitalWrite(rightMotorENB,HIGH);
  digitalWrite(leftMotorForward,HIGH);
  digitalWrite(rightMotorForward,HIGH);
  digitalWrite(leftMotorBackward,LOW);
  digitalWrite(rightMotorBackward,LOW);
}

/*** BACKWARD ***/
void MotorBackward(void) {
  digitalWrite(leftMotorENB,HIGH);
  digitalWrite(rightMotorENB,HIGH);
  digitalWrite(leftMotorBackward,HIGH);
  digitalWrite(rightMotorBackward,HIGH);
  digitalWrite(leftMotorForward,LOW);
  digitalWrite(rightMotorForward,LOW);
}

/*** TURN LEFT ***/
void TurnLeft(void) {
  digitalWrite(leftMotorENB,HIGH);
  digitalWrite(rightMotorENB,HIGH); 
  digitalWrite(leftMotorForward,HIGH);
  digitalWrite(rightMotorForward,LOW);
  digitalWrite(rightMotorBackward,LOW);
  digitalWrite(leftMotorBackward,HIGH);  
}

/*** TURN RIGHT***/
void TurnRight(void) {
  digitalWrite(leftMotorENB,HIGH);
  digitalWrite(rightMotorENB,HIGH);
  digitalWrite(leftMotorForward,HIGH);
  digitalWrite(rightMotorForward,LOW);
  digitalWrite(rightMotorBackward,HIGH);
  digitalWrite(leftMotorBackward,LOW);
}

/*** STOP ***/
void MotorStop(void) {
  digitalWrite(leftMotorENB,LOW);
  digitalWrite(rightMotorENB,LOW);
  digitalWrite(leftMotorForward,LOW);
  digitalWrite(leftMotorBackward,LOW);
  digitalWrite(rightMotorForward,LOW);
  digitalWrite(rightMotorBackward,LOW);
}

/*** RECEIVE DATA FROM the APP ***/
String checkClient (void) {
  while(!client.available()) delay(1); 
  String request = client.readStringUntil('\r');
  request.remove(0, 5);
  request.remove(request.length()-9,9);
  return request;
}

void setup() {
  Serial.begin(115200);
  connectWiFi();
  server.begin();

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  myServo.attach(SONAR_SERVO_PIN);  // Attaches the Servo 
                                    //to the Servo Object 
  /* initialize motor control pins as output */
  pinMode(leftMotorForward, OUTPUT);
  pinMode(rightMotorForward, OUTPUT); 
  pinMode(leftMotorBackward, OUTPUT);  
  pinMode(rightMotorBackward, OUTPUT);

  /* initialize motor enable pins as output */
  pinMode(leftMotorENB, OUTPUT); 
  pinMode(rightMotorENB, OUTPUT);

  /* start server communication */
}

void loop() {
  
  scan(90);  //Get the distance retrieved
  fDistance = distance;
  if(fDistance < triggerDistance){
    MotorBackward();
    delay(682); 
    MotorStop();
    scan(170);
    delay(600);
    lDistance = distance;
    scan(3);
    delay(600);
    rDistance = distance;
   if(lDistance < rDistance){
      TurnRight();
      delay(682);
      MotorStop();
      MotorForward();
    }
    else{
      TurnLeft();
      delay(682);
      MotorStop();
      MotorForward();
    }
  }
  else{
    MotorForward();
  } 
}
