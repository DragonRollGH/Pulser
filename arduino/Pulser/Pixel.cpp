#include <Arduino.h>

struct PixelColor
{
    byte H;
    byte S;
    byte L;
    byte A;
    byte B;
};

class Pixel
{
public:
    bool active = false;
    float H;
    float S;
    float L;
    float deltaL;
    byte A;

    void run(PixelColor &color)
    {
        active = true;
        H = color.H / 255.0f;
        S = color.S / 255.0f;
        L = color.L / 255.0f;
        A = color.A;
        deltaL = L / color.B;
    }
    // void run(byte rH, byte rS, byte rL, byte rA, byte rB)
    // {
    //     active = true;
    //     H = rH / 255.0f;
    //     S = rS / 255.0f;
    //     L = rL / 255.0f;
    //     A = rA;
    //     deltaL = L / rB;
    // }

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
                if (-0.0001 < L && L < 0.0001)
                {
                    L = 0;
                }
                if (L < 0)
                {
                    active = false;
                }
            }
        }
    }
};