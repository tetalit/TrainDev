// #include "TrafficCmd.h"

// // Конструктор по умолчанию
// TrafficCmd::TrafficCmd()
// {
//   trafficLeds[0] = trafficLeds[1] = trafficLeds[2] = trafficLeds[3] = 0;
// }

// // Конструктор команды
// TrafficCmd::TrafficCmd(unsigned char _first_led, unsigned char _second_led, unsigned char _third_led, unsigned char _fourth_led)
// {
//   trafficLeds[0] = _first_led;
//   trafficLeds[1] = _second_led;
//   trafficLeds[2] = _third_led;
//   trafficLeds[3] = _fourth_led;
// }

// // Перегрузка опреатора сравнения
// bool TrafficCmd::operator==(const TrafficCmd _right)
// {
//   return trafficLeds[0] == _right.trafficLeds[0] &&
//          trafficLeds[1] == _right.trafficLeds[1] &&
//          trafficLeds[2] == _right.trafficLeds[2] &&
//          trafficLeds[3] == _right.trafficLeds[3];
// }