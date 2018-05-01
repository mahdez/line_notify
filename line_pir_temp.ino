//void Line_Notify1(String message1) ;
void Line_Notify12(String message2) ;

#include <ESP8266WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <SPI.h>

#define DHTTYPE DHT11   
#define DHTPIN D4  
DHT dht(DHTPIN, DHTTYPE, 15);

///////////////////////////////////// pir detector //////////////////////////////////

#define pirPin D2
int valPIR;

////////////////////////////////////// bht1750 /////////////////////////////////////

int  BH1750address = 0x23;
byte buff[2];
/////////////////////////////////// CONNECT WIFI ///////////////////////////////////////////////

#define WIFI_SSID "xxxxxxxx"
#define WIFI_PASSWORD "xxxxxxxx"

//////////////////////////////// get token line ///////////////////////////////////////////////////////

#define LINE_TOKEN_PIR "" // LINE Notify token 1
#define LINE_TOKEN_TMP "" // LINE Notify token 2

////////////////////////////////////////////////////////////////////////////////////////////////////



String message1 = "%E0%B8%A1%E0%B8%B5%E0%B8%9C%E0%B8%B9%E0%B9%89%E0%B8%9A%E0%B8%B8%E0%B8%81%E0%B8%A3%E0%B8%B8%E0%B8%81";
String message2 = "%E0%B8%AD%E0%B8%B8%E0%B8%93%E0%B8%AB%E0%B8%A0%E0%B8%B9%E0%B8%A1%E0%B8%B4";
String message3 = "%E0%B8%84%E0%B8%A7%E0%B8%B2%E0%B8%A1%E0%B8%8A%E0%B8%B7%E0%B9%89%E0%B8%99%20%3A%20";
String message4 = "%20%25%20%E0%B8%84%E0%B8%A7%E0%B8%B2%E0%B8%A1%E0%B9%80%E0%B8%82%E0%B9%89%E0%B8%A1%E0%B9%81%E0%B8%AA%E0%B8%87%3A";
String message5 = "%E0%B8%9B%E0%B8%A5%E0%B8%AD%E0%B8%94%E0%B8%A0%E0%B8%B1%E0%B8%A2";
String message6 = "%E0%B8%A1%E0%B8%B5%E0%B8%9C%E0%B8%B9%E0%B9%89%E0%B8%9A%E0%B8%B8%E0%B8%81%E0%B8%A3%E0%B8%B8%E0%B8%81";
String message7 = "%E0%B8%99%E0%B9%89%E0%B8%AD%E0%B8%A2%E0%B9%80%E0%B8%81%E0%B8%B4%E0%B8%99%E0%B9%84%E0%B8%9B";

bool beep_state = false;
bool send_state = false;
uint32_t ts, ts1, ts2;
uint16_t val=0;
void setup() {

  Serial.begin(9600);
  Serial.println();

  Wire.begin();
  while(!Serial);
  Serial.println("Hello");
  Serial.flush();
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  dht.begin();

  Serial.println("connecting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("connecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  delay(5000);
  Serial.println("Ready!!");
  read_sensor();
  ts = ts1 = ts2 = millis();
}

void loop() {

  ts = millis();
  val=0;
  int i;
  
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  
  BH1750_Init(BH1750address);
  delay(200);
  if(2==BH1750_Read(BH1750address))
  {
    val=((buff[0]<<8)|buff[1])/1.2; 
    Serial.flush();
  }
  delay(200);


  delay(500);
  
  if ((ts - ts2 >= 10000) && (WiFi.status() == WL_CONNECTED)) {
    read_sensor();
    Serial.print(val,DEC);     
    Serial.println("[lx]");
    
    valPIR = digitalRead(pirPin);
  if (valPIR == LOW){
    Line_Notify2(message5);
    
  }else{
    Line_Notify2(message6);
   
  } 
  }

  if ((ts - ts1 >= 5000) && (beep_state == true)) {
    beep_state = false;
  }
  delay(10);

  
  
}



////////////////////////////////////////////////////////////////

void read_sensor() {

  
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
    
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" % ");
  Serial.print("PIR Sensor: ");
  Serial.println(valPIR);
  Line_Notify2(message2 + " " + t + "%C2%B0C" + " "+ message3 + " " + h + message4 + val + "lx"  );
  
}

void Line_Notify2(String message2) {
  WiFiClientSecure client;
  if (!client.connect("notify-api.line.me", 443)) {
    Serial.println("connection failed");
    return;
  }
  String req = "";
  req += "POST /api/notify HTTP/1.1\r\n";
  req += "Host: notify-api.line.me\r\n";
  req += "Authorization: Bearer " + String(LINE_TOKEN_TMP) + "\r\n";
  req += "Cache-Control: no-cache\r\n";
  req += "User-Agent: ESP8266\r\n";
  req += "Content-Type: application/x-www-form-urlencoded\r\n";
  req += "Content-Length: " + String(String("message=" + message2).length()) + "\r\n";
  req += "\r\n";
  req += "message=" + message2;
  client.print(req);
  delay(30);
  ts2 = millis();
  // Serial.println("-----");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r") {
      break;
    }
    //Serial.println(line);
  }
  // Serial.println("-----");
}

int BH1750_Read(int address) //
{
  int i=0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while(Wire.available()) //
  {
    buff[i] = Wire.read();  // receive one byte
    i++;
  }
  Wire.endTransmission();  
  return i;
}

void BH1750_Init(int address) 
{
  Wire.beginTransmission(address);
  Wire.write(0x10);//1lx reolution 120ms
  Wire.endTransmission();
} 

