#include "../include/Hic/HicManager.hpp"
#include "../include/main.hpp"

String commandVal;

HicManager::HicManager(uint8_t leftPin, uint8_t rightPin, uint8_t lightAPin, uint8_t lightBPin)
    : lightSystem(leftPin, rightPin, lightAPin, lightBPin)
{
}

void HicManager::managerConfig()
{
    canManager.canConfig();
}

void HicManager::handleCanMessage()
{
    MessageId messageId = MessageId::MESSAGE_ID_ENUM_LENGTH;
    commandVal = "";

    commandVal = canManager.readMessage(messageId);

    switch (messageId)
    {
    case MessageId::MESSAGE_ID_CONTROL_HEADLIGHTS:
        controlHeadlights();
        break;

    case MessageId::MESSAGE_ID_CONTROL_INDICATORS:
        controlIndicators();
        break;

    case MessageId::MESSAGE_ID_CONTROL_HEADLIGHTS_REMOTE:
        controlHeadlights();
        break;

    case MessageId::MESSAGE_ID_CONTROL_INDICATORS_REMOTE:
        controlIndicators();
        break;

    default:
        break;
    }
}

void HicManager::processIndicators()
{
    if (indicatorsState == MessageValue::MESSAGE_INDICATORS_TURN_ON_LEFT)
        lightSystem.blinkIndicator(LEFT_INDICATOR_PIN, INDICATORS_DELAY);
    else if (indicatorsState == MessageValue::MESSAGE_INDICATORS_TURN_ON_RIGHT)
        lightSystem.blinkIndicator(RIGHT_INDICATOR_PIN, INDICATORS_DELAY);
    else if (indicatorsState == MessageValue::MESSAGE_INDICATORS_TURN_OFF_LEFT || indicatorsState == MessageValue::MESSAGE_INDICATORS_TURN_OFF_RIGHT)
        lightSystem.turnOffIndicators();
}

void HicManager::controlHeadlights()
{
    int val_int = atoi(commandVal.c_str());
    if (((MessageValue)val_int) == MessageValue::MESSAGE_HEADLIGHTS_TURN_ON)
        lightSystem.turnOnHeadlights();
    else if (((MessageValue)val_int) == MessageValue::MESSAGE_HEADLIGHTS_TURN_OFF)
        lightSystem.turnOffHeadlights();

    Serial.println("control  headlights");
}

void HicManager::controlIndicators()
{
    indicatorsState = (MessageValue)atoi(commandVal.c_str());
}