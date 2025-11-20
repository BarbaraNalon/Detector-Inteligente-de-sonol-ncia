Detector Inteligente de Fadiga e Estresse (ESP32)
Descrição do ProjetoEste projeto consiste no desenvolvimento de um sistema de Monitoramento Fisiológico Inteligente de Baixo Custo, implementado na plataforma ESP32 e utilizando arquitetura Internet das Coisas (IoT). 

O objetivo primário é prevenir acidentes em ambientes de mobilidade e risco ocupacional, detectando proativamente os estados de fadiga profunda e estresse agudo do indivíduo.O sistema utiliza a leitura de dois sinais vitais complementares:
GSR (Resposta Galvânica da Pele / Nível de Estresse): Indicador de ativação do Sistema Nervoso Simpático.
PPG (Fotopletismografia / BPM): Para calcular a Frequência Cardíaca.🧠

Simulação Inteligente (GSR-Driven)O diferencial do código reside na Lógica de Simulação Inteligente (GSR-Driven). Para validar a detecção em um ambiente de simulação com limitação de hardware (entrada PPG fixa), o firmware implementa a seguinte correlação robusta:"O sistema correlaciona a queda no Nível de Estresse (GSR) (indicando relaxamento profundo) com a desaceleração do Batimento Cardíaco (BPM) para disparar o alerta, modulando o Intervalo R-R interno. Isso torna a detecção robusta contra alarmes falsos, pois o alerta só é disparado quando há evidência simultânea de baixo estresse e ritmo cardíaco lento."

Componentes e Arquitetura

O protótipo foi desenvolvido e validado na plataforma de simulação Wokwi.

Comunicação IoT

A transmissão dos dados é feita via protocolo MQTT (Message Queuing Telemetry Transport).
Broker Utilizado: HiveMQ Cloud.

Tópicos de Publicação: /bpm/leituras (valor de BPM e Estresse) e /bpm/status (ALERTA / NORMAL).

Instruções de Uso para ReproduçãoPara reproduzir este projeto, utilize o Arduino IDE e o ESP32 DevKit V1.1. Instalação de BibliotecasEste projeto requer as seguintes bibliotecas para gerenciar o display OLED e a comunicação MQTT.

Abra o Arduino IDE.
Vá em Sketch $\rightarrow$ Incluir Biblioteca $\rightarrow$ Gerenciar Bibliotecas...

Pesquise e instale as versões mais recentes das seguintes bibliotecas:Adafruit GFX LibraryAdafruit SSD1306PubSubClient2. 
Configuração de Rede e BrokerAntes de fazer o upload, você deve configurar o código com suas credenciais de Wi-Fi e MQTT.

Abra o arquivo principal do projeto (.ino).

Localize a seção de CONFIGURAÇÃO WIFI e MQTT e preencha as variáveis:
C++
const char * ssid = "SUA_REDE_WIFI";
const char * senha = "SUA_SENHA_WIFI";
const char * mqtt_server = "SEU_BROKER_HIVEMQ.cloud"; // Ex: c9ed373e90b2485297838c11e6fad8993.s1.eu.hivemq.cloud
const int mqtt_port = 8883;
const char * mqtt_user = "SEU_USUARIO"; 
const char * mqtt_pass = "SUA_SENHA_MQTT";

Obs.: Os dados do broker HiveMQ (URL e Porta) podem ser encontrados no painel de Visão Geral do seu cluster.3. Upload para o ESP32

No Arduino IDE, selecione a placa correta em Ferramentas $\rightarrow$ Placa $\rightarrow$ ESP32 WEMOS D1 MINI ESP32 (ou similar).
Selecione a porta serial correta.
Clique em Sketch $\rightarrow$ Upload.

Instrução de Teste Chave (Lógica GSR-Driven)Para testar a lógica de detecção de fadiga (que é o cerne do projeto), utilize o ambiente Wokwi onde o slider do sensor GSR (simulado) está ativo:
Estado Normal: Mantenha o slider do sensor GSR na posição média ou alta (ex: Estresse > 50).

O display mostrará NORMAL.Teste de Fadiga/Sonolência: Manipule o slider do sensor GSR para simular um relaxamento profundo, movendo-o abaixo do limiar de 50.Reação do Sistema: O firmware reagirá a essa queda no Estresse simulando uma queda no BPM (de ~75 BPM para ~55 BPM), acionando o alerta no display (FADIGA DETECTADA) e o buzzer (tom grave).

<img width="1336" height="630" alt="image" src="https://github.com/user-attachments/assets/4ad72d31-6a0f-4471-9ffb-a828ef9a80f1" />
<img width="1300" height="669" alt="Captura de tela 2025-11-20 182431" src="https://github.com/user-attachments/assets/1c67e5f6-7f11-44e1-a6ba-9c09165c5eba" />
<img width="1332" height="625" alt="image" src="https://github.com/user-attachments/assets/99ad09d6-f1ac-46b7-b230-2ff18fb667ed" />



