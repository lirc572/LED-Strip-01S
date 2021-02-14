#include <Arduino.h>
#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>
#include "config.h"

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

WiFiServer server(80);

String header;

#define DELAYMS 10

CRGB leds[NUM_LEDS];

CRGB color = CRGB::Green;

char Digit2hexchar(uint8_t digit)
{
  return "0123456789abcdef"[digit];
}

uint8_t HexChar2digit(char hex)
{
  // to lower case first
  if (hex >= 'A' && hex <= 'Z') {
    hex += ('a' - 'A');
  }
  return strchr("0123456789abcdef", hex) - "0123456789abcdef";
}

String Crgb2str(CRGB color)
{
  char buff[8];
  buff[0] = Digit2hexchar(color.red / 16);
  buff[1] = Digit2hexchar(color.red % 16);
  buff[2] = Digit2hexchar(color.green / 16);
  buff[3] = Digit2hexchar(color.green % 16);
  buff[4] = Digit2hexchar(color.blue / 16);
  buff[5] = Digit2hexchar(color.blue % 16);
  buff[6] = '\0';
  Serial.printf("Color: #%s\n", buff);
  return String("#") + buff;
}

CRGB Str2crgb(String color)
{
  uint8_t r, g, b;
  r = HexChar2digit(color[1]) * 16 + HexChar2digit(color[2]);
  g = HexChar2digit(color[3]) * 16 + HexChar2digit(color[4]);
  b = HexChar2digit(color[5]) * 16 + HexChar2digit(color[6]);
  Serial.printf("Color: %d %d %d\n", r, g, b);
  return CRGB(r, g, b);
}

String getStatus()
{
  String res;
  res.reserve(1024); // prevent ram fragmentation
  res = F("HTTP/1.1 200 OK\r\n"
          "Content-Type: text/html\r\n"
          "Connection: close\r\n" // the connection will be closed after completion of the response
          "\r\n");
  res += Crgb2str(color);
  res += "\r\n";
  return res;
}

String prepareHtmlPage()
{
  String htmlPage;
  htmlPage.reserve(1024); // prevent ram fragmentation
  htmlPage = F("HTTP/1.1 200 OK\r\n"
               "Server: ESP-01S\r\n"
               "Content-Type: text/html\r\n"
               "Connection: close\r\n" // the connection will be closed after completion of the response
               "\r\n"
               "<!DOCTYPE HTML>"
               "<html>"
               "<head>"
               "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
               "<link rel=\"icon\" href=\"data:,\">"
               "</head>"
               "<form method=\"post\" action=\"\">"
               "<input id=\"color\" type=\"color\" name=\"color\""
               "value=\"");
  // htmlPage += "#4c5243";
  htmlPage += Crgb2str(color);
  htmlPage += F("\">"
                "</form>"
                "<script>"
                "function updateColor(e) {"
                "console.log(e.target.value);"
                "fetch(\"/status\", {method: 'POST', body: `color=${e.target.value}`})"
                "}"
                "document.getElementById(\"color\").addEventListener(\"input\", updateColor, false);"
                "</script>"
                "</html>"
                "\r\n");
  return htmlPage;
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  pinMode(LED_PIN, OUTPUT);
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  for (;;)
  {
    for (int dot = 0; dot < NUM_LEDS; dot++)
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.println(" connected");
        Serial.println(WiFi.localIP());
        for (dot = 0; dot < NUM_LEDS; dot++)
        {
          leds[dot] = CRGB::Green;
        }
        FastLED.show();
        server.begin();
        delay(1000);
        return;
      }
      leds[dot] = CRGB::Red;
      FastLED.show();
      leds[dot] = CRGB::Black;
      Serial.print(".");
      delay(DELAYMS);
    }
  }
}

void loop()
{
  WiFiClient client = server.available();
  if (client)
  {
    Serial.println("\n[Client connected]");
    while (client.connected())
    {
      if (client.available())
      {
        // read line by line what the client (web browser) is requesting
        String line = client.readStringUntil('\r');
        // Serial.print(line);
        // wait for end of client's request, that is marked with an empty line
        if (line.length() == 1 && line[0] == '\n')
        {
          Serial.println("START");
          Serial.println(header);
          Serial.println("END");
          if (header.indexOf("GET /status") >= 0)
          {
            Serial.println("In GET /status");
            client.println(getStatus());
          }
          else if (header.indexOf("POST /status") >= 0)
          {
            Serial.println("In POST /status");
            uint8_t content_length;
            sscanf(header.substring(header.indexOf("Content-Length: ")).substring(0, header.indexOf("\r")).substring(16).c_str(), "%u", &content_length);
            String content;
            client.read();
            for (int i=0; i < content_length; i++) {
              char c = client.read();
              content += c;
            }
            Serial.println("Content: " + content);
            String colorStr = content.substring(content.indexOf("color=")).substring(6);
            Serial.printf("colorStr (%u): <%s>\n", colorStr.length(), colorStr.c_str());
            color = Str2crgb(colorStr);
            client.println(prepareHtmlPage());
          }
          else
          {
            Serial.println("In GET /");
            client.println(prepareHtmlPage());
          }
          break;
        }
        header += line;
      }
    }
    while (client.available())
    {
      // but first, let client finish its request
      // that's diplomatic compliance to protocols
      // (and otherwise some clients may complain, like curl)
      // (that is an example, prefer using a proper webserver library)
      client.read();
    }

    // Clear the header variable
    header = "";
    // close the connection:
    client.stop();
    Serial.println("[Client disconnected]");
  }
  for (int dot = 0; dot < NUM_LEDS; dot++)
  {
    leds[dot] = color;
  }
  FastLED.show();
}
