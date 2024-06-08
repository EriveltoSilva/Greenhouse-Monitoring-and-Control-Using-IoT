/**************************************************
    AUTHOR: EriveltoSilva                        **
    FOR:                                         **
    CREATED AT:25-02-2024                        **
    Principais Componentes usados:               **
      *** 1-ESP32 (1)                            **
      *** 2-DHT11                                **
      *** 3-Sensor de Chama                      **
      *** 4-Sensor de Fumo (MQ135)               **
      *** 5-Sensor de Luz                        **
      *** 6-Cooler                               **
**************************************************/

////////////////// Libraries Used  ////////////////
#include <WiFi.h>                             /////
#include "DHT.h"                              /////
#include <Wire.h>                             /////
#include "SPIFFS.h"                           /////
#include <AsyncTCP.h>                         /////
#include <ArduinoJson.h>                      /////
#include <ESPAsyncWebServer.h>                /////
#include <LiquidCrystal_I2C.h>                /////
#include <IOXhop_FirebaseESP32.h>             /////
///////////////////////////////////////////////////

////////////////  PIN CONFIGURATIONS ///////////////
#define LED 2                                  /////
#define LIGHTS 4                               /////
#define BUZZER 5                               /////
#define DHTPIN 15                              /////
#define BTN_PUMP 18                            /////
#define BTN_LIGHTS 12                          /////
#define BTN_STATUS 19                          /////
#define PUMP 23                                /////
#define FLAME_SENSOR 34                        /////
#define SMOKE_SENSOR 35                        /////
#define LDR_SENSOR 39                          /////
#define SOIL_SENSOR 36                         /////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
#define FIREBASE_HOST "https://parque-control-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "AIzaSyDljBC-KlS1MTJTXcNzbxsK-ROP30dnqsU"
////////////////////////////////////////////////////

///////////// OTHER CONSTANTS DEFINITIONS /////////
#define DHTTYPE DHT11                         /////
#define LIMIAR_FLAME 65                       /////
#define LIMIAR_SMOKE 60                       /////
///////////////////////////////////////////////////

/////////////  NETWORK CONFIGURATIONS /////////////
#define SSID "ESTUFA"                         /////
#define PASSWORD "123456789"                  /////
///////////////////////////////////////////////////

///////// VARIABLES USED IN THE PROJECT ///////////
char status = 'A';                            /////
unsigned long int timeDelay = 0;              /////
bool buzzerPreviewStatus = false;             /////
float temperature = 0, humidity = 0;          /////
bool flagSoil = false, flagAlarm=false;       /////
bool flagSmoke = false, flagFlame = false;    /////
int flame = 0, smoke = 0, lights = 0, soil = 0;////
///////////////////////////////////////////////////

///////////////  OBJECTS DEFINITIONS  /////////////
DHT dht(DHTPIN, DHTTYPE);                     /////
AsyncWebServer server(80);                    /////
LiquidCrystal_I2C lcd(0x27, 20, 4);           /////
///////////////////////////////////////////////////

////////////// FUNCTION DEFINITIONS////////////////
void wifiConfig();                            /////
void initConfig();                            /////
bool initMyFS();                              /////
bool isUser(String, String);                  /////
void serverHandlers();                        /////
void saveUserFirebase(String, String);        /////
bool isPumpOn();                              /////
bool isBuzzerOn();                            /////
bool isLightsOn();                            /////
void readSensors();                           /////
void analyseData();                           /////
void turnOnLights();                          /////
void turnOffLights();                         /////
void turnOnPump();                            /////
void turnOffPump();                           /////
void turnOnBuzzer();                          /////
void turnOffBuzzer();                         /////
void buttonsHandler();                        /////
///////////////////////////////////////////////////

///////////////////////////////////////////////////
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

///////////////////////////////////////////////////
void setup() {
  initConfig();
  wifiConfig();
  Serial.println(initMyFS() ? " ## PARTIÇÃO SPIFFS MONTADA! ##" : " ## ERRO MONTANDO A PARTIÇÃO SPIFFS ##");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  serverHandlers();
  Serial.println(" ##-------SISTEMA DE DOMÓTICA!--------##");
}

////////////////////////////////////////////////////
void loop() {
  static byte cont = 0;
  if (millis() - timeDelay > 1000) {
    timeDelay = millis();
    readSensors();
    analyseData();
    setAlarm(flagAlarm);
    if (++cont == 2) {
      cont = 0;
      printLCD();
    }
    digitalWrite(LED, !digitalRead(LED));
  }

  buttonsHandler();
  delay(20);
}

////////////////////////////////////////////////////
void initConfig() {
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

  Serial.begin(115200);
  delay(500);
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("#      ISTA       #");
  lcd.setCursor(0, 1);
  lcd.print("# FEIRA TECNOLOG. #");
  lcd.setCursor(0, 2);
  lcd.print("#      ESTUFA     #");

  delay(1000);
  Serial.println("CONFIGURAÇÕES INICIAS SETADAS!");
  delay(3000);
}

/////////////////////////////////////////////////////
void wifiConfig() {
  if (WiFi.status() == WL_CONNECTED)
    return;
  Serial.println();
  Serial.print("CONECTANDO A WIFI:");
  Serial.println(SSID);
  Serial.print("PROCURANDO");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CONECTANDO A REDE:");
  lcd.setCursor(5, 1);
  lcd.print(SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  for (int i = 0; WiFi.status() != WL_CONNECTED; i++) {
    digitalWrite(LED, !digitalRead(LED));
    delay(150);
    Serial.print(".");
    lcd.print(".");
    if (i == 100)
      ESP.restart();
  }
  Serial.print("\nCONECTADO AO WIFI NO IP:");
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("##  CONECTADO ##");
  lcd.setCursor(0, 1);
  lcd.print("REDE:");
  lcd.print(SSID);
  lcd.setCursor(0, 2);
  lcd.print("IP:");
  lcd.print(WiFi.localIP());
  delay(5000);
}

//////////////////////////////////////////////////////
void readSensors() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  // flame       = map(analogRead(FLAME_SENSOR), 0, 4095, 0, 100);
  flame = 100 - map(analogRead(FLAME_SENSOR), 0, 4095, 0, 100);
  //smoke = 100 - map(analogRead(SMOKE_SENSOR), 0, 4095, 0, 100);

  int fumo = analogRead(SMOKE_SENSOR);
  //Serial.println("----------------------------------------------------------> Fumo:"+String(fumo));
  smoke = map(fumo, 0, 4095, 0, 100);
  smoke = (smoke<55)? 0 : smoke;
  /*
   * 1401, 1460
   * 2500
   * 4095 --- 100
   * 2500  -- x
  */

  lights = 100 - map(analogRead(LDR_SENSOR), 0, 4095, 0, 100);
  soil = 100 - map(analogRead(SOIL_SENSOR), 0, 4095, 0, 100);

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Falha ao Ler os DHT11! Verifique as Conexões!"));
    humidity = temperature = 0;
  }
}

void analyseData() {
  if (status == 'A') {
    if (soil < 10 && !flagSoil) {
      flagSoil = true;
      turnOnPump();
    } else if (soil > 50 && flagSoil) {
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

  Serial.println("TEMP.....:" + String(temperature) + "%");
  Serial.println("HUM......:" + String(humidity) + "%");
  Serial.println("CHAMAS...:" + String(flame) + "%");
  Serial.println("FUMO.....:" + String(smoke) + "%");
  Serial.println("HUM. SOLO:" + String(soil) + "%");
  Serial.println("CLAREZA..:" + String(lights) + "%");
  Serial.println("LUZES....:" + isLightsOn() ? "ON" : "OFF");
  Serial.println("BOMBA....:" + isPumpOn() ? "ON" : "OFF");
  Serial.println("ESTADO PROEJCTO....:" + String(status));
}

////////////////////////////////////////////////////////////////////////
bool initMyFS() {
  return (SPIFFS.begin(true));
}

bool isUser(String username, String password) {
  String pass = Firebase.getString("/users/" + username);
  return (pass.equals(password));
}

void saveUserFirebase(String username, String password) {
  Firebase.setString("/users/" + username, password);
}

void buttonsHandler() {
  if (!digitalRead(BTN_STATUS)) {
    status = (status == 'A') ? 'M' : 'A';
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
    lcd.print(" ESTADO " + String((status == 'A') ? "AUTOMATICO" : "MANUAL"));
    lcd.setCursor(0, 3);
    lcd.print("####################");
    while (!digitalRead(BTN_STATUS))
      ;
  }

  if (!digitalRead(BTN_PUMP) && status == 'M') {
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

void turnOnPump() {
  digitalWrite(PUMP, LOW);
}

void turnOffPump() {
  digitalWrite(PUMP, HIGH);
}

void printLCD() {
  static byte flag = 0;
  lcd.init();
  lcd.clear();
  lcd.setCursor(0, 0);

  switch (flag) {
    case 0:
      lcd.print("TEMPERTURA:");
      lcd.print(temperature);
      lcd.print("*C");

      lcd.setCursor(0, 1);
      lcd.print("HUM. AR...:");
      lcd.print(humidity);
      lcd.print("%");

      lcd.setCursor(0, 2);
      lcd.print("CHAMAS....:");
      lcd.print(flame);
      lcd.print("%");

      lcd.setCursor(0, 3);
      lcd.print("FUMO......:");
      lcd.print(smoke);
      lcd.print("%");
      break;

    case 1:
      lcd.print("HUM.  SOLO:");
      lcd.print(soil);
      lcd.print("%");

      lcd.setCursor(0, 1);
      lcd.print("CLAREZA...:");
      lcd.print(lights);
      lcd.print("%");

      lcd.setCursor(0, 2);
      lcd.print("LUZES.....:");
      lcd.print(isLightsOn() ? "ON" : "OFF");

      lcd.setCursor(0, 3);
      lcd.print("BOMBA.....:");
      lcd.print(isPumpOn() ? "ON" : "OFF");
      break;
  }

  lcd.setCursor(19, 0);
  lcd.print(status);
  if (++flag == 2) flag = 0;
}


///////////////////////////////////////////////
void setAlarm(bool status) {
  if (status) 
  {
    for (int i = 0; i < 100; i++)
    {
      digitalWrite(BUZZER, HIGH);
      delay(1);
      digitalWrite(BUZZER, LOW);
      delay(1);
    }
    delay(100);

    for (int i = 0; i < 200; i++) {
      digitalWrite(BUZZER, HIGH);
      delay(1);
      digitalWrite(BUZZER, LOW);
      delay(2);
    }
    delay(900);
  } else {
    digitalWrite(BUZZER, LOW);
    delay(1);
  }
}


////////////////////////////////////////////////////////////////////////
void serverHandlers() {
  // Route to load bootstrap.min.css file
  server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/bootstrap.min.css", "text/css");
  });

  // Route to load bootstrap.bundle.min.js file
  server.on("/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/bootstrap.bundle.min.js", "text/javascript");
  });

  // Route to load login.css file
  server.on("/login.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/login.css", "text/css");
  });

  // Route to load dashboard.css file
  server.on("/dashboard.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/dashboard.css", "text/css");
  });

  // Route to load dashboard.js file
  server.on("/dashboard.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/dashboard.js", "text/javascript");
  });




  server.on("/loginIcon.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/loginIcon.png", "image/jpeg");
  });
  server.on("/humidity.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/humidity.png", "image/jpeg");
  });

  server.on("/lampOff.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/lampOff.png", "image/jpeg");
  });

  server.on("/lampOn.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/lampOn.png", "image/jpeg");
  });

  server.on("/fire.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/fire.png", "image/jpeg");
  });

  server.on("/nofire.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/nofire.png", "image/jpeg");
  });

  server.on("/nosmoke.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/nosmoke.png", "image/jpeg");
  });

  server.on("/smoke.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/smoke.png", "image/jpeg");
  });

  server.on("/tel.jpg", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/tel.jpg", "image/jpeg");
  });

  server.on("/temperature.jpeg", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/temperature.jpeg", "image/jpeg");
  });

  server.on("/pumpOff.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/pumpOff.png", "image/jpeg");
  });

  server.on("/pumpOn.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/pumpOn.png", "image/jpeg");
  });

  server.on("/estufa-capa.jpg", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/estufa-capa.jpg", "image/jpeg");
  });

  server.on("/soil.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/soil.png", "image/jpeg");
  });




  /*--------------------------ENDPOINS---------------------------*/
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("-------->redirecionando para login.html");
    request->redirect("/login");
  });



  // POST request para login
  server.on("/getin", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.println("------> Req de /getin do login.html");
    if (!(request->hasParam("username", true) && request->hasParam("password", true))) {
      Serial.println("---> Erro... Parametros de Login Incompletos!\nRedirecionando para ----> login.html");
      request->redirect("/login");
    } else {
      String username = request->getParam("username", true)->value();
      String password = request->getParam("password", true)->value();
      if (username.equals("admin") && password.equals("otlevire")) {
        Serial.println("--->Username e Senha de Admin Válidos!\nRedirecionando para ----> register.html");
        request->redirect("/register");
      } else {
        if (!isUser(username, password)) {
          Serial.println("--->Username e Senha Inválidos!\nRedirecionando para ---> login.html");
          request->redirect("/login");
        } else {
          Serial.println("--->Username e Senha Válidos!\nRedirecionando para ---> index.html");
          request->redirect("/dashboard");
        }
      }
    }
  });


  // POST request para Cadastro de Usuario
  server.on("/make-register", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.println("------> Req de /register do register.html");
    if (!(request->hasParam("username", true) && request->hasParam("passwordUser", true) && request->hasParam("passwordAdmin", true))) {
      Serial.println("---> Erro... Parametros de Login Incompletos!\nRedirecionando para ----> login.html");
      request->redirect("/login");
    } else {
      String username = request->getParam("username", true)->value();
      String passwordUser = request->getParam("passwordUser", true)->value();
      String passwordAdmin = request->getParam("passwordAdmin", true)->value();

      if (passwordAdmin.equals("otlevire")) {
        Serial.println("--->Admin Válido! Salvando dados do novo usuario no Firebase...");
        Firebase.setString("/users/" + username, passwordUser);
        //saveUserFirebase(username, passwordUser);
        Serial.println("Dados do Usuario Salvos!");
        request->redirect("/login");
      } else {
        Serial.println("--->Admin Inválido!Redirecionando para register.html");
        request->redirect("/register");
      }
    }
  });

  // Route for login / web page
  server.on("/login", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("--------> login.html");
    request->send(SPIFFS, "/login.html");
  });

  // Route for root / web page
  server.on("/register", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("--------> register.html");
    request->send(SPIFFS, "/register.html");
  });

  // Route for root / web page
  server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("--------> dashboard.html");
    request->send(SPIFFS, "/dashboard.html");
  });



  server.on("/dados", HTTP_GET, [](AsyncWebServerRequest *request) {
    /*
      D ------- 0
      temp ---- 1
      humid --- 2
      soil ---- 3
      flame --- 4
      smoke --- 5
      light --- 6
      pump ---- 7
      status -- 8
    */
    String dataStored = "D*" + String(temperature) + "*" + String(humidity) + "*" + String(soil) + "*" + String(flame) + "*" + String(smoke) + "*" + String(isLightsOn() ? "1" : "0") + "*" + String(isPumpOn() ? "1" : "0") + "*" + String(status) + "*";
    Serial.println("--------> Dados:" + dataStored);
    String resp = "{\"status\":\"success\", \"data\":\"" + dataStored + "\"}";
    request->send(200, "application/json", resp);
  });

  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("--------> mode");
    status = (status == 'A') ? 'M' : 'A';
    flagAlarm=false;
    flagSmoke=false;
    flagFlame=false;
    request->send(200, "application/json", "{\"status\":\"success\"}");
  });

  server.on("/lights", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("--------> luzes");
    if (status == 'M') {
      if (isLightsOn())
        turnOffLights();

      else
        turnOnLights();
      request->send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      request->send(200, "application/json", "{\"status\":\"error\", \"message\":\"O Sistema está no modo Automático\"}");
    }
  });

  server.on("/pump", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("--------> pump");
    if (status == 'M') {
      if (isPumpOn())
        turnOffPump();
      else
        turnOnPump();
      request->send(200, "application/json", "{\"status\":\"success\"}");
    } else
      request->send(200, "application/json", "{\"status\":\"error\", \"message\":\"O Sistema está no modo Automático\"}");
  });


  server.onNotFound(notFound);
  server.begin();
}
