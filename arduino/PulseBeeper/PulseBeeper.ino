#include <base64.hpp>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <NeoPixelBus.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

const byte PixelLen = 20;
const byte FrameRate = 17; // =1000ms/60fps
const byte SleepRun = 1;
const byte SleepIdle = 100;

const char *mqtt_server = "ajdnaud.iot.gz.baidubce.com";
const char *mqtt_clientid = "DragonRollEsp";
const char *mqtt_user = "thingidp@ajdnaud|DragonRollEsp";
const char *mqtt_passwd = "dc4e4a9497c5ada946e54ae789cf4882";
//const char *mqtt_clientid = "DragonRollEsp2";
//const char *mqtt_user = "thingidp@ajdnaud|DragonRollEsp2";
//const char *mqtt_passwd = "9cb80a11b9d6b9ab6cedf7e534328126";
const char *topic_delta = "Switch";
const char *topic_update = "Switch";

bool isCmd = 0;
bool isFlow = 0;
byte sleep = 0;
unsigned long cache = PixelLen * 4 * 20;
unsigned long now = millis();

String cmdArg;

WiFiClient WC;
WiFiManager WM;
PubSubClient MQTT(WC);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> Pixel(PixelLen);

class ByteStream
{
public:
    unsigned int buffer = 4800;
    unsigned int wi = 0;
    unsigned int ri = 0;
    unsigned int avalible = 0;
    byte stream[buffer];

    void write(byte w)
    {
        if (avalible <= buffer)
        {
            stream[wi] = w;
            wi = (wi + 1) % buffer;
            ++avalible;
        }
    }

    byte read(void)
    {
        if (avalible == 0)
        {
            return '\0';
        }
        else
        {
            ri = (ri + 1) % buffer;
            --avalible;
            return stream[ri - 1];
        }
    }

    void undo(unsigned int ui)
    {
        wi = (wi + buffer - ui) % buffer;
        avalible -= ui;
    }
};

ByteStream BS;

void mqtt_callback(const char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(length);
    Serial.print("] ");
    Serial.println();

    if (payload[0] == ':')
    {
        isCmd = 1;
        cmdArg = "";
        for (byte i = 1; i < length; i++)
        {
            cmdArg += (char)payload[i]
        }
    }
    else
    {
        Unzip(payload, length)
    }
}
// else
// {
//     Unzip(unzipStr, payload, length);
//     arrIdx = Frame;
// }
// }

void ConnectMQTT(void)
{
    MQTT.connect(mqtt_clientid, mqtt_user, mqtt_passwd);
    MQTT.subscribe(topic_delta);
    Serial.println("Baidu IoT connected");
}

void cmdUpdate(String cmdArg)
{
    String url = "https://github.com/DragonRollGH/PulseBeeper/raw/main/arduino/latest.bin";
    if (cmdArg.length >= 1)
    // if (cmdArg.substring(1,7) == "http://")
    {
        url = cmdArg.substring(1);
    }
    Serial.println("Starting update from" + url);
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    t_httpUpdate_return ret = ESPhttpUpdate.update(url);
    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

    case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
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

// void Unzip(unsigned char *unzipStr, unsigned char *zipStr, unsigned int length)
// {
//     unsigned int zipIdx, unzipIdx = 0;
//     for (zipIdx = 0; zipIdx < length; zipIdx++)
//     {
//         if (zipStr[zipIdx] == '*')
//         {
//             byte count = Hex2Byte(zipStr[zipIdx + 1], zipStr[zipIdx + 2]);
//             zipIdx += 2;
//             for (byte i = 0; i < count; i++)
//             {
//                 for (byte j = 6; j > 2; j--)
//                 {
//                     unzipStr[unzipIdx++] = zipStr[zipIdx - j];
//                 }
//             }
//         }
//         else
//         {
//             unzipStr[unzipIdx++] = zipStr[zipIdx];
//         }
//     }
// }

void Unzip(byte *payload, unsigned int length)
{
    byte zipWindow = 4;
    unsigned int validation = 0;
    for (unsigned int p = 0; p < length; p++)
    {
        if (buffer[p] == '*')
        {
            byte count = Hex2Byte(buffer[p + 1], buffer[p + 2]);
            p += 2;
            for (byte i = 0; i < count; i++)
            {
                for (byte j = zipWindow + 2; j > 2; j--)
                {
                    BS.write(buffer[p - j]);
                    ++validation;
                }
            }
        }
        else
        {
            BS.write(buffer[p]);
            ++validation;
        }
    }
    if (validation % (PixelLen * 4) != 0)
    {
        BS.undo(validation);
    }
}

// void SetPixelsColor(unsigned char *colorStr)
// {
//     unsigned char colorList[PixelLen * 3];
//     decode_base64(colorStr, colorList);
//     for (byte i = 0; i < PixelLen; i++)
//     {
//         Serial.print(i);
//         Serial.print(": ");
//         Serial.print(colorList[i * 3]);
//         Serial.print("|");
//         Pixel.SetPixelColor(i, RgbColor(colorList[i * 3], colorList[i * 3 + 1], colorList[i * 3 + 2]));
//     }
//     Serial.println("");
//     Pixel.Show();
// }

void SetPixelsColor(void)
{
    if (BS.avalible >= PixelLen * 4)
    {
        byte arry[PixelLen * 3];
        byte base[PixelLen * 4];
        for (byte i = 0; i < PixelLen * 4; i++)
        {
            base[i] = BS.read();
        }
        decode_base64(base, arry);
        for (byte i = 0; i < PixelLen; i++)
        {
            Serial.print(i);
            Serial.print(": ");
            Serial.print(arry[i * 3]);
            Serial.print("|");
            Pixel.SetPixelColor(i, RgbColor(arry[i * 3], arry[i * 3 + 1], arry[i * 3 + 2]));
        }
        Serial.println("");
        Pixel.Show();
    }
    else
    {
        isFlow = 0;
        sleep = SleepIdle;
        Pixel.ClearTo(RgbColor(0, 0, 0));
        Pixel.Show();
    }
}

void setup()
{
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.setSleepMode(WIFI_LIGHT_SLEEP);

    WM.autoConnect("PulseBeeper");

    MQTT.setServer(mqtt_server, 1883);
    MQTT.setCallback(mqtt_callback);
    MQTT.setBufferSize(4800);

    Pixel.Begin();

    Serial.println("\nESP OK");
}

void loop()
{
    if (!MQTT.connected())
    {
        ConnectMQTT();
    }
    MQTT.loop();

    if (isFlow)
    {
        if (millis() - now >= FrameRate)
        {
            now = millis();
            SetPixelsColor();
        }
    }
    else if (isCmd)
    {
        isCmd = 0;
        switch (cmdArg[0])
        {
        case 'a':
            cmdUpdate(cmdArg);
            break;
        default:
            Serial.println("Unknown command.")
        }
    }
    else
    {
        if (BS.avalible >= cache)
        {
            isFlow = 1;
            sleep = SleepRun;
        }
    }

    delay(sleep);
}
