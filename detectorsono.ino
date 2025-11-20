#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h> // <--- BIBLIOTECA NOVA NECESSÁRIA
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// PINOS 
#define PULSE_PIN 34
#define BUZZER_PIN 4

// VARIÁVEIS
int batimentos = 0;
unsigned long ultimoEnvio = 0;

// ---------------------
// CONFIGURAÇÃO WIFI
// ---------------------
// Se for Wokwi:
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Se for Físico (Mackenzie):
// const char* ssid = "mackteste";
// const char* password = "senha";

// ---------------------
// CONFIGURAÇÃO HIVEMQ CLOUD
// ---------------------
// Coloque a URL que você copiou do painel (sem "http://")
const char* mqtt_server = "c9ed373e90b2485297838c11e6fad383.s1.eu.hivemq.cloud"; 
const int   mqtt_port   = 8883; // Porta segura é 8883, não 1883!

// Usuário que você criou na aba "Access Management"
const char* mqtt_user = "babi00";
const char* mqtt_pass = "Babi0022@";

// CLIENTE SEGURO
WiFiClientSecure espClient; // <--- Mudança aqui: Secure
PubSubClient client(espClient);

void conectarWiFi() {
  Serial.print("Conectando ao WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao HiveMQ Cloud... ");
    
    // O HiveMQ Cloud exige um ID de cliente único. Geramos um randômico:
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Conectado!");
      client.subscribe("bpm/comando");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" (Verifique URL, User ou Senha)");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUZZER_PIN, OUTPUT);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Erro OLED"));
    for(;;);
  }
  display.clearDisplay();
  display.display();

  conectarWiFi();

  // --- CONFIGURAÇÃO DE SEGURANÇA ---
  // Isso permite conectar sem baixar o certificado CA (menos seguro, mas funciona rápido para testes)
  espClient.setInsecure(); 
  
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Lógica de leitura
  int leitura = analogRead(PULSE_PIN);
  batimentos = map(leitura, 0, 4095, 60, 140);

  // Lógica de envio (a cada 3s)
  if (millis() - ultimoEnvio > 3000) {
    ultimoEnvio = millis();
    
    char msg[10];
    sprintf(msg, "%d", batimentos);
    
    // Publica no tópico "bpm/leituras"
    client.publish("bpm/leituras", msg);
    Serial.print("BPM: ");
    Serial.println(msg);
    
    // Atualiza OLED
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,10);
    display.print("BPM: ");
    display.println(msg);
    display.display();
  }
  
  // Alarme simples
  if(batimentos > 120) tone(BUZZER_PIN, 1000);
  else noTone(BUZZER_PIN);

  delay(100);
}
