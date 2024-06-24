// Включение заголовочного файла определений
#include "DCC.h"

// Расчет контрольной суммы
unsigned char DCC::SetCRC()
{
  // Если прtвышена длина сообщения
  if (lengthCMD >= MAX_SEND_CMD_LENGTH)
    return 0x00;
  // Рассчитываем контрольную сумму
  unsigned char crc = 0x00;
  for (size_t i = 0x00; i < lengthCMD; i++)
    crc ^= command[i];

  return crc;
}

// Команда в строковом формате
String DCC::CommandToString()
{
  // Формируем ответное сообшение
  String answer = String(command[0x00], DEC);
  for (size_t i = 0x01; i < lengthCMD; i++)
    answer += " " + String(command[i], DEC);
  return answer;
}

// Обработка прерывания таймера
void IRAM_ATTR DCC::OnTimerISR(hw_timer_t* _DCC_timer)
{
  timerWrite(_DCC_timer, passInterval);
  if (pass)
  {
    // Смена потенциалов
    gpio_set_level(RAIL_L, 1);
    gpio_set_level(RAIL_R, 0);
    pass = 0x00;
    // Если команда переданна, формируем новую команду
    if (!command_ready) /// while(lengthCMD == 0x00) {}
    {
      // Формируем команды для стрелочных переводов
      if (switchDevice == 0 && arrows.size() > 0)
      {
        if (arrowIterator < arrows.size())
        {
          if (arrows.at(arrowIterator).CommandsAvailable())
          {
            // Формируем команду стрелочного перевода
            command_ready = CreateArrowCommand();
          }
          arrowIterator++;
        }
        else
        {
          //switchDevice = 1;
          arrowIterator = 0;
        }
      }
      // Формируем комманды для светофоров
      else if (switchDevice == 1 && traffics.size() > 0)
      {
        if (trafficIterator < traffics.size())
        {
          if (traffics.at(trafficIterator).CommandsAvailable())
          {
            // Формируем команду светофора
            command_ready = CreateTrafficCommand();
            if (ledsTrafficIterator < 4)
            {
              ledsTrafficIterator++;
            }
            else
            {
              ledsTrafficIterator = 0;
              //switchDevice = 0;
              trafficIterator++;
            }
          }
        }
        else
        {
          trafficIterator = 0;
        }
      }
      switchDevice ^= 1;
      // Повторить 3 раза
      repeatSend = 0x03;
      return;
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
          // Сброс флага готовности команды
          command_ready = false;
        }
      }
    }
  }
}

// Формирование DCC команды из текущей команды светофора
// {Преамбула} 0 10 A5A4 A3A2A1A0 0 0A10A9A8 0 A7A6 1 0 100 C1C0D2D1D0 1
// A - адрес, C - вкл/выкл, D - светодиод
bool DCC::CreateTrafficCommand()
{
  // Номер светофора
  unsigned char currentNumber = traffics.at(trafficIterator).GetNum();
  // Текущая команда
  TrafficCmd cmd = traffics.at(trafficIterator).GetCurCmd();
  // Формирование команды светофора
  lengthCMD = 0;
  // Установка адреса светофора
  // Первый байт
  command[lengthCMD++] = (currentNumber & 0b00111111 | 0b10000000);
  // Второй байт
  command[lengthCMD++] = (((currentNumber >> 5) & 0b00000110) | 0b00000001) | ((currentNumber >> 4) & 0b01110000);
  // Устанавливаем вкл/выкл для светодиода светофора
  command[lengthCMD++] = ((cmd.trafficLeds[ledsTrafficIterator]) ? (1 << 3) | ledsTrafficIterator : ledsTrafficIterator) | 0b10000000;
  // Обнуление байта crc и его рассчет
  command[lengthCMD] = 0;
  for (size_t i = 0; i < lengthCMD; i++)
  {
    command[lengthCMD] ^= command[i];
  }

  return true;
}

// Формирование DCC команды из текущей команды стрелочных переводов
// 10 A5A4A3A2A1A0 1 A8A7A6 C D2D1D0
// A - адрес, C - направление, D - номер стрелочного перевода
bool DCC::CreateArrowCommand()
{
  // Номер стрелочного перевода
  unsigned char currentNumber = arrows.at(arrowIterator).GetNum();
  // Текущая команда
  ArrowCmd cmd = arrows.at(arrowIterator).GetCurCmd();
  // Формирование команды стрелочного перевода
  lengthCMD = 0;

  // Предварительная подготовка элемента
  unsigned char tempCommand = currentNumber;
  // Направление
  if (cmd.direction)
    tempCommand |= 1 << 15;

  // Установка адреса стрелочного перевода
  // Первый байт
  command[lengthCMD++] = ((((tempCommand - 1) & 0b11111111) / 4 + 1) & 0b00111111) | 0b10000000;
  // Второй байт
  command[lengthCMD] = ((((tempCommand - 1) >> 8) ^ 0b00001111) << 4) | 0b10001000;
  // Направление переключения
  if (tempCommand >> 15)
  {
    command[lengthCMD] |= 1; // Перевод влево, если 0, то вправо
  }
  // Добавление к адресу
  command[lengthCMD++] |= ((tempCommand - 1) % 4) << 1;

  // Обнуление байта crc и его расчет
  command[lengthCMD] = 0;
  for (size_t i = 0; i < lengthCMD; i++)
  {
    command[lengthCMD] ^= command[i];
  }

  return true;
}