#ifdef DEBUG
  #define Debug(x)    Serial.print(x)
  #define Debugln(x)  Serial.println(x)
  #define Debugf(...) Serial.printf(__VA_ARGS__)
  #define Debugflush  Serial.flush
#else
  #define Debug(x)    {}
  #define Debugln(x)  {}
  #define Debugf(...) {}
  #define Debugflush  {}
#endif

#define OTAUPDATECLASS "TWO_LINES_SWITCH" // OTA: Имя класса устройства
#define OTAUPDATEVERSION "0.1.1" // OTA: Версия прошивки устройства
#define OTAUPDATESERVER "ota.i-alice.ru" // OTA: Сервер обновления
#define OTAUPDATEPATH "/update/index.php" // OTA: Путь обновления на сервере

// Структура настроек контроллера
typedef struct
{
  String ename = "";
  String esid = "";
  String epass = "";
  String pubTopic = "";
  String mqttServer = "";
} options;

// Переменные для чтения настроек из SPIFFS
options Opt;
// SSID в режиме точки доступа
const char *ssid = "twoLinesSwitch";

//##### Глобальные переменные ##### 
int reset_button_counter = 0;
int buttonA_counter = 0;
int buttonB_counter = 0;
int channelA_state = 0;
int channelB_state = 0;
String topic_to_send = "";
String msg_to_send = "";
char buf[40]; // буфер для данных MQTT

//##### Флаги ##### нужны, чтобы не выполнять длительные операции вне цикла loop
int configToClear=0; // флаг стирания конфигурации
int APMode=0;        // режим работы (клиент/точка доступа)
int toPub=0; // флаг публикации MQTT
