#include <AWS_IOT.h>
#include <Servo.h>
#include <ArduinoJson.h>
#include <WiFi.h>

AWS_IOT hornbill;

// tthe number of the LED pin
const int led1RPin = 12;  // 16 corresponds to GPIO16
const int led1GPin = 13; // 17 corresponds to GPIO17
const int led1BPin = 14;  // 5 corresponds to GPIO5

const int led2RPin = 26;  // 16 corresponds to GPIO16
const int led2GPin = 25; // 17 corresponds to GPIO17
const int led2BPin = 33;  // 5 corresponds to GPIO5
static const int servoPin = 4;
int motion_pin = 27;
int motion_pin1 = 18;
int light_sensor_pin=32;

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
int period = 1000;
unsigned long time_now = 0;
Servo servo1;
int status = WL_IDLE_STATUS;
int tick=0,msgCount=0,msgReceived = 0;
int i=0;

//Secrets
char WIFI_SSID[]="Brown_Nation";
char WIFI_PASSWORD[]="MadHouse91";
char HOST_ADDRESS[]="a37g54y6ddcht7-ats.iot.us-east-1.amazonaws.com";
char CLIENT_ID[]= "TestESP32Client";

//Topics
char TOPIC_OOH_STATE[]= "OutOfHome/state";
char TOPIC_BULB_STATE[]="Bulb/state";
char TOPIC_BLIND_STATE[]="Blind/state";
char TOPIC_INTRUSION[]="Intrusion";
char TOPIC_SUNRISE[]="Sunrise";
char TOPIC_SUNSET[]="Sunset";

//Expected Messages from AWS
char YES[]="yes";
char NO[]="no";
char ON[]="on";
char OFF[]="off";
char OPEN[]="open";
char CLOSE[]="close";

//Received Messages
char MESSAGE_OOH[512]="";
char MESSAGE_BULB[512]="";
char MESSAGE_BLIND[512]="";
char MESSAGE_INTRUSION[512]="";
char MESSAGE_SUNRISE[512]="";
char MESSAGE_SUNSET[512]="";

const int BULB_OFF_STATE=0;
const int BULB_ON_STATE=1;
const int BULB_SLOW_ON_STATE=2;
const int BULB_SLOW_OFF_STATE=3;
const int BLIND_CLOSE_STATE=0;
const int BLIND_OPEN_STATE=1;
const int OOH_YES=1;
const int OOH_NO=0;
const int INTRUSION_YES=1;
const int INTRUSION_NO=0;

//Connected Device States
int CURRENT_OOH_STATE=OOH_NO;
int CURRENT_BULB_STATE=BULB_OFF_STATE;
int CURRENT_BLIND_STATE=BLIND_CLOSE_STATE;
int CURRENT_INTRUSION_STATE=INTRUSION_NO;

void setup()
{
  subscribeAWSTopics();
  servo_setup();

  pinMode(led1RPin, OUTPUT);
  pinMode(led2RPin, OUTPUT);
  pinMode(led1GPin, OUTPUT);
  pinMode(led2GPin, OUTPUT);
  pinMode(led1BPin, OUTPUT);
  pinMode(led2BPin, OUTPUT);

  // configure LED PWM functionalitites
  //ledcSetup(ledChannel, freq, resolution);
  //ledcSetup(ledChannel1, freq, resolution);
  pinMode(motion_pin,INPUT);

  // attach the channel to the GPIO to be controlled
  //ledcAttachPin(ledPin, ledChannel);
  //ledcAttachPin(ledPin2, ledChannel);
  //ledcAttachPin(ledPin3, ledChannel);

  // attach the channel to the GPIO to be controlled
  //ledcAttachPin(ledPin4, ledChannel1);
  //ledcAttachPin(ledPin5, ledChannel1);
  //ledcAttachPin(ledPin6, ledChannel1);
}

void loop()
{ 
  bool isIntrusionDetected1 = false;
  bool isIntrusionDetected2 = false;
  
  
  if(millis() >= time_now + period){
    isIntrusionDetected1 = digitalRead(motion_pin);
    isIntrusionDetected2 = digitalRead(motion_pin1);
  }

  if(CURRENT_OOH_STATE == OOH_YES){

    if(isIntrusionDetected1 || isIntrusionDetected2){
      publishIntrusion();
    }

    if(isIntrusionDetected1)
    {
      Serial.println("Intrusion detected");
      flickrLight(1);
    }

    if(isIntrusionDetected2)
    {
      Serial.println("Intrusion detected");
      flickrLight(2);
    }
  }
  
  delay(500);
  //increase the LED brightness
}

void subscribeAWSTopics()
{
  Serial.println("Subscribing AWS Topics");
  Serial.begin(115200);
  delay(2000);

  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    //wait 5 seconds for retry
    delay(5000);
  }

  Serial.println("Connected to wifi");

  //Connect to AWS
  if(hornbill.connect(HOST_ADDRESS,CLIENT_ID)==0)
  {
    Serial.println("Connected to AWS");
    delay(1000);

    if(0==hornbill.subscribe(TOPIC_OOH_STATE,awsSubscribeCallBackHandler))
    {
      Serial.println("TOPIC_OOH_STATE Subscribe Successfull");
    }
    else
    {
      Serial.println("TOPIC_OOH_STATE Subscribe Failed, Check the Thing Name and Certificates");
    }
    
    if(0==hornbill.subscribe(TOPIC_BULB_STATE,awsSubscribeCallBackHandler))
    {
      Serial.println("TOPIC_BULB_STATE Subscribe Successfull");
    }
    else
    {
      Serial.println("TOPIC_BULB_STATE Subscribe Failed, Check the Thing Name and Certificates");
    }
    
    if(0==hornbill.subscribe(TOPIC_BLIND_STATE,awsSubscribeCallBackHandler))
    {
      Serial.println("TOPIC_BLIND_STATE Subscribe Successfull");
    }
    else
    {
      Serial.println("TOPIC_BLIND_STATE Subscribe Failed, Check the Thing Name and Certificates");
      
    }
    
    if(0==hornbill.subscribe(TOPIC_INTRUSION,awsSubscribeCallBackHandler))
    {
      Serial.println("TOPIC_INTRUSION Subscribe Successfull");
    }
    else
    {
      Serial.println("TOPIC_INTRUSION Subscribe Failed, Check the Thing Name and Certificates");
      
    }

  if(0==hornbill.subscribe(TOPIC_SUNSET,awsSubscribeCallBackHandler))
    {
      Serial.println("TOPIC_SUNSET Subscribe Successfull");
    }
    else
    {
      Serial.println("TOPIC_SUNSET Subscribe Failed, Check the Thing Name and Certificates");      
    }

  if(0==hornbill.subscribe(TOPIC_SUNRISE,awsSubscribeCallBackHandler))
    {
      Serial.println("TOPIC_SUNRISE Subscribe Successfull");
    }
    else
    {
      Serial.println("TOPIC_SUNRISE Subscribe Failed, Check the Thing Name and Certificates");
      
    }

  
  }
  else
  {
    Serial.println("AWS connection failed, Check the HOST Address");
    
  }
}

//light sensor setting
void read_light_sensor()
{
 delay(2000);
  
  int value = analogRead(light_sensor_pin);

 Serial.println(value);
 if (value<500 && CURRENT_BULB_STATE==BULB_OFF_STATE && CURRENT_BLIND_STATE!=BLIND_CLOSE_STATE)
 {
  changeBulbState();

  if(hornbill.publish(TOPIC_BULB_STATE,ON) == 0)
    {        
      Serial.print("Bulb Message Successfully Published:");
    }
    else
    {
      Serial.println("Publish failed");
    }
    delay(20);
  
 }

 delay(100);
}

//motor settings
void servo_setup() 
{
  Serial.println("Servo Setup");
  Serial.begin(115200);
    servo1.attach(
        servoPin, 
        Servo::CHANNEL_NOT_ATTACHED, 
        45,
        120
    );
}

void awsSubscribeCallBackHandler (char *topicName, int payloadLen, char *payLoad)
{
  Serial.print("Received Message:");
  Serial.print(topicName);
  Serial.print("#");
  Serial.println(payLoad);

  if (strncmp(TOPIC_OOH_STATE,topicName,strlen(TOPIC_OOH_STATE))==0)
  {
    strncpy(MESSAGE_OOH,payLoad,payloadLen);
    MESSAGE_OOH[payloadLen]=0;

  if(strcmp(MESSAGE_OOH,YES)==0)
  {
    CURRENT_OOH_STATE=OOH_YES;
  }
  else if(strcmp(MESSAGE_OOH,NO)==0)
  {
    CURRENT_OOH_STATE=OOH_NO;
  }
  }
  else if (strncmp(TOPIC_BULB_STATE,topicName,strlen(TOPIC_BULB_STATE))==0)
  {
    strncpy(MESSAGE_BULB,payLoad,payloadLen);
    MESSAGE_BULB[payloadLen] = 0;
    
    if(strcmp(MESSAGE_BULB,ON)==0)
    {
      Serial.println("Received Message to Switch on Bulb");
      if (CURRENT_BULB_STATE!=BULB_ON_STATE)
      {
        changeBulbState();      
      }
    }
    else if(strcmp(MESSAGE_BULB,OFF)==0)
    {
      Serial.println("Received Message to Switch off Bulb");
      if (CURRENT_BULB_STATE!=BULB_OFF_STATE)
      {
        changeBulbState();
        delay(10);
        read_light_sensor();
      }
    }
  }
  else if (strncmp(TOPIC_BLIND_STATE,topicName,strlen(TOPIC_BLIND_STATE))==0)
  {
    strncpy(MESSAGE_BLIND,payLoad,payloadLen);
    MESSAGE_BLIND[payloadLen] = 0;

    if (strcmp(MESSAGE_BLIND,OPEN)==0)
    {
    Serial.println("Received Message to open blinds");
      if (CURRENT_BLIND_STATE==BLIND_CLOSE_STATE)
      {
        changeBlindState();
      }
    }
    else if (strcmp(MESSAGE_BLIND,CLOSE)==0)
    {
    Serial.println("Received Message to close blinds");
      if (CURRENT_BLIND_STATE==BLIND_OPEN_STATE)
      {
        changeBlindState();
      }
    }
  }
  else if (strcmp(topicName,TOPIC_INTRUSION)==0)
  {
  strncpy(MESSAGE_INTRUSION,payLoad,payloadLen);
    MESSAGE_INTRUSION[payloadLen]=0;

  if(strcmp(MESSAGE_INTRUSION,YES)==0)
  {
    CURRENT_INTRUSION_STATE=INTRUSION_YES;
  }
  else if(strcmp(MESSAGE_INTRUSION,NO)==0)
  {
    CURRENT_INTRUSION_STATE=INTRUSION_NO;
  }
  }
  else if (strcmp(topicName,TOPIC_SUNRISE)==0)
  {
    strncpy(MESSAGE_SUNRISE,payLoad,payloadLen);
    MESSAGE_SUNRISE[payloadLen]=0;
  }
  else if (strncmp(TOPIC_SUNSET,topicName,strlen(TOPIC_SUNSET))==0)
  {
    strncpy(MESSAGE_SUNSET,payLoad,payloadLen);
    Serial.println("In sunset function");
    MESSAGE_SUNSET[payloadLen]=0;

    switchOnBulbAndCloseBlinds();
  }
}

void switchOnBulbAndCloseBlinds()
{
  if (CURRENT_BULB_STATE==BULB_OFF_STATE)
  {     
    digitalWrite(led1RPin, HIGH);
    digitalWrite(led1GPin, HIGH);
    digitalWrite(led1BPin, HIGH);
    digitalWrite(led2RPin, HIGH);
    digitalWrite(led2GPin, HIGH);
    digitalWrite(led2BPin, HIGH);
    CURRENT_BULB_STATE=BULB_ON_STATE;
    Serial.println(CURRENT_BULB_STATE);
    delay(20);

    if(hornbill.publish(TOPIC_BULB_STATE,ON) == 0)
    {        
      Serial.print("Bulb Message Successfully Published:");
    }
    else
    {
      Serial.println("Publish failed");
    }
    delay(20);
  }

  if (CURRENT_BLIND_STATE==BLIND_OPEN_STATE)
  {
    Serial.println("Blinds close 2nd");
    for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
        servo1.write(posDegrees);
        //Serial.println(posDegrees);
        delay(20);
    }
  CURRENT_BLIND_STATE=BLIND_CLOSE_STATE;

  delay(20);

  if(hornbill.publish(TOPIC_BLIND_STATE,CLOSE) == 0)
  {        
    Serial.print("Blind Message Successfully Published:");
  }
  else
  {
    Serial.println("Publish failed");
  }
  }  
}

void changeBulbState()
{
  Serial.println("Changing Bulb State");
  Serial.println(CURRENT_BULB_STATE);
  if (CURRENT_BULB_STATE==BULB_OFF_STATE)
  {   
    //for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++)
    //{   
      // changing the LED brightness with PWM
      //ledcWrite(ledChannel, dutyCycle);
      //delay(20);
    //}
    //ledcWrite(ledChannel, 255);
    //ledcWrite(ledChannel1, 255);
    digitalWrite(led1RPin, HIGH);
    digitalWrite(led1GPin, HIGH);
    digitalWrite(led1BPin, HIGH);
    digitalWrite(led2RPin, HIGH);
    digitalWrite(led2GPin, HIGH);
    digitalWrite(led2BPin, HIGH);
    CURRENT_BULB_STATE=BULB_ON_STATE;
    Serial.println(CURRENT_BULB_STATE);
  }
  else if (CURRENT_BULB_STATE==BULB_SLOW_ON_STATE)
  {
    //ledcWrite(ledChannel, 255);
  CURRENT_BULB_STATE=BULB_ON_STATE;
  }
  else if (CURRENT_BULB_STATE==BULB_SLOW_OFF_STATE)
  {
    //ledcWrite(ledChannel, 0);
  CURRENT_BULB_STATE=BULB_OFF_STATE;
  }
  else if (CURRENT_BULB_STATE==BULB_ON_STATE)
  {
    //for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--)
    //{
      // changing the LED brightness with PWM
      //ledcWrite(ledChannel, dutyCycle);   
      //delay(15);
    //}
    //ledcWrite(ledChannel, 0);
    //ledcWrite(ledChannel1, 0);

    digitalWrite(led1RPin, LOW);
    digitalWrite(led1GPin, LOW);
    digitalWrite(led1BPin, LOW);
    digitalWrite(led2RPin, LOW);
    digitalWrite(led2GPin, LOW);
    digitalWrite(led2BPin, LOW);
  CURRENT_BULB_STATE=BULB_OFF_STATE;
  }

  Serial.print("Changed Bulb State:");
  Serial.println(CURRENT_BULB_STATE);
}

void changeBlindState()
{

  Serial.println("Blind Changed State");
  if (CURRENT_BLIND_STATE==BLIND_OPEN_STATE)
  {
    Serial.println("Blinds close 2nd");
    for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
        servo1.write(posDegrees);
        //Serial.println(posDegrees);
        delay(20);
    }
  CURRENT_BLIND_STATE=BLIND_CLOSE_STATE;
  }
  else if (CURRENT_BLIND_STATE==BLIND_CLOSE_STATE)
  {
    Serial.println("Blinds open 2nd");
    for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
        servo1.write(posDegrees);
        //Serial.println(posDegrees);
        delay(20);
    }
  CURRENT_BLIND_STATE=BLIND_OPEN_STATE;
  }
}

void publishIntrusion()
{
  char intrusionmsg[512];
  sprintf(intrusionmsg,YES);
  if(hornbill.publish(TOPIC_INTRUSION,intrusionmsg) == 0)
  {        
    Serial.print("Message Successfully Published:");
  }
  else
  {
    Serial.println("Publish failed");
  }
}

void flickrLight(int channelId)
{  
  int tik=0;

  if(channelId==1)
  {
    while( tik<30)
    {
      //ledcWrite(channelId, 255);
      digitalWrite(led1RPin, HIGH);
      digitalWrite(led1GPin, HIGH);
      digitalWrite(led1BPin, HIGH);
      delay(30);
      //ledcWrite(channelId, 0);
      digitalWrite(led1RPin, LOW);
      digitalWrite(led1GPin, LOW);
      digitalWrite(led1BPin, LOW);
      delay(30);
      tik=tik+1;
    }
    digitalWrite(led1RPin, LOW);
    digitalWrite(led1GPin, LOW);
    digitalWrite(led1BPin, LOW); 
  }
  else if(channelId==2)
  {
    while( tik<30)
    {
      //ledcWrite(channelId, 255);
      digitalWrite(led2RPin, HIGH);
      digitalWrite(led2GPin, HIGH);
      digitalWrite(led2BPin, HIGH);
      delay(30);
      //ledcWrite(channelId, 0);
      digitalWrite(led2RPin, LOW);
      digitalWrite(led2GPin, LOW);
      digitalWrite(led2BPin, LOW);
      delay(30);
      tik=tik+1;
    }

    digitalWrite(led2RPin, LOW);
    digitalWrite(led2GPin, LOW);
    digitalWrite(led2BPin, LOW);
  }

  
  CURRENT_BULB_STATE = BULB_OFF_STATE;
}
