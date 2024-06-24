#ifndef __DCC_H_
#define __DCC_H_

#include "Definitions.h"
#include "Traffic.h"
#include "Arrow.h"
#include <esp_system.h>
#include <rom/ets_sys.h>

struct DCC
{
public:
  // Объект для работы со стрелочными переводами
  std::vector<Arrow> arrows;
  // Объект для работы со светофорами
  std::vector<Traffic> traffics;
  // Полупериод одного бита
  volatile unsigned int passInterval = ONE;

  // Передаваемая команда
  volatile unsigned char command[MAX_SEND_CMD_LENGTH];
  // Обработка прерывания таймера
  void IRAM_ATTR OnTimerISR(hw_timer_t* _DCC_timer);

private:
  // Флаг готовности команды к передаче
  volatile bool command_ready = false;
  // Длина передаваемой команды
  volatile unsigned char lengthCMD;
  // Указатель передачи одного бита команды
  volatile unsigned char pass = 0x00;
  // Количество переданных байт команды
  volatile unsigned char iteratorCMD = 0x00;
  // Указатель на передаваемый бит
  volatile unsigned char iteratorBitCMD = 0x00;
  // Колличество повторов команды
  volatile unsigned short repeatSend = 0x00;
  // Прембула между командами
  volatile unsigned short preamble = LENGTH_PREAMBLE;
  // Переключение между устройствами для формирования команд
  volatile unsigned char switchDevice = 0;

  // Итератор стрелочных переводов
  volatile unsigned char arrowIterator = 0;
  // Итератор светофоров
  volatile unsigned char trafficIterator = 0;
  // Итератор для светодиодов светофора
  volatile unsigned char ledsTrafficIterator = 0;

  // Расчет контрольной суммы
  unsigned char SetCRC();
  // Команда в строковом формате
  String CommandToString();
  // Формирование DCC команды светофора
  bool CreateTrafficCommand();
  // Формирование DCC команды стрелочных переводов
  bool CreateArrowCommand();
};

#endif // __DCC_H_