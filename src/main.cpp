#include "main.h"

// Обработка прерывания таймера
void IRAM_ATTR OnTimerISR()
{
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
      else if (command_ready)
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
  }
}

// Прерывание от часов
void IRAM_ATTR CLOCKINT()
{
  if (millis() - timer_to_clock > 500)
  {
    // Флаг начала работы депо
    clock_flag = true;
    // Сброс флага ошибки
    error_flag = false;
    // Сброс метки
    t_uid = 0;
    // Запуск таймера с параметрами
    timerWrite(DCC_timer, passInterval);
    // Сброс таймера времени работы депо
    timer_work = millis();
    // Сброс таймера-ограничения на срабатывание сигнала
    timer_to_clock = millis();
    // Сброс таймера превышения допустимого времени работы
    timer_error = millis();
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
  Serial.begin(115200);
  Serial.printf("Serial port запущен на скорости %d\n", Serial.baudRate());

  // Создание точки доступа
  wifi_m.InitWiFi();
  wifi_m.InitDns("ESP32Depot");
  wifi_m.InitUDP();

  // Конфигурация GPIO
  gpio_pad_select_gpio(RAIL_L);
  gpio_set_direction(RAIL_L, GPIO_MODE_OUTPUT);
  gpio_pad_select_gpio(RAIL_R);
  gpio_set_direction(RAIL_R, GPIO_MODE_OUTPUT);

  gpio_pad_select_gpio(CLOCK_PIN);
  gpio_set_direction(RAIL_R, GPIO_MODE_INPUT);

  Serial.println("GPIO сконфигурированы");

  trains.push_back(Train(1));

  TrainCmd cmd(1, 0, 1, 0, 0, 0, 0, 0, 0);
  TrainCmd cmd1(1, 1, 1, 0, 0, 0, 0, 0, 0);
  TrainCmd cmd2(1, 0, 1, 0, 0, 0, 0, 0, 0);
  TrainCmd cmd3(1, 1, 1, 0, 0, 0, 0, 0, 0);

  trains.at(0).AddCmd(cmd);
  trains.at(0).AddCmd(cmd1);
  trains.at(0).AddCmd(cmd2);
  trains.at(0).AddCmd(cmd3);

  Serial.printf("Количество поездов: %s\n", String(trains.size()));

  // Конфигурация таймера
  DCC_timer = timerBegin(0, 16, false);
  timerAttachInterrupt(DCC_timer, &OnTimerISR, true);
  timerAlarmWrite(DCC_timer, 0, true);
  timerAlarmEnable(DCC_timer);
  timerWrite(DCC_timer, 600);
  Serial.println("Таймер настроен");

  // Активация прерывания на ножке часов
  attachInterrupt(digitalPinToInterrupt(CLOCK_PIN), CLOCKINT, RISING);
  Serial.println("Прерывния с часов активно");

  timer_send = millis();

  delay(2000);

  // Сбрасываем команду для поезда
  trains.at(0).SetCommandIterator(0);
}

void loop()
{
  // if (millis() - timer_send > 1000)
  // {

  //   wifi_m.CheckCountClients();
  //   SendsCommands();
  //   Serial.print("Текущий поезд: 0, текущая команда: ");
  //   Serial.println(CommandTrainToString(trains.at(0).GetCurCmd()));
  //   // for (size_t i = 0; i < 4; i++)
  //   // {
  //   //   Serial.printf("%s, ", String(traffics.at(0).GetCurCmd().trafficLeds[i]));
  //   // }
  //   // Serial.println();
  //   timer_send = millis();
  // }

  // t_uid = wifi_m.CheckPacket();
  // wifi_m.GetPacket();

  // // Логика
  // // Если пришла команда от часов
  // if (clock_flag == true)
  // {
  //   // Переключаемся на следующую команду - двжиение
  //   trains.at(0).NextCommand();

  //   // Если была считана первая метка
  //   if (t_uid == 0x00)
  //   {
  //     // Переключаемся на следующую команду - остановка
  //     trains.at(0).NextCommand();
  //     // Ожидание 30 сек
  //     delay(30000);
  //     // Продолжаем движение
  //     trains.at(0).NextCommand();
  //   }
  //   // Если была считана финальная метка - остановка
  //   else if (t_uid == 0x01)
  //   {
  //     // Сбрасываем итератор на команду остановки
  //     trains.at(0).SetCommandIterator(0);
  //   }
  // }

  arrows.SetDirection(3, LEFT);
  delay(3000);
  arrows.SetDirection(3, RIGHT);
  delay(3000);

  // delay(5000);
}