// #include <ESP8266WiFi.h>
// #include <PubSubClient.h>

// WiFiClient tcp;
// PubSubClient mqtt(tcp);

// const char* wifi_ssid = "iTongji-manul";
// const char* wifi_passwd = "YOUYUAN4411";

// const char* mqtt_server = "ajdnaud.iot.gz.baidubce.com";
// const char* mqtt_clientid = "DragonRollEsp";
// const char* mqtt_user = "thingidp@ajdnaud|DragonRollEsp|0|MD5";
// const char* mqtt_passwd = "dc4e4a9497c5ada946e54ae789cf4882";
// const char* topic_delta = "Switch";
// const char* topic_update = "Switch";

// void mqtt_callback(const char* topic, byte* payload, unsigned int length)
// {
//  Serial.print("Message arrived [");
//  Serial.print(topic);
//  Serial.print("] ");
//  for (int i = 0; i < length; i++) {
//    Serial.print((char)payload[i]);
//  }
//  Serial.print("    byte:");
//  for (int i = 0; i < length; i++) {
//    Serial.print(" ");
//    Serial.print(payload[i]);
//  }
//  Serial.println();

//  // Switch on the Pixel if an 1 was received as first character
//  if ((char)payload[0] == '1') {
//    digitalWrite(LED_BUILTIN, LOW);   // Turn the Pixel on (Note that LOW is the voltage level
//    // but actually the Pixel is on; this is because
//    // it is active low on the ESP-01)
//  } else {
//    digitalWrite(LED_BUILTIN, HIGH);  // Turn the Pixel off by making the voltage HIGH
//  }

// }

// void setup() {
//  Serial.begin(115200);
//  pinMode(LED_BUILTIN, OUTPUT);
//  digitalWrite(LED_BUILTIN, HIGH);

//  Serial.println("WiFi Connecting...");
//  WiFi.mode(WIFI_STA);
//  WiFi.begin(wifi_ssid, wifi_passwd);
//  while (!WiFi.isConnected())
//  {
//    delay(100);
//  }
//  Serial.println("WiFi is connected");

//  mqtt.setServer(mqtt_server, 1883);
//  mqtt.setCallback(mqtt_callback);
// }

// void loop() {
//  if (!mqtt.connected())
//  {
//    mqtt.connect(mqtt_clientid, mqtt_user, mqtt_passwd);
//    mqtt.subscribe(topic_delta);
//    Serial.println("Baidu IoT Hub connected");
//  }
//  mqtt.loop();
// //  unsigned long now = millis();
// //  if (now - lastMsg > 2000) {
// //    lastMsg = now;
// //    Serial.print("Publish message: ");
// //    Serial.println(msg);
// //    client.publish(topic_update, msg.c_str());
// }

// #include <ESP8266WiFi.h>
// #include <DNSServer.h>
// #include <ESP8266WebServer.h>
// #include <WiFiManager.h>

// void setup() {
//     WiFi.mode(WIFI_STA);
//     Serial.begin(115200);
//     WiFiManager wm;
//     wm.autoConnect("PulseBeeper");

// }

// void loop() {

// }

//#include <NeoPixelBus.h>
//#include "Base64.h"

//#define PixelLen 20;

// unsigned char[] decodeBase64(String colorStr) {
//     colorStr = "AAAAAAAABgAAHQAALwAAQAAAUgAAYwAAegAAkgAArwAAtQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
//     unsigned char colorList[PixelLen*3];
//     decode_base64(colorStr, colorList);
//     return colorList;
// }

// void SetPixelsColor(String colorsStr) {

// }

//void setup() {
//    // NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> Pixel(16);
//    // Pixel.Begin();
//    // Pixel.ClearTo(RgbColor(50, 0, 0));
//    // Pixel.Show();
//    char colorStr[] = "AAAAAAAABgAAHQAALwAAQAAAUgAAYwAAegAAkgAArwAAtQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
//    char colorList[60];
//    char input2[] = "BgAAHQAALwAA";
//    int input2Len = sizeof(input2);
//
//    int decodedLen = base64_dec_len(input2, input2Len);
//    char decoded[decodedLen];
//
//    base64_decode(decoded, input2, input2Len);
//    Serial.begin(115200);
//    Serial.println("");
//    Serial.print(input2); Serial.print(" = "); Serial.println(decoded);
//}
//
//
//void loop() {
//
//}

// unsigned char base64[] = "hfR1zrLD";
// unsigned char binary[6];

// unsigned int binary_length = decode_base64(base64, binary);

#include <base64.hpp>
#include <NeoPixelBus.h>

#define PixelLen 16

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> Pixel(PixelLen);
unsigned char Buffer[] = "qQAAuwAAzAAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHQAAOgAAYwAAjAAAowAAtQAAxgAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFwAANAAAXQAAhgAAnQAArwAAwAAAzAAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAEQAALwAAVwAAgAAAmAAAqQAAuwAAxgAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAADAAAKQAAUgAAegAAkgAAowAAtQAAwAAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAABgAAIwAATAAAdQAAjAAAnQAArwAAuwAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHQAARgAAbwAAhgAAmAAAqQAAtQAAxgAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFwAAQAAAaQAAgAAAkgAAowAArwAAwAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEQAAOgAAYwAAegAAjAAAnQAAqQAAuwAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAADAAANAAAXQAAdQAAhgAAmAAAowAAtQAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAABgAALwAAVwAAbwAAgAAAkgAAnQAArwAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAKQAAUgAAaQAAegAAjAAAmAAAqQAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIwAATAAAYwAAdQAAhgAAkgAAowAAxgAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHQAARgAAXQAAbwAAgAAAjAAAnQAAwAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFwAAQAAAVwAAaQAAegAAhgAAmAAAuwAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAEQAAOgAAUgAAYwAAdQAAgAAAkgAAtQAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAADAAANAAATAAAXQAAbwAAegAAjAAArwAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAABgAALwAARgAAVwAAaQAAdQAAhgAAqQAAzAAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAKQAAQAAAUgAAYwAAbwAAgAAAowAAxgAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAIwAAOgAATAAAXQAAaQAAegAAnQAAwAAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAHQAANAAARgAAVwAAYwAAdQAAmAAAuwAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAFwAALwAAQAAAUgAAXQAAbwAAkgAAtQAAxgAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAEQAAKQAAOgAATAAAVwAAaQAAjAAArwAAwAAAzAAAzAAAAAAAAAAAAAAAAAAAAAAADAAAIwAANAAARgAAUgAAYwAAhgAAqQAAuwAAzAAAzAAAAAAAAAAAAAAAAAAAAAAABgAAHQAALwAAQAAATAAAXQAAgAAAowAAtQAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAFwAAKQAAOgAARgAAVwAAegAAnQAArwAAxgAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAEQAAIwAANAAAQAAAUgAAdQAAmAAAqQAAwAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAADAAAHQAALwAAOgAATAAAbwAAkgAAowAAuwAAzAAAAAAAAAAAAAAAAAAAAAAAAAAABgAAFwAAKQAANAAARgAAaQAAjAAAnQAAtQAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAEQAAIwAALwAAQAAAYwAAhgAAmAAArwAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAADAAAHQAAKQAAOgAAXQAAgAAAkgAAqQAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAABgAAFwAAIwAANAAAVwAAegAAjAAAowAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEQAAHQAALwAAUgAAdQAAhgAAnQAAxgAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADAAAFwAAKQAATAAAbwAAgAAAmAAAwAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABgAAEQAAIwAARgAAaQAAegAAkgAAuwAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADAAAHQAAQAAAYwAAdQAAjAAAtQAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABgAAFwAAOgAAXQAAbwAAhgAArwAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEQAANAAAVwAAaQAAgAAAqQAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADAAALwAAUgAAYwAAegAAowAAxgAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABgAAKQAATAAAXQAAdQAAnQAAwAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIwAARgAAVwAAbwAAmAAAuwAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHQAAQAAAUgAAaQAAkgAAtQAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFwAAOgAATAAAYwAAjAAArwAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEQAANAAARgAAXQAAhgAAqQAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADAAALwAAQAAAVwAAgAAAowAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABgAAKQAAOgAAUgAAegAAnQAAxgAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIwAANAAATAAAdQAAmAAAwAAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHQAALwAARgAAbwAAkgAAuwAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFwAAKQAAQAAAaQAAjAAAtQAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEQAAIwAAOgAAYwAAhgAArwAAxgAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADAAAHQAANAAAXQAAgAAAqQAAwAAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABgAAFwAALwAAVwAAegAAowAAuwAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEQAAKQAAUgAAdQAAnQAAtQAAzAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADAAAIwAATAAAbwAAmAAArwAAxgAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABgAAHQAARgAAaQAAkgAAqQAAwAAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFwAAQAAAYwAAjAAAowAAuwAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEQAAOgAAXQAAhgAAnQAAtQAAzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADAAANAAAVwAAgAAAmAAArwAAxgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABgAALwAAUgAAegAAkgAAqQAAwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAKQAATAAAdQAAjAAAowAAuwAAAAAA";

void SetPixelsColor(unsigned char* colorStr)
{
    unsigned char colorList[PixelLen * 3 + 1];
    decode_base64(colorStr, colorList);
    for (int i; i < PixelLen; i++)
    {
        Serial.print(colorList[i * 3]);
        Serial.print(" ");
        Pixel.SetPixelColor(i, RgbColor(colorList[i * 3], colorList[i * 3 + 1], colorList[i * 3 + 2]));
    }
    Serial.println("");
    Pixel.Show();
}

void setup()
{
    Serial.begin(115200);
    Serial.println("");
    Pixel.Begin();
}

void loop()
{
    unsigned char colorStr[PixelLen * 4 + 1];
    for (int i = 0; i < 60; i++)
    {
        int now = millis();
        for (int j = 0; j < PixelLen * 4; j++)
        {
            colorStr[j] = Buffer[i * PixelLen * 4 + j];
        }
        SetPixelsColor(colorStr);
        while(1) {
            if (millis() - now >= 17){
                break;
            }
        }
    }
}
