#include "DCCDeviceBase.h"

DCCDeviceBase::DCCDeviceBase() {}

DCCDeviceBase::DCCDeviceBase(unsigned char _number)
{
  if (_number > 0 && _number < MAX_CMD_COUNT)
  {
    number = _number;
  }
};

bool DCCDeviceBase::Available()
{
  return writing_command_it != reading_command_it;
}

// Расчет контрольной суммы
unsigned char DCCDeviceBase::CalcCRC(volatile unsigned char *_command, volatile unsigned char &_lengthCMD)
{
  // Если привышена длина сообщения,
  if (_lengthCMD >= MAX_CMD_COUNT)
    return 0x00;
  // Рассчитываем контрольную сумму
  uint8_t i,
      crc = 0x00;
  for (i = 0x00; i < _lengthCMD; i++)
    crc ^= _command[i];
  // Сохраняем контрольную сумму. Изменяем длину пакета
  _command[_lengthCMD] = crc;
  ++_lengthCMD;
  // Вернуть подтвержение расчета контрольной суммы
  return _lengthCMD;
}