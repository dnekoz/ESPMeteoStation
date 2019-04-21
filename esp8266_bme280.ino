#include "Wire.h"
#include "Adafruit_Sensor.h"
#include "Adafruit_BME280.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Nokia 5110
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// ESP8266 Software SPI (slower updates, more fledisplay.setContrast(50);xible pin options):
// pin 14 - Serial clock out (SCLK)
// pin 13 - Serial data out (DIN)
// pin 12 - Data/Command select (D/C)
// pin 5 - LCD chip select (CS)
// pin 4 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(14, 13, 12, 5, 4);

const int DELAY = 3000;
const int STARTUP_DELAY = 500;

Adafruit_BME280 bme;
const byte I2C_SDA = D4;
const byte I2C_SCL = D3;

const char *ssid = "dn_24G"; // Имя вайфай точки доступа
const char *pass = "ASUSnet1"; // Пароль от точки доступа

const char *mqtt_server = "192.168.1.158"; // Имя сервера MQTT
const int   mqtt_port = 1883; // Порт для подключения к серверу MQTT
const char *mqtt_user = "mos_user"; // Логи от сервер
const char *mqtt_pass = "Qq12345"; // Пароль от сервера

float tempC = 0;
float humidity = 0;
float pressurePascals = 0;
float pressureMmOfMercury = 0;

boolean isConnectMQTT = false;

WiFiClient wclient; 
PubSubClient client(wclient, mqtt_server, mqtt_port);

void(* resetFunc) (void) = 0;

void setup() 
{
 Serial.begin(115200);
 Serial.println("Start");

  display.begin();
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(60);

  display.display(); // show splashscreen
  delay(2000);
  display.clearDisplay();   // clears the screen and buffer
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("Hello, BME280!");
  display.display();
  delay(DELAY);

 Wire.begin(I2C_SDA,I2C_SCL);
 Wire.setClock(100000); 
 if(!bme.begin())
 {
  Serial.println("Could not find a valid BME280 sensor, check wiring!");
 while (1)
 {
 yield();
 delay(DELAY);
 }
 }
  display.clearDisplay();   // clears the screen and buffer
   // text display tests
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("BME280 ready!");
  display.display();
  delay(STARTUP_DELAY);

}

void loop() 
{

  if (millis() > 7200000) {resetFunc();}
  // подключаемся к wi-fi
if (WiFi.status() != WL_CONNECTED) {
  // Serial.print("Connecting to ");
  // Serial.print(ssid);
  // Serial.println("...");
  WiFi.begin(ssid, pass);

if (WiFi.waitForConnectResult() != WL_CONNECTED)
return;
 Serial.println("WiFi connected");
}
// подключаемся к MQTT серверу
if (WiFi.status() == WL_CONNECTED) {
if (!client.connected()) {
 Serial.println("Connecting to MQTT server");
if (client.connect(MQTT::Connect("esp8266_1").set_auth(mqtt_user, mqtt_pass))) {
  isConnectMQTT = true;
// Serial.println("Connected to MQTT server");
} else {
  isConnectMQTT = false;
 Serial.println("Could not connect to MQTT server");
}
}
}
 tempC    = round((bme.readTemperature()-1.2)*10)/10;
 display.clearDisplay();   // clears the screen and buffer
   // text display tests
 display.setTextSize(1);
 display.setTextColor(BLACK);
 display.setCursor(0,0);
 display.print("Temp: ");
 display.println(tempC);
 display.display();
 
 humidity = round(bme.readHumidity()*10)/10;
 pressurePascals = bme.readPressure();
 

// Print to serial monitor
// printToSerial(tempC, humidity, pressurePascals);

 delay(DELAY);
 if (client.connected() and isConnectMQTT){
   client.loop();
   SendToMQTTBroker(tempC, humidity, pressurePascals);
}
}

// Функция отправки показаний с термодатчика
void SendToMQTTBroker(float tempC, float humidity, float pressurePascals){
client.publish("home/temperature",String(tempC)); // отправляем в топик значение температуры
client.publish("home/humidity",String(humidity)); // отправляем в топик значение влажности
client.publish("home/pressureMMHg",String(pressurePascals*7.5/1000)); // отправляем в топик значение давления в мм рт. ст.
}

void printToSerial(float tempC, float humidity, float pressurePascals)
{

// Serial.println("Temperature:");
 printValueAndUnits(tempC, "*C");
 // Serial.println("");

// Serial.println("Pressure:");
 printValueAndUnits(pressurePascals, "Pa");
 printValueAndUnits(pressurePascals*7.5/1000, "mmHg");
 // Serial.println("");

// Humidity
 // Serial.println("Humidity:");
 printValueAndUnits(humidity, "%");
 // Serial.println("");
}

void printValueAndUnits(float value, String units)
{
  Serial.print(" ");
  Serial.print(value);
  Serial.print(" ");
  Serial.println(units);
}
