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
const char *AP_SSID = "Rolls_Pulser";
const char *Name = "Roll";

// const char *MQTTServer = "";
// const int   MQTTPort = 1883;
// const char *MQTTUsername = "";
// const char *MQTTPassword = "";
// const char *MQTTClientid = "";
// const char *MQTTSub1 = "";
// const char *MQTTSub2 = "";
// const char *MQTTPub = "";


//define in FS
byte H = 0;
byte S = 255;
byte L = 50;
byte A = 5;
byte B = 35;
byte sleepRun = 1;
byte sleepIdle = 100;
unsigned int flowCache = 1;

byte sleep = sleepIdle;
unsigned int flowFrame;
unsigned long flowStart;

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
    // Serial.println("MQTT connected");
    cmdACK();
}

void mqttMsg(String &topic, String &payload)
{
    // Serial.println("Message arrived [" + topic + "] " + payload);

    if (payload.length() > 1 && payload[0] == ':')
    {
        switch (payload[1])
        {
        case 'A':
            cmdACK();
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
            MQTT.publish(MQTTPub, "Unknown command");
            break;
        }
    }
}

void cmdACK(void)
{
    MQTT.publish(MQTTPub, Name);
}

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
    // Serial.println("Starting update from " + url);
    MQTT.publish(MQTTPub, "Starting update from " + url);

    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    ESPhttpUpdate.onStart([] { MQTT.publish(MQTTPub, "[httpUpdate] Started"); });
    ESPhttpUpdate.onError([](int err) { MQTT.publish(MQTTPub, String("[httpUpdate] Error: ") + ESPhttpUpdate.getLastErrorString().c_str()); });
    ESPhttpUpdate.update(url);
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

void useDefault(void) {}

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

void runFlow(void)
{
    flowFrame = 1;
    flowStart = millis();
    sleep = sleepRun;
    heart.Begin();
}

void stopFlow(void)
{
    flowStart = 0;
    sleep = sleepIdle;
    pinMode(3, INPUT);
}

void setup()
{
    // Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.setSleepMode(WIFI_LIGHT_SLEEP);

    WM.autoConnect(AP_SSID);

    MQTT.begin(MQTTServer, MQTTPort, WLAN);
    MQTT.onMessage(mqttMsg);

    // Serial.println("\nESP OK");
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
        if (BS.avalible >= flowCache)
        {
            runFlow();
        }
    }

    delay(sleep);
}
