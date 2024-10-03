#ifndef __WIFI_MANAGER_H_
#define __WIFI_MANAGER_H_

#include <WiFi.h>
#include <Udp.h>
#include <ESPmDNS.h>

class WiFiManager
{
private:
  // Локальный порт
  unsigned short local_port;
  // Максимальный размер пакета
  const unsigned short max_packet_size;
  // Название точки доступа
  const char *ap_name;
  // Пароль точки доступа
  const char *ap_psd;
  // DNS сервера
  String dns_name;
  // Количество клиентов
  unsigned char clients;
  // IP адрес станции
  IPAddress ap_ip;
  // IP адрес шлюза
  IPAddress gateway_ip;
  // Маска
  IPAddress network_mask;
  // Для работы с UDP
  WiFiUDP udp;
  // Входящий пакет
  char incoming_packet[255];
  // Размер очереди пакетов UDP
  unsigned char packet_available = 0;
  // Длина считываемого пакета
  unsigned char packet_length = 0;
  // IP адрес последнего клиента
  IPAddress last_client_ip;

public:
  // Конструктор, инициализирующий WiFi и UDP
  WiFiManager(IPAddress _ap_ip = IPAddress(192, 168, 0, 1),
              IPAddress _gateway_ip = IPAddress(192, 168, 0, 1),
              IPAddress _network_mask = IPAddress(255, 255, 252, 0));
  // Инициализация WiFi
  void InitWiFi(const char *_ap_name = "TrainAccessPoint", const char *_ap_psd = "123456789");
  // Инициализация DNS
  void InitDns(String _dns_name);
  // Инициализация UDP
  void InitUDP(unsigned short _port = 4210);
  // Отображение информации о пришедшем пакете
  void ShowPacketInfo();
  // Проверить наличие входящего пакета пакет
  unsigned char CheckPacket();
  // Получить пакет
  char *GetPacket();
  // Отправка пакета
  void SendPacket(IPAddress _recipient_ip, String _message, unsigned short _port = 4210);
  // Очистка буфера принятого UDP
  void ClearUDPBuffer();
  // Получение IP адреса точки доступа (депо)
  IPAddress GetAPIP();
  // Получение порта работы UDP
  unsigned short GetLocalPort();
  // Сравнение количества клиентов
  // При их изменении отображение их количества
  void CheckCountClients();
  // Получить IP адрес последнего подключенного клиента
  IPAddress GetLastIP();
  void SenderIP(IPAddress senderIP);
};

#endif //__WIFI_MANAGER_H_
