

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <UniversalTelegramBot.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "index.h" //Our HTML webpage contents with javascripts
#include "DHTesp.h"  //DHT11 Library for ESP
#define DHTpin 14    //D5 of NodeMCU is GPIO14

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHTesp dht;

const char* ssid = "XXXXXXXXXX";
const char* password = "XXXXXXXXXX";

#define BOTtoken "XXXXXXXXXXXXXXXX"
String chat_id = "XXXXXXXXXXXXX";
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
ESP8266WebServer server(80); //Server on port 80

void handleRoot() {
  String s = MAIN_page; //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}
int suhu, kelembapan;
float humidity, temperature;
byte termometru[8] = //icon for termometer
{
  B00100,
  B01010,
  B01010,
  B01110,
  B01110,
  B11111,
  B11111,
  B01110
};

byte picatura[8] = //icon for water droplet
{
  B00100,
  B00100,
  B01010,
  B01010,
  B10001,
  B10001,
  B10001,
  B01110,
};

void handleADC() {
  int a = analogRead(A0);
  //Ref 1: https://circuits4you.com/2019/01/11/nodemcu-esp8266-arduino-json-parsing-example/
  //Ref 2: https://circuits4you.com/2019/01/25/arduino-how-to-put-quotation-marks-in-a-string/
  String data = "{\"ADC\":\"" + String(a) + "\", \"Temperature\":\"" + String(temperature) + "\", \"Humidity\":\"" + String(humidity) + "\"}";

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); //Toggle LED on data request ajax
  server.send(200, "text/plane", data); //Send ADC value, temperature and humidity JSON to client ajax request

  //Get Humidity temperatue data after request is complete
  //Give enough time to handle client to avoid problems
  delay(dht.getMinimumSamplingPeriod());

  humidity = dht.getHumidity();
  temperature = dht.getTemperature();
  kelembapan = dht.getHumidity();
  suhu = dht.getTemperature();
  Serial.print(humidity, 1);
  Serial.print(temperature, 1);
  Serial.print(dht.toFahrenheit(temperature), 1);
}
 bool v = false;
void setup()
{
  
  Serial.begin(115200);
  Serial.println();
  lcd.init();
  lcd.backlight();
  lcd.createChar(1, termometru);
  lcd.createChar(2, picatura);
  //-------------------------------------------------------------
  //Ref 3: https://circuits4you.com/2019/01/25/interfacing-dht11-with-nodemcu-example/
  // Autodetect is not working reliable, don't use the following line
  // dht.setup(17);
  // use this instead:
//  dht.setup(DHTpin, DHTesp::DHT11); //for DHT11 Connect DHT sensor to GPIO 17
  dht.setup(DHTpin, DHTesp::DHT22); //for DHT22 Connect DHT sensor to GPIO 17
  //------------------------------------------------------------

  lcd.setCursor(0, 0);
  lcd.print("MONITORING SUHU");
  lcd.setCursor(1, 1);
  lcd.print("TUBUH MANUSIA");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to: ");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");
  pinMode(LED_BUILTIN, OUTPUT);
  while (WiFi.status() != WL_CONNECTED) {
    lcd.setCursor(14, 0);
    lcd.print(".");
    delay(500);
    lcd.setCursor(15, 0);
    lcd.print(".");
    delay(500);
    lcd.setCursor(15, 0);
    lcd.print(" ");
    delay(500);
    lcd.setCursor(14, 0);
    lcd.print(" ");
    delay(500);
  }
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Connected to:");
  lcd.setCursor(0, 1); lcd.print(ssid);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.on("/readADC", handleADC); //This page is called by java Script AJAX
  server.begin();                  //Start server
  Serial.println("HTTP server started");
  delay(3000);
  lcd.clear();
}

void loop() {
 
  server.handleClient();          //Handle client requests
  Serial.print("Suhu = ");        Serial.println(temperature);
  Serial.print("Temperature = "); Serial.println(humidity);
  lcd.setCursor(1, 0); lcd.write(1);
  lcd.setCursor(2, 0);lcd.print(":");
  lcd.setCursor(3, 0); lcd.print(suhu);lcd.print((char)223);
  lcd.setCursor(10, 0); lcd.write(2);
  lcd.setCursor(11, 0);lcd.print(":");
  lcd.setCursor(12, 0); lcd.print(kelembapan);lcd.print("%");
  lcd.setCursor(1, 1);lcd.print(WiFi.localIP());
  if (suhu > 37 && v == false) {
    bot.sendMessage(chat_id, "Suhu Tubuh >=37 derajat");
    v = !v;
  }
  else if (suhu < 20 && v) {
    bot.sendMessage(chat_id, "Suhu ruangan terlalu dingin");
    v = !v;
  }
  //  String suhu = "Intensitas suhu : ";
  //  suhu += int(temperature);
  //  suhu += " *C\n";
  //  suhu += "Suhu maksimal gaes!\n";
  //  bot.sendMessage(chat_id, suhu, "");
  Serial.println("Mengirim data sensor ke telegram");
  delay(1000);
}
