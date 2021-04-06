#include <base64.hpp>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <MQTT.h>
#include <NeoPixelBus.h>
#include <WiFiManager.h>

const byte PixelLen = 20;
const byte FrameRate = 17; // =1000ms/60fps
const byte SleepRun = 1;
const byte SleepIdle = 100;
const char *AP_SSID = "Rolls_Pulser";

// const char *MQTTServer = "";
// const int   MQTTPort = 1883;
// const char *MQTTUsername = "";
// const char *MQTTPassword = "";
// const char *MQTTClientid = "";
// const char *MQTTSub1 = "";
// const char *MQTTSub2 = "";
// const char *MQTTPub = "";


byte H = 0;
byte S = 255;
byte L = 50;
byte live = 5;
byte dead = 35;
byte sleep = 0;
unsigned long cache = PixelLen * 4 * 20;
unsigned long now = millis();
unsigned long flowStart;
unsigned long flowFrame;

WiFiClient WLAN;
MQTTClient MQTT(1024);

WiFiManager WM;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> Pixel(PixelLen);

class Pixel
{
public:
    bool active = false;
    float H;
    float S;
    float L;
    float deltaL;
    byte live;

    void run(byte rH, byte rS, byte rL, byte rlive, byte rdead)
    {
        active = true;
        H = rH / 255.0f;
        S = rS / 255.0f;
        L = rL / 255.0f;
        deltaL = L / rdead;
        live = rlive;
    }

    void update(void)
    {
        if (active)
        {
            if (live)
            {
                live -= 1;
            }
            else if (L > 0)
            {
                L -= delatL;
            }
            else
            {
                active = false;
            }
        }
    }
};

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

void mqttConnect(void)
{
    while (!MQTT.connect(MQTTClientid, MQTTUsername, MQTTPassword))
    {
        delay(500);
    }
    MQTT.subscribe(MQTTSub1);
    delay(10);
    MQTT.subscribe(MQTTSub2);
    Serial.println("MQTT connected");
}

void mqttMsg(String &topic, String &payload)
{
    // Serial.print("Message arrived [");
    // Serial.print(topic);
    // Serial.print("] ");
    // Serial.println();
    Serial.println("Message arrived [" + topic + "] ");

    if (payload.length > 1 && payload[0] == ':')
    {
        switch (payload[1])
        {
        case 'A':
            cmdACK(payload);
            break;
        case 'D':
            cmdDefault(payload);
            break;
        case 'H':
            cmdHSL(payload);
            break;
        case 'P':
            cmdPulse(payload);
            break;
        case 'R':
            cmdRGB(payload);
            break;
        case 'U':
            cmdUpdate(payload);
            break;
        default:
            break;
        }
    }
}
void cmdACK(String &payload);

void cmdDefault(String &payload);

void cmdHSL(String &payload)
{
    if (payload.length > 2)
    {
        for (unsigned int i = 2; i < payload.length; i++)
        {
            BS.write(payload[i]);
        }
    }
}

void cmdPulse(String &payload);

void cmdRGB(String &payload);

void cmdUpdate(String &paylaod)
{
    String url = "https://github.com/DragonRollGH/PulseBeeper/raw/main/arduino/latest.bin";
    if (paylaod.length > 1)
    // if (paylaod.substring(1,7) == "http://")
    {
        url = paylaod.substring(1);
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

unsigned int toInt64(unsigned char H, unsigned char L)
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

unsigned int toInt16(unsigned char H, unsigned char L)
    // void Unzip(unsigned char *unzipStr, unsigned char *zipStr, unsigned int length)
    // {
    //     unsigned int zipIdx, unzipIdx = 0;
    //     for (zipIdx = 0; zipIdx < length; zipIdx++)
    //     {
    //         if (zipStr[zipIdx] == '*')
    //         {
    //             byte count = toHex(zipStr[zipIdx + 1], zipStr[zipIdx + 2]);
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
            byte count = toHex(buffer[p + 1], buffer[p + 2]);
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

void runFlow(void)
{
    flowFrame = 1;
    flowStart = millis();
    sleep = SleepRun;
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
    bool a = 1;
    while (a)
    {
        switch (BS.read())
        {
        case '&':
            switch (BS.read())
            {
            case 'H':
                H = toByte(BS.read(), BS.read());
                break;
            case 'S':
                S = toByte(BS.read(), BS.read());
                break;
            case 'L':
                L = toByte(BS.read(), BS.read());
                break;
            case 'l':
                live = toByte(BS.read(), BS.read());
                break;
            case 'd':
                dead = toByte(BS.read(), BS.read());
                break;
            case 'C':
                setHSL(BS.read(), BS.read(), BS.read(), BS.read());
                break;
            case 'N':
                runPixel(BS.read(), BS.read(), BS.read(), BS.read());
                break;
            default:
                break;
            }
            break;
        case ';':
            a = 0;
            break;
        default:
            break;
        }
    }
    if (BS.read() == '?')
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

    WM.autoConnect(AP_SSID);

    MQTT.begin(MQTTServer, MQTTPort, WLAN);
    MQTT.onMessage(mqttMsg);

    Pixel.Begin();

    Serial.println("\nESP OK");
}

void loop()
{
    if (!MQTT.connected())
    {
        mqttConnect();
    }
    MQTT.loop();

    if (flowStart)
    {
        if (millis() - flowStart >= flowFrame * FrameRate)
        {
            ++flowFrame;
            SetPixelsColor();
        }
    }
    else
    {
        if (BS.avalible >= cache)
        {
            runFlow();
        }
    }

    delay(sleep);
}
