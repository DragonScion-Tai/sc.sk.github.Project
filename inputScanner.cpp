#include "utils.h"

#define CHECK_DELAY 10

InputScanner::InputScanner(PinName pin) : btn(pin, PullNone)
{
    bOn = 0==btn.read() ? false : true;
    checkNum = bOn ? CHECK_DELAY : 0;

    pRisePtr = NULL;
    pFallPtr = NULL;
}

InputScanner::~InputScanner()
{
    
}

void InputScanner::setHandler(Callback<void()> rise, Callback<void()> fall)
{
    pRisePtr = rise;
    pFallPtr = fall;
}

void InputScanner::check()
{
    if(!bOn)
    {
        if(btn)
        {
            checkNum ++;
            if(checkNum >= CHECK_DELAY)
            {
                bOn = true;

                if(pRisePtr)
                    pRisePtr();
            }
        }
    }
    else
    {
        if(!btn)
        {
            checkNum --;
            if(checkNum <= 0)
            {
                bOn = false;

                if(pFallPtr)
                    pFallPtr();
            }
        }
    }
}

int InputScanner::read()
{
    return btn.read();
}
