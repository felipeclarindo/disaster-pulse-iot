#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

// Intervalo de tempo entre as leituras (em milissegundos)
long tempoAnterior = 0;
int intervaloLeitura = 1200;

// Pinos dos LEDs
const int ledVermelho = 23;
const int ledAmarelo = 19;
const int ledVerde = 18;

// Pino analógico simulado (use 34 no Wokwi)
const int pinoSensor = 34;

// Credenciais Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* senha = "";

// Informações do broker MQTT
const char* brokerMQTT = "broker.hivemq.com";
const char* nomeClienteMQTT = "MonitorNivelAgua";
const int portaMQTT = 1883;

// Objetos para conexão Wi-Fi e MQTT
WiFiClient clienteWiFi;
PubSubClient clienteMQTT(clienteWiFi);

// Função para conectar ao Wi-Fi
void conectarWiFi() {
  WiFi.begin(ssid, senha);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Conectado ao Wi-Fi!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// Callback para mensagens recebidas via MQTT
void tratarMensagem(char* topico, byte* carga, unsigned int tamanho) {
  String mensagemRecebida;

  for (int i = 0; i < tamanho; i++) {
    mensagemRecebida += (char)carga[i];
  }

  Serial.print("📩 Mensagem recebida do tópico ");
  Serial.print(topico);
  Serial.print(": ");
  Serial.println(mensagemRecebida);
}

// Função para conectar ao broker MQTT
void conectarMQTT() {
  while (!clienteMQTT.connected()) {
    Serial.print("🔄 Conectando ao broker MQTT... ");
    if (clienteMQTT.connect(nomeClienteMQTT)) {
      Serial.println("✅ Conectado com sucesso!");
      clienteMQTT.subscribe("NivelAgua/Esgoto");
    } else {
      Serial.print("❌ Falha. Código: ");
      Serial.print(clienteMQTT.state());
      Serial.println(" - Tentando novamente em 2s...");
      delay(2000);
    }
  }
}

// Setup inicial
void setup() {
  Serial.begin(9600);
  delay(3000); // Tempo para abrir monitor serial

  // Configura os pinos dos LEDs como saída
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVermelho, OUTPUT);

  // Apaga todos os LEDs inicialmente
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledAmarelo, LOW);
  digitalWrite(ledVermelho, LOW);

  // Conecta ao Wi-Fi e ao broker MQTT
  conectarWiFi();
  clienteMQTT.setServer(brokerMQTT, portaMQTT);
  clienteMQTT.setCallback(tratarMensagem);
}

// Loop principal
void loop() {
  // Reconecta se perder a conexão MQTT
  if (!clienteMQTT.connected()) {
    conectarMQTT();
  }
  clienteMQTT.loop(); // Mantém a conexão ativa

  long tempoAtual = millis();
  if (tempoAtual - tempoAnterior >= intervaloLeitura) {
    tempoAnterior = tempoAtual;

    // Simula leitura analógica do sensor
    int leituraAnalogica = analogRead(pinoSensor);

    // Converte para string e publica via MQTT
    char mensagemMQTT[10];
    sprintf(mensagemMQTT, "%d", leituraAnalogica);
    clienteMQTT.publish("NivelAgua/Esgoto", mensagemMQTT);

    Serial.print("📤 Nível de água enviado: ");
    Serial.println(leituraAnalogica);

    // Lógica para acender os LEDs com base na faixa de valores
    if (leituraAnalogica <= 1500) {
      // Nível BAIXO: LEDs verde aceso
      digitalWrite(ledVerde, HIGH);
      digitalWrite(ledAmarelo, LOW);
      digitalWrite(ledVermelho, LOW);
    }
    else if (leituraAnalogica <= 3000) {
      // Nível MÉDIO: LED amarelo aceso
      digitalWrite(ledVerde, LOW);
      digitalWrite(ledAmarelo, HIGH);
      digitalWrite(ledVermelho, LOW);
    }
    else {
      // Nível ALTO: LED vermelho aceso
      digitalWrite(ledVerde, LOW);
      digitalWrite(ledAmarelo, LOW);
      digitalWrite(ledVermelho, HIGH);
    }
  }
}
