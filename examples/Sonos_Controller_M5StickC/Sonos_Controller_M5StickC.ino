#include <SonosUPnP.h>
#include <MicroXPath_P.h>
#include <M5StickC.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <DNSServer.h>

#define SERIAL_DATA_THRESHOLD_MS 500
#define SERIAL_ERROR_TIMEOUT "E: Serial"
#define ETHERNET_ERROR_DHCP "E: DHCP"
#define ETHERNET_ERROR_CONNECT "E: Connect"

void handleSerialRead();
void ethConnectError();
//EthernetClient g_ethClient;
WiFiClient client;
SonosUPnP g_sonos = SonosUPnP(client, ethConnectError);

// Living room
String ipaddress = "192.168.13.4";
IPAddress g_KitchenIP(192, 168, 13, 4);
const char g_KitchenID[] = ""; // Sonos Serial Number

const char* ssid     = ""; // Wi-Fi SSID
const char* password = ""; // Wi-Fi Password

bool isPlaying = true; //Assume we're playing so always pause first.

char uri[200] = "";
String lastCmd;

// display a.jpg
uint16_t x = 0;
uint16_t y = 0;
uint16_t width = 0;
uint16_t height = 0;
uint16_t offset_x = 0;
uint16_t offset_y = 0;
jpeg_div_t scale = JPEG_DIV_8; // JPEG_DIV_NONE, JPEG_DIV_2, JPEG_DIV_4, JPEG_DIV_8

#define HTTPPORT 88
WebServer server(HTTPPORT);

void handleRoot();
void handleCmd();
void handleNotFound();
void handleResponse();
void handleGet();
void handleGt();

void setup()
{
  Serial.begin(115200);
  M5.begin();
  M5.Lcd.setRotation(1); // display size = 80x160, setRotation = 0:M5, 1:Power Btn, 2:up side down, 3:Btn
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("Connecting...");
  WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

  Serial.println("connected to WiFi");
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("Wifi Connected!");
  
  server.on("/", handleRoot);
  server.on("/cmd", handleCmd);
  server.on("/get", handleGt);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.print("HTTP server started on ");
  Serial.println(HTTPPORT);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  int volume = g_sonos.getVolume(g_KitchenIP);

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("Playing (%d)",volume);
  M5.Lcd.fillRect(72,66,3,20,M5.Lcd.color565(255, 255, 255)); // (x,y,w,h)
  M5.Lcd.fillRect(82,66,3,20,M5.Lcd.color565(255, 255, 255)); // (x,y,w,h)
  M5.Lcd.fillTriangle(17,70,25,63,25,76,M5.Lcd.color565(255, 255, 255));
  M5.Lcd.fillTriangle(27,70,35,63,35,76,M5.Lcd.color565(255, 255, 255));
  M5.Lcd.fillTriangle(132,70,125,63,125,76,M5.Lcd.color565(255, 255, 255));
  M5.Lcd.fillTriangle(142,70,135,63,135,76,M5.Lcd.color565(255, 255, 255));

//  String cmd = "ti";
//  byte b1 = cmd[0];
//  byte b2 = cmd[1];
//  handleInput(cmd,b1,b2);
}

void ethConnectError()
{
  Serial.println(ETHERNET_ERROR_CONNECT);
  Serial.println("Wifi died.");
}

void loop()
{
  M5.update();
  server.handleClient();
  if (M5.Axp.GetBtnPress() == 2) { // Power Btn
    g_sonos.skip(g_KitchenIP, SONOS_DIRECTION_BACKWARD);
    int volume = g_sonos.getVolume(g_KitchenIP);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("Playing (%d)",volume);
    M5.Lcd.fillRect(72,66,3,30,M5.Lcd.color565(255, 255, 255)); // (x,y,w,h)
    M5.Lcd.fillRect(82,66,3,30,M5.Lcd.color565(255, 255, 255)); // (x,y,w,h)
    M5.Lcd.fillTriangle(17,70,25,63,25,76,M5.Lcd.color565(255, 255, 255));
    M5.Lcd.fillTriangle(27,70,35,63,35,76,M5.Lcd.color565(255, 255, 255));
    M5.Lcd.fillTriangle(132,70,125,63,125,76,M5.Lcd.color565(255, 255, 255));
    M5.Lcd.fillTriangle(142,70,135,63,135,76,M5.Lcd.color565(255, 255, 255));

 //   String cmd = "ti";
 //   byte b1 = cmd[0];
 //   byte b2 = cmd[1];
 //   handleInput(cmd,b1,b2);

  } else if (M5.BtnA.wasReleased()) {
    if(isPlaying)
    {
      g_sonos.pause(g_KitchenIP);
      isPlaying = false;
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(10, 10);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setTextSize(2);
      M5.Lcd.printf("Paused");
      M5.Lcd.fillTriangle(92,70,75,63,75,76,M5.Lcd.color565(255, 255, 255));
      M5.Lcd.fillTriangle(17,70,25,63,25,76,M5.Lcd.color565(255, 255, 255));
      M5.Lcd.fillTriangle(27,70,35,63,35,76,M5.Lcd.color565(255, 255, 255));
      M5.Lcd.fillTriangle(132,70,125,63,125,76,M5.Lcd.color565(255, 255, 255));
      M5.Lcd.fillTriangle(142,70,135,63,135,76,M5.Lcd.color565(255, 255, 255));
    }
    else
    {
      g_sonos.play(g_KitchenIP);
      isPlaying = true;
      int volume = g_sonos.getVolume(g_KitchenIP);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(10, 10);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setTextSize(2);
      M5.Lcd.printf("Playing (%d)",volume);
//      M5.Lcd.drawJpgFile(SD, "/a.jpg", x, y, width, height, offset_x, offset_y, scale);
      M5.Lcd.fillRect(72,66,3,30,M5.Lcd.color565(255, 255, 255)); // (x,y,w,h)
      M5.Lcd.fillRect(82,66,3,30,M5.Lcd.color565(255, 255, 255)); // (x,y,w,h)
      M5.Lcd.fillTriangle(17,70,25,63,25,76,M5.Lcd.color565(255, 255, 255));
      M5.Lcd.fillTriangle(27,70,35,63,35,76,M5.Lcd.color565(255, 255, 255));
      M5.Lcd.fillTriangle(132,70,125,63,125,76,M5.Lcd.color565(255, 255, 255));
      M5.Lcd.fillTriangle(142,70,135,63,135,76,M5.Lcd.color565(255, 255, 255));
    }

  } else if (M5.BtnB.wasReleased()) {
    g_sonos.skip(g_KitchenIP, SONOS_DIRECTION_FORWARD);
    int volume = g_sonos.getVolume(g_KitchenIP);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("Playing (%d)",volume);
    M5.Lcd.fillRect(72,66,3,30,M5.Lcd.color565(255, 255, 255)); // (x,y,w,h)
    M5.Lcd.fillRect(82,66,3,30,M5.Lcd.color565(255, 255, 255)); // (x,y,w,h)
    M5.Lcd.fillTriangle(17,70,25,63,25,76,M5.Lcd.color565(255, 255, 255));
    M5.Lcd.fillTriangle(27,70,35,63,35,76,M5.Lcd.color565(255, 255, 255));
    M5.Lcd.fillTriangle(132,70,125,63,125,76,M5.Lcd.color565(255, 255, 255));
    M5.Lcd.fillTriangle(142,70,135,63,135,76,M5.Lcd.color565(255, 255, 255));

//    String cmd = "ti";
//    byte b1 = cmd[0];
//    byte b2 = cmd[1];
//    handleInput(cmd,b1,b2);
  }
}


bool isCommand(const char *command, byte b1, byte b2)
{
  return *command == b1 && *++command == b2;
}

String handleGet(String cmd)
{
    Serial.println("Handling command " + cmd);
    if (cmd == "gv")
    {
      int volume = g_sonos.getVolume(g_KitchenIP);
      return String(volume);
    }
    else if (cmd == "gs")
    {
      String response = "";
      char uri[25] = "";
      TrackInfo track = g_sonos.getTrackInfo(g_KitchenIP, uri, sizeof(uri));
      byte source = g_sonos.getSourceFromURI(track.uri);
      switch (source)
      {
        case SONOS_SOURCE_FILE:
          response += "File: ";
          break;
        case SONOS_SOURCE_HTTP:
          response += "HTTP: ";
          break;
        case SONOS_SOURCE_RADIO:
          response += "Radio: ";
          break;
        case SONOS_SOURCE_LINEIN:
          response += "Line-In: ";
          break;
        case SONOS_SOURCE_MASTER:
          response += "Other Speaker: ";
          break;
        default:
          response += "Unknown";
          break;
      }
      if (source == SONOS_SOURCE_FILE || source == SONOS_SOURCE_HTTP)
      {
        response += ", track = ";
        response += track.number, DEC;
        response += ", pos = ";
        response += track.position, DEC;
        response += " of ";
        response += track.duration, DEC;
      }
      return response;
    }
    else
    {
      return "-1";
    }
}

void handleInput(String cmd, byte b1, byte b2)
{
  // Read 2 bytes from serial buffer
    // Play
    Serial.println("Handling command " + cmd);
    if (cmd == "pl")
    {
      g_sonos.play(g_KitchenIP);
    }
    // Pause
    else if (isCommand("pa", b1, b2))
    {
      g_sonos.pause(g_KitchenIP);
    }
    // Stop
    else if (isCommand("st", b1, b2))
    {
      g_sonos.stop(g_KitchenIP);
    }
    // Previous
    else if (isCommand("pr", b1, b2))
    {
      g_sonos.skip(g_KitchenIP, SONOS_DIRECTION_BACKWARD);
    }
    // Next
    else if (isCommand("nx", b1, b2))
    {
      g_sonos.skip(g_KitchenIP, SONOS_DIRECTION_FORWARD);
    }
    // Play File
    else if (isCommand("fi", b1, b2))
    {
      g_sonos.playFile(g_KitchenIP, "192.168.188.22/Music/ringtone/ring1.mp3");

    }
    // Play HTTP
    else if (isCommand("ht", b1, b2))
    {
      // Playing file from music service WIMP (SID = 20)
      //g_sonos.playHttp(g_KitchenIP, "trackid_37554547.mp4?sid=20&amp;flags=32");

      //g_sonos.playHttp(g_KitchenIP, "http://192.168.188.1:49200/AUDIO/DLNA-1-0/MXT-USB-StorageDevice-01/take_me_to_church.mp3");
      g_sonos.playHttp(g_KitchenIP, "http://192.168.188.28:88/gong.mp3");
    }
    // Play Radio
    else if (isCommand("ra", b1, b2))
    {
      g_sonos.playRadio(g_KitchenIP, "//lyd.nrk.no/nrk_radio_p3_mp3_h.m3u", "NRK P3");

    }
    // Play Line In
    else if (isCommand("li", b1, b2))
    {
      g_sonos.playLineIn(g_KitchenIP, g_KitchenID);
    }
    // Repeat On
    else if (isCommand("re", b1, b2))
    {
      g_sonos.setPlayMode(g_KitchenIP, SONOS_PLAY_MODE_REPEAT);
    }
    // Shuffle On
    else if (isCommand("sh", b1, b2))
    {
      g_sonos.setPlayMode(g_KitchenIP, SONOS_PLAY_MODE_SHUFFLE);
    }
    // Repeat and Shuffle On
    else if (isCommand("rs", b1, b2))
    {
      g_sonos.setPlayMode(g_KitchenIP, SONOS_PLAY_MODE_SHUFFLE_REPEAT);
    }
    // Repeat and Shuffle Off
    else if (isCommand("no", b1, b2))
    {
      g_sonos.setPlayMode(g_KitchenIP, SONOS_PLAY_MODE_NORMAL);
    }
    // Loudness On
    else if (isCommand("lo", b1, b2))
    {
      g_sonos.setLoudness(g_KitchenIP, true);
    }
    // Loudness Off
    else if (isCommand("l_", b1, b2))
    {
      g_sonos.setLoudness(g_KitchenIP, false);
    }
    // Mute On
    else if (isCommand("mu", b1, b2))
    {
      g_sonos.setMute(g_KitchenIP, true);
    }
    // Mute Off
    else if (isCommand("m_", b1, b2))
    {
      g_sonos.setMute(g_KitchenIP, false);
    }
    // Volume/Bass/Treble
    else if (b2 >= '0' && b2 <= '9')
    {
      // Volume 0 to 99
      if (b1 >= '0' && b1 <= '9')
      {
        g_sonos.setVolume(g_KitchenIP, ((b1 - '0') * 10) + (b2 - '0'));
      }
      // Bass 0 to -9
      else if (b1 == 'b')
      {
        g_sonos.setBass(g_KitchenIP, (b2 - '0') * -1);
      }
      // Bass 0 to 9
      else if (b1 == 'B')
      {
        g_sonos.setBass(g_KitchenIP, b2 - '0');
      }
      // Treble 0 to -9
      else if (b1 == 't')
      {
        g_sonos.setTreble(g_KitchenIP, (b2 - '0') * -1);
      }
      // Treble 0 to 9
      else if (b1 == 'T')
      {
        g_sonos.setTreble(g_KitchenIP, b2 - '0');
      }
    }

    else if (isCommand("ti", b1, b2))
    {
      Serial.println("we want the track uri");
      TrackInfo track = g_sonos.getTrackInfo(g_KitchenIP, uri, sizeof(uri));
      Serial.println(uri);
      HTTPClient http;
      String urlencoded_uri = urlencode(uri);
      String request_uri = "http://" + ipaddress +":1400/getaa?s=1&u=";
      request_uri += urlencoded_uri;
      Serial.println(request_uri);
      http.begin(request_uri);
      int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.println(httpCode);
        if(httpCode == HTTP_CODE_OK) {
          int len = http.getSize();
          Serial.print("size: ");
          Serial.println(len);

//        String payload = http.getString();
//        Serial.print("data: ");
//        Serial.println(payload);

          // file open to write
//          File file = SD.open("/a.jpg", FILE_WRITE);
          Serial.println("file open");

          // create buffer for read
          uint8_t buff[128] = { 0 };
          // get tcp stream
          WiFiClient * stream = http.getStreamPtr();
          // read all data from server
          while(http.connected() && (len > 0 || len == -1)) {
            // get available data size
            size_t size = stream->available();
            if(size) {
              // read up to 128 byte
              int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
              // write it to Serial
//              Serial.write(buff, c);
//              Serial.println(String(c));

//              file.write(buff, c);
              if(len > 0) {
                len -= c;
              }
            }
            delay(1);
          }
        
//          file.close();
//          Serial.println("file close");

          // verify a.jpg
//          File file_R = SD.open("/a.jpg", FILE_READ);
//          int file_size = file_R.size();
//          Serial.print("file_size: ");
//          Serial.println(file_size);
//          file_R.close();

          // display a.jpg
//          M5.Lcd.drawJpgFile(SD, "/a.jpg", x, y, width, height, offset_x, offset_y, scale);
          
          Serial.println();
          Serial.print("[HTTP] connection closed or file end.\n");
        }
      } else {
        Serial.println("Error on HTTP request");
      }
      http.end(); //Free the resources
    }

}

String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
    
}



/* WebServer Stuff */

void handleRoot() {
  int vol = g_sonos.getVolume(g_KitchenIP);
  String msg = "<html>\n";
  msg += "<head>\n";
  msg += "<title>M5Stack(ESP32) Sonos Controller</title>\n";
  msg += "<link rel=\"stylesheet\" type=\"text/css\" href=\"http://joeybabcock.me/iot/hosted/hosted-sonos.css\">";
  msg += "<script src=\"https://code.jquery.com/jquery-3.1.1.min.js\"></script>\n";
  msg += "<script src=\"http://joeybabcock.me/iot/hosted/hosted-sonos.js\"></script>\n";
  msg += "</head>\n";
  msg += "<body>\n";
  msg += "<div id=\"container\">\n";
  msg += "<h1>Sonos - M5Stack(ESP32) Web Controller!</h1>\n";
  msg += "<p id=\"linkholder\"><a href=\"#\" onclick=\"sendCmd('pr');\"><img src=\"http://joeybabcock.me/iot/hosted/rw.png\"/></a> \n";
  msg += "<a href=\"#\" onclick=\"sendCmd('pl');\"><img src=\"http://joeybabcock.me/iot/hosted/play.png\"/></a> \n";
  msg += "<a href=\"#\" onclick=\"sendCmd('pa');\"><img src=\"http://joeybabcock.me/iot/hosted/pause.png\"/></a> \n";
  msg += "<a href=\"#\" onclick=\"sendCmd('nx');\"><img src=\"http://joeybabcock.me/iot/hosted/ff.png\"/></a></p>\n";
  msg += "<h3>Volume: <span id=\"vol\">"+String(vol)+"</span><input type=\"hidden\" id='volume' value='"+String(vol)+"' onchange=\"setVolume(this.value)\"/></h3><br/>\n";
  msg += "<input type=\"range\" class=\"slider\"  min=\"0\" max=\"99\" value=\""+String(vol)+"\" name=\"volume-slider\" id=\"volume-slider\" onchange=\"setVolume(this.value)\" />\n";
  msg += "<p>Server Response:<div id=\"response\" class=\"response\"></div></p>\n";
  msg += "<p><form action=\"/\" method=\"get\" id=\"console\"><input placeholder=\"Enter a command...\" type=\"text\" id='console_text'/></form></p>\n";
  msg += "<script>var intervalID = window.setInterval(getVolume, 50000);\n$('#console').submit(function(){parseCmd($(\"#console_text\").val());\nreturn false;\n});\n</script>\n";
  msg += "</div>\n";
  msg += "<div id=\"tips\"></div>\n";
  msg == "</body>\n";
  msg += "</html>\n";
  server.send(200, "text/html", msg);
}

void handleCmd(){
  for (uint8_t i=0; i<server.args(); i++){
    if(server.argName(i) == "cmd") 
    {
      lastCmd = server.arg(i);
      byte b1 =  server.arg(i)[0];
      byte b2 = server.arg(i)[1];
      handleInput(lastCmd,b1,b2);
    }
  }
  handleResponse();
}

void handleGt(){
  String resp;
  for (uint8_t i=0; i<server.args(); i++){
    if(server.argName(i) == "cmd") 
    {
      lastCmd = server.arg(i);
      byte b1 =  server.arg(i)[0];
      byte b2 = server.arg(i)[1];
      resp = handleGet(lastCmd);
    }
  }
  handleGetResponse(resp);
}

void handleNotFound() {

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);

}

void handleResponse() {
      server.send(200, "text/html", "Worked("+lastCmd+")");
      Serial.println("Got client.");
}

void handleGetResponse(String response) {
      server.send(200, "text/html", response);
      Serial.println("Got client.");
}
