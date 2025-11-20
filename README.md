 😴 Detector Inteligente de Sonolência (ESP32)

Descrição do Funcionamento: Explique o problema (fadiga ao volante) e como o sistema o resolve (monitorando sinais vitais para prever o risco). Destaque a simulação inteligente:

"Este sistema previne acidentes monitorando sinais fisiológicos. O diferencial do código é a Simulação Controlada pelo Estresse (GSR): o sistema correlaciona a queda no Nível de Estresse (GSR) com a desaceleração do Batimento Cardíaco (BPM) para disparar o alerta, tornando a detecção robusta contra alarmes falsos."

Instruções de Uso para Reprodução:

Como instalar as bibliotecas (Adafruit_GFX, Adafruit_SSD1306).

Como fazer o upload para o ESP32.

Instrução de Teste Chave: "Para testar a lógica de sonolência, manipule o slider do sensor GSR para simular um relaxamento profundo. O sistema reagirá com a queda do BPM e acionará o alerta no display e buzzer."
<img width="1875" height="786" alt="image" src="https://github.com/user-attachments/assets/ef74794c-b554-45c4-9f76-46cdf2922b29" />
<img width="1024" height="1536" alt="Fluxo" src="https://github.com/user-attachments/assets/23a871c9-3e74-402b-aa88-5d847c9d8430" />
