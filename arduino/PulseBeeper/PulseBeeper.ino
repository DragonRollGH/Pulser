#include <base64.hpp>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NeoPixelBus.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

const byte PixelLen = 16;
const byte Frame = 60;

const char *mqtt_server = "ajdnaud.iot.gz.baidubce.com";
const char *mqtt_clientid = "DragonRollEsp";
const char *mqtt_user = "thingidp@ajdnaud|DragonRollEsp";
const char *mqtt_passwd = "dc4e4a9497c5ada946e54ae789cf4882";
const char *topic_delta = "Switch";
const char *topic_update = "Switch";

// unsigned char buffer[] = "AAAA*0dzAAAAAAA*0bzAAA*03AAAA*09zAAA*05AAAA*09zAAA*05AAAA*08zAAA*06AAAA*08zAAA*05xgAAAAAA*08zAAA*02xgAA*02wAAAAAAA*08zAAA*01xgAAwAAA*02uwAAAAAA*08zAAAxgAAwAAAuwAA*02tQAAAAAA*08zAAAwAAAuwAAtQAA*02rwAAAAAA*08zAAAuwAAtQAArwAA*02qQAAzAAAAAAA*07zAAAtQAArwAAqQAA*02owAAzAAAAAAA*07zAAArwAAqQAAowAA*02nQAAzAAAAAAA*07xgAAqQAAowAAnQAA*02mAAAzAAAAAAA*07wAAAowAAnQAAmAAA*02zAAA*01AAAA*07uwAAzAAAmAAAkgAA*02zAAA*01AAAA*07zAAA*01kgAAjAAA*02zAAA*01AAAA*07zAAA*01jAAAhgAA*02zAAA*01AAAA*06zAAA*02hgAAgAAA*02zAAAxgAAAAAA*06zAAA*02gAAAegAA*02xgAAwAAAzAAAAAAA*05zAAA*01xgAAegAAdQAA*02wAAAuwAAzAAAAAAA*05zAAA*01wAAAdQAAbwAA*02uwAAtQAAzAAAAAAA*05zAAAxgAAuwAAbwAAaQAA*02tQAArwAAzAAAAAAA*05xgAAwAAAtQAAaQAAYwAA*02rwAAqQAAzAAA*01AAAA*04wAAAuwAArwAAYwAAXQAA*02qQAAowAAxgAAzAAA*04AAAAuwAAtQAAqQAAXQAAVwAA*02owAAnQAAwAAAzAAA*06rwAAowAAVwAAUgAA*02nQAAmAAAuwAAzAAA*06qQAAnQAAUgAATAAA*02mAAAkgAAtQAAzAAA*06owAAmAAATAAARgAA*02kgAAjAAArwAAxgAAzAAA*05nQAAkgAARgAAQAAA*02jAAAhgAAqQAAwAAAxgAAzAAA*04mAAAjAAAQAAAOgAA*02hgAAgAAAowAAuwAAzAAA*05kgAAhgAAOgAANAAA*02gAAAegAAnQAAtQAAzAAA*05jAAAgAAANAAALwAA*02egAAdQAAmAAArwAAzAAA*04xgAAhgAAegAALwAAKQAA*02dQAAbwAAkgAAqQAAzAAA*03xgAAwAAAgAAAdQAAKQAAIwAA*02bwAAaQAAjAAAowAAzAAA*02xgAAwAAAuwAAegAAbwAAIwAAHQAA*02aQAAYwAAhgAAnQAAzAAA*03uwAAtQAAdQAAaQAAHQAAFwAA*02YwAAXQAAgAAAmAAAzAAA*03tQAArwAAbwAAYwAAFwAAEQAA*02XQAAVwAAegAAkgAAxgAAzAAA*02rwAAqQAAaQAAXQAAEQAADAAA*02VwAAUgAAdQAAjAAAwAAAxgAAzAAA*01qQAAowAAYwAAVwAADAAABgAA*02UgAATAAAbwAAhgAAuwAAwAAAxgAAzAAAowAAnQAAXQAAUgAABgAAAAAA*02TAAARgAAaQAAgAAAtQAAuwAAwAAAzAAAnQAAmAAAVwAATAAAAAAA*03RgAAQAAAYwAAegAArwAAtQAAuwAAxgAAmAAAkgAAUgAARgAAAAAA*03QAAAOgAAXQAAdQAAqQAArwAAtQAAwAAAkgAAjAAATAAAQAAAAAAA*03OgAANAAAVwAAbwAAowAAqQAArwAAuwAAjAAAhgAARgAAOgAAAAAA*03NAAALwAAUgAAaQAAnQAAowAAqQAAtQAAhgAAgAAAQAAANAAAAAAA*03LwAAKQAATAAAYwAAmAAAnQAAowAArwAAgAAAegAAOgAALwAAAAAA*03KQAAIwAARgAAXQAAkgAAmAAAnQAAqQAAegAAdQAANAAAKQAAAAAA*03IwAAHQAAQAAAVwAAjAAAkgAAmAAAowAAdQAAbwAALwAAIwAAAAAA*03HQAAFwAAOgAAUgAAhgAAjAAAkgAAnQAAbwAAaQAAKQAAHQAAAAAA*03FwAAEQAANAAATAAAgAAAhgAAjAAAmAAAaQAAYwAAIwAAFwAAAAAA*03EQAADAAALwAARgAAegAAgAAAhgAAkgAAYwAAXQAAHQAAEQAAAAAA*03DAAABgAAKQAAQAAAdQAAegAAgAAAjAAAXQAAVwAAFwAADAAAAAAA*03BgAAAAAAIwAAOgAAbwAAdQAAegAAhgAAVwAAUgAAEQAABgAAAAAA*05HQAANAAAaQAAbwAAdQAAgAAAUgAATAAADAAAAAAA*06FwAALwAAYwAAaQAAbwAAegAATAAARgAABgAAAAAA*06EQAAKQAAXQAAYwAAaQAAdQAARgAAQAAAAAAA*07DAAAIwAAVwAAXQAAYwAAbwAAQAAAOgAAAAAA*07BgAAHQAAUgAAVwAAXQAAaQAAOgAANAAAAAAA*08FwAATAAAUgAAVwAAYwAANAAALwAAAAAA*07";

bool isReceive = 0;
byte setIdx = 0;
unsigned long now = millis();

// unsigned char *buffer;
// unsigned int bufferLen;
unsigned char unzipStr[4 * PixelLen * Frame];

WiFiClient WC;
WiFiManager WM;
PubSubClient MQTT(WC);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> Pixel(PixelLen);

void mqtt_callback(const char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(length);
    Serial.print("] ");
    Serial.println();

    Unzip(unzipStr, payload, length);
    setIdx = Frame;
    // buffer = payload;
    // bufferLen = length;
}

byte Hex2Byte(unsigned char H, unsigned char L)
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
            byte count = Hex2Byte(zipStr[zipIdx + 1], zipStr[zipIdx + 2]);
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

    MQTT.setServer(mqtt_server, 1883);
    MQTT.setCallback(mqtt_callback);
    MQTT.setBufferSize(4096);

    Pixel.Begin();

    Serial.println("\nESP OK");
}

void loop()
{
    if (!MQTT.connected())
    {
        MQTT.connect(mqtt_clientid, mqtt_user, mqtt_passwd);
        MQTT.subscribe(topic_delta);
        Serial.println("Baidu IoT connected");
    }
    MQTT.loop();

    if (setIdx && millis() - now >= 17)
    {
        now = millis();
        setIdx %= Frame;
        // if(setIdx == Frame+1) { setIdx = 0; }
        unsigned char colorStr[PixelLen * 4];
        for (byte j = 0; j < PixelLen * 4; j++)
        {
            colorStr[j] = unzipStr[setIdx * PixelLen * 4 + j];
        }
        SetPixelsColor(colorStr);
        setIdx = (setIdx + 1) % Frame;
        if (!setIdx)
        {
            Pixel.ClearTo(RgbColor(0,0,0));
            Pixel.Show();
        }
    }
}
