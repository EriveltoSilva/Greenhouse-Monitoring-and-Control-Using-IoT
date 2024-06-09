/**************************************************
    AUTHOR: EriveltoSilva                        **
    FOR: Filipe Lopes & Leonel Cachinga          **
    CREATED AT:09-06-2024                        **
    Principais Componentes usados:               **
      *** 1-ESP32 (1)                            **
      *** 2-DHT11                                **
      *** 3-Sensor de Chama                      **
      *** 4-Sensor de Fumo (MQ135)               **
      *** 5-Sensor de Luz                        **
      *** 6-Cooler                               **
**************************************************/

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID    "TMPL2Ztxrhb6i"
#define BLYNK_TEMPLATE_NAME  "Quickstart Template"
#define BLYNK_AUTH_TOKEN     "kbk4xTfU3SqmPJwoPP6zbjJVBqsp3eYl"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

//////////////  PIN CONFIGURATIONS /////////////
#define LED 2                              /////
#define LIGHTS 4                           /////
#define BUZZER 5                           /////
#define DHTPIN 15                          /////
#define BTN_PUMP 18                        /////
#define BTN_LIGHTS 12                      /////
#define BTN_STATUS 19                      /////
#define PUMP 23                            /////
#define FLAME_SENSOR 34                    /////
#define SMOKE_SENSOR 35                    /////
#define LDR_SENSOR 39                      /////
#define SOIL_SENSOR 36                     /////
////////////////////////////////////////////////

///////////// OTHER CONSTANTS DEFINITIONS /////////
#define DEBUG true                            /////
#define DHTTYPE DHT11                         /////
#define LIMIAR_FLAME 65                       /////
#define LIMIAR_SMOKE 60                       /////
///////////////////////////////////////////////////

////////////  NETWORK CONFIGURATIONS ///////////
char ssid[] = "NETHOUSE";                  /////
char pass[] = "Eduanara3130";              /////
////////////////////////////////////////////////

/////////////// Libraries Used  ////////////////
#include <WiFi.h>                          /////
#include "DHT.h"                           /////
#include <Wire.h>                          /////
#include <WiFiClient.h>                    /////
#include <BlynkSimpleEsp32.h>              /////
#include <LiquidCrystal_I2C.h>             /////
////////////////////////////////////////////////

/////////////// OBJECTS DEFINITIONS  ///////////
BlynkTimer timer;                          /////
DHT dht(DHTPIN, DHTTYPE);                  /////
LiquidCrystal_I2C lcd(0x27, 20, 4);        /////
////////////////////////////////////////////////

//////// VARIABLES USED IN THE PROJECT //////////
byte cont=0;                                /////
char projectMode = 'A';                     /////
unsigned long int timeDelay = 0;            /////
bool buzzerPreviewStatus = false;           /////
float temperature = 0, humidity = 0;        /////
bool flagSoil = false, flagAlarm = false;   /////
bool flagSmoke = false, flagFlame = false;  /////
int flame = 0, smoke = 0;                   /////
int lights = 0, soilMeasure = 0;            /////
/////////////////////////////////////////////////


// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  int value = param.asInt();
  Blynk.virtualWrite(V1, value);
}

BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url",         "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

void sendDataToBlynk()
{
  Blynk.virtualWrite(V0, isPumpOn()?"BOMBA LIGADA": "BOMBA DESLIGADA");
  Blynk.virtualWrite(V1, lights);
  Blynk.virtualWrite(V2, soilMeasure);
  Blynk.virtualWrite(V3, humidity);
  Blynk.virtualWrite(V4, temperature);
  Blynk.virtualWrite(V5, flame);
  Blynk.virtualWrite(V6, smoke);
}


void setup()
{
  initSetup();
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(2000L, sendDataToBlynk);
}


void loop()
{
  if(millis()-timeDelay >1500)
  {
    timeDelay = millis();
    readSensors();
    analyseData();
    printDataLCD();
    if(flagAlarm)
      turnOnBuzzer();
    else
      turnOffBuzzer();
    digitalWrite(LED, !digitalRead(LED));
  }

  buttonsHandler();
  Blynk.run();
  timer.run();
}

/////////////////////////////////////////////////////
void initSetup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  
  pinMode(PUMP, OUTPUT);
  turnOffPump();

  pinMode(LIGHTS, OUTPUT);
  turnOffLights();

  pinMode(BTN_STATUS, INPUT_PULLUP);
  pinMode(BTN_PUMP, INPUT_PULLUP);
  pinMode(BTN_LIGHTS, INPUT_PULLUP);
  
  lcd.init();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("UNIV. METODISTA");
  lcd.setCursor(0, 1);
  lcd.print("#  ESTUFA IoT  #");
  delay(2000);
}

bool isLightsOn() {
  return (digitalRead(LIGHTS));
}

void turnOnLights() {
  digitalWrite(LIGHTS, HIGH);
}

void turnOffLights() {
  digitalWrite(LIGHTS, LOW);
}

bool isBuzzerOn() {
  return (digitalRead(BUZZER));
}

void bipBuzzer() {
  buzzerPreviewStatus = isBuzzerOn();
  turnOffBuzzer();
  turnOnBuzzer();
  delay(250);
  turnOffBuzzer();
  delay(250);
  turnOnBuzzer();
  delay(250);
  turnOffBuzzer();

  if (buzzerPreviewStatus)
    turnOnBuzzer();
  else
    turnOffBuzzer();
}

void turnOnBuzzer() {
  digitalWrite(BUZZER, HIGH);
}

void turnOffBuzzer() {
  digitalWrite(BUZZER, LOW);
}

bool isPumpOn() {
  return (!digitalRead(PUMP));
}

void turnOnPump() 
{
  digitalWrite(PUMP, LOW);
}

void turnOffPump() 
{
  digitalWrite(PUMP, HIGH);
}

/////////////////////////////////////////////////////
void readSensors() 
{
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  flame = 100 - map(analogRead(FLAME_SENSOR), 0, 4095, 0, 100);
  smoke = map(analogRead(SMOKE_SENSOR), 0, 4095, 0, 100);
  lights = 100 - map(analogRead(LDR_SENSOR), 0, 4095, 0, 100);
  soilMeasure = 100 - map(analogRead(SOIL_SENSOR), 0, 4095, 0, 100);

  smoke = (smoke<55)? 0 : smoke;
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Falha ao ler o sensor!"));
    temperature = humidity = 0;
  }
}

void analyseData()
{
  if (projectMode == 'A') 
  {
    if (soilMeasure < 10 && !flagSoil) {
      flagSoil = true;
      turnOnPump();
    } else if (soilMeasure > 50 && flagSoil) {
      flagSoil = false;
      turnOffPump();
    }

    if (lights < 50)
      turnOnLights();
    else
      turnOffLights();

    if (flame >= LIMIAR_FLAME && !flagFlame) {
      flagFlame = true;
      flagAlarm = true;
    } else if (flame < LIMIAR_FLAME && flagFlame) {
      flagFlame = false;
      flagAlarm = false;
    }

    if (smoke >= LIMIAR_SMOKE && !flagSmoke) {
      flagSmoke = true;
      flagAlarm = true;
    } else if (flame < LIMIAR_SMOKE && flagSmoke) {
      flagSmoke = false;
      flagAlarm = false;
    }

    if (flagFlame && flagSmoke)
      turnOnPump();
    else if(!flagSoil)
      turnOffPump();  
  }

  if(DEBUG){
    Serial.println("TEMP.....:" + String(temperature) + "%");
    Serial.println("HUM......:" + String(humidity) + "%");
    Serial.println("CHAMAS...:" + String(flame) + "%");
    Serial.println("FUMO.....:" + String(smoke) + "%");
    Serial.println("HUM. SOLO:" + String(soilMeasure) + "%");
    Serial.println("CLAREZA..:" + String(lights) + "%");
    Serial.println("LUZES....:" + isLightsOn() ? "ON" : "OFF");
    Serial.println("BOMBA....:" + isPumpOn() ? "ON" : "OFF");
    Serial.println("ESTADO PROEJCTO....:" + String(projectMode));
  }

}

void printDataLCD() {
  static byte counter=0;
  lcd.init();
  lcd.clear();
  lcd.setCursor(0, 0);

  switch(counter)
  {
    case 0:
      lcd.print("TEMPERTURA:");
      lcd.print(temperature);
      lcd.print("*C");

      lcd.setCursor(0, 1);
      lcd.print("HUM. AR...:");
      lcd.print(humidity);
      lcd.print("%");
    break;

    case 1:
      lcd.print("CHAMAS....:");
      lcd.print(flame);
      lcd.print("%");

      lcd.setCursor(0, 1);
      lcd.print("FUMO......:");
      lcd.print(smoke);
      lcd.print("%");
    break;

    case 2:
      lcd.print("HUM.  SOLO:");
      lcd.print(soilMeasure);
      lcd.print("%");

      lcd.setCursor(0, 1);
      lcd.print("FUMO......:");
      lcd.print(smoke);
      lcd.print("%");
    break;

    case 3:
      lcd.print("CLAREZA...:");
      lcd.print(lights);
      lcd.print("%");

      lcd.setCursor(0, 1);
      lcd.print("LUZES.....:");
      lcd.print(isLightsOn() ? "ON" : "OFF");
    break;

    case 4:
      lcd.print("BOMBA.....:");
      lcd.print(isPumpOn() ? "ON" : "OFF");
    break;
  }

  lcd.setCursor(19, 0);
  lcd.print(projectMode);
  if (++counter >= 5) counter = 0;
}

void buttonsHandler() {
  if (!digitalRead(BTN_STATUS)) 
  {
    projectMode = (projectMode == 'A') ? 'M' : 'A';
    flagAlarm=false;
    flagSmoke=false;
    flagFlame=false;
    flagSoil =false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("####################");
    lcd.setCursor(0, 1);
    lcd.print(" MUDANDO DE ESTADO ");
    lcd.setCursor(0, 2);
    lcd.print(" ESTADO " + String((projectMode == 'A') ? "AUTOMATICO" : "MANUAL"));
    lcd.setCursor(0, 3);
    lcd.print("####################");
    while (!digitalRead(BTN_STATUS));
  }

  if (!digitalRead(BTN_PUMP) && projectMode == 'M') 
  {
    unsigned long int pressTime = millis();
    while (!digitalRead(BTN_PUMP)) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("####################");
      lcd.setCursor(0, 1);
      lcd.print(" BOTAO PRESSIONADO ");
      lcd.setCursor(0, 2);
      lcd.print("      TEMPO:" + String((millis() - pressTime) / 1000) + "s");
      lcd.setCursor(0, 3);
      lcd.print("####################");
      delay(500);
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("####################");
    lcd.setCursor(0, 1);


    if (millis() - pressTime > 3000) {
      if (isLightsOn())
        turnOffLights();
      else
        turnOnLights();

      lcd.print(" ESTADO DAS LUZES ");
      lcd.setCursor(5, 2);
      lcd.print(String((isLightsOn()) ? "LIGADAS" : "DESLIGADAS"));
    } else {
      if (isPumpOn())
        turnOffPump();
      else
        turnOnPump();

      lcd.print(" ESTADO DA BOMBA ");
      lcd.setCursor(5, 2);
      lcd.print(String((isPumpOn()) ? "LIGADA" : "DESLIGADA"));
    }

    lcd.setCursor(0, 3);
    lcd.print("####################");
  }
}