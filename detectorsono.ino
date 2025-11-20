#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

// --- Definições de Pinos e Sensores ---
// NOTA: O PPG_PIN 34 será lido, mas seu valor será usado como BASE de simulação.
#define PPG_PIN 34          // Pino do sensor PPG (Entrada Analógica)
#define GSR_PIN 35          // Pino do sensor GSR (Entrada Analógica)
#define SCL_PIN 26          // Pino SCL para display OLED
#define SDA_PIN 25          // Pino SDA para display OLED
#define BUZZER_PIN 4        // Pino para o buzzer

// --- Constantes de Filtro e Análise ---
#define WINDOW_SIZE 5       
#define RR_BUFFER_SIZE 10   
#define ADC_MAX_VAL 4095    // Valor máximo para ESP32 ADC (12 bits)
#define SIMULATED_BPM_BASE 75 // BPM base de simulação (Batimento normal)

// --- Timers de Amostragem ---
const long PPG_SAMPLE_INTERVAL = 20;     // Amostra PPG a cada 20ms (50Hz)
const long DISPLAY_UPDATE_INTERVAL = 1000; // Atualiza Display a cada 1000ms (1Hz)
unsigned long lastPPGSample = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastBeatTime = 0;

// --- Variáveis Globais de Análise de Sinal ---
int RR_data[RR_BUFFER_SIZE]; 
int rr_index = 0;
int currentBPM = SIMULATED_BPM_BASE; // Começa com o BPM base
int currentPRV = 40;                 // PRV base (simulado)
int currentStressLevel = 0;          

// --- Configuração do Display OLED ---
#define OLED_RESET -1
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

// --- Protótipos das Funções ---
int medianFilter(int data);
void simulateBeatDetection(int ppgBaseValue); // FUNÇÃO CHAVE MODIFICADA
void updateBPM_PRV();
int calculateStressLevel(int gsrFiltered);
bool isDrowsy(int bpm, int prv, int stressLevel);


void setup() {
  Serial.begin(115200); 
  pinMode(PPG_PIN, INPUT);
  pinMode(GSR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  Wire.begin(SDA_PIN, SCL_PIN);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Sistema Iniciado!");
  display.display();
  delay(1000);
  display.clearDisplay();
  // Inicializa o tempo da última batida
  lastBeatTime = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Amostragem Rápida do PPG (50Hz)
  if (currentMillis - lastPPGSample >= PPG_SAMPLE_INTERVAL) {
    lastPPGSample = currentMillis;
    
    // Leitura do valor fixo (157) para simular o baseline
    int ppgBaseValue = analogRead(PPG_PIN);
    
    // Lógica de Simulação de Batimento
    simulateBeatDetection(ppgBaseValue); 
  }

  // 2. Atualização Lenta da Análise, Display e Alerta (1Hz)
  if (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdate = currentMillis;

    // Recalcula BPM e PRV
    updateBPM_PRV();
    
    // Aquisição e cálculo do Stress Level
    int gsrData = analogRead(GSR_PIN);
    currentStressLevel = calculateStressLevel(medianFilter(gsrData));
    
    // Impressão no Serial Monitor (para debug)
    Serial.print("BPM: "); Serial.print(currentBPM); 
    Serial.print(" | PRV(RMSSD): "); Serial.print(currentPRV); 
    Serial.print(" | Stress: "); Serial.println(currentStressLevel);

    // Análise e detecção de sonolência
    if(isDrowsy(currentBPM, currentPRV, currentStressLevel)) {
      // Ativar o buzzer e exibir alerta
      digitalWrite(BUZZER_PIN, HIGH);
      display.clearDisplay();
      display.setTextSize(2); 
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0,0);
      display.println("ALERTA!");
      display.setTextSize(1);
      display.println("SONOLENCIA");
      display.println("DETECTADA!");
      display.display();
      delay(3000); 
      digitalWrite(BUZZER_PIN, LOW);
      display.clearDisplay();
    }
    
    // Exibição dos dados no display OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.print("BPM: "); display.println(currentBPM);
    display.print("PRV: "); display.println(currentPRV);
    display.print("Stress: "); display.println(currentStressLevel);
    display.display();
  }
}

// ====================================================================
// --- Implementação das Funções Otimizadas ---
// ====================================================================

// --- 1. Filtro de Mediana (Mantido) ---
int medianFilter(int data) {
  static int window[WINDOW_SIZE];
  static int currentIndex = 0; 
  window[currentIndex] = data; 
  currentIndex = (currentIndex + 1) % WINDOW_SIZE; 
  int tempWindow[WINDOW_SIZE];
  for (int i = 0; i < WINDOW_SIZE; i++) tempWindow[i] = window[i];
  // Bubble Sort simplificado
  for (int i = 0; i < WINDOW_SIZE - 1; i++) {
    for (int j = 0; j < WINDOW_SIZE - i - 1; j++) {
      if (tempWindow[j] > tempWindow[j + 1]) {
        int temp = tempWindow[j];
        tempWindow[j] = tempWindow[j + 1];
        tempWindow[j + 1] = temp;
      }
    }
  }
  return tempWindow[WINDOW_SIZE / 2];
}


// --- 2. Detecção de Batimento SIMULADA (Ajustada) ---
void simulateBeatDetection(int ppgBaseValue) {
  // A Lógica aqui SIMULA um batimento cardíaco, pois a entrada é fixa (157).
  // O valor 157 lido do pino PPG é ignorado no cálculo de tempo,
  // mas o código mantém a leitura para fins de estrutura.
  
  // Condições de Sonolência (para simular a queda do BPM)
  const int LOW_STRESS_THRESHOLD = 50; // Se o stress cair abaixo de 50
  const int DROWSY_BPM = 55; // Simula BPM caindo para 55 quando há sonolência
  const int NORMAL_BPM = SIMULATED_BPM_BASE;

  // Calcula o Intervalo R-R esperado para o BPM atual (60000 / BPM)
  int targetBPM = (currentStressLevel < LOW_STRESS_THRESHOLD) ? DROWSY_BPM : NORMAL_BPM;
  long targetRR = 60000 / targetBPM;
  
  // Se for hora de simular a próxima batida
  if (millis() - lastBeatTime >= targetRR) {
    long timeSinceLastBeat = millis() - lastBeatTime;
    
    // Armazena o Intervalo R-R simulado
    RR_data[rr_index] = timeSinceLastBeat;
    rr_index = (rr_index + 1) % RR_BUFFER_SIZE;
    
    lastBeatTime = millis();
  }
}

// --- 3. Cálculo de BPM e PRV (RMSSD) ---
void updateBPM_PRV() {
  // O cálculo é mantido, mas agora ele reflete o RR_data SIMULADO.
  long sumRR = 0;
  int count = 0;
  for (int i = 0; i < RR_BUFFER_SIZE; i++) {
    if (RR_data[i] > 0) { 
      sumRR += RR_data[i];
      count++;
    }
  }

  if (count >= RR_BUFFER_SIZE) {
    long averageRR = sumRR / count; 
    currentBPM = 60000 / averageRR; 

    // Cálculo do PRV (RMSSD)
    long sumOfSquaredDifferences = 0;
    for (int i = 0; i < RR_BUFFER_SIZE - 1; i++) {
      long diff = RR_data[i] - RR_data[i+1];
      sumOfSquaredDifferences += diff * diff;
    }

    if (RR_BUFFER_SIZE > 1) {
      long meanOfSquaredDifferences = sumOfSquaredDifferences / (RR_BUFFER_SIZE - 1);
      currentPRV = (int)sqrt(meanOfSquaredDifferences); 
    }
  } else {
    currentBPM = SIMULATED_BPM_BASE;
    currentPRV = 40; 
  }
}

// --- 4. Cálculo do Nível de Estresse (Condutância) ---
int calculateStressLevel(int gsrFiltered) {
  // Mapeia o valor do ADC (0-4095) para um nível de estresse (0-100)
  const int CONDUCTANCE_MAX_LEVEL = 100;

  // GSR é a leitura REAL (não simulada), então mantemos o mapeamento
  int excitationLevel = map(gsrFiltered, 0, ADC_MAX_VAL, 0, CONDUCTANCE_MAX_LEVEL);

  return constrain(excitationLevel, 0, CONDUCTANCE_MAX_LEVEL);
}


// --- 5. Detecção de Sonolência (Aprimorada) ---
bool isDrowsy(int bpm, int prv, int stressLevel) {
  // Ajustado para refletir as simulações
  const int LOW_BPM_THRESHOLD = 65;      
  const int LOW_STRESS_THRESHOLD = 50;   // Limiar para queda do BPM (visto em simulateBeatDetection)
  const int TYPICAL_PRV_MIN = 25; 

  bool lowExcitement = (stressLevel < LOW_STRESS_THRESHOLD);
  bool slowHeartRate = (bpm < LOW_BPM_THRESHOLD && bpm > 0); 
  bool lowVariability = (prv < TYPICAL_PRV_MIN && prv > 0);

  // Se o estresse estiver baixo, e o BPM simulado tiver caído
  if (lowExcitement && slowHeartRate) {
    return true;
  }
  
  // Se o estresse estiver muito baixo e a variabilidade também for baixa
  if (lowExcitement && lowVariability) {
      return true;
  }

  return false;
}
