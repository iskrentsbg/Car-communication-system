#include "main.hpp"
#ifndef IINDICATOR_HPP
#define IINDICATOR_HPP

class IIndicator {
public:
    virtual void blinkIndicator(uint8_t pin_nr, uint32_t delay) = 0;
    virtual void turnOffIndicators() = 0;
};

#endif