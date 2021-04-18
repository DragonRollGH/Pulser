#include <base64.hpp>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <MQTT.h>
#include <NeoPixelBus.h>
#include <OneButton.h>
#include <WiFiManager.h>
#include <Wire.h>

// const int MPU = 0x68;  //MPU-6050的I2C地址
// const int nValCnt = 4; //一次读取寄存器的数量

const byte PixelLen = 20;
const byte FrameRate = 17; // =1000ms/60fps
const byte PinTouch = 12;
const char *AP_SSID = "Rolls_Pulser";
const char *Name = "Roll_v1.0.04172156";

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
byte sleepIdle = 50; // > 300 is useless
unsigned int flowCache = 1;

byte sleep = sleepIdle;
unsigned int flowFrame;
unsigned long flowStart;

unsigned int test = 0;
unsigned long start;
unsigned long end;

WiFiClient WLAN;
MQTTClient MQTT(512);

WiFiManager WM;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> heart(PixelLen);

OneButton button(PinTouch, false, false);

// class MenuSystem
// {
// public:
//     byte tapType[5];
//     void (*callback)();
//     MenuSystem()
//     {}
//     MenuSystem(byte tT[], void (*cb)())
//     {
//         for (byte i = 0; i < 5; ++i)
//         {
//             tapType[i] = tT[i];
//         }
//         callback = cb;
//     }
// };
// MenuSystem menu[6];
// byte menuCurrent = 0;


// void menuHome()
// {
//     Serial.println("Home");
// }
// void menuMain()
// {
//     Serial.println("Main");
// }
// void menuAck()
// {
//     Serial.println("Ack");
// }
// void menuBattery()
// {
//     Serial.println("Battery");
// }
// void menuPulse()
// {
//     Serial.println("Pulse");
// }
// void menuSubmenu()
// {
//     Serial.println("Submenu");
// }
ICACHE_RAM_ATTR void checkTicks()
{
    button.tick();
}
// void pressStop()
// {
//     Serial.println("pressStop");
// }
// void singleClick()
// {
//     Serial.println("singleClick");
// }
// void doubleClick()
// {
//     Serial.println("doubleClick");
// }
// void multiClick()
// {
//     Serial.println("multiClick");
// }
// void pressStart()
// {
//     Serial.println("pressStart");
// }
// void pressStop()
// {
//     menuCurrent = menu[menuCurrent].tapType[0];
//     menu[menuCurrent].callback();
// }
// void singleClick()
// {
//     menuCurrent = menu[menuCurrent].tapType[1];
//     menu[menuCurrent].callback();
// }
// void doubleClick()
// {
//     menuCurrent = menu[menuCurrent].tapType[2];
//     menu[menuCurrent].callback();
// }
// void multiClick()
// {
//     menuCurrent = menu[menuCurrent].tapType[3];
//     menu[menuCurrent].callback();
// }
// void pressStart()
// {
//     menuCurrent = menu[menuCurrent].tapType[4];
//     menu[menuCurrent].callback();
// }


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

    void write(String w)
    {
        for(int i = 0; i < w.length(); ++i)
        {
            write(w[i]);
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
ByteStream stream;

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
        case 'B':
            cmdBattery();
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

void cmdBattery(void)
{
    unsigned int adcs = 0;
    for (byte i = 0; i < 10; i++)
    {
        adcs += analogRead(A0);
        delay(10);
    }
    float voltage = (adcs / 10) * 247.0f / 1024 / 47 - 0.19;
    MQTT.publish(MQTTPub, String(voltage));
}

void cmdDefault(String &payload) {}

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
    while (stream.avalible && running)
    {
        switch (stream.read())
        {
        case '&':
            switch (stream.read())
            {
            case 'D':
                useDefault();
                break;
            case 'H':
                H = toByte(stream.read(), stream.read());
                break;
            case 'S':
                S = toByte(stream.read(), stream.read());
                break;
            case 'L':
                L = toByte(stream.read(), stream.read());
                break;
            case 'A':
                A = toByte(stream.read(), stream.read());
                break;
            case 'B':
                B = toByte(stream.read(), stream.read());
                break;
            case 'C':
                setHSL(stream.read(), stream.read(), stream.read(), stream.read());
                break;
            case 'N':
                runPixels(stream.read(), stream.read(), stream.read(), stream.read());
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
            // heart.SetPixelColor(i, HslColor(0, pixels[i].S, pixels[i].L));
            heart.SetPixelColor(i, RgbColor(10, 0, 0));
            anyActive = 1;
        }
        else
        {
            heart.SetPixelColor(i, RgbColor(0, 0, 0));
        }
        pixels[i].update();
    }
    heart.Show();
    if (!anyActive)
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
    // heart.Begin();
}

void stopFlow(void)
{
    flowStart = 0;
    sleep = sleepIdle;
    // pinMode(3, INPUT);
}

// //向MPU6050写入一个字节的数据
// //指定寄存器地址与一个字节的值
// void WriteMPUReg(int nReg, unsigned char nVal)
// {
//     Wire.beginTransmission(MPU);
//     Wire.write(nReg);
//     Wire.write(nVal);
//     Wire.endTransmission(true);
// }

// //从MPU6050读出加速度计三个分量、温度和三个角速度计
// //保存在指定的数组中
// void ReadAccGyr(int *pVals)
// {
//     Wire.beginTransmission(MPU);
//     Wire.write(0x3B);
//     Wire.requestFrom(MPU, nValCnt * 2, true);
//     Wire.endTransmission(true);
//     for (long i = 0; i < nValCnt; ++i)
//     {
//         pVals[i] = Wire.read() << 8 | Wire.read();
//     }
// }

// //对读数进行纠正，消除偏移，并转换为物理量。公式见文档。
// void Rectify(int *pReadout, float *pRealVals)
// {
//     for (int i = 0; i < 3; ++i)
//     {
//         pRealVals[i] = pReadout[i] / 16384.0f;
//     }
//     pRealVals[3] = pReadout[3] / 340.0f + 36.53;
// }

// void MPUloop(void)
// {
//     int readouts[nValCnt];
//     ReadAccGyr(readouts); //读出测量值

//     float realVals[4];
//     Rectify(readouts, realVals); //根据校准的偏移量进行纠正

//     Serial.print("aX: ");
//     Serial.print(String(realVals[0]));
//     Serial.print("| aY: ");
//     Serial.print(String(realVals[1]));
//     Serial.print("| aZ: ");
//     Serial.print(String(realVals[2]));
//     Serial.print("| Tp: ");
//     Serial.print(String(realVals[3]));
//     Serial.println();
//     delay(10);
// }

void setup()
{
    // Serial.begin(115200);
    stream.write("&L05&N////;");
    heart.Begin();
    runFlow();
    setPixelsColor();

    WiFi.mode(WIFI_STA);
    WiFi.setSleepMode(WIFI_LIGHT_SLEEP);

    WM.autoConnect(AP_SSID);

    MQTT.begin(MQTTServer, MQTTPort, WLAN);
    MQTT.onMessage(mqttMsg);

    // Wire.begin();
    // WriteMPUReg(0x6B, 0);
    // MenuSystem m0({ 0, 1, 2, 3, 4 }, menuHome); //0
    // MenuSystem m1({ 0, 5, 2, 3, 4 }, menuMain);     //1
    // MenuSystem m2({ 0, 0, 0, 3, 4 }, menuACK);      //2
    // MenuSystem m3({ 0, 0, 2, 0, 4 }, menuBattery);  //3
    // MenuSystem m4({ 0, 0, 0, 0, 0 }, menuPulse);    //4
    // MenuSystem m5({ 0, 0, 2, 3, 4 }, menuSubmenu);  //5
    // menu = {m0,m1,m2,m3,m4,m5};
    attachInterrupt(digitalPinToInterrupt(PinTouch), checkTicks, CHANGE);
    button.attachClick([]() {stream.write("&NgAAA;");});
    button.attachDoubleClick([]() {stream.write("&NwAAA;");});
    button.attachMultiClick([]() {stream.write("&N4AAA;");});
    button.attachLongPressStart([]() {stream.write("&N8AAA;");});
    button.attachLongPressStop([]() {stream.write("&N+AAA;");});

    stream.write("&N////;");
    // Serial.println("\nESP OK");
}

void loop()
{
    // test = (test + 1) % 1000;
    // if (test == 0)
    // {
    //     start = millis();
    // }

    if (!MQTT.connected())
    {
        mqttConnect();
    }
    MQTT.loop();

    button.tick();

    if (flowStart)
    {
        if (millis() - flowStart >= flowFrame * FrameRate)
        {
            setPixelsColor();
        }
    }
    else
    {
        if (stream.avalible >= flowCache)
        {
            runFlow();
        }
    }

    // MPUloop();

    // if (test == 0)
    // {
    //     end = millis();
    //     MQTT.publish(MQTTPub, String(end) + ": " + String(end - start));
    // }

    delay(sleep);
}
