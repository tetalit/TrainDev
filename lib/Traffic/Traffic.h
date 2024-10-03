#ifndef __TRAFFIC_H_
#define __TRAFFIC_H_

#include "DCCDeviceBase.h"

// Класс светофора
class Traffic : public DCCDeviceBase
{
private:
      // Переменная для сохранения конфигурации светодиода
      unsigned char lightConfig[2];

public:
      // Конструктор по умолчанию
      Traffic();
      // Конструктор с номером
      Traffic(unsigned char _number);
      // Конструктор с номером и командами
      // Traffic(unsigned char _number, std::vector<TrafficCmd> _commands);

      // Переключение направления стрелочного перевода
      void SetLight(unsigned char _number, unsigned char _light, unsigned char _state = OFF);

      // Формирование DCC команды для стрелочного перевода
      void CreateDCCCommand(volatile unsigned char *_command, volatile unsigned char &_lengthCMD);

      // Сформировать команду по заданным параметрам и отобразить на экране
      void ShowCommand();
};

#endif //__TRAFFIC_H_
