#include <EEPROM.h>

#include "mqtt.h"
#include "leds.h"

void grava_eeprom(int addr, byte* variavel){
  
}




void grava_eeprom_wifi() {
  EEPROM.begin(70);

  EEPROM.write(10, 'M');
  EEPROM.write(11, 'B');
  for (int i = 12; i < 40; i++)
    EEPROM.write(i, ssid_config[i - 12]);
  for (int i = 40; i < 70; i++)
    EEPROM.write(i, pass_config[i - 40]);
  EEPROM.commit();
}

bool le_eeprom_wifi() {
  return false;
  
  EEPROM.begin(70);

  if (EEPROM.read(10) == 'M' && EEPROM.read(11) == 'B') {
    for (int i = 12; i < 40; i++) {
      if (EEPROM.read(i) == '\0') break;
      ssid_config[i - 12] = EEPROM.read(i);
    }
    for (int i = 40; i < 70; i++) {
      if (EEPROM.read(i) == '\0') break;
      pass_config[i - 40] = EEPROM.read(i);
    }
    return true;
  }
  return false;
}

void grava_eeprom_leds() {
  EEPROM.begin(81);

  EEPROM.write(10, 'M');
  EEPROM.write(11, 'B');
  for (int i = 72; i < 81; i++) {
    EEPROM.write(i, config_leds[i - 72]);
    EEPROM.commit();
  }
}

bool le_eeprom_leds() {
  
  return false;
  
  EEPROM.begin(81);

  if (EEPROM.read(70) == 'M' && EEPROM.read(71) == 'B') {
    for (int i = 72; i < 81; i++)
      config_leds[i - 72] = EEPROM.read(i);
    return true;
  }
  return false;
}



void inicio(bool reconectar) {
  if (reconectar) {
    Serial.print("WiFi Salvo na EEPROM? ");
    bool wifi_salvo = le_eeprom_wifi();
    Serial.println(wifi_salvo);

    if (wifi_salvo) acessa_wifi();
    else acessa_wifi_inicio();
  }

  Serial.println("Conectando no servidor MQTT");
  reconnect();

  Serial.print("Iniciando Subscricao em ");
  busca_ip_externo();
  client->subscribe(ip_externo);
  Serial.println(ip_externo);
}

void setup() {
  Serial.begin(9600);

  Serial.println("Inicializando fita de leds");
  strip.Begin();
  strip.Show();

  strip_foot.Begin();
  strip_foot.Show();

  Serial.print("WiFi Salvo na EEPROM? ");
  bool wifi_salvo = le_eeprom_wifi();
  Serial.println(wifi_salvo);

  if (wifi_salvo) acessa_wifi();
  else acessa_wifi_inicio();

  Serial.println("Inicializando Little FS");
  LittleFS.begin();
  setDateTime();

  Serial.println("Lendo Certificados");
  int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
  Serial.printf("Numero de certificados lidos: %d\n", numCerts);
  if (numCerts == 0) {
    Serial.println("Nenhum certificado encontrado. Voce instalou os certificados usando o LittleFS?");
    return; // Can't connect to anything w/o certs!
  }

  BearSSL::WiFiClientSecure *bear = new BearSSL::WiFiClientSecure();
  // Integrate the cert store with this connection
  bear->setCertStore(&certStore);

  Serial.println("Cria os clientes com base no ceritificado");
  client = new PubSubClient(*bear);

  Serial.println("Define o servidor e a funcao de CallBack");
  client->setServer(mqtt_server, 8883);
  client->setCallback(callback);

  inicio(false);
}


void callback(char* topic, byte * payload, unsigned int length) {
  Serial.print("Messagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  //Serial.println((char)payload);

  //Testa se o tópico é o IP de configuração
  if ((char)topic[0] >= '0' && (char)topic[0] <= '9') {
    Serial.println("Entrando na configuracao pelo IP");
    switch ((char)payload[0]) {

      case 's': //define a wifi
        Serial.println("Alterando o nome da WiFi");
        for (int i = 1; i < length; i++)
          ssid_config[i - 1] = (char)payload[i];
        grava_eeprom_wifi();
        client->publish("conexao", "OK");
        break;

      case 'p': //define a senha
        Serial.println("Alterando a Senha");
        for (int i = 1; i < length; i++)
          pass_config[i - 1] = (char)payload[i];
        grava_eeprom_wifi();
        client->publish("conexao", "OK");
        break;

      case 'w': //Conecta na nova wifi
        inicio(1);
        break;

      case 'c': //configuracao
        Serial.println("Altera a configuracao dos leds");
        int i;
        if ((char)payload[1] == 'i') i = 0; //inicio
        if ((char)payload[1] == 'b') i = 3; //boulder
        if ((char)payload[1] == 't') i = 6; //top
        i=0;
        for (int j = 0; j < 3; j++, i++)
          config_leds[i] = (int)payload[j + 2];
        grava_eeprom_leds();
        client->publish("conexao", "OK");
        break;

      case 'a': //acionamento
        {
          for (int i = 0; i < NUMPIXELS; i++)
            strip.SetPixelColor(i, black);
          strip.Show();

          for (int i = 0; i < NUMPIXELS_FOOT; i++)
            strip_foot.SetPixelColor(i, black);
          strip_foot.Show();
          
          Serial.println("Aciona led");
          char situacao;
          int numero_led;
          int s = 0;
          for (int i = 1; i < length; i += 2) {
            situacao = (char)payload[i];  //'d' desligado 'i' inicio 'b' boulder 't' top
            numero_led = (int)payload[i + 1];    //0 a 255

            Serial.print(situacao);
            Serial.print(" ");
            Serial.println((int)numero_led);

            numero_led--;

            RgbColor inicio(config_leds[0], config_leds[1], config_leds[2]);
            RgbColor boulder(config_leds[3], config_leds[4], config_leds[5]);
            RgbColor top(config_leds[6], config_leds[7], config_leds[8]);

            if (situacao == 'i') strip.SetPixelColor(numero_led, inicio);
            if (situacao == 'b') strip.SetPixelColor(numero_led, boulder);
            if (situacao == 't') strip.SetPixelColor(numero_led, top);
            //if (situacao == 'd') strip.SetPixelColor(numero_led, black);

            if (situacao == 'j') strip.SetPixelColor(numero_led + 120, inicio);
            if (situacao == 'c') strip.SetPixelColor(numero_led + 120, boulder);
            if (situacao == 'u') strip.SetPixelColor(numero_led + 120, top);
            //if (situacao == 'e') strip.SetPixelColor(numero_led + 120, black);

            if (situacao == 'k') strip_foot.SetPixelColor(numero_led + 5, inicio);
            if (situacao == 'd') strip_foot.SetPixelColor(numero_led + 5, boulder);
            if (situacao == 'v') strip_foot.SetPixelColor(numero_led + 5, top);
            //if (situacao == 'f') strip.SetPixelColor(numero_led + 120, black);
            
          }
          strip.Show();
          strip_foot.Show();
          client->publish("conexao", "OK");
          break;
        }

      case 'd': //apaga todos os leds
        Serial.println("Apaga todos os leds");
        for (int i = 0; i < NUMPIXELS; i++)
          strip.SetPixelColor(i, black);
        strip.Show();
        client->publish("conexao", "OK");
        break;

      case 'v':
        Serial.println("Respondendo conexao");
        client->publish("conexao", "OK");
    }
  }
}


void loop() {

  static unsigned long lastMsg = 0;

  if (!client->connected()) {
    reconnect();
  }
  client->loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
  }
}
