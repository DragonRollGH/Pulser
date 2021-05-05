#include <Arduino.h>

struct PixelColor
{
    byte H;
    byte S;
    byte L;
    byte A;
    int B;
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
        if (color.B > 0)
        {
            deltaL = L / color.B;
        }
        else{
            deltaL = (0.125 - L) / color.B;
        }
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
                if (-0.0001 < L && L < 0.0001)
                {
                    L = 0;
                }
                else if (L > 0.5)
                {
                    L = 0;
                    deltaL = 1;
                }
                if (L < 0)
                {
                    active = false;
                }
            }
        }
    }
};