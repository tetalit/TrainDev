#include "Arrow.h"

// Конструктор по умолчанию
Arrow::Arrow() : DCCDeviceBase() {}

// Конструктор
Arrow::Arrow(unsigned char _number) : DCCDeviceBase(_number)
{
}

// Конструктор с указанными параметрами
// Arrow::Arrow(unsigned char _number, std::vector<ArrowCmd> _commands)  : DeviceBase(_number, _commands)
// {
// }

// Переключение направления стрелочного перевода
void Arrow::SetDirection(unsigned char _number, unsigned char _direction)
{
  // Проверка номера светофора на максимльный
  if ((_number > MAX_DEVICE) || (_number <= 0))
    return;
  // Сохраняем номер стрелочного перевода
  commandsList[writing_command_it] = _number;
  // Устанавливаем направление переключения
  if (_direction)
    commandsList[writing_command_it] |= 1 << 0x0F;
  // Изменение номера указателя
  if (++writing_command_it >= MAX_CMD_COUNT)
    writing_command_it = 0x00;
}

// Формирование DCC команды для стрелочного перевода
void Arrow::CreateDCCCommand(volatile unsigned char *_command, volatile unsigned char &_lengthCMD)
{
  // Проверка есть ли доступные переводы
  if (!Available())
    return;
  // Формирование команды перевода
  // Сброс длины отправляемой команды
  _lengthCMD = 0x00;
  // Установка адреса стрелочного перевода
  // Первый байт
  _command[_lengthCMD] = (((commandsList[reading_command_it] - 0x01) & 0xFF) / 0x04 + 0x01) & 0x3F | 0x80;
  ++_lengthCMD;
  // Второй байт
  _command[_lengthCMD] = (((commandsList[reading_command_it] - 0x01) >> 0x08) ^ 0x0F) << 0x04 | 0x88;
  // Направление переключения стрелочного перевода
  if (commandsList[reading_command_it] >> 0x0F)
    _command[_lengthCMD] |= 0x01; // Перевод влево, иначе вправо
  // Добавление к адресу
  _command[_lengthCMD] |= (commandsList[reading_command_it] - 0x01) % 0x04 << 0x01;
  ++_lengthCMD;
  // Рассчитываем контрольную сумму
  _lengthCMD = CalcCRC(_command, _lengthCMD);
  // Изменяем номер итератора
  // Принудительно задаем тот же номер, что и для записываемого элемента
  reading_command_it = writing_command_it;
  // if (++reading_command_it >= MAX_CMD_COUNT)
  //   reading_command_it = 0x00;
}