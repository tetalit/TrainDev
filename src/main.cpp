#include "Train.h"
#include "Arrow.h"
#include "Traffic.h"
#include <Arduino.h>
#include "WiFiManager.h"
#include <esp_system.h>
#include <Udp.h>
#include <rom/ets_sys.h>
// !!! ДЛЯ РАБОТЫ В MAIN !!! //

// Флаг прерывания от часов
volatile bool clock_flag = false;
// Флаг выполнения задач сразу после прерывания
volatile bool first_task_flag = false;
// Флаг завершения работы 
volatile bool is_end = false;
// Фоаг ожидания поезда
bool is_stop = false;
// Флаг ошибки по времени
// volatile bool error_flag = false;
// Переменная состояния конечного автомата работы депо
unsigned char state = 0;
// Номер первой считанной метки
unsigned char first_read_uid;
// Временная переменная метки
unsigned char t_uid = 0;
int stop_counter = 0;

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
// Таймер завершения работы 
unsigned int timer_is_end = 0;
// Таймер ожидания поезда
unsigned int timer_stop = 0;
unsigned int timer_stop_2 = 0;
unsigned int timer_stop_3 = 0;
// Время для проезда поезда. Если превышено - взводится флаг ошибки
// unsigned int timer_error = 0;
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
// первое включение светофоров
uint8_t traffics_first_itr = 1;
// переменная для сравнения ip поездов
IPAddress senderIP;

// Обработка прерывания таймера
void IRAM_ATTR OnTimerISR()
{
  // if (clock_flag)
  // {
  timerWrite(DCC_timer, passInterval);
  if (pass)
  {
    // Смена потенциалов
    gpio_set_level(RAIL_L, 1);
    gpio_set_level(RAIL_R, 0);
    pass = 0x00;
    // Если команда переданна, формируем новую командуw
    if (lengthCMD == 0x00) /// while(lengthCMD == 0x00) {}
    {
      if (lengthCMD == 0x00) /// while(lengthCMD == 0x00) {}
      {
        // Если есть комады стрелочного перевода
        if (arrows.Available())
        {
          // Формируем команду стрелочного перевода
          arrows.CreateDCCCommand(command, lengthCMD);
          // Повторить 3 раза
          repeatSend = 0x03;
          //
          // DEBUG();
          return;
        }
        // Если есть команды светофоров
        else
        {
          // Если есть команды светофоров
          if (traffics.Available())
          {
            // Формируем команду светофора
            traffics.CreateDCCCommand(command, lengthCMD);
            // Повторить 3 раза
            repeatSend = 0x03;
            // DEBUG();
          }
        }
      }
    }
  }
  else
  {
    // Смена потенциалов
    gpio_set_level(RAIL_L, 0);
    gpio_set_level(RAIL_R, 1);
    pass = 0x01;
    // По умолчанию передача 1
    passInterval = ONE;
    // Если передается преамбула
    if (preamble > 0x00)
      preamble >>= 0x01;
    // Иначе передача команды
    else if (lengthCMD >= 3)
    {
      // Если команда еще передается
      if (iteratorCMD != lengthCMD)
      {
        // Передача 0 между байтами команды
        if (iteratorBitCMD == 0x00)
        {
          // Передача 0
          passInterval = ZERO;
          // Установить передачу бита
          iteratorBitCMD = 0x80;
        }
        // Передача команды
        else
        {
          // Проверяем по битам
          passInterval = (command[iteratorCMD] & iteratorBitCMD) ? ONE : ZERO;
          // Следующий
          iteratorBitCMD >>= 0x01;
          // Последний бит
          if (iteratorBitCMD == 0x00)
            ++iteratorCMD;
        }
      }
      // Команда передана
      else
      {
        // Передача 1, Конец команды
        passInterval = ONE;
        // Устанавливаем преамбуду
        preamble = LENGTH_PREAMBLE;
        // Сброс указателя команды
        iteratorCMD = 0x00;
        // Если есть повторы, повторять команду
        if (repeatSend)
        {
          repeatSend--;
        }
        // Иначе другая команда
        else
        {
          lengthCMD = 0x00;
        }
      }
    }
  }
  // }
}

// Прерывание от часов
void IRAM_ATTR CLOCKINT()
{
  if (esp_timer_get_time() - timer_to_clock > 500)
  {
    digitalWrite(CLOCK_SIGNAL_INT, !digitalRead(CLOCK_SIGNAL_INT));
    // Флаг начала работы депо
    clock_flag = true;
    // Флаг выполнения задач сразу после прерывания
    first_task_flag = true;
    // Сброс флага ошибки
    // error_flag = false;
    // Сброс метки
    t_uid = 0;
    // Запуск таймера с параметрами
    timerWrite(DCC_timer, passInterval);
    // Сброс таймера времени работы депо
    timer_work = esp_timer_get_time();
    // Сброс таймера-ограничения на срабатывание сигнала
    timer_to_clock = esp_timer_get_time();
    // Сброс таймера превышения допустимого времени работы
    // timer_error = millis();
  }
}

String CommandTrainToString(TrainCmd _deviceCmd)
{
  String tempCmd = String(_deviceCmd.direction) + ":";
  tempCmd += String(_deviceCmd.speed) + ":";
  tempCmd += String(_deviceCmd.front_light) + ":";
  tempCmd += String(_deviceCmd.rear_light) + ":";
  tempCmd += String(_deviceCmd.front_buf_light) + ":";
  tempCmd += String(_deviceCmd.rear_buf_light) + ":";
  tempCmd += String(_deviceCmd.one_sound) + ":";
  tempCmd += String(_deviceCmd.two_sound) + ":";
  tempCmd += String(_deviceCmd.smoke);
  return tempCmd;
}

// Рассылка команд поездам
void SendsCommands()
{
  if (millis() - send_timer > 1000)
  {
    for (size_t i = 0; i < trains.size(); i++)
    {
      IPAddress deviceIP = IPAddress(192, 168, 1, trains.at(i).GetNum());
      TrainCmd deviceCmd = trains.at(i).GetCurCmd();
      for (size_t i = 0; i < 3; i++)
      {
        wifi_m.SendPacket(deviceIP, CommandTrainToString(deviceCmd));
      }
    }
    send_timer = millis();
  }
}

void setup()
{
  // Инициализация COM порта
  Serial.begin(921600);
  Serial.printf("Serial port запущен на скорости %d\n", Serial.baudRate());

  // Создание точки доступа
  wifi_m.InitWiFi();
  wifi_m.InitDns("ESP32Depot");
  wifi_m.InitUDP();

  // Конфигурация GPIO
  // Выходны на рельсы
  gpio_pad_select_gpio(RAIL_L);
  gpio_set_direction(RAIL_L, GPIO_MODE_OUTPUT);
  gpio_pad_select_gpio(RAIL_R);
  gpio_set_direction(RAIL_R, GPIO_MODE_OUTPUT);

  // Входя для питания с подтяжкой
  pinMode(CLOCK_PIN, INPUT_PULLUP);
  // gpio_pad_select_gpio(CLOCK_PIN);
  // gpio_set_direction(CLOCK_PIN, GPIO_MODE_INPUT);
  // gpio_set_pull_mode(CLOCK_PIN, GPIO_PULLUP_ONLY);

  // Устаревший пин часов, конфигурирую на вход на всякий случай
  pinMode(CLOCK_PIN_PHYS, INPUT);
  // gpio_pad_select_gpio(CLOCK_PIN_PHYS);
  // gpio_set_direction(CLOCK_PIN_PHYS, GPIO_MODE_INPUT);

  // Настройка сигального светодиода
  pinMode(CLOCK_SIGNAL_INT, OUTPUT);
  // gpio_pad_select_gpio(CLOCK_SIGNAL_INT);
  // gpio_set_direction(CLOCK_SIGNAL_INT, GPIO_MODE_OUTPUT);

  // pinMode(34, INPUT_PULLUP);

  Serial.println("GPIO сконфигурированы");

  trains.push_back(Train(1));
  trains.push_back(Train(2));

  TrainCmd cmd(1, 0, 1, 0, 0, 0, 0, 0, 0);
  TrainCmd cmd1(1, 5, 1, 0, 0, 0, 0, 0, 0);
  // TrainCmd cmd2(1, 0, 1, 0, 0, 0, 0, 0, 0);
  TrainCmd cmd2_0(1, 0, 1, 0, 0, 0, 0, 0, 0);
  TrainCmd cmd2_1(1, 5, 1, 0, 0, 0, 0, 0, 0);

  trains.at(0).AddCmd(cmd);
  trains.at(0).AddCmd(cmd1);
  // trains.at(0).AddCmd(cmd2);
  trains.at(1).AddCmd(cmd2_0);
  trains.at(1).AddCmd(cmd2_1);

  Serial.printf("Количество поездов: %s\n", String(trains.size()));

  // Конфигурация таймера
  DCC_timer = timerBegin(1, 16, false);
  timerAttachInterrupt(DCC_timer, &OnTimerISR, true);
  timerAlarmWrite(DCC_timer, 0, true);
  timerAlarmDisable(DCC_timer);
  timerAlarmEnable(DCC_timer);
  timerWrite(DCC_timer, 600);
  Serial.println("Таймер настроен");

  // Активация прерывания на ножке часов
  attachInterrupt(CLOCK_PIN, CLOCKINT, FALLING);
  Serial.println("Прерывния с часов активно");

  timer_send = millis();
  timer_to_clock = millis();

  delay(2000);

  // Сбрасываем команду для поезда
  trains.at(0).SetCommandIterator(0);
  trains.at(1).SetCommandIterator(0);

  // Первое включение светофоров
  for(int i = 0; i <= 3; i++){
      traffics.SetLight(1, 1, ON); // 1 - желтый верх 2 - зелёный  3 - красный 4 - желтый низ
      delay(250);
      traffics.SetLight(2, 2, ON);
      delay(250);
      traffics.SetLight(3, 2, ON);
      delay(250);
      traffics.SetLight(4, 1, ON);
      traffics.ShowCommand();
    }
}

bool dir = false;

void loop()
{

  if (clock_flag)
  {
    if (first_task_flag)
    {
      timerAlarmEnable(DCC_timer);
      timerWrite(DCC_timer, passInterval);
      trains.at(0).SetCommandIterator(1);
      trains.at(1).SetCommandIterator(1);
      delay(5000);
      first_task_flag = false;
    }

    if (millis() - timer_send > 1000)
    {
      //arrows.SetDirection(3,dir);
      dir^=1;
      wifi_m.CheckCountClients();
      SendsCommands();
      Serial.print("Текущий поезд: ");
      Serial.print(String(trains.at(0).GetNum()));
      Serial.print(", текущая команда: ");
      Serial.println(CommandTrainToString(trains.at(0).GetCurCmd()));
      // for (size_t i = 0; i < 4; i++)
      // {
      //   Serial.printf("%s, ", String(traffics.at(0).GetCurCmd().trafficLeds[i]));
      // }
      // Serial.println();
      timer_send = millis();
    }

    t_uid = wifi_m.CheckPacket();
    if (t_uid != 0)
    {
      t_uid = wifi_m.GetPacket()[0];
      senderIP = wifi_m.GetLastIP(); // берём ip поезда отправившего метку
      wifi_m.ClearUDPBuffer();
    }
     
    // П2 доехал до м10, С4 = К
    if (t_uid == 10 && senderIP == IPAddress(192,168,1,2)){
      traffics.SetLight(4, 3, ON);
    }

    // П1 доехал до м3, С1 = К
    if (t_uid == 3 && senderIP == IPAddress(192,168,1,1))
    {
      traffics.SetLight(1, 3, ON); 
    }

    // П2 доехал до м9, П2 остановка
    if (t_uid == 9 && senderIP == IPAddress(192,168,1,2)){
      trains.at(1).SetCommandIterator(0); //команда остановки 
      stop_counter += 1;
    }

    // П1 доехал до м4, П2 остановка
    if (t_uid == 4 && senderIP == IPAddress(192,168,1,1)) 
    {
      // Переключаемся на следующую команду - остановка
      trains.at(0).SetCommandIterator(0);
      stop_counter += 1;
    }

    if(stop_counter >=2){
        stop_counter = 0;
        is_stop = true;
        timer_stop = millis();
    }

    // оба поезда остановились запускаем таймер на 30с
    if (is_stop) 
    {
      if (millis() - timer_stop > 30000)
      {
        // запускаем П1
        trains.at(0).SetCommandIterator(1); 
        // П1 считал М6,  C1 = зелёный
        if(t_uid == 6 && senderIP == IPAddress(192,168,1,1))
        {
          traffics.SetLight(1,2,ON); 
        }
        // П1 считал М7, C3 = красный
        if(t_uid == 7 && senderIP == IPAddress(192,168,1,1))
        {
          traffics.SetLight(3,3,ON); 
        }       
        is_stop = false;
      }
      // П1 считал М8, остановка на 30с и запуск П2
        if (t_uid == 8 && senderIP == IPAddress(192,168,1,1))
        {
          trains.at(0).SetCommandIterator(0);
          // timer_stop_2 = millis();
          trains.at(1).SetCommandIterator(1);
          //П2 = М7, С4 = Ж + Ж
          if(t_uid == 4 && senderIP == IPAddress(192,168,1,2))
          {
            traffics.SetLight(4,1,ON); //Сделать 1 желтый сигнал мигает, другой желтый статичный  
            traffics.SetLight(4,4,ON);
          }
          // П2 = М6, C2 = К
          if(t_uid == 6 && senderIP == IPAddress(192,168,1,2))
          {
            traffics.SetLight(2,3,ON); 
          }
          // П2 = М5 остановка
          if(t_uid == 5 && senderIP == IPAddress(192,168,1,2)){
            trains.at(1).SetCommandIterator(0);
            timer_stop_2 = millis(); 
          }
          // П2 = М5 и П1 = М8 и прошло 30 сек, запускаем оба поезда
          if((t_uid == 8 && senderIP == IPAddress(192,168,1,1)) && (t_uid == 5 && senderIP == IPAddress(192,168,1,2)))
          {
            if(millis() - timer_stop_2 > 30000)
            {
              trains.at(1).SetCommandIterator(1);
              trains.at(0).SetCommandIterator(1);
            }
          }
        }
        // П1 = М10, С3 = зелёный
        if(t_uid == 10 && senderIP == IPAddress(192,168,1,1))
        {
          traffics.SetLight(3,2,ON);
        }
        // П2 = М3, С2 = зелёный
        if(t_uid == 3 && senderIP == IPAddress(192,168,1,2))
        {
          traffics.SetLight(2,2,ON);
        }
        // П1 = М2 остановка в депо и возврат к началу
        if(t_uid == 2 && senderIP == IPAddress(192,168,1,1))
        {
          trains.at(0).SetCommandIterator(0);
          return;
        }
        // П2 = М1 остановка в депо и возврат к началу 
        if(t_uid == 1 && senderIP == IPAddress(192,168,1,2))
        {
          trains.at(1).SetCommandIterator(0);
          return;
        }
    }

    if (t_uid != 0)
    {
      Serial.print("UID: ");
      Serial.println(t_uid);
    }
    t_uid = 0;
  }

  if (is_end)
  {
    if (millis() - timer_is_end > 1000)
    {
      timerAlarmDisable(DCC_timer);
      clock_flag = false;
      is_end = false;
      digitalWrite(RAIL_L, LOW);
      digitalWrite(RAIL_R, LOW);
      wifi_m.ClearUDPBuffer();
    }
  }


  // for (int tr_num = 1; tr_num < 7; tr_num++)
  // {
  //   Serial.printf("TL %s on\n", String(tr_num));
  //   traffics.SetLight(tr_num, 1, ON);
  //   delay(100);
  //   traffics.SetLight(tr_num, 2, ON);
  //   delay(100);
  //   traffics.SetLight(tr_num, 3, ON);
  //   delay(100);
  //   traffics.SetLight(tr_num, 4, ON);
  //   delay(100);
  // }                               
  
  //traffics.ShowCommand();
//  delay(3000);
/*
  for (int tr_num = 1; tr_num < 7; tr_num++)
  {
    Serial.printf("TL %s off\n", String(tr_num));
    traffics.SetLight(tr_num, 1, OFF);
    delay(1000);
    traffics.SetLight(tr_num, 2, OFF);
    delay(1000);
    traffics.SetLight(tr_num, 3, OFF);
    delay(1000);
    traffics.SetLight(tr_num, 4, OFF);
    delay(1000);
  }

  traffics.ShowCommand();
  delay(3000);
  */
}