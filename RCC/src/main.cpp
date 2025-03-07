#include <Arduino.h>
#include "communicationManager.hpp"
#include "../include/remoteController/MyRemoteMQTT.hpp"
#include "../include/canCommunication/Headers/canManager.hpp"
#include "../include/canCommunication/Enums/MessageId.hpp"
#include "../include/canCommunication/Enums/MessageValue.hpp"
#include "../include/canCommunication/Headers/canManager.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

WiFiClient espClient;
MyRemoteMQTT myRemoteMQTT(espClient, MY_MQTT_HOST_SERVER, MY_NETWORK_PORT, MY_ID, callback);

using namespace canCommunication;
canCommunication::canManager canmanager;

uint32_t startTime;
uint32_t currentTime;

String sensorValue;

void canTask(void *pvParameters)
{
  for (;;)
  {
    handleCanMessage();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void mqttTask(void *pvParameters)
{
  for (;;)
  {
    myRemoteMQTT.myNetLoop();
    vTaskDelay(pdMS_TO_TICKS(110));
  }
}

void setup()
{
  Serial.begin(9600);
  sensorValue = EMPTY_STRING;

  startTime = millis();
  WiFi.begin(MY_NETWORK_SSID, MY_NETWORK_PASSWORD);

  Serial.println("Connected to WiFi");

  while (WiFi.status() != WL_CONNECTED && (millis() - startTime >= WIFI_CONNECT_TIMEOUT))
  {
    startTime = millis();
    // NOTE - Wait to connect or timeout.
  }

  Serial.println("Connected to WiFi!");
  Serial.println("subscribing.");

  myRemoteMQTT.subscribeToMyNetwork(LIGHTING_TOPIC);
  myRemoteMQTT.subscribeToMyNetwork(ENGINE_TOPIC);
  myRemoteMQTT.subscribeToMyNetwork(SENSOR_TOPIC);

  canmanager.canConfig();

  xTaskCreate(canTask, "CAN Task", 4096, NULL, 1, NULL);
  xTaskCreate(mqttTask, "MQTT Task", 4096, NULL, 2, NULL);
  vTaskStartScheduler();
}

void loop()
{
  currentTime = millis();
  if (currentTime - startTime >= COM_DELAY)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("Connection lost, restarting device!");
      ESP.restart();
    }

    if (!myRemoteMQTT.amIconnected())
    {
      Serial.println("re-subscribing.");
      myRemoteMQTT.connectToMyNetwork();
      myRemoteMQTT.subscribeToMyNetwork(LIGHTING_TOPIC);
      myRemoteMQTT.subscribeToMyNetwork(ENGINE_TOPIC);
      myRemoteMQTT.subscribeToMyNetwork(SENSOR_TOPIC);
    }
    startTime = currentTime;
  }
}

/**
 * @brief
 *
 * @param topic
 * @param payload
 * @param length
 */
void callback(char *topic, byte *payload, unsigned int length)
{
  String message = EMPTY_STRING;

  /*NOTE - This function mostly only for receive message from remote.
           The publish message should be added in functions which operates
           response from the CAN messages.*/

  for (int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  if (strcmp(topic, LIGHTING_TOPIC) == 0)
  {
    sortCommandToLighting(message);
  }

  if (strcmp(topic, ENGINE_TOPIC) == 0)
  {
    sortCommandToEngine(message);
  }
}

/**
 * @brief
 *
 * @param input
 */
void sortCommandToEngine(String input)
{
  if (input == R_SPEED_UP)
  {
    speedUpTheCar();
  }
  else if (input == R_SPEED_DOWN)
  {
    speedDownTheCar();
  }
  else if (input == R_BRAKE)
  {
    stopTheCar();
  }
}

/**
 * @brief
 *
 * @param input
 */
void sortCommandToLighting(String input)
{
  if (input == R_HEADLIGHT_ON)
  {
    turnOnHeadlights();
  }
  else if (input == R_HEADLIGHT_OFF)
  {
    turnOffHeadlights();
  }
  else if (input == R_TURN_LEFT)
  {
    turnLeft();
  }
  else if (input == R_TURN_RIGHT)
  {
    turnRight();
  }
  else if (input == R_GO_STRAIGHT)
  {
    goStraight();
  }
}

/*NOTE - These functions below will be used for the CAN communication.*/

/**
 * @brief
 *
 */
void speedUpTheCar()
{
  Serial.println("<CAN msg to speed up>");
  canmanager.sendMessage(MessageId::MESSAGE_ID_SPEED_UP_REMOTE, MessageValue::MESSAGE_VALUE_ON);
}

/**
 * @brief
 *
 */
void speedDownTheCar()
{
  Serial.println("<CAN msg to speed up>");
  canmanager.sendMessage(MessageId::MESSAGE_ID_SPEED_DOWN_REMOTE, MessageValue::MESSAGE_VALUE_ON);
}

/**
 * @brief
 *
 */
void stopTheCar()
{
  Serial.println("<CAN msg to brake>");
  canmanager.sendMessage(MessageId::MESSAGE_ID_STOP_REMOTE, MessageValue::MESSAGE_VALUE_ON);
}

/**
 * @brief
 *
 */
void turnOnHeadlights()
{
  Serial.println("<CAN msg to turn on headlights>");
  canmanager.sendMessage(MessageId::MESSAGE_ID_CONTROL_HEADLIGHTS_REMOTE, MessageValue::MESSAGE_HEADLIGHTS_TURN_ON);
}

/**
 * @brief
 *
 */
void turnOffHeadlights()
{
  Serial.println("<CAN msg to turn off headlights>");
  canmanager.sendMessage(MessageId::MESSAGE_ID_CONTROL_HEADLIGHTS_REMOTE, MessageValue::MESSAGE_HEADLIGHTS_TURN_OFF);
}

/**
 * @brief
 *
 */
void turnLeft()
{
  Serial.println("<CAN msg to turn left>");
  canmanager.sendMessage(MessageId::MESSAGE_ID_CONTROL_INDICATORS_REMOTE, MessageValue::MESSAGE_INDICATORS_TURN_ON_LEFT);
}

/**
 * @brief
 *
 */
void turnRight()
{
  Serial.println("<CAN msg to turn right>");
  canmanager.sendMessage(MessageId::MESSAGE_ID_CONTROL_INDICATORS_REMOTE, MessageValue::MESSAGE_INDICATORS_TURN_ON_RIGHT);
}

/**
 * @brief
 *
 */
void goStraight()
{
  Serial.println("<CAN msg to go straight>");
  canmanager.sendMessage(MessageId::MESSAGE_ID_CONTROL_INDICATORS_REMOTE, MessageValue::MESSAGE_INDICATORS_TURN_OFF_LEFT);
  canmanager.sendMessage(MessageId::MESSAGE_ID_CONTROL_INDICATORS_REMOTE, MessageValue::MESSAGE_INDICATORS_TURN_OFF_RIGHT);
}

/**
 * @brief
 *
 */
void handleCanMessage()
{
  MessageId message_id;

  // sensor_state = canManager.readStaticMessage(message_id);
  sensorValue = canmanager.readMessage(&message_id);

  switch (message_id)
  {
  case MessageId::MESSAGE_ID_ENGINE_TEMPERATURE_TOO_HIGH:
    showTemperatureCaution();
    break;

  case MessageId::MESSAGE_ID_TIRE_PRESSURE_TOO_LOW:
    showPressureCaution();
    break;

  case MessageId::MESSAGE_ID_SHOW_TEMPERATURE:
    showTemperatureVal(sensorValue);
    break;

  case MessageId::MESSAGE_ID_SHOW_PRESSURE:
    showPressureVal(sensorValue);
    break;

  default:
    break;
  }
}

/**
 * @brief
 *
 * @param value
 */
void showTemperatureVal(String value)
{
  String msg_format = TEMPERATURE_FORMAT(value);
  const char *message_to_publish = msg_format.c_str();
  myRemoteMQTT.publishToMyNetwork(SENSOR_TOPIC, message_to_publish);
}

/**
 * @brief
 *
 * @param value
 */
void showPressureVal(String value)
{
  String msg_format = PRESSURE_FORMAT(value);
  const char *message_to_publish = msg_format.c_str();
  myRemoteMQTT.publishToMyNetwork(SENSOR_TOPIC, message_to_publish);
}

/**
 * @brief
 *
 */
void showTemperatureCaution()
{
  String msg_format = OVERHEAT;
  const char *message_to_publish = msg_format.c_str();
  myRemoteMQTT.publishToMyNetwork(SENSOR_TOPIC, message_to_publish);
}

/**
 * @brief
 *
 */
void showPressureCaution()
{
  String msg_format = FLAT_TIRE;
  const char *message_to_publish = msg_format.c_str();
  myRemoteMQTT.publishToMyNetwork(SENSOR_TOPIC, message_to_publish);
}