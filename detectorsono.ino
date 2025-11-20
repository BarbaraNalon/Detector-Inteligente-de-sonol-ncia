#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h> 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); 

// --- PINOS CORRIGIDOS ---
#define OLED_SDA 25 
#define OLED_SCL 26 

#define PULSE_PIN 34  // Sensor de Pulso (BPM)
#define GSR_PIN 35    // NOVO: Potenciômetro para simular Estresse/GSR
#define BUZZER_PIN 4 

// VARIÁVEIS DE CONTROLE
int batimentos = 0;
int nivelEstresse = 0; // Novo
unsigned long ultimoEnvio = 0;

// CONFIGURAÇÃO WIFI e MQTT (Mantidas)
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "c9ed373e90b2485297838c11e6fad383.s1.eu.hivemq.cloud"; 
const int   mqtt_port   = 8883; 
const char* mqtt_user = "babi00";
const char* mqtt_pass = "Babi0022@"; // <<< INSIRA SUA SENHA AQUI!!!

WiFiClientSecure espClient; 
PubSubClient client(espClient);

// Funções conectarWiFi e reconnect (Mantidas)
void conectarWiFi() {
  Serial.print("Conectando ao WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
    display.clearDisplay(); display.setCursor(0, 0); display.println("Conectando WiFi..."); display.display();
  }
  Serial.println("\nWiFi conectado!");
}

void reconnect() {
  display.clearDisplay(); display.setTextSize(1); display.setCursor(0, 0);
  display.println("Conexao perdida!"); display.println("Tentando MQTT..."); display.display();
  
  while (!client.connected()) {
    Serial.print("Conectando ao HiveMQ Cloud... ");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX); 

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Conectado!");
      client.subscribe("bpm/comando");
      display.clearDisplay(); display.setCursor(0, 0); display.println("MQTT Conectado!"); display.display();
      delay(1000); 
    } else {
      Serial.print("Falha, rc="); Serial.print(client.state()); Serial.println(" (Tentando em 5s)");
      display.setCursor(0, 20); display.print("Erro: "); display.println(client.state()); display.display();
      delay(5000);
    }
  }
}

// -----------------------------------
// SETUP
// -----------------------------------
void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PULSE_PIN, INPUT); 
  pinMode(GSR_PIN, INPUT); // Configura o novo pino

  Wire.begin(OLED_SDA, OLED_SCL); 
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Falha ao iniciar OLED"); for(;;);
  }

  display.clearDisplay(); display.setTextSize(1); display.setTextColor(WHITE);
  display.setCursor(0, 0); display.println("Iniciando Sistema..."); display.display();

  conectarWiFi();
  espClient.setInsecure(); 
  client.setServer(mqtt_server, mqtt_port);
  
  display.clearDisplay(); display.setCursor(0, 0); display.println("Pronto para Operar!"); display.display();
  delay(1000);
}

// -----------------------------------
// LOOP PRINCIPAL (LÓGICA AVANÇADA DE DECISÃO)
// -----------------------------------
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // --- LEITURAS E CONVERSÕES ---
  int leituraRawPulse = analogRead(PULSE_PIN);
  int leituraRawGSR = analogRead(GSR_PIN);

  // Converte Pulse para BPM (Faixa Fisiológica)
  batimentos = map(leituraRawPulse, 0, 4095, 60, 140); 
  
  // Converte GSR para Nível de Estresse (0-100)
  nivelEstresse = map(leituraRawGSR, 0, 4095, 0, 100); 

  // --- DEFINIÇÃO DOS GATILHOS (CRITÉRIO DE DECISÃO CIENTÍFICO) ---
  // Gatilho de Fadiga: Coração lento (BPM < 65) E Mente relaxada (GSR < 50)
  bool alertaSono = (batimentos <= 65) && (nivelEstresse <= 50); 
  
  // Gatilho de Estresse: Coração acelerado (BPM > 100) E Mente excitada (GSR > 90)
  bool alertaEstresse = (batimentos >= 100) && (nivelEstresse >= 90); 
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  // ----------------------------------------------------
  // LÓGICA DO DISPLAY E BUZZER
  // ----------------------------------------------------
  if (alertaSono) {
    // MODO ALERTA DE FADIGA/SONO
    tone(BUZZER_PIN, 1000); 
    display.println("!!! ALERTA !!!");
    display.setTextSize(1);
    display.setCursor(0, 30);
    display.print("FADIGA (E:");
    display.print(nivelEstresse);
    display.println(")");

  } else if (alertaEstresse) {
    // MODO ALERTA DE ESTRESSE
    tone(BUZZER_PIN, 2000); 
    display.println("!! ATENCAO !!");
    display.setTextSize(1);
    display.setCursor(0, 30);
    display.print("ESTRESSE (E:");
    display.print(nivelEstresse);
    display.println(")");

  } else {
    // MODO NORMAL
    noTone(BUZZER_PIN); 
    display.println("BPM NORMAL");
    display.setTextSize(1);
    display.setCursor(0, 30);
    display.print("Status OK (E:");
    display.print(nivelEstresse);
    display.println(")");
  }

  // Mostra o BPM atual sempre
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print("BPM: ");
  display.println(batimentos);
  display.display();


  // ----------------------------------------------------
  // ENVIO MQTT (Atualizado para 3 critérios de status)
  // ----------------------------------------------------
  if (millis() - ultimoEnvio > 3000) {
    ultimoEnvio = millis();

    // 1. Envia o valor do BPM e Estresse (Para o HiveMQ)
    char mensagem[30];
    sprintf(mensagem, "%d,%d", batimentos, nivelEstresse);
    client.publish("bpm/leituras", mensagem); 

    // 2. Envia o Status DETALHADO (FADIGA, ESTRESSE ou NORMAL)
    if(alertaSono) {
        client.publish("bpm/status", "FADIGA_SONO"); 
    } else if (alertaEstresse) {
        client.publish("bpm/status", "ESTRESSE_ALTO"); 
    } else {
        client.publish("bpm/status", "NORMAL"); 
    }

    Serial.print("Dados enviados: BPM="); Serial.print(batimentos);
    Serial.print(", Estresse="); Serial.println(nivelEstresse);
  }

  delay(100); 
}
