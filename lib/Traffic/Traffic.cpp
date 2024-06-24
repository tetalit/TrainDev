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

// Переключение направления стрелочного перевода
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
  commandsList[writing_command_it] = (_state) ? (1UL << 0x0F) | _light : _light;
  // Изменение номера указателя
  if (++writing_command_it >= MAX_CMD_COUNT)
    writing_command_it = 0x00;
}

// Формирование DCC команды для стрелочного перевода
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
  _command[_lengthCMD] = (commandsList[reading_command_it] >> 0x0F) | 0x80;
  ++_lengthCMD;
  // Рассчитываем контрольную сумму
  _lengthCMD = CalcCRC(_command, _lengthCMD);
  // Изменяем номер итератора
  if (++reading_command_it >= MAX_CMD_COUNT)
    reading_command_it = 0x00;
}