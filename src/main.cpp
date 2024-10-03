#include "main.h"


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
}

//unsigned char tr_num = 3;

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
      arrows.SetDirection(3, RIGHT);
      first_task_flag = false;
    }

    if (millis() - timer_send > 1000)
    {
      arrows.SetDirection(3,dir);
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
      wifi_m.ClearUDPBuffer();
    }

    if (t_uid == 30){
      trains.at(1).SetCommandIterator(0);
      stop_counter += 1;
      arrows.SetDirection(3, LEFT);
    }

    // Если была считана первая метка
    if (t_uid == 43)
    {
      // Переключаемся на следующую команду - остановка
      trains.at(0).SetCommandIterator(0);
      stop_counter += 1;
      
       arrows.SetDirection(3, LEFT);

      // traffics.SetLight(6, 1, ON);
      // traffics.SetLight(6, 2, ON);
      // traffics.SetLight(6, 3, OFF);
      // traffics.SetLight(6, 4, OFF);
    }
    if(stop_counter >=2){
        stop_counter = 0;
        is_stop = true;
        timer_stop = millis();
    }

    if (is_stop)
    {
      if (millis() - timer_stop > 15000)
      {
        trains.at(0).SetCommandIterator(1);
        trains.at(1).SetCommandIterator(1);
        is_stop = false;
      }
    }

    if (t_uid == 229){
      trains.at(1).SetCommandIterator(0);
      arrows.SetDirection(3, RIGHT);
    }

    // Если была считана финальная метка - остановка
    if (t_uid == 160)
    {
      // Сбрасываем итератор на команду остановки
      trains.at(0).SetCommandIterator(0);
      
       arrows.SetDirection(3, RIGHT);

      // traffics.SetLight(6, 1, OFF);
      // traffics.SetLight(6, 2, OFF);
      // traffics.SetLight(6, 3, ON);
      // traffics.SetLight(6, 4, ON);
    }
    // Если была считана финальная метка - остановка
    // else if (t_uid == 30)
    // {
    //   // Сбрасываем итератор на команду остановки
    //   trains.at(0).SetCommandIterator(2);
    //   arrows.SetDirection(3, LEFT);

    //   traffics.SetLight(6, 1, OFF);
    //   traffics.SetLight(6, 2, OFF);
    //   traffics.SetLight(6, 3, OFF);
    //   traffics.SetLight(6, 4, OFF);

    //   is_end = true;
    //   timer_is_end = millis();
    // }
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
  traffics.SetLight(3, 2, ON); // 1 - красный 2 - жёлтый 3 - красный
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