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
const char *Version = "v2.0.05061200";
//const String Pulse1 = "&C&A00&b80&L00&N////;;;;;;;;;;;;;;;;;;;;&b20&L05&N////;;;;;;;;;;;;;;;;;;;;&B14&L19&N////;;;;;;;;;;;;;;;;;;;;&b80&L00&N////;;;;;;;;;;;;;;;;;;;;&b20&L05&N////;;;;;;;;;;;;;;;;;;;;&B14&L19&N////;;;;;;;;;;;;;;;;;;;;&b80&L00&N////;;;;;;;;;;;;;;;;;;;;&b20&L05&N////;;;;;;;;;;;;;;;;;;;;&B14&L19&N////;;;;;;;;;;;;;;;;;;;;&b80&L00&N////;;;;;;;;;;;;;;;;;;;;&b20&L05&N////;;;;;;;;;;;;;;;;;;;;&B14&L19&N////;;;;;;;;;;;;;;;;;;;;&C;";

//const String Pulse2 = "&C&A00&b40&L00&N////;;;;;;;;;;&b18&L05&N////;;;;;;;;;;;;;;;&B0f&L19&N////;;;;;;;;;;;;;;;&b40&L00&N////;;;;;;;;;;&b18&L05&N////;;;;;;;;;;;;;;;&B0f&L19&N////;;;;;;;;;;;;;;;&b40&L00&N////;;;;;;;;;;&b18&L05&N////;;;;;;;;;;;;;;;&B0f&L19&N////;;;;;;;;;;;;;;;&b40&L00&N////;;;;;;;;;;&b18&L05&N////;;;;;;;;;;;;;;;&B0f&L19&N////;;;;;;;;;;;;;;;&C;";

//const String Pulse3 = "&C&A00&b80&L00&N////;;;;;;;;&b08&L02&N////;;;;;;&b80&L1a&N////;;;;;;;;&B54&L1c&N////;;;;;;&B0b&L1a&N////;;;;;;;;;;&B02&L01&N////;;&b80&L00&N////;;;;;;;;&b08&L02&N////;;;;;;&b80&L1a&N////;;;;;;;;&B54&L1c&N////;;;;;;&B0b&L1a&N////;;;;;;;;;;&B02&L01&N////;;&b80&L00&N////;;;;;;;;&b08&L02&N////;;;;;;&b80&L1a&N////;;;;;;;;&B54&L1c&N////;;;;;;&B0b&L1a&N////;;;;;;;;;;&B02&L01&N////;;&b80&L00&N////;;;;;;;;&b08&L02&N////;;;;;;&b80&L1a&N////;;;;;;;;&B54&L1c&N////;;;;;;&B0b&L1a&N////;;;;;;;;;;&B02&L01&N////;;&C;";

// const String Pulse = "&bff&L00&N////;;;;;;;;;;;;;;;;&b0e&L02&N////;;;;;;;;&b18&L15&N////;;;;&B0c&L18&N////;;;;;;;;;;;;";
const String Pulse = "&b0d&L00&N////;;;;;;;;&b20&L14&N////;;;;&B16&L18&N////;;;;;;;;;;;;;;;;;;;;&B08&L02&N////;;;;;;;;";

//const String Pulse5 = "&C&A00&b60&L00&N////;;;;;;;;;;;;&b0e&L02&N////;;;;;;;;&b18&L15&N////;;;;&B0d&L18&N////;;;;;;;;;;;;&B04&L02&N////;;;;&b60&L00&N////;;;;;;;;;;;;&b0e&L02&N////;;;;;;;;&b18&L15&N////;;;;&B0d&L18&N////;;;;;;;;;;;;&B04&L02&N////;;;;&b60&L00&N////;;;;;;;;;;;;&b0e&L02&N////;;;;;;;;&b18&L15&N////;;;;&B0d&L18&N////;;;;;;;;;;;;&B04&L02&N////;;;;&C;";

String Name;
String MQTTUsername;
String MQTTPassword;
String MQTTClientid;
String MQTTPub[2];
String MQTTSub[2];
float BatteryOffset;

//define in FS
// byte H = 0;
// byte S = 255;
// byte L = 20;
// byte A = 5;
// byte B = 35;

bool heartBeginFlag = 0;

byte indicatorLightness = 20;
byte indicatorPin = 10;
bool indicatorToggleFlag = 0;

bool menuBeginFlag = 0;

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
Ticker pulseTicker;
byte pulseTickerTimeout = 10;

DataStream stream;
Pixel pixels[PixelLen];
PixelColor colors[2] = {{0, 255, 20, 5, 35}, {120, 255, 45, 0, 45}};
bool colorsIdx = 0;

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
        BatteryOffset = -0.03;
    }
    else if (ID == 10409937)
    {
        MQTTPub[0] = "PB/U/R";
        MQTTPub[1] = "PB/D/M";
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
        MQTTPub[0] = "";
        MQTTPub[1] = "";
        MQTTSub[0] = "";
        MQTTSub[1] = "";
        BatteryOffset = 0;
    }
}

void attachs()
{
    streamOpen();
    attachInterrupt(digitalPinToInterrupt(PinTouch), buttonTickIrq, CHANGE);
    button.attachClick([]() { stream.write("&C1&A00;"+Pulse+"&C0;"); });
    button.attachDoubleClick([]() { menuBeginFlag = 1; });
    // button.attachMultiClick([]() { stream.write(Pulse3); });
    button.attachLongPressStart(menuPulseStart);
    button.attachLongPressStop(menuPulseStop);
}

void detachs()
{
    streamEnd();
    streamClose();
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
        heartClear(i);
        heart.Show();
        delay(1000 / battery);
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
    float voltage = (adcs / 10) * 247.0f / 1024 / 47 + BatteryOffset;
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
    heart.SetPixelColor(0, HslColor(battery / 300.0f, 1, indicatorLightness / 510.0f));
    heart.Show();
    if (battery == 0)
    {
        delay(5000);
        heartClear();
        ESP.deepSleepInstant(INT32_MAX);
    }
}

void buttonTickTmr()
{
    button.tick();
}

void cmdACK()
{
    MQTT.publish(MQTTPub[0], Name + '_' + Version);
}

void cmdBattery()
{
    MQTT.publish(MQTTPub[0], String(batteryGet()) + '%');
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
    MQTT.publish(MQTTPub[0], String(ID));
}

void cmdPulse(String &payload)
{
    switch (payload[2])
    {
    case 'A':
        menuPulseBegin();
        MQTT.publish(MQTTPub[1], ":Pa");
        break;
    case 'a':
        /* code */
        break;
    case 'B':
        menuPulseEnd();
        break;
    default:
        break;
    }
}

void cmdRGB(String &payload) {}

void cmdUpdate(String &paylaod)
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
        colorsIdx = Idx - '0';
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
            pixels[i].run(colors[colorsIdx]);
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
                colors[colorsIdx].H = parseHex(stream.read(), stream.read());
                break;
            case 'S':
                colors[colorsIdx].S = parseHex(stream.read(), stream.read());
                break;
            case 'L':
                colors[colorsIdx].L = parseHex(stream.read(), stream.read());
                break;
            case 'A':
                colors[colorsIdx].A = parseHex(stream.read(), stream.read());
                break;
            case 'B':
                colors[colorsIdx].B = parseHex(stream.read(), stream.read());
                break;
            case 'b':
                colors[colorsIdx].B = -1 * parseHex(stream.read(), stream.read());
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

void menuLoop()
{
    if (menuBeginFlag)
    {
        menuBeginFlag = 0;
        battryAnimation();
    }
}

void menuPulseBegin()
{
    colors[1].H = colors[0].H;
    stream.write("&C1&A00;");
    pulseTickerTimeout = 10;
    pulseTicker.attach_ms(675, menuPulseTick);
}

void menuPulseEnd()
{
    pulseTicker.detach();
    stream.write("&C0;");
}

void menuPulseStart()
{
    MQTT.publish(MQTTPub[1], ":PA");
    menuPulseBegin();
}

void menuPulseStop()
{
    menuPulseEnd();
    MQTT.publish(MQTTPub[1], ":PB");
}

void menuPulseTick()
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

void MQTTConnect()
{
    // heartClear(); cause to turn off battert indicator.
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
