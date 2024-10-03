#ifndef __SERVICE_H_
#define __SERVICE_H_

#include <Arduino.h>

class Service
{
public:
  // Разделение строки по спец. символу (Аналог SplitToUint8)
  static void SplitToUint8(String str, char del, uint8_t *arr);
  // Функция для поиска символа подстроки, возвращает его позицию, -1, если символ не найден
  // _str - строка, в которой осуществляется поиск
  // _find_str - искомая подстрока
  // _count - количество вхождений (какой по счету элемент нужен) (начиная от 1)
  // _start - позиция поиска: false - от начала, true - от конца
  static int FindSubStrPosition(String _str, char _find_str, int _count = 1, bool _start = false);

  // Проверяет введенный параметр на попадание в отрезок от 0 до 1
  static bool CheckOneZeroParam(unsigned char _param, String _param_name);
};

#endif //__SERVICE_H_
