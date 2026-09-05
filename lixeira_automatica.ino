/*
  LIXEIRA AUTOMATICA COM ACIONAMENTO POR PROXIMIDADE
  Firmware de controle embarcado

  Disciplina...: Robotica
  Plataforma...: Arduino Uno
  Linguagem....: C++ (framework Arduino)
  Revisao......: C

  DESCRICAO
    Implementa o controle de uma tampa acionada por atuador a partir da
    deteccao de proximidade por ultrassom. O sistema opera como uma maquina
    de estados finitos de dois estados, FECHADA e ABERTA, com transicoes
    determinadas por um limiar de distancia e por temporizacao nao
    bloqueante.

  ARQUITETURA DE ALIMENTACAO
    Dominio unico. O cabo USB alimenta a placa, e a placa distribui as
    tensoes aos demais componentes:
      Pino 5V   -> atuador SG90 e sensor HC-SR04-P
      Pino GND  -> retorno comum de ambos

    Diferente da plataforma anterior, o Arduino Uno opera em nivel logico
    de 5 V. O pino ECHO do sensor devolve 5 V, compativel com as entradas
    da placa, o que dispensa o divisor resistivo e a exigencia de alimentar
    o sensor em 3,3 V.

  OBSERVACAO SOBRE CORRENTE
    Alimentado por USB, o Uno limita a corrente disponivel em torno de
    500 mA. O atuador SG90 apresenta pico de aproximadamente 700 mA. Na
    pratica a montagem funciona, porque o pico e breve, mas convem manter
    o capacitor de 1000 uF em paralelo com a alimentacao do atuador.

    Caso a placa reinicie no instante da abertura, a solucao e alimentar o
    Uno por fonte externa no conector de energia, em vez de USB.

  LIGACOES
    HC-SR04-P   VCC   -> 5V
                TRIG  -> pino digital 4
                ECHO  -> pino digital 5
                GND   -> GND
    Atuador     SINAL -> pino digital 6   (fio laranja)
                V+    -> 5V               (fio vermelho)
                GND   -> GND              (fio marrom)
    Capacitor eletrolitico de 1000 uF / 16 V em paralelo com a alimentacao
    do atuador, com a faixa de polaridade voltada ao GND.

  IMPLANTACAO
    Abrir este arquivo na Arduino IDE, selecionar a placa em
    Ferramentas > Placa > Arduino Uno, escolher a porta em
    Ferramentas > Porta, e carregar. As mensagens sao lidas em
    Ferramentas > Monitor Serial, ajustado para 9600 baud.
*/

#include <Servo.h>

// ---------------------------------------------------------------------
// PARAMETROS DE CALIBRACAO
//
// Os valores abaixo constituem os parametros ajustaveis do sistema e devem
// ser determinados empiricamente em bancada, antes da fixacao mecanica dos
// componentes na lixeira.
// ---------------------------------------------------------------------

const int PIN_TRIG    = 4;
const int PIN_ECHO    = 5;
const int PIN_ATUADOR = 6;

// Limiar de acionamento. Valores reduzidos exigem aproximacao excessiva da
// mao; valores elevados produzem acionamentos indevidos por transeuntes.
const float DIST_GATILHO_CM = 25.0;

// Limite superior de validade da leitura. Medidas acima deste valor sao
// interpretadas como ausencia de obstaculo no campo de deteccao.
const float DIST_MAX_CM = 200.0;

// Intervalo de permanencia em estado ABERTA, contado a partir da ultima
// deteccao de presenca. Ver TEMPORIZACAO NAO BLOQUEANTE, no laco loop.
const unsigned long ESPERA_MS = 4000;

// Janela de supressao de leitura posterior a qualquer comando de movimento
// do atuador. Ver SUPRESSAO DE LEITURA, no laco loop.
const unsigned long TRAVA_ATUADOR_MS = 600;

// Intervalo minimo entre disparos consecutivos do transdutor. Intervalos
// inferiores permitem que o eco residual do disparo anterior seja captado
// como se fosse o eco do disparo corrente.
const unsigned long ESPACO_AMOSTRAS_MS = 30;

// Posicoes angulares extremas da tampa, em graus.
//
// Esta e a diferenca mais visivel em relacao a implementacao anterior. A
// biblioteca Servo trabalha diretamente em graus, de 0 a 180, enquanto o
// MicroPython exigia o calculo da largura de pulso. A calibracao passa a
// ser direta: basta observar o angulo resultante e ajustar o numero.
const int ANGULO_FECHADA = 10;
const int ANGULO_ABERTA  = 100;

// ---------------------------------------------------------------------
// ESTADO DO SISTEMA
// ---------------------------------------------------------------------

Servo atuador;

bool aberta = false;
unsigned long tMovimento     = 0;
unsigned long ultimaPresenca = 0;

// ---------------------------------------------------------------------
// SUBSISTEMA DE SENSORIAMENTO
// ---------------------------------------------------------------------

/*
  Executa um ciclo de medicao por tempo de voo.

  O transdutor e excitado por um pulso de 10 us no pino TRIG. A funcao
  pulseIn mede por quanto tempo o pino ECHO permanece em nivel alto, valor
  proporcional ao percurso da onda ate o obstaculo e de retorno.

  A conversao para centimetros emprega o divisor 58, obtido a partir da
  velocidade do som no ar, aproximadamente 343 m/s a 20 graus Celsius, e da
  divisao por dois, uma vez que a onda percorre o trajeto duas vezes.

  Retorna a distancia em centimetros, ou -1 quando a medicao falha.
*/
float disparo() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(3);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // pulseIn devolve zero quando o tempo limite se esgota, o que corresponde
  // a ausencia de obstaculo no alcance util do sensor.
  unsigned long us = pulseIn(PIN_ECHO, HIGH, 30000UL);
  if (us == 0) {
    return -1.0;
  }

  return us / 58.0;
}

/*
  Retorna a mediana de tres medicoes consecutivas.

  Sensores ultrassonicos de baixo custo apresentam medidas espurias
  isoladas, decorrentes de reflexoes multiplas e de superficies obliquas ao
  feixe. Adotou-se a mediana em vez da media aritmetica porque a media
  incorpora o valor espurio ao resultado de forma proporcional, enquanto a
  mediana o descarta por completo desde que as demais amostras sejam
  validas.

  Retorna a distancia em centimetros, ou -1 quando nenhuma das tres
  medicoes e valida.
*/
float medir() {
  float amostras[3];
  int n = 0;

  for (int i = 0; i < 3; i++) {
    float d = disparo();
    if (d > 2.0 && d < DIST_MAX_CM) {
      amostras[n] = d;
      n++;
    }
    delay(ESPACO_AMOSTRAS_MS);
  }

  if (n == 0) {
    return -1.0;
  }

  // Ordenacao por trocas sucessivas. Com no maximo tres elementos, o
  // metodo mais simples e tambem o mais adequado.
  for (int i = 0; i < n - 1; i++) {
    for (int j = i + 1; j < n; j++) {
      if (amostras[j] < amostras[i]) {
        float t = amostras[i];
        amostras[i] = amostras[j];
        amostras[j] = t;
      }
    }
  }

  return amostras[n / 2];
}

// ---------------------------------------------------------------------
// SUBSISTEMA DE ATUACAO
// ---------------------------------------------------------------------

/*
  Comanda a tampa para a posicao aberta.

  O sinal e mantido enquanto o sistema permanecer neste estado, uma vez que
  a sustentacao da tampa depende do torque continuo do atuador.
*/
void abrir() {
  atuador.attach(PIN_ATUADOR);
  atuador.write(ANGULO_ABERTA);
  aberta = true;
  tMovimento = millis();
}

/*
  Comanda a tampa para a posicao fechada e desativa o sinal.

  A chamada a detach equivale a interromper o sinal PWM, e a decisao e
  deliberada. Um atuador sob sinal permanente executa correcoes continuas
  de posicao, o que produz ruido audivel e consumo constante de corrente.
  Como a tampa fechada se apoia sobre a borda da lixeira, a posicao e
  mantida mecanicamente e o sinal se torna dispensavel neste estado.

  A mesma estrategia nao se aplica ao estado aberto: sem sinal, a tampa
  retornaria a posicao fechada por acao da gravidade.
*/
void fechar() {
  atuador.attach(PIN_ATUADOR);
  atuador.write(ANGULO_FECHADA);
  aberta = false;
  tMovimento = millis();

  delay(TRAVA_ATUADOR_MS);
  atuador.detach();
}

// ---------------------------------------------------------------------
// INICIALIZACAO
// ---------------------------------------------------------------------

void setup() {
  Serial.begin(9600);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  Serial.println("Sistema iniciado");

  // Estabelece condicao inicial conhecida, independentemente da posicao em
  // que a tampa se encontrava antes da energizacao.
  fechar();

  ultimaPresenca = millis();
}

// ---------------------------------------------------------------------
// LACO DE CONTROLE
//
// TEMPORIZACAO NAO BLOQUEANTE
//   A permanencia em estado ABERTA nao e implementada por suspensao da
//   execucao. Uma chamada bloqueante interromperia a aquisicao de dados
//   durante todo o intervalo, e a tampa se fecharia sobre o usuario ainda
//   em processo de descarte.
//
//   Adotou-se, em seu lugar, o registro do instante da ultima deteccao de
//   presenca. O laco permanece em execucao continua e o fechamento e
//   decidido pela comparacao entre o instante corrente e esse registro.
//   Enquanto houver presenca, o registro e renovado e o intervalo nunca se
//   completa.
//
// SUPRESSAO DE LEITURA
//   Durante e imediatamente apos o deslocamento da tampa, a vibracao
//   mecanica transmitida a estrutura compromete a confiabilidade das
//   medidas. As leituras sao portanto descartadas por uma janela fixa
//   posterior a cada comando de movimento.
//
// ARITMETICA DE TEMPO
//   O contador retornado por millis possui largura finita e reinicia apos
//   aproximadamente 49 dias. A subtracao entre variaveis do tipo unsigned
//   long produz resultado correto mesmo quando ocorre esse transbordo, o
//   que torna a construcao millis() - instanteAnterior segura. A ordem dos
//   operandos, contudo, e obrigatoria: inverte-la anula essa propriedade.
// ---------------------------------------------------------------------

void loop() {

  // Etapa 1: supressao de leitura durante o transitorio mecanico.
  if (millis() - tMovimento < TRAVA_ATUADOR_MS) {
    delay(50);
    return;
  }

  // Etapa 2: aquisicao e avaliacao do limiar.
  float d = medir();
  bool presenca = (d > 0.0) && (d < DIST_GATILHO_CM);

  // Etapa 3: avaliacao das transicoes de estado.
  if (presenca) {
    ultimaPresenca = millis();

    if (!aberta) {
      Serial.print("Transicao FECHADA para ABERTA a ");
      Serial.print(d, 0);
      Serial.println(" cm");
      abrir();
    }

  } else if (aberta) {
    unsigned long decorrido = millis() - ultimaPresenca;

    if (decorrido > ESPERA_MS) {
      Serial.print("Transicao ABERTA para FECHADA apos ");
      Serial.print(decorrido);
      Serial.println(" ms");
      fechar();
    }
  }

  // Etapa 4: intervalo de repouso do laco.
  delay(20);
}
