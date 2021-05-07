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
const String Pulse = "&b0d&L00&N////;;;;;;;;&b20&L14&N////;;;;&B16&L18&N////;;;;;;;;;;;;;;;;;;;;&B08&L02&N////;;;;;;;;";
const char *Version = "v2.1.05062000";

String Name;
String MQTTUsername;
String MQTTPassword;
String MQTTClientid;
String MQTTPub[2];
String MQTTSub[2];
float batteryOffset;

//define in FS
// byte H = 0;
// byte S = 255;
// byte L = 20;
// byte A = 5;
// byte B = 35;

bool batteryBeginFlag = 0;

bool heartBeginFlag = 0;

byte indicators[2] = {0, 10};
bool indicatorFlags[2] = {0, 0};
byte indicatorBattery = 0;
byte indicatorNetwork = 1;
byte indicatorLightness = 20;
bool indicatorToggleFlag = 0;

bool pulseConflictFlag = 0;
byte pulseOtherH;
bool pulseOtherOnline = 0;
byte pulseTickerTimeout;

bool streamBeginFlag = 0;
byte streamCache = 1;

MQTTClient MQTT(512);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> heart(PixelLen);
OneButton button(PinTouch, false, false);
WiFiClient WLAN;
WiFiManager WM;
Ticker buttonTicker[3];
Ticker heartTicker;
Ticker pulseTicker;

DataStream stream;
Pixel pixels[PixelLen];
PixelColor colors[2] = {{0, 255, 20, 5, 35}, {85, 255, 0, 0, 1}};
bool colorCurrent = 0;

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
        MQTTPub[0] = "PB/U/M";
        MQTTPub[1] = "PB/D/R";
        MQTTSub[0] = "PB/D/M";
        MQTTSub[1] = "PB/D/MR";
        batteryOffset = -0.03;
        colors[0].H = 120;
    }
    else if (ID == 10409937)
    {
        MQTTPub[0] = "PB/U/R";
        MQTTPub[1] = "PB/D/M";
        MQTTSub[0] = "PB/D/R";
        MQTTSub[1] = "PB/D/MR";
        batteryOffset = -0.19;
        colors[0].H = 0;
    }
    else
    {
        Name = "Anonymous";
        MQTTUsername = "";
        MQTTPassword = "";
        MQTTClientid = "";
        MQTTPub[0] = "";
        MQTTPub[1] = "";
        MQTTSub[0] = "";
        MQTTSub[1] = "";
        batteryOffset = 0;
    }
}

void attachs()
{
    streamOpen();
    attachInterrupt(digitalPinToInterrupt(PinTouch), buttonTickIrq, CHANGE);
    button.attachClick([]() { stream.write("&C1&A00;" + Pulse + "&C0;"); });
    button.attachDoubleClick([]() { batteryBeginFlag = 1; });
    // button.attachMultiClick([]() { stream.write(Pulse3); });
    button.attachLongPressStart(pulseStart);
    button.attachLongPressStop(pulseStop);
}

void detachs()
{
    streamEnd();
    streamClose();
    button.reset();
    detachInterrupt(digitalPinToInterrupt(PinTouch));
    buttonTicker[0].detach();
    buttonTicker[1].detach();
    buttonTicker[2].detach();
}

void ackMsg()
{
    MQTT.publish(MQTTPub[0], Name + '_' + Version);
}

void battryAnimation()
{
    byte battery = batteryGet();
    heartClear();
    detachs();
    heartBegin();
    for (byte i = 0; i < PixelLen; ++i)
    {
        heart.SetPixelColor(i, HslColor(i / (PixelLen * 3.0f), 1, indicatorLightness / 510.0f));
    }
    heart.Show();
    battery = battery * PixelLen / 100;
    for (byte i = PixelLen - 1; i >= battery; --i)
    {
        delay(1000 / (PixelLen - battery));
        heartClear(i);
        heart.Show();
    }
    if (battery == 0)
    {
        delay(5000);
        heartClear();
        ESP.deepSleepInstant(INT32_MAX);
    }
    delay(3000);
    heartClear();
    attachs();
}

byte batteryGet()
{
    unsigned int adcs = 0;
    for (byte i = 0; i < 10; i++)
    {
        adcs += analogRead(A0);
        delay(10);
    }
    float voltage = (adcs / 10) * 247.0f / 1024 / 47 + batteryOffset;
    byte percent = (voltage - 3.2) * 100;
    if (percent > 150)
    {
        percent = 0;
    }
    else if (percent > 100)
    {
        percent = 100;
    }
    return percent;
}

void batteryInitialize()
{
    byte battery = batteryGet();
    heartBegin();
    heart.SetPixelColor(indicators[indicatorBattery], HslColor(battery / 300.0f, 1, indicatorLightness / 510.0f));
    heart.Show();
    if (battery == 0)
    {
        delay(5000);
        heartClear();
        ESP.deepSleepInstant(INT32_MAX);
    }
}

void batteryMsg()
{
    MQTT.publish(MQTTPub[0], String(batteryGet()) + '%');
}

ICACHE_RAM_ATTR void buttonTickIrq()
{
    button.tick();
    buttonTicker[0].once_ms(60, buttonTickTmr);
    buttonTicker[1].once_ms(310, buttonTickTmr);
    buttonTicker[2].once_ms(810, buttonTickTmr);
}

void buttonTickTmr()
{
    button.tick();
}

void configMsg()
{
    LittleFS.begin();
    WiFiConfigNew();
    LittleFS.end();
}

void defaultMsg(String &payload) {}

void hslMsg(String &payload)
{
    if (payload.length() > 2)
    {
        for (unsigned int i = 2; i < payload.length(); i++)
        {
            stream.write(payload[i]);
        }
    }
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
    // heartEnd(); need delay > 1 after show()
}

void heartClear(byte i)
{
    heartBegin();
    heart.SetPixelColor(i, RgbColor(0, 0, 0));
    heart.Show();
}

void heartColorSets(byte Idx)
{
    if ((Idx >= '0') && (Idx <= '1'))
    {
        colorCurrent = Idx - '0';
    }
}

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
            pixels[i].run(colors[colorCurrent]);
        }
        arry[i / 8] <<= 1;
    }
}

void heartTick()
{
    bool running = 1;
    while (stream.avalible && running)
    {
        switch (stream.read())
        {
        case '&':
            switch (stream.read())
            {
            case 'H':
                colors[colorCurrent].H = parseHex(stream.read(), stream.read());
                break;
            case 'S':
                colors[colorCurrent].S = parseHex(stream.read(), stream.read());
                break;
            case 'L':
                colors[colorCurrent].L = parseHex(stream.read(), stream.read());
                break;
            case 'A':
                colors[colorCurrent].A = parseHex(stream.read(), stream.read());
                break;
            case 'B':
                colors[colorCurrent].B = parseHex(stream.read(), stream.read());
                break;
            case 'b':
                colors[colorCurrent].B = -1 * parseHex(stream.read(), stream.read());
                break;
            case 'C':
                heartColorSets(stream.read());
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

void idMsg()
{
    unsigned int ID = ESP.getChipId();
    MQTT.publish(MQTTPub[0], String(ID));
}

void indicatorClear(byte indicator)
{
    indicatorFlags[indicator] = false;
    heartClear(indicators[indicator]);
}

void indicatorSet(byte indicator, char c)
{
    indicatorFlags[indicator] = true;
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
    heart.SetPixelColor(indicators[indicator], color);
    heart.Show();
}

void indicatorToggle(byte indicator, char c)
{
    indicatorFlags[indicator] = !indicatorFlags[indicator];
    if (indicatorFlags[indicator])
    {
        indicatorSet(indicator, c);
    }
    else
    {
        indicatorClear(indicator);
    }
}

void menuLoop()
{
    if (batteryBeginFlag)
    {
        batteryBeginFlag = 0;
        battryAnimation();
    }
}

void MQTTConnect()
{
    // heartClear(); cause to turn off battery indicator.
    detachs();
    for (byte i = 0; i < 120; ++i)
    {
        WiFiConnect();
        if (MQTT.connect((MQTTClientid + millis()).c_str(), MQTTUsername.c_str(), MQTTPassword.c_str()))
        {
            break;
        }
        delay(500);
        indicatorToggle(indicatorNetwork, 'g');
    }
    MQTT.subscribe(MQTTSub[0]);
    delay(10);
    MQTT.subscribe(MQTTSub[1]);
    ackMsg();
    heartClear();
    attachs();
    delay(10);
    heartEnd();
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
            ackMsg();
            break;
        case 'B':
            batteryMsg();
            break;
        case 'C':
            configMsg();
            break;
        case 'D':
            defaultMsg(payload);
            break;
        case 'H':
            hslMsg(payload);
            break;
        case 'I':
            idMsg();
            break;
        case 'P':
            pulseMsg(payload);
            break;
        case 'R':
            rgbMsg(payload);
            break;
        case 'U':
            updateMsg(payload);
            break;
        default:
            MQTT.publish(MQTTPub[0], "Unknown command");
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

void pulseBegin(bool other)
{
    colors[1].H = (other) ? pulseOtherH : colors[0].H;
    if (!pulseTicker.active())
    {
        stream.write("&C1&A00;");
        pulseTickerTimeout = 20;
        pulseTicker.attach_ms(675, pulseTick);
    }
    else
    {
        pulseConflictFlag = 1;
    }
}

void pulseEnd(bool other)
{
    if (pulseConflictFlag)
    {
        pulseConflictFlag = 0;
        colors[1].H = (other) ? colors[0].H : pulseOtherH;
    }
    else
    {
        pulseTicker.detach();
        stream.write("&C0;");
    }
}

void pulseMsg(String &payload)
{
    switch (payload[2])
    {
    case 'A':
        pulseOtherH = parseHex(payload[3], payload[4]);
        pulseBegin(1);
        // MQTT.publish(MQTTPub[1], ":Pa");
        break;
    case 'a':
        /* code */
        break;
    case 'B':
        pulseEnd(1);
        break;
    default:
        break;
    }
}

void pulseOnline()
{

    MQTT.publish(MQTTPub[1], ":PO");
}

void pulseStart()
{
    MQTT.publish(MQTTPub[1], ":PA" + toHex(colors[0].H));
    pulseBegin(0);
}

void pulseStop()
{
    pulseEnd(0);
    MQTT.publish(MQTTPub[1], ":PB");
}

void pulseTick()
{
    if (pulseTickerTimeout)
    {
        --pulseTickerTimeout;
        stream.write(Pulse);
    }
    else
    {
        pulseTicker.detach();
    }
}

void rgbMsg(String &payload) {}

void streamBegin()
{
    heartBegin();
    heartTicker.attach_ms(FrameRate, heartTick);
}

void streamClose()
{
    stream.flush();
    stream.close();
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

void streamOpen()
{
    stream.open();
}

String toHex(byte h)
{
    String s = String(h, 16);
    if (h < 16)
    {
        s = '0' + s;
    }
    return s;
}

void updateMsg(String &paylaod)
{
    String url = "http://tj.dragonroll.cn:5500/arduino/Pulser/Pulser.ino.generic.bin";
    if (paylaod.length() > 2)
    {
        url = paylaod.substring(2);
    }
    MQTT.publish(MQTTPub[0], "Starting update from " + url);

    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    ESPhttpUpdate.onStart([] { MQTT.publish(MQTTPub[0], "[httpUpdate] Started"); });
    ESPhttpUpdate.onError([](int err) { MQTT.publish(MQTTPub[0], String("[httpUpdate] Error: ") + ESPhttpUpdate.getLastErrorString().c_str()); });
    ESPhttpUpdate.update(url);
}

void WiFiAdd(String SSID, String PASS)
{
    WiFiList.push_back(WiFiEntry{SSID, PASS});
}

void WiFiConfigNew()
{
    StaticJsonDocument<128> doc;
    doc["len"] = 0;
    File WiFiConfig = LittleFS.open("/WiFi.json", "w");
    serializeJson(doc, WiFiConfig);
    WiFiConfig.close();
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
        WiFiConfigNew();
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
        indicatorSet(indicatorNetwork, 'r');
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
                    indicatorToggle(indicatorNetwork, 'r');
                    if (WiFi.status() == WL_CONNECTED)
                    {
                        indicatorClear(indicatorNetwork);
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
    indicatorSet(indicatorNetwork, 'b');
    WM.setConfigPortalTimeout(180);
    WM.startConfigPortal((Name + "s_Pulser").c_str());
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFiConfigWrite(WM.getWiFiSSID(), WM.getWiFiPass());
        WiFiAdd(WM.getWiFiSSID(), WM.getWiFiPass());
        indicatorClear(indicatorNetwork);
        return 1;
    }
    else
    {
        indicatorClear(indicatorNetwork);
        ESP.deepSleepInstant(INT32_MAX);
    }
}

void setup()
{
    PreDefines();

    batteryInitialize();
    WiFiInitialize();
    MQTTInitialize();

    attachs();
}

void loop()
{
    MQTTLoop(); //will keep connect wifi and mqtt in this function

    streamLoop();

    menuLoop();

    delay(Sleep);
}
