#include "TrainCmd.h"

// Конструктор по умолчанию
TrainCmd::TrainCmd()
{
   direction = FORWARD;
   speed = 0;
   front_light = OFF;
   rear_light = OFF;
   front_buf_light = OFF;
   rear_buf_light = OFF;
   one_sound = OFF;
   two_sound = OFF;
   smoke = OFF;
}

// Конструктор команды
TrainCmd::TrainCmd(unsigned char _direction, unsigned char _speed, unsigned char _front_light, unsigned char _rear_light,
                   unsigned char _front_buf_light, unsigned char _rear_buf_light, unsigned char _one_sound, unsigned char _two_sound, unsigned char _smoke)
{
   direction = _direction;
   speed = _speed;
   front_light = _front_light;
   rear_light = _rear_light;
   front_buf_light = _front_buf_light;
   rear_buf_light = _rear_buf_light;
   one_sound = _one_sound;
   two_sound = _two_sound;
   smoke = _smoke;
}

// Перегрузка опреатора сравнения
bool TrainCmd::operator==(const TrainCmd _right)
{
   return direction == _right.direction &&
          speed == _right.speed &&
          front_light == _right.front_light &&
          rear_light == _right.rear_light &&
          front_buf_light == _right.front_buf_light &&
          rear_buf_light == _right.rear_buf_light &&
          one_sound == _right.one_sound &&
          two_sound == _right.two_sound &&
          smoke == _right.smoke;
}