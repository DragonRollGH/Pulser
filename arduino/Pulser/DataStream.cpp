#include <Arduino.h>

class DataStream
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
        for (int i = 0; i < w.length(); ++i)
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