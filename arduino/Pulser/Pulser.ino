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


byte dH = 0;
byte dS = 255;
byte dL = 50;
byte dA = 5;
byte dB = 35;

byte H = dH;
byte S = dS;
byte L = dL;
byte A = dA;
byte B = dB;

byte sleep = 0;

unsigned int cache = 1;
unsigned long flowStart;
unsigned long flowFrame;

WiFiClient WLAN;
MQTTClient MQTT(512);

WiFiManager WM;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> heart(PixelLen);

class Pixel
{
public:
    bool active = false;
    float H;
    float S;
    float L;
    float deltaL;
    byte A;

    void run(byte rH, byte rS, byte rL, byte rA, byte rB)
    {
        active = true;
        H = rH / 255.0f;
        S = rS / 255.0f;
        L = rL / 255.0f;
        A = rA;
        deltaL = L / rB;
    }

    void update(void)
    {
        if (active)
        {
            if (A)
            {
                A -= 1;
            }
            else
            {
                L -= deltaL;
                if (L <= 0)
                {
                    active = false;
                }
            }
        }
    }
};
Pixel pixels[PixelLen];

class ByteStream
{
public:
    const static unsigned int buf = 2048;
    unsigned int wi = 0;
    unsigned int ri = 0;
    unsigned int avalible = 0;
    byte stream[buf];

    void begin(void)
    {
        wi = 0;
        ri = 0;
        avalible = 0;
    }

    void write(byte w)
    {
        if (avalible <= buf)
        {
            stream[wi] = w;
            wi = (wi + 1) % buf;
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
            ri = (ri + 1) % buf;
            --avalible;
            return stream[ri - 1];
        }
    }

    void unwrite(unsigned int ui)
    {
        wi = (wi + buf - ui) % buf;
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
    Serial.println("Message arrived [" + topic + "] " + payload);

    if (payload.length() > 1 && payload[0] == ':')
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

void cmdACK(String &payload) {}

void cmdDefault(String &payload) {}

void cmdHSL(String &payload)
{
    if (payload.length() > 2)
    {
        for (unsigned int i = 2; i < payload.length(); i++)
        {
            BS.write(payload[i]);
        }
    }
}

void cmdPulse(String &payload) {}

void cmdRGB(String &payload) {}

void cmdUpdate(String &paylaod)
{
    String url = "http://192.168.1.110:5500/arduino/Pulser/Pulser.ino.generic.bin";
    if (paylaod.length() > 2)
    {
        url = paylaod.substring(2);
    }
    Serial.println("Starting update from " + url);
    MQTT.publish(MQTTPub, "Starting update from " + url);

    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    ESPhttpUpdate.onStart(updateStarted);
    ESPhttpUpdate.onEnd(updateFinished);
    ESPhttpUpdate.onError(updateError);
    ESPhttpUpdate.update(url);
    // switch (ret)
    // {
    // case HTTP_UPDATE_FAILED:
    //     Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
    //     MQTT.publish(MQTTPub, String("HTTP_UPDATE_FAILED Error: ") + ESPhttpUpdate.getLastErrorString().c_str());
    //     break;

    // case HTTP_UPDATE_OK:
    //     Serial.println("HTTP_UPDATE_OK");
    //     MQTT.publish(MQTTPub, "HTTP_UPDATE_OK");
    //     break;
    // }
}

void updateStarted()
{
    MQTT.publish(MQTTPub, "[httpUpdate] Started");
}

void updateFinished()
{
    MQTT.publish(MQTTPub, "[httpUpdate] Finished");
}

void updateError(int err)
{
    MQTT.publish(MQTTPub, String("[httpUpdate] Error: ") + ESPhttpUpdate.getLastErrorString().c_str());
}

byte toByte(byte H, byte L)
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

void useDefault(void) {}

void setHSL(byte b1, byte b2, byte b3, byte b4) {}

void runPixels(byte b1, byte b2, byte b3, byte b4)
{
    byte base[4] = {b1, b2, b3, b4};
    byte arry[3];
    decode_base64(base, arry);
    for (byte i = 0; i < PixelLen; i++)
    {
        if (arry[i / 8] & (byte)128)
        {
            pixels[i].run(H, S, L, A, B);
        }
        arry[i / 8] <<= 1;
    }
}

void setPixelsColor(void)
{
    ++flowFrame;
    bool running = 1;
    while (BS.avalible && running)
    {
        switch (BS.read())
        {
        case '&':
            switch (BS.read())
            {
            case 'D':
                useDefault();
                break;
            case 'H':
                H = toByte(BS.read(), BS.read());
                break;
            case 'S':
                S = toByte(BS.read(), BS.read());
                break;
            case 'L':
                L = toByte(BS.read(), BS.read());
                break;
            case 'A':
                A = toByte(BS.read(), BS.read());
                break;
            case 'B':
                B = toByte(BS.read(), BS.read());
                break;
            case 'C':
                setHSL(BS.read(), BS.read(), BS.read(), BS.read());
                break;
            case 'N':
                runPixels(BS.read(), BS.read(), BS.read(), BS.read());
                break;
            default:
                break;
            }
            break;
        case ';':
            running = 0;
            break;
        default:
            break;
        }
    }
    running = 0;
    for (byte i = 0; i < PixelLen; i++)
    {
        if (pixels[i].active)
        {
            heart.SetPixelColor(i, HslColor(pixels[i].H, pixels[i].S, pixels[i].L));
            running = 1;
        }
        else
        {
            heart.SetPixelColor(i, HslColor(0, 0, 0));
        }
        pixels[i].update();
    }
    heart.Show();
    if (!running)
    {
        stopFlow();
    }
}

void runFlow(void)
{
    flowFrame = 1;
    flowStart = millis();
    sleep = SleepRun;
}

void stopFlow(void)
{
    flowStart = 0;
    sleep = SleepIdle;
}

void setup()
{
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.setSleepMode(WIFI_LIGHT_SLEEP);

    WM.autoConnect(AP_SSID);

    MQTT.begin(MQTTServer, MQTTPort, WLAN);
    MQTT.onMessage(mqttMsg);

    heart.Begin();

    BS.begin();

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
            setPixelsColor();
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
