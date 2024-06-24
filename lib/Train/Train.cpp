#include "Train.h"

// Конструктор
Train::Train(unsigned char _number)
{
}

// Конструктор с указанными параметрами
Train::Train(unsigned char _number, std::vector<TrainCmd> _commands)
{
}

// Задать команду(итератор)
bool Train::SetCommandIterator(unsigned char _command_it)
{
  // Если итератор лежит в допустимых пределах
  if (_command_it < commands.size() && _command_it > 0)
  {
    iterator = _command_it;
    return true;
  }
  return false;
};

// Добавить команду
bool Train::AddCmd(TrainCmd _cmd)
{
  // Не выходим за рамки допустимого количества команд
  if (commands.size() < MAX_CMD_COUNT)
  {
    commands.push_back(_cmd);
    return true;
  }
  return false;
};

// Добавить команду на позицию
bool Train::AddCmdOnIndex(unsigned char _index, TrainCmd _cmd)
{
  // Не выходим за рамки допустимого количества команд и индекс должен соответствовать допустимому
  if (commands.size() < MAX_CMD_COUNT && (_index < commands.size() && _index > 0))
  {
    auto iter = commands.begin();
    commands.insert(iter + _index, _cmd);
    return true;
  }
  return false;
};

// Удалить команду
bool Train::DeleteCmd(TrainCmd _cmd)
{
  auto iter = commands.begin();
  for (auto cmd : commands)
  {
    if (cmd == _cmd)
    {
      commands.erase(iter);
      return true;
    }
    iter++;
  }
  return false;
};

// Удалить команду по индексу
bool Train::DeleteCmdOnIndex(unsigned char _index)
{
  // Если итератор в допустимых диапазонах
  if (_index < commands.size() && _index > 0)
  {
    auto iter = commands.begin();
    commands.erase(iter + _index);
    return true;
  }
  return false;
};

// Получить текущую команду
TrainCmd Train::GetCurCmd()
{
  return commands.at(iterator);
};

// Получить команду по индексу
TrainCmd Train::GetIndCmd(unsigned char _index)
{
  // Если итератор в допустимых диапазонах
  if (_index < commands.size() && _index > 0)
  {
    return commands.at(_index);
  }
  return TrainCmd();
};

// Изменить команду по индексу
bool Train::ChangeCmd(unsigned char _index, TrainCmd _new_cmd)
{
  // Если итератор в допустимых диапазонах
  if (_index < commands.size() && _index > 0)
  {
    commands.at(_index) = _new_cmd;
    return true;
  }
  return false;
};

// Получить номер устройства
unsigned char Train::GetNum()
{
  return number;
};

// Проверка на наличие команд для устройства
bool Train::CommandsAvailable()
{
  return (commands.size() > 0);
};

// Следующая команда
void Train::NextCommand()
{
  Serial.printf("Размер вектора команд: %s\n", String(commands.size()));
  Serial.printf("Итератор: %s\n", String(iterator));
  if (iterator < commands.size() - 1)
    iterator++;
  else
    iterator = 0;
};

// Предыдущая команда
void Train::PrevCommand()
{
  if (iterator > 0)
    iterator--;
  else
    iterator = commands.size() - 1;
};

// Выбрать текущую команду
void Train::SelectCommand(unsigned char _cmd_number)
{
  if (_cmd_number >= 0 && _cmd_number <= commands.size())
  {
    iterator = _cmd_number;
  }
  else
  {
    Serial.println("Недопустимый номер команды");
  }
};