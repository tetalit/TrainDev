#pragma once
#ifndef __DEVICE_BASE_H_
#define __DEVICE_BASE_H_

#include "DefineStorage.h"

class DCCDeviceBase
{
protected:
  // Номер устройства
  unsigned char number;
  // Список команд для поезда
  unsigned short commandsList[MAX_CMD_COUNT];
  // Итератор добавляемой команды
  unsigned short writing_command_it;
  // Итератор считываемой команды
  unsigned short reading_command_it;

  // Конструктор по умолчанию
  DCCDeviceBase();

  // Конструктор с номером
  DCCDeviceBase(unsigned char _number);

public:
  // Проверка на возможность исполнения команды
  bool Available();

  // Расчет контрольной суммы
  unsigned char CalcCRC(volatile unsigned char *_command, volatile unsigned char &_lengthCMD);
};

#endif //__DEVICE_BASE_H_
