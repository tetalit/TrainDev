#include "Service.h"

int Service::FindSubStrPosition(String _str, char _find_str, int _count, bool _start)
{
  int pos = 0;        // Переменная позиции
  int cur_index = -1; // Текущий индекс найденной подстроки
  int count = 0;      // Переменная количества вхождений
  String temp_str;    // Промежуточная строка для вычислений
  if (_start)
  {
    while (count != _count)
    {
      // Находим очередное вхождение символа
      cur_index = _str.lastIndexOf(_find_str);
      if (cur_index == -1)
        return --pos;
      pos = cur_index;
      ++count;
      if (count == _count)
        break; 
      _str = _str.substring(0, cur_index - 1);
    }
  }
  else
  {
    while (count != _count)
    {
      // Находим очередное вхождение символа
      cur_index = _str.indexOf(_find_str);
      if (cur_index == -1)
        return --pos;
      pos += cur_index;
      ++count;
      if (count == _count)
        break;
      _str = _str.substring(++cur_index);
    }
  }

  if (!_start)
    pos += _count - 1;
  return pos;
}

// Разделение строки по спец. символу (Аналог SplitToUint8)
void Service::SplitToUint8(String str, char del, uint8_t *arr)
{
  String word = "";
  uint8_t iter = 0;
  for (uint8_t i = 0; i < str.length(); i++)
  {
    if (str[i] != del)
    {
      word += str[i];
    }
    else
    {
      if (word != "")
        arr[iter++] = atoi(word.c_str());
      else
        break;
      word = "";
    }

    if (i == str.length() - 1 && str[i] != del)
    {
      arr[iter] = atoi(word.c_str());
    }
  }
}

// Проверяет введенный параметр на попадание в отрезок от 0 до 1
bool Service::CheckOneZeroParam(unsigned char _param, String _param_name)
{
  if (_param != 0 && _param != 1)
  {
    Serial.printf("Недопустимое значение параметра %s\nЕго значение: %d\n", _param_name, _param);
    return false;
  }
  return true;
}