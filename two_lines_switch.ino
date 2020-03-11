#define DEBUG
// включаем или отключаем режим отладки

#include <ESP8266WiFi.h>
#include "variables.h"
#include "wemos.h"
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ESP8266httpUpdate.h>
#include "FS.h"
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <TaskScheduler.h>

AsyncWebServer server(80);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Scheduler taskScheduler;

// Обработчики задач
void mainTaskCallback();
void buttonCallback();
void channelCallback();
void mqttCallback();

// Задачи
Task mainTask(5, TASK_FOREVER, &mainTaskCallback);
Task buttonTask(100, TASK_FOREVER, &buttonCallback);
Task channelTask(5, TASK_FOREVER, &channelCallback);
Task mqttTask(5, TASK_FOREVER, &mqttCallback);

void mainTaskCallback()
{
  // Если установлен флаг сброса настроек
  if(configToClear==1){
    // сбрасываем настройки и перезагружаемся
    clearConfig();
    delay(1000);
    ESP.reset();
  }
  // Обработка MQTT-сообщений
  if (APMode==0) mqtt_handler();
}

void buttonCallback()
{
  int ed1 = digitalRead(D1);
  
  if (ed1 == 1) reset_button_counter = 0;
  if (ed1 == 0) reset_button_counter++;

  if (reset_button_counter > 20) configToClear = 1;

  int ed5 = digitalRead(D5);
  
  if (ed5 == 1) buttonA_counter = 0;
  if (ed5 == 0) buttonA_counter++;

  if (buttonA_counter > 3) {
    Debugln("Button A Pressed");
    buttonA_counter = 0;
    if (channelA_state == 0) channelA_state = 1; else channelA_state = 0;
  }

  int ed6 = digitalRead(D6);
  
  if (ed6 == 1) buttonB_counter = 0;
  if (ed6 == 0) buttonB_counter++;

  if (buttonB_counter > 3) {
    Debugln("Button B Pressed");
    buttonB_counter = 0;
    if (channelB_state == 0) channelB_state = 1; else channelB_state = 0;
  }
}

void channelCallback()
{
  if ((channelA_state == 1) && (digitalRead(D7) == LOW)) {
    digitalWrite(D7, HIGH);
    Debugln("Enabling channel A");
    topic_to_send = "/statusA";
    msg_to_send = "1";
    toPub = 1;
  }
  if ((channelA_state == 0) && (digitalRead(D7) == HIGH)) {
    digitalWrite(D7, LOW);
    Debugln("Disabling channel A");
    topic_to_send = "/statusA";
    msg_to_send = "0";
    toPub = 1;
  }
  if ((channelB_state == 1) && (digitalRead(D8) == LOW)) {
    digitalWrite(D8, HIGH);
    Debugln("Enabling channel B");
    topic_to_send = "/statusB";
    msg_to_send = "1";
    toPub = 1;
  }
  if ((channelB_state == 0) && (digitalRead(D8) == HIGH)) {
    digitalWrite(D8, LOW);
    Debugln("Disabling channel B");
    topic_to_send = "/statusB";
    msg_to_send = "0";
    toPub = 1;
  }
}

void mqttCallback()
{
  if (toPub == 1) {
    if (pubMQTT((char*)topic_to_send.c_str(), (char*)msg_to_send.c_str())) toPub = 0;  
  }
}

void setup() {
  #ifdef DEBUG
    Serial.begin(9600, SERIAL_8N1);
  #endif
  Debugln();
  // запускаем диспетчер задач
  taskScheduler.init();
  // добавляем задачи в очередь
  taskScheduler.addTask(mainTask);
  taskScheduler.addTask(buttonTask);
  taskScheduler.addTask(channelTask);
  taskScheduler.addTask(mqttTask);
  if (!SPIFFS.begin()) {
    Debugln("[setup()] FAILED TO MOUNT FILE SYSTEM!");
  }
  // загружаем настройки
  if (!loadConfig()) { Debugln("[setup()] loadConfig() FAILED!"); }
  initWiFi();
  // если соединение прошло успешно проверяем обновление контроллера (OTA)
  if (APMode==0) {
    checkOTAUpdate();
    connectMQTT();
    mqttTask.enable();
  }
  // главная задача
  mainTask.enable();
  // подготавливаем входы МК для кнопки сброса и запускаем задачу
  pinMode(D1, INPUT_PULLUP);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  buttonTask.enable();
  channelTask.enable();
}

void loop() {
  taskScheduler.execute();
}
