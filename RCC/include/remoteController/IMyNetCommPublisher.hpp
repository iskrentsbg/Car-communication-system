#pragma once

#include <Arduino.h>
#include "remote_system.hpp"

class IMyNetCommPublisher
{
public:
    virtual bool publishToMyNetwork(const char *, const char *) = 0;
};