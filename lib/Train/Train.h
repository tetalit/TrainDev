#ifndef __TRAIN_H_
#define __TRAIN_H_

#include "TrainCmd.h"
// #include "DeviceBase.h"

// Класс поезда
class Train //: //public DeviceBase<TrainCmd>
{

private:
      // Номер устройства
      unsigned char number;
      // Очередь команд
      std::vector<TrainCmd> commands;
      // Итератор команд
      unsigned char iterator;

public:
      // Конструктор с номером
      Train(unsigned char _number);
      // Конструктор с номером и командами
      // Train(unsigned char _number, std::vector<TrainCmd> _commands);

public:
      // Задать команду(итератор)
      bool SetCommandIterator(unsigned char _command_it);

      // Добавить команду
      bool AddCmd(TrainCmd _cmd);

      // Добавить команду на позицию
      bool AddCmdOnIndex(unsigned char _index, TrainCmd _cmd);

      // Удалить команду
      bool DeleteCmd(TrainCmd _cmd);

      // Удалить команду по индексу
      bool DeleteCmdOnIndex(unsigned char _index);

      // Получить текущую команду
      TrainCmd GetCurCmd();

      // Получить команду по индексу
      TrainCmd GetIndCmd(unsigned char _index);

      // Изменить команду по индексу
      bool ChangeCmd(unsigned char _index, TrainCmd _new_cmd);

      // Получить номер устройства
      unsigned char GetNum();

      // Проверка на наличие команд для устройства
      bool CommandsAvailable();

      // Следующая команда
      void NextCommand();

      // Предыдущая команда
      void PrevCommand();

      // Выбрать текущую команду
      void SelectCommand(unsigned char _cmd_number);
};

#endif //__TRAIN_H_
