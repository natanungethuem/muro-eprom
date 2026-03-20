#include "mqtt.h"

void mqtt::acessa_wifi_inicio() {
  delay(10);
  Serial.println("Conectando a wifi de inicio");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_inicio, pass_inicio);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    pisca(200, 0, 150, 0);
  }
  Serial.println("Conectado");

  //Duas piscadas verdes para indicar que conectou
  pisca(100, 150, 0, 0);
  pisca(100, 150, 0, 0);

}



void mqtt::acessa_wifi() {
  delay(10);
  Serial.println("Conectando a wifi");
  Serial.print("SSID: ");
  Serial.println(ssid_config);
  Serial.print("PASS: ");
  Serial.println(pass_config);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_config, pass_config);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    pisca(200, 0, 0, 150);
  }
  Serial.println("Conectado");
  //Duas piscadas verdes para indicar que conectou
  pisca(100, 150, 0, 0);
  pisca(100, 150, 0, 0);

}


void mqtt::busca_ip_externo() {
  WiFiClient client_wifi;
  if (!client_wifi.connect("api.ipify.org", 80)) {
    Serial.println("Falha em acessar 'api.ipify.org' !");
  }
  else {
    int timeout = millis() + 5000;
    client_wifi.print("GET /?format=text HTTP/1.1\r\nHost: api.ipify.org\r\n\r\n");
    while (client_wifi.available() == 0) {
      if (timeout - millis() < 0) {
        Serial.println(">>> Client Timeout !");
        client_wifi.stop();
        return;
      }
    }
    int size;
    while ((size = client_wifi.available()) > 0) {
      uint8_t* msg = (uint8_t*)malloc(size);
      size = client_wifi.read(msg, size);

      int j = size;
      while (msg[--j] != '\n');

      int cnt = 0;
      for (int i = j; i < size; i++)
        if ((msg[i] >= '0' && msg[i] <= '9') || msg[i] == '.')
          ip_externo[cnt++] = msg[i];
      ip_externo[cnt] = '\0';
      free(msg);
      Serial.println(ip_externo);
    }
  }
}



void mqtt::setDateTime() {
  // You can use your own timezone, but the exact time is not used at all.
  // Only the date is needed for validating the certificates.
  configTime(TZ_Europe_Berlin, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(100);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println();

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.printf("%s %s", tzname[0], asctime(&timeinfo));
}


void mqtt::reconnect() {
  // Loop enquanto não conecta
  while (!client->connected()) {
    Serial.print("Tentando estabelecer conecao MQTT…");
    String clientId = "ESP8266Client - Primeiro Muro"; //REVISAR - CADA ESP TERA QUE TER SEU "MAC"

    if (client->connect(clientId.c_str(), "muroboard", "Mur0board")) {
      Serial.println("Conectado");
    } else {
      Serial.print("Falha, rc = ");
      Serial.print(client->state());
      Serial.println(" Tentando novamente em 5 segundos");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
