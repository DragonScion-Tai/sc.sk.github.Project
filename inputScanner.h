#ifndef __INPUT_SCANNER_H__
#define __INPUT_SCANNER_H__

class InputScanner
{
public:
    InputScanner(PinName pin);
    virtual ~InputScanner();

    void setHandler(Callback<void()> rise, Callback<void()> fall);
    void check();

    //debug
    int read();

private:
    DigitalIn btn;
    bool bOn;
    char checkNum;

    Callback<void()> pRisePtr, pFallPtr;
    
};

#endif //__INPUT_SCANNER_H__
