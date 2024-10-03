// https://diytech.ru/projects/spravochnik-po-raspinovke-esp32-kakie-vyvody-gpio-sleduet-ispolzovat

#include <Arduino.h>
#include <vector>

// Максимальная скорость поезда 15
// Контакт часов (инверсный) 34-39 не работают с прерываниями!
#define CLOCK_PIN GPIO_NUM_13// GPIO_NUM_25 // 25
// Физически подключенный контакт - не нужен, но убрать не могу
#define CLOCK_PIN_PHYS GPIO_NUM_13//GPIO_NUM_34 // 34
// Ножка со встроенным светодиодом для сигнализации прерывания
#define CLOCK_SIGNAL_INT GPIO_NUM_2 // 2 
// Максимальное количество устройств поддерживаемое системой
#define MAX_DEVICE 253
// Максимальный размер стека команд (8 - 255)
#define MAX_CMD_COUNT 253
// Максимальная длина передоваемой команды
#define MAX_SEND_CMD_LENGTH 10
// Длина команды светофора
#define TRAFFIC_CMD_LENGTH 5
// Длина команды стрелочного перевода
#define ARROW_CMD_LENGTH 2
// Длина команды поезда
#define TRAIN_CMD_LENGTH 9
// Направление движения вперед
#define FORWARD 1
// Направление движения назад
#define BACKWARD 0
// Включено
#define ON 1
// Выключено
#define OFF 0
// Влево
#define LEFT 1
// Вправо
#define RIGHT 0
// Замеры подсчитаны для 80 МГц
// ESP32 на 240 => При увеличении частоты увеличиваем период
// Требуемые временные интервалы
// Для 1: 58 мкс, Для 1 сигнала: мин: 55, макс: 61, Для 2 сигнала: мин: 52, макс: 64
// Максимальная допустимая разница между сигналами 1: 3 мкс или 6 мкс
// Для 0: 100 мкс, Для 1 сигнала: мин: 95, макс: 9900, Для 2 сигнала: мин: 90, макс: 10000
// Растянутый 0 - 12000 мкс
// Уменьшил единицу из-за транзисторов, при переключении которых время растягивается до 70 мкс
// Единица
#define ONE 207//47
// Ноль
#define ZERO 540//120
// Длина преамбулы
#define LENGTH_PREAMBLE 0b0001000000000000
// Левый рельс
#define RAIL_L GPIO_NUM_32//GPIO_NUM_5// // 32
// Правый рельс
#define RAIL_R GPIO_NUM_33//GPIO_NUM_4// // 33
// Скорость 1 поезда
#define SPEED_1_TRAIN 14
// Скорость 2 поезда
#define SPEED_2_TRAIN 10
// Пин управления блокировкой драйвера
//#define DRIVER_PIN GPIO_NUM_18 // 18

// Позволяет получить имя переменной
#define VAR_NAME(varible) String(#varible)