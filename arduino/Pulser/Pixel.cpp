typedef unsigned char byte;

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
                if (L < 0) //<= or < ?
                {
                    active = false;
                }
            }
        }
    }
};