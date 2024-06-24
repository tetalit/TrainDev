#pragma once
#ifndef __TRAIN_CMD_H_
#define __TRAIN_CMD_H_

#include "DefineStorage.h"

struct TrainCmd
{
public:
  // Направление движения
  unsigned char direction;
  // Скорость поезда (0-)
  unsigned char speed;
  // Передний свет
  unsigned char front_light;
  // Задний свет
  unsigned char rear_light;
  // Передние буферные огни
  unsigned char front_buf_light;
  // Задние буферные огни
  unsigned char rear_buf_light;
  // Звук 1
  unsigned char one_sound;
  // Звук 2
  unsigned char two_sound;
  // Дымогенератор
  unsigned char smoke;

  // Конструктор по умолчанию
  TrainCmd();
  // Конструктор команды
  TrainCmd(unsigned char _direction, unsigned char _speed, unsigned char _front_light, unsigned char _rear_light,
           unsigned char _front_buf_light, unsigned char _rear_buf_light, unsigned char _one_sound, unsigned char _two_sound, unsigned char _smoke);
  // Перегрузка опреатора сравнения
  bool operator==(const TrainCmd _right);
};

#endif //__TRAIN_CMD_H_
