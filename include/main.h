#include "Train.h"
#include "Arrow.h"
#include "Traffic.h"

#include "WiFiManager.h"
#include <esp_system.h>
#include <rom/ets_sys.h>
// !!! ДЛЯ РАБОТЫ В MAIN !!! //

// Флаг прерывания от часов
volatile bool clock_flag = false;
// Флаг ошибки по времени
volatile bool error_flag = false;
// Переменная состояния конечного автомата работы депо
unsigned char state = 0;
// Номер первой считанной метки
unsigned char first_read_uid;
// Временная переменная метки
unsigned char t_uid = 0;

// !!! ДЛЯ РАБОТЫ В MAIN !!! //

// !!! ТАЙМЕРЫ !!! //
// Таймер рассылки
unsigned int timer_send = 0;
// Таймер для исключения двойного сигнала от часов
unsigned int timer_to_clock = 0;
// Таймер общего назначения. Используется в разных местах основного цикла работы
unsigned int timer_loop = 0;
// Таймер времени работы депо
unsigned int timer_work = 0;
// Время для проезда поезда. Если превышено - взводится флаг ошибки
unsigned int timer_error = 0;
// Таймер отправки команд
unsigned int send_timer = 0;
// Таймер для DCC
hw_timer_t *DCC_timer = NULL;

// !!! ТАЙМЕРЫ !!! //

WiFiManager wifi_m;
bool driver_work = false;

// !!! ИЗ DCC !!! //

// Объект для работы со стрелочными переводами
Arrow arrows;
// Объект для работы со светофорами
Traffic traffics;
// Объект для работы с поездами
std::vector<Train> trains;
// Полупериод одного бита
volatile unsigned int passInterval = ONE;

// Передаваемая команда
volatile unsigned char command[MAX_SEND_CMD_LENGTH];

// Флаг готовности команды к передаче
volatile bool command_ready = false;
// Длина передаваемой команды
volatile unsigned char lengthCMD;
// Указатель передачи одного бита команды
volatile unsigned char pass = 0x00;
// Количество переданных байт команды
volatile unsigned char iteratorCMD = 0x00;
// Указатель на передаваемый бит
volatile unsigned char iteratorBitCMD = 0x00;
// Колличество повторов команды
volatile unsigned short repeatSend = 0x00;
// Прембула между командами
volatile unsigned short preamble = LENGTH_PREAMBLE;
// Переключение между устройствами для формирования команд
volatile unsigned char switchDevice = 0;

// Итератор стрелочных переводов
volatile unsigned char arrowIterator = 0;
// Итератор светофоров
volatile unsigned char trafficIterator = 0;
// Итератор для светодиодов светофора
volatile unsigned char ledsTrafficIterator = 0;