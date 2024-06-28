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

/******* Fill-in information from Blynk Device Info here *******/
#define BLYNK_TEMPLATE_ID    "TMPL2Ztxrhb6i"
#define BLYNK_TEMPLATE_NAME  "Quickstart Template"
#define BLYNK_AUTH_TOKEN     "kbk4xTfU3SqmPJwoPP6zbjJVBqsp3eYl"
/******* Comment this out to disable prints and save space *******/
#define BLYNK_PRINT Serial

//////////////  PIN CONFIGURATIONS /////////////
#define LED 2                              /////
#define LIGHTS 4                           /////
#define BUZZER 5                           /////
#define FUN 12                             /////
#define HEATER 13                          /////
#define DHTPIN 15                          /////
#define BTN_LOADS 18                        /////
#define BTN_STATUS 19                      /////
#define PUMP 23                            /////
#define FLAME_SENSOR 34                    /////
#define SMOKE_SENSOR 35                    /////
#define LDR_SENSOR 39       //pino VN      /////
#define SOIL_SENSOR 36      //pino VP      /////
////////////////////////////////////////////////

///////////// OTHER CONSTANTS DEFINITIONS ////////
#define DEBUG false                           /////
#define DHTTYPE DHT11                        /////
#define LIMIAR_FLAME 65                      /////
#define LIMIAR_SMOKE 60                      /////
//////////////////////////////////////////////////

////////////  NETWORK CONFIGURATIONS /////////////
char ssid[] = "ESTUFA";                      /////
char pass[] = "123456789";                   /////
//////////////////////////////////////////////////

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
LiquidCrystal_I2C lcd(0x27, 16, 4);        /////
////////////////////////////////////////////////

//////// VARIABLES USED IN THE PROJECT //////////
byte cont = 0;                              /////
char projectMode = 'A';                     /////
unsigned long int timeDelay = 0;            /////
bool buzzerPreviewStatus = false;           /////
float temperature = 0, humidity = 0;        /////
bool flagSoil = false, flagAlarm = false;   /////
bool flagSmoke = false, flagFlame = false;  /////
int flame = 0, smoke = 0;                   /////
int lights = 0, soilMeasure = 0;            /////
/////////////////////////////////////////////////
String statusAlarm="";                      /////
String statusFun="", statusPump="";         /////
String statusHeater="";                     /////
String statusSmoke="", statusFlame="";      /////
String statusLights="", statusTemperature="";////
String statusHumidity="", statusSoilMeasure="";//
/////////////////////////////////////////////////


BLYNK_WRITE(V16)
{
  /* MUDANÇA DO MODO*/
  int value = param.asInt();
  projectMode = (value==0)?'A':'M';
  Serial.println("MUDANÇA DE ESTADO:"+String(value)+", PARA:"+String(projectMode));
}

BLYNK_WRITE(V17)
{
  /* ACIONANDO BOMBA */
  int value = param.asInt();
  if(projectMode =='M' && value==1)
    turnOnPump();
  else 
    turnOffPump();
  Serial.println("MUDANÇA DA BOMBA:"+String(value)+", PARA:"+isPumpOn());
}

BLYNK_WRITE(V18)
{
  /* ACIONANDO VENTILADOR */
  int value = param.asInt();
  if(projectMode =='M' && value==1)
    turnOnFun();
  else
    turnOffFun();  
    Serial.println("MUDANÇA DO VENTILADOR:"+String(value) + " PARA:"+isFunOn());
}

BLYNK_WRITE(V19)
{
  /* ACIONANDO RESISTENCIA TÉRMICA */
  int value = param.asInt();
  if(projectMode =='M' && value==1)
    turnOnHeater();
  else
    turnOffHeater();  
    Serial.println("MUDANÇA DO HEATER:"+String(value) + " PARA:"+isHeaterOn());
}

BLYNK_CONNECTED()
{
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url",         "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

void sendDataToBlynk()
{
  Blynk.virtualWrite(V0, lights);
  Blynk.virtualWrite(V1, statusLights);
  
  Blynk.virtualWrite(V2, statusFun);
  Blynk.virtualWrite(V3, statusPump);
  
  Blynk.virtualWrite(V4, soilMeasure);
  Blynk.virtualWrite(V5, statusSoilMeasure);

  Blynk.virtualWrite(V6, humidity);
  Blynk.virtualWrite(V7, statusHumidity);
    
  Blynk.virtualWrite(V8, temperature);
  Blynk.virtualWrite(V9, statusTemperature);

  Blynk.virtualWrite(V10, smoke);
  Blynk.virtualWrite(V11, statusSmoke);

  Blynk.virtualWrite(V12, flame);
  Blynk.virtualWrite(V13, statusFlame);  
  Blynk.virtualWrite(V14, statusAlarm);
  Blynk.virtualWrite(V15, (projectMode=='A')?"AUTOMATICO":"MANUAL");
}


void setup()
{
  initSetup();
  Serial.begin(115200);
  dht.begin();
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
  
  pinMode(FUN, OUTPUT);
  turnOffFun();

  pinMode(HEATER, OUTPUT);
  turnOffHeater();
  
  pinMode(PUMP, OUTPUT);
  turnOffPump();

  pinMode(LIGHTS, OUTPUT);
  turnOffLights();

  pinMode(BTN_STATUS, INPUT_PULLUP);
  pinMode(BTN_LOADS, INPUT_PULLUP);
  
  lcd.init();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("UNIV. METODISTA");
  lcd.setCursor(0, 1);
  lcd.print("  ESTUFA IoT ");
  lcd.setCursor(-4, 2);
  lcd.print("  ENG. FILIPE ");
  lcd.setCursor(-4, 3);
  lcd.print("   ENG. LEO   ");
  
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

bool isFunOn() {
  return (!digitalRead(FUN));
}

void turnOnFun() 
{
  digitalWrite(FUN, LOW);
}

void turnOffFun() 
{
  digitalWrite(FUN, HIGH);
}

bool isHeaterOn() {
  return (!digitalRead(HEATER));
}

void turnOnHeater() 
{
  digitalWrite(HEATER, LOW);
}

void turnOffHeater() 
{
  digitalWrite(HEATER, HIGH);
}

bool isPumpOn() {
  return (digitalRead(PUMP));
}

void turnOnPump() 
{
  digitalWrite(PUMP, HIGH);
}

void turnOffPump() 
{
  digitalWrite(PUMP, LOW);
}

/////////////////////////////////////////////////////
void readSensors() 
{
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  flame = 100 - map(analogRead(FLAME_SENSOR), 0, 4095, 0, 100);
  smoke = 100 - map(analogRead(SMOKE_SENSOR), 0, 4095, 0, 100);
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
    if (soilMeasure < 40 && !flagSoil) {
      soilMeasure = 0;
      flagSoil = true;
      turnOnPump();
    } else if (soilMeasure >= 60 && flagSoil) {
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
      statusFlame="FOGO DETECTADO";
    } else if (flame < LIMIAR_FLAME && flagFlame) {
      flagFlame = false;
      flagAlarm = false;
      statusFlame="SEM FOGO";
    }

    if (smoke >= LIMIAR_SMOKE && !flagSmoke) {
      flagSmoke = true;
      flagAlarm = true;
      statusSmoke = "FUMO DETECTADO";
    } else if (flame < LIMIAR_SMOKE && flagSmoke) {
      flagSmoke = false;
      flagAlarm = false;
      statusSmoke = "SEM FUMO";
    }

    if (flagFlame && flagSmoke)
      turnOnPump();
    else if(!flagSoil)
      turnOffPump();  
  }

  statusLights = isLightsOn() ? "LUZES ON" : "LUZES OFF";
  statusPump = isPumpOn() ? "BOMBA ON" : "BOMBA OFF";
  statusFun = isFunOn() ? "VENTILADOR ON" : "VENTILADOR OFF";
  statusHeater = isHeaterOn() ? "RES TERMICA ON" : "RES TERMICA OFF";
  statusAlarm= flagAlarm? "ALARME ON":"ALARME OFF";

  if (temperature > 27){
    statusTemperature = "ALTA";
    if(projectMode=='A'){
      turnOffHeater();
      turnOnFun();
    }
  }
  else if(temperature >= 20){
    statusTemperature = "NORMAL";
    if(projectMode=='A'){
      turnOffHeater();
      turnOffFun();
    }
  }
  else{
    statusTemperature = "BAIXA";
    if(projectMode=='A'){
      turnOnHeater();
      turnOffFun();
    }
  }


  if (humidity >= 85)
    statusHumidity = "ALTA";
  else if (humidity >= 45)
    statusHumidity = "NORMAL";
  else
    statusHumidity = "BAIXA";

  if (soilMeasure >= 85)
    statusSoilMeasure = "HUMIDO";
  else if (soilMeasure > 40)
    statusSoilMeasure = "NORMAL";
  else
    statusSoilMeasure = "SECO";

  
  if(DEBUG){
    Serial.println("TEMP.....:" + String(temperature) + "%");
    Serial.println("HUM......:" + String(humidity) + "%");
    Serial.println("CHAMAS...:" + String(flame) + "%");
    Serial.println("FUMO.....:" + String(smoke) + "%");
    Serial.println("HUM. SOLO:" + String(soilMeasure) + "%");
    Serial.println("CLAREZA..:" + String(lights) + "%");
    Serial.println("LUZES....:" +statusLights);
    Serial.println("BOMBA....:" +statusPump);
    Serial.println("ESTADO PROEJCTO....:" + String(projectMode));
  }
}

void printDataLCD() 
{
  static byte counter = 0;
  static bool flagS = false;
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

      lcd.setCursor(-4, 2);
      lcd.print("CHAMAS....:");
      lcd.print(flame);
      lcd.print("%");

      lcd.setCursor(-4, 3);
      lcd.print("FUMO......:");
      lcd.print(smoke);
      lcd.print("%");
    break;

    case 1:
      lcd.print("HUM.  SOLO:");
      lcd.print(soilMeasure);
      lcd.print("%");

      lcd.setCursor(0, 1);
      lcd.print("FUMO......:");
      lcd.print(smoke);
      lcd.print("%");

      lcd.setCursor(-4, 2);
      lcd.print("CLAREZA...:");
      lcd.print(lights);
      lcd.print("%");

      lcd.setCursor(-4, 3);
      lcd.print("LUZES.....:");
      lcd.print(statusLights);

    break;
    case 2:
      lcd.print("BOMBA.....:");
      lcd.print(statusPump);
      lcd.setCursor(0, 1);
      lcd.print("VENTILADOR:");
      lcd.print(statusFun);
      lcd.setCursor(-4, 2);
      lcd.print("RES TERMICA:");
      lcd.print(statusHeater);
      lcd.setCursor(-4, 3);
      lcd.print("ESTADO:");
      lcd.print((projectMode=='A')? "AUTOMATICO" : "MANUAL");
    break;
  }
  if (++counter >= 3) counter = 0;
}

void buttonsHandler() 
{
  if (!digitalRead(BTN_STATUS)) 
  {
    projectMode = (projectMode == 'A') ? 'M' : 'A';
    flagAlarm=false;
    flagSmoke=false;
    flagFlame=false;
    flagSoil =false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("################");
    lcd.setCursor(0, 1);
    lcd.print("MUDANDO ESTADO");
    lcd.setCursor(0, 2);
    lcd.print("ESTADO " + String((projectMode == 'A') ? "AUTOMATICO" : "MANUAL"));
    lcd.setCursor(0, 3);
    lcd.print("################");
    while (!digitalRead(BTN_STATUS));
  }

  if (!digitalRead(BTN_LOADS) && projectMode == 'M') 
  {
    unsigned long int pressTime = millis();
    int time =0;
    while (!digitalRead(BTN_LOADS)) {
      time = int((millis() - pressTime) / 1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("################");
      lcd.setCursor(0, 1);
      lcd.print("BOTAO ACIONADO");
      lcd.setCursor(-4, 2);
      lcd.print("TEMPO:" + String(time) + "s");
      lcd.setCursor(-4, 3);
      lcd.print("CARGA:"+selectLoadText(time));
      delay(500);
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("################");
    lcd.setCursor(0, 1);

    if (time>9) {
      if (isHeaterOn())
        turnOffHeater();
      else
        turnOnHeater();
      lcd.print("RES.TERMICA:");
      lcd.setCursor(5, 2);
      lcd.print(statusHeater);
    }
     
    else if (time>6) {
      if (isFunOn())
        turnOffFun();
      else
        turnOnFun();
      lcd.print("VENTILADOR: ");
      lcd.setCursor(5, 2);
      lcd.print(statusFun);
    }
    else if (time>3) {
      if (isPumpOn())
        turnOffPump();
      else
        turnOnPump();

      lcd.print("ESTADO DA BOMBA ");
      lcd.setCursor(5, 2);
      lcd.print(statusPump);
    }
    else {
      if (isLightsOn())
        turnOffLights();
      else
        turnOnLights();

      lcd.print(" ESTADO DAS LUZES ");
      lcd.setCursor(5, 2);
      lcd.print(statusLights);
    } 
    lcd.setCursor(-4, 3);
    lcd.print("################");
  }
}

String selectLoadText(int time)
{ 
  if(time>=9)
    return "RES. TERMICA";
  else if(time>=6)
    return "VENTILADOR";
  else if(time>=3)
    return "BOMBA";
  return "LUZES";
}