#include "Traffic.h"

// Конструктор по умолчанию
Traffic::Traffic() : DCCDeviceBase() {}

// Конструктор
Traffic::Traffic(unsigned char _number) : DCCDeviceBase(_number)
{
}

// Конструктор с указанными параметрами
// Traffic::Traffic(unsigned char _number, std::vector<TrafficCmd> _commands)  : DeviceBase(_number, _commands)
// {
// }

void Traffic::SetLight(unsigned char _number, unsigned char _light, unsigned char _state)
{
  // Проверка номера светофора на максимльный
  if ((_number >= MAX_DEVICE) || (_number < 0x00))
    return;
  // Сохраняем номер
  commandsList[writing_command_it] = _number;
  // Номера огней от 1 до 7
  _light %= 0x08;
  // Устанавливаем режим работы
  commandsList[writing_command_it] |= (((_state) ? (1UL << 0x03) | _light : _light) << 0x08);
  // Изменение номера указателя
  if (++writing_command_it >= MAX_CMD_COUNT)
    writing_command_it = 0x00;
}


void Traffic::CreateDCCCommand(volatile unsigned char *_command, volatile unsigned char &_lengthCMD)
{
  // Проверка есть ли доступные светофоры
  if (!Available())
    return;
  // Формирование команды светофора
  // Сброс длины отправляемой команды
  _lengthCMD = 0x00;
  // Установка адреса светофора
  // Первый байт
  _command[_lengthCMD] = (commandsList[reading_command_it] & 0xFF) & 0x3F | 0x80;
  ++_lengthCMD;
  // Второй байт
  _command[_lengthCMD] = ((commandsList[reading_command_it] & 0xFF) >> 0x05) & 0x06 | 0x01;
  _command[_lengthCMD] |= ((commandsList[reading_command_it] & 0xFF) >> 0x04) & 0x70;
  ++_lengthCMD;
  // Устанавливаем вкл/выкл света светофора
  _command[_lengthCMD] = (commandsList[reading_command_it] >> 0x08) | 0x80;
  ++_lengthCMD;
  // Рассчитываем контрольную сумму
  _lengthCMD = CalcCRC(_command, _lengthCMD);
  // Изменяем номер итератора
  // Принудительно задаем тот же номер, что и для записываемого элемента
  reading_command_it = writing_command_it;
  // if (++reading_command_it >= MAX_CMD_COUNT)
  //   reading_command_it = 0x00;
}

// Сформировать команду по заданным параметрам и отобразить на экране
void Traffic::ShowCommand()
{
  // Проверка есть ли доступные светофоры
  if (!Available())
    return;
  unsigned char temp[3];

  temp[0] = (commandsList[reading_command_it] & 0xFF) & 0x3F | 0x80;
  // Второй байт
  temp[1] = ((commandsList[reading_command_it] & 0xFF) >> 0x05) & 0x06 | 0x01;
  temp[1] |= ((commandsList[reading_command_it] & 0xFF) >> 0x04) & 0x70;
  // Устанавливаем вкл/выкл света светофора
  temp[2] = (commandsList[reading_command_it] >> 0x08) | 0x80;

  unsigned char crc = 0;

  for (int i = 0; i < 3; i++)
  {
    crc ^= temp[i];
  }
  
  // Установка адреса светофора
  Serial.print("Первый байт: ");
  Serial.println(temp[0], BIN);
  Serial.print("Второй байт: ");
  Serial.println(temp[1], BIN);
  Serial.print("Третий байт: ");
  Serial.println(temp[2], BIN);
  Serial.print("CRC: ");
  Serial.println(crc, BIN);
  Serial.println();
}