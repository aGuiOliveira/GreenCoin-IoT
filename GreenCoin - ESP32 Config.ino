#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Configurações - variáveis editáveis
const char* default_SSID = "Wokwi-GUEST"; // Nome da rede Wi-Fi, importante a sua rede wi-fi ter suporte a dispositivos de 2,4GHz devido ao ESP32 utilizar essa frequência 
const char* default_PASSWORD = ""; // Senha da rede Wi-Fi
const char* default_BROKER_MQTT = "191.232.172.130"; // IP do Broker MQTT - IP da VM da Azure
const int default_BROKER_PORT = 1883; // Porta do Broker MQTT
const char* default_TOPICO_SUBSCRIBE = "/TEF/device010/cmd"; // Tópico MQTT de escuta
const char* default_TOPICO_PUBLISH_1 = "/TEF/device010/attrs/temp"; // Tópico MQTT de envio de informações para Broker
const char* default_TOPICO_PUBLISH_2 = "/TEF/device010/attrs/hum"; // Tópico MQTT de envio de informações para Broker
const char* default_TOPICO_PUBLISH_3 = "/TEF/device010/attrs/energy"; // Tópico MQTT de envio de informações para Broker
const char* default_ID_MQTT = "fiware_001"; // ID MQTT
// Declaração da variável para o prefixo do tópico
const char* topicPrefix = "device010";

// Variáveis para configurações editáveis
char* SSID = const_cast<char*>(default_SSID);
char* PASSWORD = const_cast<char*>(default_PASSWORD);
char* BROKER_MQTT = const_cast<char*>(default_BROKER_MQTT);
int BROKER_PORT = default_BROKER_PORT;
char* TOPICO_SUBSCRIBE = const_cast<char*>(default_TOPICO_SUBSCRIBE);
char* TOPICO_PUBLISH_1 = const_cast<char*>(default_TOPICO_PUBLISH_1);
char* TOPICO_PUBLISH_2 = const_cast<char*>(default_TOPICO_PUBLISH_2);
char* TOPICO_PUBLISH_3 = const_cast<char*>(default_TOPICO_PUBLISH_3);
char* ID_MQTT = const_cast<char*>(default_ID_MQTT);

//Variáveis dos componentes
#define DHTTYPE DHT22
#define DHTPIN  4
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient MQTT(espClient);
char EstadoSaida = '0';

void initSerial() {
    Serial.begin(115200);
}

void initWiFi() {
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    reconectWiFi();
}

void initMQTT() {
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
}

void setup() {
    initSerial();
    initWiFi();
    initMQTT();
    delay(5000);
}

void loop() {
    VerificaConexoesWiFIEMQTT();
    handleTemperatureandHumidity(); // Função para ler e escrever a temperatura e humidade
    handleEnergy(); // Função para ler e escrever a energia
    MQTT.loop();
    delay(1000);
}

void reconectWiFi() {
    if (WiFi.status() == WL_CONNECTED)
        return;
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

void VerificaConexoesWiFIEMQTT() {
    if (!MQTT.connected())
        reconnectMQTT();
    reconectWiFi();
}

void reconnectMQTT() {
    while (!MQTT.connected()) {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE);
        } else {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Haverá nova tentativa de conexão em 2s");
            delay(2000);
        }
    }
}

void handleTemperatureandHumidity() {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    String temp = String(temperature);
    String hum = String(humidity);
    Serial.println("Temperatura: " + temp + "°C");
    Serial.println("Humidade: " + hum + "%");
    MQTT.publish(TOPICO_PUBLISH_1, temp.c_str());
    MQTT.publish(TOPICO_PUBLISH_2, hum.c_str());
}

void handleEnergy() {
  const int potPin = 34;
  char msgBuffer[4];
  int sensorValue = analogRead(potPin);
  int energy = map(sensorValue, 0, 4095, 0, 100);
  String mensagem = String(energy);
  Serial.print("Valor da energia: ");
  Serial.println(mensagem.c_str());
  MQTT.publish(TOPICO_PUBLISH_3, mensagem.c_str());
}
