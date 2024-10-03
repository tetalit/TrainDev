#ifndef __ARROW_H_
#define __ARROW_H_

#include "DCCDeviceBase.h"

// Класс поезда
class Arrow : public DCCDeviceBase
{

public:
  // Конструктор по умолчанию
  Arrow();
  // Конструктор с номером
  Arrow(unsigned char _number);

  // Переключение направления стрелочного перевода
  void SetDirection(unsigned char _number, unsigned char _direction = LEFT);

  // Формирование DCC команды для стрелочного перевода
  void CreateDCCCommand(volatile unsigned char *_command, volatile unsigned char &_lengthCMD);
};

#endif //__ARROW_H_
