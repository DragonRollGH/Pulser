

void setup() {

 Serial.println("WiFi is connected");

 MQTT.setServer(mqtt_server, 1883);
 MQTT.setCallback(mqtt_callback);
}

void loop() {
 if (!MQTT.connected())
 {
   MQTT.connect(mqtt_clientid, mqtt_user, mqtt_passwd);
   MQTT.subscribe(topic_delta);
   Serial.println("Baidu IoT Hub connected");
 }
 MQTT.loop();
 unsigned long now = millis();
 if (now - lastMsg > 2000) {
   lastMsg = now;
   Serial.print("Publish message: ");
   Serial.println(msg);
   client.publish(topic_update, msg.c_str());
}

#include <base64.hpp>
#include <NeoPixelBus.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

const byte PixelLen = 16;
const byte Frame = 60;

// const char* mqtt_server = "aidnaud.iot.gz.baidubce.com";
// const char* mqtt_clientid = "DragonRollEsp";
// const char* mqtt_user = "thingidp@aidnaud|DragonRollEsp|0|MD5";
// const char* mqtt_passwd = "dc4e4a9497c5ada946e54ae789cf4882";
// const char* topic_delta = "Switch";
// const char* topic_update = "Switch";

unsigned char buffer[] = "AAAA*0dzAAAAAAA*0bzAAA*03AAAA*09zAAA*05AAAA*09zAAA*05AAAA*08zAAA*06AAAA*08zAAA*05xgAAAAAA*08zAAA*02xgAA*02wAAAAAAA*08zAAA*01xgAAwAAA*02uwAAAAAA*08zAAAxgAAwAAAuwAA*02tQAAAAAA*08zAAAwAAAuwAAtQAA*02rwAAAAAA*08zAAAuwAAtQAArwAA*02qQAAzAAAAAAA*07zAAAtQAArwAAqQAA*02owAAzAAAAAAA*07zAAArwAAqQAAowAA*02nQAAzAAAAAAA*07xgAAqQAAowAAnQAA*02mAAAzAAAAAAA*07wAAAowAAnQAAmAAA*02zAAA*01AAAA*07uwAAzAAAmAAAkgAA*02zAAA*01AAAA*07zAAA*01kgAAjAAA*02zAAA*01AAAA*07zAAA*01jAAAhgAA*02zAAA*01AAAA*06zAAA*02hgAAgAAA*02zAAAxgAAAAAA*06zAAA*02gAAAegAA*02xgAAwAAAzAAAAAAA*05zAAA*01xgAAegAAdQAA*02wAAAuwAAzAAAAAAA*05zAAA*01wAAAdQAAbwAA*02uwAAtQAAzAAAAAAA*05zAAAxgAAuwAAbwAAaQAA*02tQAArwAAzAAAAAAA*05xgAAwAAAtQAAaQAAYwAA*02rwAAqQAAzAAA*01AAAA*04wAAAuwAArwAAYwAAXQAA*02qQAAowAAxgAAzAAA*04AAAAuwAAtQAAqQAAXQAAVwAA*02owAAnQAAwAAAzAAA*06rwAAowAAVwAAUgAA*02nQAAmAAAuwAAzAAA*06qQAAnQAAUgAATAAA*02mAAAkgAAtQAAzAAA*06owAAmAAATAAARgAA*02kgAAjAAArwAAxgAAzAAA*05nQAAkgAARgAAQAAA*02jAAAhgAAqQAAwAAAxgAAzAAA*04mAAAjAAAQAAAOgAA*02hgAAgAAAowAAuwAAzAAA*05kgAAhgAAOgAANAAA*02gAAAegAAnQAAtQAAzAAA*05jAAAgAAANAAALwAA*02egAAdQAAmAAArwAAzAAA*04xgAAhgAAegAALwAAKQAA*02dQAAbwAAkgAAqQAAzAAA*03xgAAwAAAgAAAdQAAKQAAIwAA*02bwAAaQAAjAAAowAAzAAA*02xgAAwAAAuwAAegAAbwAAIwAAHQAA*02aQAAYwAAhgAAnQAAzAAA*03uwAAtQAAdQAAaQAAHQAAFwAA*02YwAAXQAAgAAAmAAAzAAA*03tQAArwAAbwAAYwAAFwAAEQAA*02XQAAVwAAegAAkgAAxgAAzAAA*02rwAAqQAAaQAAXQAAEQAADAAA*02VwAAUgAAdQAAjAAAwAAAxgAAzAAA*01qQAAowAAYwAAVwAADAAABgAA*02UgAATAAAbwAAhgAAuwAAwAAAxgAAzAAAowAAnQAAXQAAUgAABgAAAAAA*02TAAARgAAaQAAgAAAtQAAuwAAwAAAzAAAnQAAmAAAVwAATAAAAAAA*03RgAAQAAAYwAAegAArwAAtQAAuwAAxgAAmAAAkgAAUgAARgAAAAAA*03QAAAOgAAXQAAdQAAqQAArwAAtQAAwAAAkgAAjAAATAAAQAAAAAAA*03OgAANAAAVwAAbwAAowAAqQAArwAAuwAAjAAAhgAARgAAOgAAAAAA*03NAAALwAAUgAAaQAAnQAAowAAqQAAtQAAhgAAgAAAQAAANAAAAAAA*03LwAAKQAATAAAYwAAmAAAnQAAowAArwAAgAAAegAAOgAALwAAAAAA*03KQAAIwAARgAAXQAAkgAAmAAAnQAAqQAAegAAdQAANAAAKQAAAAAA*03IwAAHQAAQAAAVwAAjAAAkgAAmAAAowAAdQAAbwAALwAAIwAAAAAA*03HQAAFwAAOgAAUgAAhgAAjAAAkgAAnQAAbwAAaQAAKQAAHQAAAAAA*03FwAAEQAANAAATAAAgAAAhgAAjAAAmAAAaQAAYwAAIwAAFwAAAAAA*03EQAADAAALwAARgAAegAAgAAAhgAAkgAAYwAAXQAAHQAAEQAAAAAA*03DAAABgAAKQAAQAAAdQAAegAAgAAAjAAAXQAAVwAAFwAADAAAAAAA*03BgAAAAAAIwAAOgAAbwAAdQAAegAAhgAAVwAAUgAAEQAABgAAAAAA*05HQAANAAAaQAAbwAAdQAAgAAAUgAATAAADAAAAAAA*06FwAALwAAYwAAaQAAbwAAegAATAAARgAABgAAAAAA*06EQAAKQAAXQAAYwAAaQAAdQAARgAAQAAAAAAA*07DAAAIwAAVwAAXQAAYwAAbwAAQAAAOgAAAAAA*07BgAAHQAAUgAAVwAAXQAAaQAAOgAANAAAAAAA*08FwAATAAAUgAAVwAAYwAANAAALwAAAAAA*07";

unsigned char unzipStr[4 * PixelLen * Frame];

WiFiClient WC;
WiFiManager WM;
PubSubClient MQTT(WC);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> Pixel(PixelLen);

void mqtt_callback(const char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Switch on the Pixel if an 1 was received as first character
    if ((char)payload[0] == '1')
    {
        digitalWrite(LED_BUILTIN, LOW); // Turn the Pixel on (Note that LOW is the voltage level
                                        // but actually the Pixel is on; this is because
                                        // it is active low on the ESP-01)
    }
    else
    {
        digitalWrite(LED_BUILTIN, HIGH); // Turn the Pixel off by making the voltage HIGH
    }
}

byte hex2byte(unsigned char H, unsigned char L)
{
    byte bH, bL, b;
    if (H > '9')
    {
        bH = (H - 'a' + 10) * 16;
    }
    else
    {
        bH = (H - '0') * 16;
    }
    if (L > '9')
    {
        bL = L - 'a' + 10;
    }
    else
    {
        bL = L - '0';
    }
    b = bH + bL;
    return b;
}

void Unzip(unsigned char *unzipStr, unsigned char *zipStr, unsigned int length)
{
    unsigned int zipIdx, unzipIdx = 0;
    for (zipIdx = 0; zipIdx < length; zipIdx++)
    {
        if (zipStr[zipIdx] == '*')
        {
            byte count = hex2byte(zipStr[zipIdx + 1], zipStr[zipIdx + 2]);
            zipIdx += 2;
            for (byte i = 0; i < count; i++)
            {
                for (byte j = 6; j > 2; j--)
                {
                    unzipStr[unzipIdx++] = zipStr[zipIdx - j];
                }
            }
        }
        else
        {
            unzipStr[unzipIdx++] = zipStr[zipIdx];
        }
    }
}

void SetPixelsColor(unsigned char *colorStr)
{
    unsigned char colorList[PixelLen * 3];
    decode_base64(colorStr, colorList);
    for (byte i = 0; i < PixelLen; i++)
    {
        Serial.print(i);
        Serial.print(": ");
        Serial.print(colorList[i * 3]);
        Serial.print("|");
        Pixel.SetPixelColor(i, RgbColor(colorList[i * 3], colorList[i * 3 + 1], colorList[i * 3 + 2]));
    }
    Serial.println("");
    Pixel.Show();
}

void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WM.autoConnect("PulseBeeper");
    Serial.println("\nESP OK");
    Pixel.Begin();

    Unzip(unzipStr, buffer, 2440);
}

void loop()
{
    unsigned char colorStr[PixelLen * 4];
    for (byte i = 0; i < Frame; i++)
    {
        unsigned long now = millis();
        for (byte j = 0; j < PixelLen * 4; j++)
        {
            colorStr[j] = unzipStr[i * PixelLen * 4 + j];
        }
        SetPixelsColor(colorStr);
        while (1)
        {
            if (millis() - now >= 17)
            {
                break;
            }
        }
    }
}
