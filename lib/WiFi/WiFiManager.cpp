#include <WiFiManager.h>

// Конструктор, инициализирующий WiFi и UDP
WiFiManager::WiFiManager(IPAddress _ap_ip, IPAddress _gateway_ip, IPAddress _network_mask)
    : ap_ip(_ap_ip), gateway_ip(_gateway_ip), network_mask(_network_mask), max_packet_size(9), clients(0)
{
}

// Инициализация WiFi
void WiFiManager::InitWiFi(const char *_ap_name, const char *_ap_psd)
{
  // Сохранение названия и пароля
  ap_name = _ap_name;
  ap_psd = _ap_psd;

  // Настройка WiFi
  // Включение режима точки доступа
  WiFi.mode(WIFI_AP);
  // Установка названия и пароля точки доступа
  WiFi.softAP(ap_name, ap_psd);
  // Установка статического IP адреса
  WiFi.softAPConfig(ap_ip, gateway_ip, network_mask);
  // Сообщение о старте WiFi
  Serial.printf("Точка доступа запущена.\nIP адрес AP в локальной сети: %s\n", ap_ip.toString());
}

// Инициализация DNS
void WiFiManager::InitDns(String _dns_name)
{

  dns_name = _dns_name;
  Serial.println("Запуск DNS .");
  while (!MDNS.begin(dns_name))
  {
    Serial.print(".");
    delay(200);
  }
  Serial.printf("\nDNS с именем %s запущен\n", dns_name);
}

// Инициализация UDP
void WiFiManager::InitUDP(unsigned short _local_port)
{
  // Сохранение номера порта
  local_port = _local_port;
  udp.begin(local_port);

  Serial.printf("UDP запущен.\nПрослушивается порт: %d\n", local_port);
}

// Отображение информации о пришедшем пакете
void WiFiManager::ShowPacketInfo()
{
  // Выводи информации о пакете
  Serial.println("==========");
  Serial.print("IP отправителя: ");
  Serial.println(udp.remoteIP());
  Serial.print("Порт отправителя: ");
  Serial.println(udp.remotePort());
  Serial.print("Количество доступных байт: ");
  Serial.println(packet_available);
  Serial.print("Длина пришедшего пакета: ");
  Serial.println(packet_length);

  for (uint8_t i = 0; i < packet_length; i++)
  {
    Serial.print("Байт №");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(incoming_packet[i], DEC);
  }

  Serial.println("==========");
}

// Получить пакет
unsigned char WiFiManager::CheckPacket()
{
  // Проверка, что есть доступный пакет для чтения
  packet_available = udp.parsePacket();

  // Если в буфере есть пакет для чтения
  if (packet_available)
  {
    Serial.print("\nДоступно для чтения:");
    Serial.println(packet_available);
    Serial.println();
    // Получение длины читаемого пакета и сохранение его в буфер
    packet_length = udp.read(incoming_packet, 255);
    last_client_ip = udp.remoteIP();
    ShowPacketInfo();

    // Завершение строки 0 для корректной печати в Serial
    incoming_packet[packet_length] = 0;
    return packet_length;
  }
  return 0;
}

// Получить пакет
char *WiFiManager::GetPacket()
{
  return incoming_packet;
}

// Отправка пакета
void WiFiManager::SendPacket(IPAddress _recipient_ip, String _message, unsigned short _port)
{
  if (_recipient_ip == IPAddress(0, 0, 0, 0))
    return;

  if (clients > 0)
  {
    // Подготовка к отправке пакета по указанному ip
    udp.beginPacket(_recipient_ip, local_port);

    for (size_t i = 0; i < _message.length(); i++)
    {
      udp.write(_message[i]);
    }

    Serial.printf("Sended\n");
    Serial.printf("Sender IP: %s\n", _recipient_ip.toString());

    // Каждый раз отправляется целая команда
    udp.endPacket();
    udp.flush();
  }
}

// Очистка буфера принятого UDP
void WiFiManager::ClearUDPBuffer()
{
  udp.flush();
}

// Получение IP адреса точки доступа (депо)
IPAddress WiFiManager::GetAPIP()
{
  return ap_ip;
}

// Получение порта работы UDP
unsigned short WiFiManager::GetLocalPort()
{
  return local_port;
}

// Сравнение количества клиентов
// При их изменении отображение их количества
void WiFiManager::CheckCountClients()
{
  if (clients != WiFi.softAPgetStationNum())
  {
    clients = WiFi.softAPgetStationNum();
    Serial.printf("Количество клиентов: %d\n", clients);
  }
}

// Получить IP адрес последнего подключенного клиента
IPAddress WiFiManager::GetLastIP()
{
  if (last_client_ip)
    return last_client_ip;
  else
    return IPAddress(0, 0, 0, 0);
}
