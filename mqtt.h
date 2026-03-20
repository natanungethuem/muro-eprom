#include "Arduino.h"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include <TZ.h>
#include <FS.h>
#include <LittleFS.h>
#include <CertStoreBearSSL.h>

struct WIFI {
  char[32] ssid;
  char[32] senha;
}

class mqtt
{
  public:
    //Variaveis
    WIFI wifi_conf;

    //Funcoes
    void reconnect();

  private:
    //variaveis
    const WIFI wifi_inicio = {"Muro","Muro4000"};
    char ip_externo[16];
    const char* mqtt_server = "b4a5c2c5ebbd4e9a87e07ab57b7fb677.s1.eu.hivemq.cloud";

    BearSSL::CertStore certStore;

    WiFiClientSecure espClient;
    PubSubClient * client;

    //funcoes
    void acessa_wifi_inicio();
    void acessa_wifi();
    void busca_ip_externo();
    void setDateTime();
    
    
};
