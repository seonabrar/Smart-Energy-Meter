#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "time.h"
#include <EEPROM.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <NTPClient.h>

// Energy calculation code is not pushed as it is confidential


// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
String ButtonStyle = "input[type=Scan] {background-color: #1F62B9; color: white;padding: 12px 20px; border: none; border-radius: 4px;cursor: pointer;text-align: center;}";
//Variables
int i = 0;
int statusCode;
const char* ssid = "text";
const char* passphrase = "text";
String st;
String content;
// Insert your network credentials
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
// Insert Firebase project API Key
#define API_KEY ""

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL ""
#define USER_PASSWORD ""

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL ""

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

float importunits = 0;
float Wapda_importunits = 0;
float Load_importunits = 0;
float exportunits = 0;
float Wapda_exportunits = 0;
float Load_exportunits = 0;
/*________________________ PIN DEFINITIONS  _____________________________*/
#define Button 0
/*________________________ GLOBAL VARIABLES  _____________________________*/
int val = 0;   
hw_timer_t * timer = NULL;
volatile int flag ;
int a = 0;
// Variable to save USER UID
String uid;
// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String WVPath = "/Grid_voltage";
String WIPath = "/Grid_current";
String WPPath = "/Grid_power";
String LVPath = "/Inverter_voltage";
String LIPath = "/Inverter_current";
String LPPath = "/Inverter_power";
String LpFPath = "/Inverter_Power_Factor";
String WpFPath = "/Grid_Power_Factor";
String WeUPath = "/Export_Units";
String WiUPath = "/Import_Units";
String LiUPath = "/Units_Generated";
String fPath = "/Frequency";
String tPath = "/Temperature";
String hPPath = "/Home_Power";
String hCPath = "/Home_Consumption";
// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 18000;
const int   daylightOffset_sec = 3600;

//Function Decalration
void launchWeb(void);
void setupAP(void);

int counter= 0;
// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 180000;


//Establishing Local server at port 80 whenever required
WebServer server(80);

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}


void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("Energy Meter", "123456789");
  Serial.println("softap");
  launchWeb();
  Serial.println("over");
}
void createWebServer()
{
  {
    server.on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html><h1>GREENWEND ENERGY </h1> ";
      content += "<head>\n"
                 "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
                 "<style>\n"
                 "body {font-family: Arial, Helvetica, sans-serif;}\n"
                 "* {box-sizing: border-box;}\n"
                 "\n"
                 "input[type=text], select, textarea {\n"
                 "  width: 100%;\n"
                 "  padding: 12px;\n"
                 "  border: 1px solid #ccc;\n"
                 "  border-radius: 4px;\n"
                 "  box-sizing: border-box;\n"
                 "  margin-top: 6px;\n"
                 "  margin-bottom: 16px;\n"
                 "  resize: vertical;\n"
                 "}\n"
                 "input[type=password], select, textarea {\n"
                 "  width: 100%;\n"
                 "  padding: 12px;\n"
                 "  border: 1px solid #ccc;\n"
                 "  border-radius: 4px;\n"
                 "  box-sizing: border-box;\n"
                 "  margin-top: 6px;\n"
                 "  margin-bottom: 16px;\n"
                 "  resize: vertical;\n"
                 "}\n"
                 "input[type=submit] {\n"
                 "  background-color: #04AA6D;\n"
                 "  color: white;\n"
                 "  padding: 12px 20px;\n"
                 "  border: none;\n"
                 "  border-radius: 4px;\n"
                 "  cursor: pointer;\n"
                 "}\n"
                 "\n"
                 "input[type=submit]:hover {\n"
                 "  background-color: #45a049;\n"
                 "}\n"
                 "\n"
                 "input[type=Scan] {\n"
                 "  background-color: #1F62B9;\n"
                 "  color: white;\n"
                 "  padding: 12px 20px;\n"
                 "  border: none;\n"
                 "  border-radius: 4px;\n"
                 "  cursor: pointer;\n"
                 "  text-align: center;\n"
                 "}\n"
                 "\n"
                 "input[type=Scan]:hover {\n"
                 "  background-color: #1F62B9;\n"
                 "}\n"
                 "\n"
                 "\n"
                 ".container {\n"
                 "  border-radius: 5px;\n"
                 "  background-color: #f2f2f2;\n"
                 "  padding: 20px;\n"
                 "  width: 600px;\n"
                 "}\n"
                 "</style>\n"
                 "</head>";

      content += ipStr;
      content += "<p>";
      content += st;
      content += " <input type=\"submit\" value=\"Scan\">";
      content += "<body>\n"
                 "\n"
                 "\n"
                 "<div class=\"container\">\n"
                 "   \n"
                 "  <form action=\"/setting\" method=\"get\">\n"
                 "    <label for=\"fname\">SSID</label>\n"
                 "    <input type=\"text\" id=\"fname\"  placeholder=\"Enter SSID\" autocomplete=\"current-password\" required=\"\" autofocus=\"\" name='ssid' length=32 >\n"
                 "\n"
                 "    <label for=\"lname\">PASSWORD </label>\n"
                 "    <input type=\"password\" id=\"lname\"  placeholder=\"Enter Password\" autocomplete=\"current-password\" required=\"\"name='pass' length=32>\n"
                 "\n"
                 "    <label for=\"subject\">USER EMAIL</label>\n"
                 "   <input type=\"text\" id=\"lname\" placeholder=\"Enter Email\" autocomplete=\"current-password\" required=\"\" name='email' length=32>\n"
                 "\n"
                 "    <label for=\"subject\">USER PASSWORD</label>\n"
                 "   <input type=\"text\" id=\"lname\" placeholder=\"Enter Password\" autocomplete=\"current-password\" required=\"\" name='upass' length=32>\n"
                 "\n"
                 "    <input type=\"submit\" value=\"Submit\">\n"
                 "  </form>\n"
                 "</div>\n"
                 "\n"
                 "</body>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      String qemail = server.arg("email");
      String qupass = server.arg("upass");
      if ((qsid.length() > 0) && (qpass.length() > 0) && (qemail.length() > 0) && (qupass.length() > 0)) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 128; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");
        Serial.println(qemail);
        Serial.println("");
        Serial.println(qupass);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        Serial.println("writing eeprom email:");
        for (int i = 0; i < qemail.length(); ++i)
        {
          EEPROM.write(64 + i, qemail[i]);
          Serial.print("Wrote: ");
          Serial.println(qemail[i]);
        }
        Serial.println("writing eeprom user password:");
        for (int i = 0; i < qupass.length(); ++i)
        {
          EEPROM.write(96 + i, qupass[i]);
          Serial.print("Wrote: ");
          Serial.println(qupass[i]);
        }
        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.restart();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);

    });
  }
}

void reconnect() {
  // Loop until we're reconnected
 
  char sid[] = "Enter WiFi";
  if(flag == 1)
  {
    for (int i = 0; i < 10; ++i)
         {
           EEPROM.write(i, sid[i]);
    //          Serial.println(qsid[i]);
         }
         EEPROM.commit();
         Serial.println("Disconnecting previously connected WiFi");
         WiFi.disconnect();
         Serial.println("Press reset button");
         flag = 0;
  }
      // Wait 5 seconds before retrying
      delay(5000);
    }

void IRAM_ATTR ISR(){
  
    delay(500);
    Serial.println("Resetting WiFi and MQTT credentials");
    flag = 1;   

}
void setup(){
  Serial.begin(115200);
  delay(1000);
  Serial.println("Disconnecting previously connected WiFi");
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);
  pinMode(Button, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  attachInterrupt(Button, ISR, RISING); 
  Serial.println();
  Serial.println();
  Serial.println("Startup");
  Serial.println("Reading EEPROM ssid");
  String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 64; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);

  Serial.println("Reading EEPROM email");

  String eemail = "";
  for (int i = 64; i < 96; ++i)
  {
    eemail += char(EEPROM.read(i));
  }
  Serial.print("EMAIL: ");
  Serial.println(eemail);
  Serial.println("Reading EEPROM pass");

  String eupass = "";
  for (int i = 96; i < 128; ++i)
  {
    eupass += char(EEPROM.read(i));
  }
  Serial.print("USER PASS: ");
  Serial.println(eupass);
  // Initialize BME280 sensor
  WiFi.begin(esid.c_str(), epass.c_str());
  Serial.print("Connecting to WiFi ..");
  Serial.println(WiFi.localIP());
  

  Serial.println();
  Serial.println("Waiting.");
 int c = 0;
  while ((WiFi.status() != WL_CONNECTED))
  {
    c++;
    Serial.print(".");
    delay(1000);
    server.handleClient();
    reconnect();
    if (c == 5){
      break;
    }
  }
  if (c < 5){
    Serial.println("internet");
    
  }
   if (c == 5){
    Serial.println("no internet");
    Serial.println("Turning the HotSpot On");
    setupAP();// Setup HotSpot
    
  }
  Serial.println();
  

  // Getting the user UID might take a few seconds
  if (WiFi.status() == WL_CONNECTED)
  {
    configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = eemail.c_str();
  auth.user.password = eupass.c_str();

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    a++;
    Serial.print('.');
    delay(1000);
    if (a == 50)
    {
      break;
    }
  }
  a = 0;
      // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);
  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(18000);
  
  }



  
  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }
}

void loop(){
   float voltageA, voltageC, totalVoltage, Acin_current, Acout_current, totalCurrent, Wapda_realPower,Load_realPower,
   Wapda_powerFactor,Load_powerFactor,temp, freq, importEnergy, Wapda_importEnergy, Load_importEnergy,exportEnergy, Wapda_exportEnergy,Load_exportEnergy;

     char sid[] = "Enter WiFi";
   while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);

  if(flag == 1)
  {
    for (int i = 0; i < 10; ++i)
         {
           EEPROM.write(i, sid[i]);
    //          Serial.println(qsid[i]);
         }
         EEPROM.commit();
         Serial.println("Disconnecting previously connected WiFi");
         WiFi.disconnect();
         Serial.println("Press reset button");
         flag = 0;
  }
  // Send new readings to database
  if (WiFi.status() == WL_CONNECTED){
    counter ++;
  if (counter == 8)
  {
  if (Firebase.ready()){
    sendDataPrevMillis = millis();

    parentPath= databasePath + "/" + String(dayStamp)+ "/" + String(timeStamp);

    json.set(WVPath.c_str(), String(voltageA));
    json.set(WIPath.c_str(), String(Acin_current));
    json.set(WPPath.c_str(), String(Wapda_realPower));  
    json.set(LVPath.c_str(), String(voltageC));
    json.set(LIPath.c_str(), String(Acout_current));
    json.set(LPPath.c_str(), String(Load_realPower));
    json.set(LpFPath.c_str(), String(Load_powerFactor));
    json.set(WpFPath.c_str(), String(Wapda_powerFactor));
    json.set(WeUPath.c_str(), String(Wapda_exportunits));
    json.set(WiUPath.c_str(), String(Wapda_importunits));  
    json.set(LiUPath.c_str(), String(Load_importunits));
    json.set(fPath.c_str(), String(freq));
    json.set(tPath.c_str(), String(temp));
    json.set(hPPath.c_str(), String(120));
    json.set(hCPath.c_str(), String(1.2));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
  counter= 0;
  Load_importunits= 0;
  Wapda_importunits= 0;
  Wapda_exportunits = 0;
  }   
}
delay(14000);
}
