#include <ArduinoJson.h>
#include <base64.hpp>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <LittleFS.h>
#include <MQTT.h>
#include <NeoPixelBus.h>
#include <OneButton.h>
#include <Ticker.h>
#include <vector>
#include <WiFiManager.h>
#include <Wire.h>
#include "DataStream.cpp"
#include "Pixel.cpp"

// const int MPU = 0x68;  //MPU-6050的I2C地址
// const int nValCnt = 4; //一次读取寄存器的数量

const byte PixelLen = 20;
const byte FrameRate = 17; // =1000ms/60fps
const byte PinTouch = 12;
const byte Sleep = 100;
const int MQTTPort = 1883;
const char *MQTTServer = "ajdnaud.iot.gz.baidubce.com";
const char *Version = "v1.2.04282130";

String Name;
String MQTTUsername;
String MQTTPassword;
String MQTTClientid;
String MQTTPub;
String MQTTSub[2];
float BatteryOffset;

//define in FS
byte H = 0;
byte S = 255;
byte L = 50;
byte A = 5;
byte B = 35;

bool heartBeginFlag = 0;

byte indicatorLightness = 10;
byte indicatorPin = 10;
bool indicatorToggleFlag = 0;

bool streamBeginFlag = 0;
unsigned int streamCache = 1;

MQTTClient MQTT(512);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> heart(PixelLen);
OneButton button(PinTouch, false, false);
WiFiClient WLAN;
WiFiManager WM;
Ticker buttonTicker1;
Ticker buttonTicker2;
Ticker buttonTicker3;
Ticker heartTicker;

DataStream stream;
Pixel pixels[PixelLen];

struct WiFiEntry
{
    String SSID;
    String PASS;
};
std::vector<WiFiEntry> WiFiList;

void PreDefines()
{
    unsigned int ID = ESP.getChipId();
    if (ID == 15406060)
    {
        MQTTPub = "PB/U/M";
        MQTTSub[0] = "PB/D/M";
        MQTTSub[1] = "PB/D/MR";
        BatteryOffset = -0.03;
    }
    else if (ID == 10409937)
    {
        MQTTPub = "PB/U/R";
        MQTTSub[0] = "PB/D/R";
        MQTTSub[1] = "PB/D/MR";
        BatteryOffset = -0.19;
    }
    else
    {
        Name = "Anonymous";
        MQTTUsername = "";
        MQTTPassword = "";
        MQTTClientid = "";
        MQTTPub = "";
        MQTTSub[0] = "";
        MQTTSub[1] = "";
        BatteryOffset = 0;
    }
}

void attachs()
{
    attachInterrupt(digitalPinToInterrupt(PinTouch), buttonTickIrq, CHANGE);
    button.attachClick([]() { stream.write("&NgAAA;"); });
    button.attachDoubleClick([]() { stream.write("&NwAAA;"); });
    button.attachMultiClick([]() { stream.write("&N4AAA;"); });
    button.attachLongPressStart([]() { stream.write("&N8AAA;"); });
    button.attachLongPressStop([]() { stream.write("&N+AAA;"); });
}

void detachs()
{
    streamEnd();
    button.reset();
    detachInterrupt(digitalPinToInterrupt(PinTouch));
    buttonTicker1.detach();
    buttonTicker2.detach();
    buttonTicker3.detach();
}

ICACHE_RAM_ATTR void buttonTickIrq()
{
    button.tick();
    buttonTicker1.once_ms(60, buttonTickTmr);
    buttonTicker2.once_ms(310, buttonTickTmr);
    buttonTicker3.once_ms(810, buttonTickTmr);
}

void buttonTickTmr()
{
    button.tick();
}

void cmdACK()
{
    MQTT.publish(MQTTPub, Name + '_' + Version);
}

void cmdBattery()
{
    MQTT.publish(MQTTPub, String(getBattery()));
}

void cmdDefault(String &payload) {}

void cmdFile(String &payload)
{
    LittleFS.begin();
    LittleFS.end();
}

void cmdHSL(String &payload)
{
    if (payload.length() > 2)
    {
        for (unsigned int i = 2; i < payload.length(); i++)
        {
            stream.write(payload[i]);
        }
    }
}

void cmdID()
{
    unsigned int ID = ESP.getChipId();
    MQTT.publish(MQTTPub, String(ID));
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
    MQTT.publish(MQTTPub, "Starting update from " + url);

    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    ESPhttpUpdate.onStart([] { MQTT.publish(MQTTPub, "[httpUpdate] Started"); });
    ESPhttpUpdate.onError([](int err) { MQTT.publish(MQTTPub, String("[httpUpdate] Error: ") + ESPhttpUpdate.getLastErrorString().c_str()); });
    ESPhttpUpdate.update(url);
}

float getBattery()
{
    unsigned int adcs = 0;
    for (byte i = 0; i < 10; i++)
    {
        adcs += analogRead(A0);
        delay(10);
    }
    float voltage = (adcs / 10) * 247.0f / 1024 / 47 + BatteryOffset;
    return voltage;
}

void heartBegin()
{
    if (!heartBeginFlag)
    {
        heartBeginFlag = 1;
        heart.Begin();
    }
}

void heartClear()
{

    heartBegin();
    heart.ClearTo(RgbColor(0, 0, 0));
    heart.Show();
}

void heartClear(byte i)
{
    heartBegin();
    heart.SetPixelColor(i, RgbColor(0, 0, 0));
    heart.Show();
}

void heartDefult() {}

void heartEnd()
{
    if (heartBeginFlag)
    {
        heartBeginFlag = 0;
        pinMode(3, INPUT);
    }
}

void heartRun(byte b1, byte b2, byte b3, byte b4)
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

void heartSet()
{
    bool running = 1;
    while (stream.avalible && running)
    {
        switch (stream.read())
        {
        case '&':
            switch (stream.read())
            {
            case 'D':
                heartDefult();
                break;
            case 'H':
                H = parseHex(stream.read(), stream.read());
                break;
            case 'S':
                S = parseHex(stream.read(), stream.read());
                break;
            case 'L':
                L = parseHex(stream.read(), stream.read());
                break;
            case 'A':
                A = parseHex(stream.read(), stream.read());
                break;
            case 'B':
                B = parseHex(stream.read(), stream.read());
                break;
            case 'N':
                heartRun(stream.read(), stream.read(), stream.read(), stream.read());
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
    bool anyActive = 0;
    for (byte i = 0; i < PixelLen; i++)
    {
        if (pixels[i].active)
        {
            heart.SetPixelColor(i, HslColor(pixels[i].H, pixels[i].S, pixels[i].L));
            anyActive = 1;
        }
        else
        {
            heart.SetPixelColor(i, HslColor(0, 0, 0));
        }
        pixels[i].update();
    }
    heart.Show();
    if (!anyActive)
    {
        streamEnd();
    }
}

void heartTick()
{
    heartSet();
}

void indicatorClear()
{
    indicatorToggleFlag = false;
    heartClear(indicatorPin);
}

void indicatorSet(char c)
{
    indicatorToggleFlag = true;
    heartBegin();
    RgbColor color;
    switch (c)
    {
    case 'r':
        color = RgbColor(indicatorLightness, 0, 0);
        break;
    case 'g':
        color = RgbColor(0, indicatorLightness, 0);
        break;
    case 'b':
        color = RgbColor(0, 0, indicatorLightness);
        break;
    default:
        color = RgbColor(0, 0, 0);
        break;
    }
    heart.SetPixelColor(indicatorPin, color);
    heart.Show();
}

void indicatorToggle(char c)
{
    indicatorToggleFlag = !indicatorToggleFlag;
    if (indicatorToggleFlag)
    {
        indicatorSet(c);
    }
    else
    {
        indicatorClear();
    }
}

void MQTTConnect()
{
    detachs();
    for (byte i = 0; i < 120; ++i)
    {
        WiFiConnect();
        if (MQTT.connect((MQTTClientid + millis()).c_str(), MQTTUsername.c_str(), MQTTPassword.c_str()))
        {
            break;
        }
        delay(500);
        indicatorToggle('g');
    }
    MQTT.subscribe(MQTTSub[0]);
    delay(10);
    MQTT.subscribe(MQTTSub[1]);
    cmdACK();
    indicatorClear();
    attachs();
}

void MQTTInitialize()
{
    MQTT.begin(MQTTServer, MQTTPort, WLAN);
    MQTT.onMessage(MQTTMsg);
}

void MQTTLoop()
{
    if (!MQTT.connected())
    {
        MQTTConnect();
    }
    MQTT.loop();
}

void MQTTMsg(String &topic, String &payload)
{
    if (payload.length() > 1 && payload[0] == ':')
    {
        switch (payload[1])
        {
        case 'A':
            cmdACK();
            break;
        case 'B':
            cmdBattery();
            break;
        case 'D':
            cmdDefault(payload);
            break;
        case 'F':
            WiFiConfigNew();
            break;
        case 'H':
            cmdHSL(payload);
            break;
        case 'I':
            cmdID();
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

byte parseHex(byte L)
{
    if ((L >= '0') && (L <= '9'))
        return L - '0';
    if ((L >= 'A') && (L <= 'F'))
        return L + 10 - 'A';
    if ((L >= 'a') && (L <= 'f'))
        return L + 10 - 'a';
    return -1;
}

byte parseHex(byte H, byte L)
{
    return parseHex(H) * 16 + parseHex(L);
}

void streamBegin()
{
    heartBegin();
    heartTicker.attach_ms(FrameRate, heartTick);
}

void streamEnd()
{
    heartTicker.detach();
    heartEnd();
}

void streamLoop()
{
    if (stream.avalible >= streamCache && !streamBeginFlag)
    {
        streamBegin();
    }
}

void WiFiAdd(String SSID, String PASS)
{
    WiFiList.push_back(WiFiEntry{SSID, PASS});
}

void WiFiConfigNew()
{
    LittleFS.begin();
    StaticJsonDocument<128> doc;
    doc["len"] = 0;
    File WiFiConfig = LittleFS.open("/WiFi.json", "w");
    serializeJson(doc, WiFiConfig);
    WiFiConfig.close();
    LittleFS.end();
}

void WiFiConfigRead()
{
    LittleFS.begin();
    if (LittleFS.exists("/WiFi.json"))
    {
        StaticJsonDocument<512> doc;
        File WiFiConfig = LittleFS.open("/WiFi.json", "r");
        deserializeJson(doc, WiFiConfig);
        for (byte i = 0; i < doc["len"]; ++i)
        {
            WiFiAdd(doc["ssid"][i], doc["pass"][i]);
        }
        WiFiConfig.close();
    }
    else
    {
        StaticJsonDocument<128> doc;
        doc["len"] = 0;
        File WiFiConfig = LittleFS.open("/WiFi.json", "w");
        serializeJson(doc, WiFiConfig);
        WiFiConfig.close();
    }
    LittleFS.end();

}

void WiFiConfigWrite(String SSID, String PASS)
{
    LittleFS.begin();
    File WiFiConfig = LittleFS.open("/WiFi.json", "r");
    StaticJsonDocument<512> doc;
    deserializeJson(doc, WiFiConfig);
    WiFiConfig.close();
    byte len = doc["len"];
    bool exist = 0;
    for (byte i = 0; i < len; ++i)
    {
        if (doc["ssid"][i] == SSID)
        {
            exist = 1;
            if (doc["pass"][i] != PASS)
            {
                doc["pass"][i] = PASS;
            }
        }
    }
    if (!exist)
    {
        ++len;
        doc["len"] = len;
        doc["ssid"].add(SSID);
        doc["pass"].add(PASS);
    }
    WiFiConfig = LittleFS.open("/WiFi.json", "w");
    serializeJson(doc, WiFiConfig);
    WiFiConfig.close();
    LittleFS.end();
}

int WiFiConnect()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        indicatorSet('r');
        WiFi.disconnect();
        WiFi.scanDelete();
        for (byte k = 0; k < 3; ++k)
        {
            byte scanResult = WiFi.scanNetworks();
            byte bestWiFi = 255;
            if (scanResult > 0)
            {
                int bestRSSI = INT_MIN;
                for (byte i = 0; i < scanResult; ++i)
                {
                    for (byte j = 0; j < WiFiList.size(); ++j)
                    {
                        if (WiFi.SSID(i) == WiFiList[j].SSID)
                        {
                            if (WiFi.RSSI(i) > bestRSSI)
                            {
                                bestWiFi = j;
                            }
                            break;
                        }
                    }
                }
            }
            WiFi.scanDelete();

            if (bestWiFi != 255)
            {
                WiFi.begin(WiFiList[bestWiFi].SSID, WiFiList[bestWiFi].PASS);
                for (byte i = 0; i < 60; ++i)
                {
                    indicatorToggle('r');
                    if (WiFi.status() == WL_CONNECTED)
                    {
                        indicatorClear();
                        return 1;
                    }
                    delay(500);
                }
            }
        }

        WiFiPortal();
    }
}

void WiFiInitialize()
{
    WiFi.mode(WIFI_STA);
    WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
    WiFiConfigRead();
}

int WiFiPortal()
{
    indicatorSet('b');
    WM.setConfigPortalTimeout(180);
    WM.startConfigPortal((Name + "s_Pulser").c_str());
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFiConfigWrite(WM.getWiFiSSID(), WM.getWiFiPass());
        WiFiAdd(WM.getWiFiSSID(), WM.getWiFiPass());
        indicatorClear();
        return 1;
    }
    else
    {
        indicatorClear();
        ESP.deepSleepInstant(INT32_MAX);
    }
}

void setup()
{
    PreDefines();

    WiFiInitialize();
    MQTTInitialize();

    attachs();
}

void loop()
{
    MQTTLoop(); //will keep connect wifi and mqtt in this function

    streamLoop();

    delay(Sleep);
}
