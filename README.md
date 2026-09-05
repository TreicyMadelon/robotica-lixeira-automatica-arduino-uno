# Lixeira Automática com Acionamento por Proximidade

Projeto de robótica de baixo custo

**Disciplina:** Robótica
**Plataforma:** Arduino Uno
**Linguagem:** C++, framework Arduino

---

## Sumário

1. [Resumo](#1-resumo)
2. [Sobre a noção de projeto de baixo custo](#2-sobre-a-noção-de-projeto-de-baixo-custo)
3. [Escolha do objeto de estudo](#3-escolha-do-objeto-de-estudo)
4. [Seleção da plataforma de controle](#4-seleção-da-plataforma-de-controle)
5. [Conceitos da disciplina aplicados ao projeto](#5-conceitos-da-disciplina-aplicados-ao-projeto)
6. [Divergências em relação ao equipamento comercial observado](#6-divergências-em-relação-ao-equipamento-comercial-observado)
7. [Arquitetura do sistema](#7-arquitetura-do-sistema)
8. [Dimensionamento](#8-dimensionamento)
9. [Lógica de controle](#9-lógica-de-controle)
10. [Lista de materiais](#10-lista-de-materiais)
11. [Procedimento de montagem e ensaio](#11-procedimento-de-montagem-e-ensaio)
12. [Riscos identificados e mitigações](#12-riscos-identificados-e-mitigações)
13. [Limitações e trabalhos futuros](#13-limitações-e-trabalhos-futuros)
14. [Registro de decisões](#14-registro-de-decisões)
15. [Apêndices](#15-apêndices)

---

## 1. Resumo

Este documento descreve o projeto de uma lixeira de acionamento automático, na
qual a abertura da tampa é comandada pela detecção de proximidade de um
usuário, sem contato físico com o equipamento.

O sistema é composto por um sensor ultrassônico de tempo de voo, uma placa
Arduino Uno e um atuador que traciona a tampa de uma lixeira comercial de
tampa articulada. A lógica embarcada implementa uma máquina de estados de dois
estados, com temporização não bloqueante para o fechamento.

O projeto tem finalidade acadêmica. O objetivo não é produzir um equipamento
de uso doméstico contínuo, mas demonstrar a integração entre sensoriamento,
processamento e atuação sob restrição explícita de custo, e registrar o
raciocínio de projeto que conduziu a cada decisão técnica.

---

## 2. Sobre a noção de projeto de baixo custo

### 2.1 Dimensões do custo consideradas

O preço unitário dos componentes é a dimensão mais visível do custo, mas não a
única. Foram consideradas cinco dimensões:

| Dimensão | Descrição |
|---|---|
| Custo de aquisição | Preço dos componentes, em moeda corrente |
| Custo de acesso | Disponibilidade no mercado nacional; necessidade de importação |
| Custo de infraestrutura | Equipamento que o projetista já possui ou precisaria adquirir |
| Custo de aprendizado | Tempo necessário para dominar a plataforma escolhida |
| Custo de operação | Consumo, manutenção e substituição de insumos ao longo do uso |

Um componente barato que exige importação, que demanda um adaptador não
disponível, ou que consome insumos periódicos, pode ter custo total superior a
uma alternativa nominalmente mais cara. Essa observação orientou diversas
decisões documentadas nas seções seguintes.

### 2.2 Critérios estabelecidos para este projeto

1. Custo total dos materiais inferior a R$ 250,00.
2. Todos os componentes disponíveis no mercado brasileiro, sem importação.
3. Compatibilidade com o equipamento de desenvolvimento já disponível.
4. Ausência de insumos de reposição periódica durante o período de ensaio.
5. Ferramental de montagem restrito a instrumentos domésticos comuns.

---

## 3. Escolha do objeto de estudo

### 3.1 Pesquisa de campo

A definição do objeto de estudo não foi imediata. Uma primeira proposta de
projeto havia sido esboçada, porém foi substituída após uma etapa de
observação de equipamentos automatizados em ambientes cotidianos.

Durante essa etapa, identificou-se uma lixeira de acionamento automático em
uso real. A observação do equipamento em funcionamento revelou-se mais
informativa do que a leitura de especificações, e três características foram
registradas:

1. O equipamento operava por **bateria**, sem cabo de alimentação visível.
2. O sensor encontrava-se posicionado na **região superior**, próximo à
   abertura.
3. A tampa **permanecia aberta** enquanto o usuário estivesse próximo, e
   fechava apenas após o afastamento.

O terceiro ponto mostrou-se determinante para a lógica de controle e é
retomado na Seção 9. O primeiro e o segundo foram reavaliados e parcialmente
rejeitados, conforme documentado na Seção 6.

### 3.2 Justificativa da escolha

A lixeira automática foi selecionada por reunir, em um objeto de escala
reduzida, os três subsistemas fundamentais de um sistema robótico:

```
   PERCEPÇÃO            DECISÃO                AÇÃO
   ---------            -------                ----
   Sensor de       ->   Microcontrolador  ->   Atuador
   proximidade          Máquina de             Movimento
                        estados                da tampa
```

Além disso, o problema apresenta um comportamento temporal não trivial: a
tampa não deve simplesmente abrir e fechar, mas permanecer aberta pelo tempo
necessário à ação do usuário. Essa característica exige tratamento explícito
de temporização e distingue o projeto de um acionamento puramente reativo.

---

## 4. Seleção da plataforma de controle

### 4.1 Alternativas consideradas

Foram avaliadas as três famílias de plataformas indicadas em aula:

| Plataforma | Natureza | Avaliação |
|---|---|---|
| Raspberry Pi | Computador de placa única com sistema operacional | Descartada por inadequação de escala |
| ESP32 | Microcontrolador de 32 bits com rádio integrado | Descartada por exigir soldagem dos conectores |
| Arduino Uno | Microcontrolador de 8 bits, ecossistema maduro | **Adotada** |

### 4.2 Inadequação de escala

O Raspberry Pi foi descartado por desproporção entre recurso e necessidade. A
tarefa consiste em ler um sensor e comandar um atuador, sem demanda de sistema
operacional, sistema de arquivos ou interface gráfica.

Empregar um computador completo para essa finalidade implicaria maior custo,
maior consumo, tempo de inicialização mais longo e sensibilidade a
desligamentos abruptos — característica indesejável em um equipamento que se
pretende ligar e desligar da tomada livremente.

### 4.3 Exequibilidade da montagem

Este critério foi decisivo entre as duas alternativas restantes, e merece
registro por não ser evidente em comparações de especificação.

Placas da família ESP32 em formato reduzido são comumente fornecidas com os
**conectores de pino separados do circuito impresso**. A soldagem torna-se,
nesse caso, etapa obrigatória de montagem.

A soldagem constitui competência distinta daquelas exigidas pelo projeto
eletrônico e pela programação. Sua execução requer ferramental específico,
prática prévia e tolerância a erro reduzida: uma trilha de cobre descolada por
excesso de calor inutiliza a placa.

O Arduino Uno é fornecido com **barras de conectores fêmea soldadas de
fábrica**. Os condutores são inseridos diretamente na placa, sem etapa
intermediária e sem ferramental adicional.

Adotou-se, portanto, a exequibilidade da montagem diante das competências
disponíveis como critério explícito de seleção, ao lado de capacidade, custo e
disponibilidade.

### 4.4 Restrição de interface

O Arduino Uno emprega conector USB-B, de geração anterior, e cabo com
terminação USB-A do lado do computador.

Essa característica condicionou o equipamento de desenvolvimento. Notebooks
recentes dispõem apenas de portas USB-C, e a conexão exigiria um adaptador não
disponível no momento da montagem. O desenvolvimento foi conduzido em um
computador com porta USB-A.

Registra-se o caso como exemplo da dimensão **infraestrutura** definida em
2.1: a escolha de um componente determinou qual equipamento poderia ser
utilizado, custo que não aparece em especificação técnica alguma.

### 4.5 Decisão

Adotou-se o **Arduino Uno**, pelos seguintes atributos:

- conectores fêmea soldados de fábrica, dispensando qualquer soldagem;
- ligação direta dos condutores à placa, sem circuito intermediário;
- nível lógico de 5 V, compatível com o sensor sem adaptação;
- documentação e ambiente de desenvolvimento amplamente difundidos;
- custo aproximado de R$ 60,00 no mercado nacional.

### 4.6 Consequência sobre a linguagem

O Arduino Uno é programado em C++, por meio do framework Arduino. A plataforma
não admite Python embarcado: o microcontrolador ATmega328P dispõe de 2 KB de
memória RAM, enquanto interpretadores Python para sistemas embarcados requerem
aproximadamente 16 KB apenas para operar. Trata-se de limitação de hardware,
não de escolha de projeto.

A linguagem constitui, portanto, decorrência da plataforma adotada em 4.5, e
não critério de seleção.

Registra-se que a biblioteca de controle de atuadores do framework Arduino
opera diretamente em graus, de 0 a 180, o que simplifica a calibração dos
limites de curso em relação a implementações que exigem o cálculo da largura
de pulso.

---

## 5. Conceitos da disciplina aplicados ao projeto

Dois conceitos discutidos em aula foram diretamente mobilizados: sensoriamento
e temporização. A aplicação de ambos exigiu adaptações não previstas
inicialmente.

### 5.1 Sensoriamento

Adotou-se o sensor ultrassônico HC-SR04, que determina distância pelo tempo de
voo de um pulso acústico. O transdutor emite um pulso e mede o intervalo até o
retorno do eco. A conversão para distância emprega a velocidade do som no ar,
aproximadamente 343 m/s a 20 graus Celsius, com divisão por dois, uma vez que
a onda percorre o trajeto de ida e volta.

O princípio de funcionamento foi validado rapidamente. Uma adaptação, porém,
foi necessária.

**Tratamento estatístico da medida.** Verificou-se que sensores ultrassônicos
de baixo custo produzem medidas espúrias isoladas, decorrentes de reflexões
múltiplas e de superfícies oblíquas ao feixe. Implementou-se a **mediana de
três medições consecutivas**. A escolha da mediana sobre a média é deliberada:
a média incorpora o valor espúrio ao resultado de forma proporcional, ao passo
que a mediana o descarta integralmente, desde que as demais amostras sejam
válidas.

O comportamento foi observado empiricamente. Em ensaio com o código sem filtro,
leituras válidas de 20,5 cm, 29,0 cm e 32,2 cm apareceram intercaladas com
valores de 0,0 cm, sem correspondência com qualquer obstáculo presente.

> **Nota sobre compatibilidade de tensão.** O pino de eco do sensor é uma saída
> referenciada à tensão de alimentação do módulo. Em plataformas de 3,3 V, essa
> característica exigiria um divisor resistivo ou a aquisição do modelo
> HC-SR04-P, capaz de operar em tensão reduzida.
>
> O Arduino Uno opera em nível lógico de 5 V, condição em que o sensor se
> conecta diretamente às entradas digitais. A restrição não se aplica. O modelo
> HC-SR04-P, empregado neste projeto, é igualmente compatível, uma vez que
> opera de 3 a 5,5 V.

### 5.2 Temporização

O conceito de temporização, discutido em aula, mostrou-se necessário em
**três aplicações distintas** neste projeto. A distinção entre elas é
importante, pois atendem a finalidades diferentes.

#### 5.2.1 Temporização não bloqueante do fechamento

Determina por quanto tempo a tampa permanece aberta. A implementação ingênua
consistiria em suspender a execução por um intervalo fixo após a abertura.
Essa abordagem é inadequada: durante a suspensão, o programa não lê o sensor,
e a tampa se fecharia sobre o usuário ainda em processo de descarte.

Adotou-se, em substituição, o registro do instante da última detecção de
presença. O laço de controle permanece em execução contínua e o fechamento é
decidido pela comparação entre o instante corrente e esse registro. Enquanto
houver presença, o registro é renovado e o intervalo nunca se completa.

O comportamento resultante corresponde ao observado no equipamento comercial
descrito na Seção 3.1: a contagem inicia no afastamento do usuário, não na
abertura da tampa.

#### 5.2.2 Supressão de leitura durante o movimento

Determina um intervalo, posterior a cada comando de movimento do atuador,
durante o qual as leituras do sensor são descartadas. A vibração mecânica
transmitida à estrutura no instante da parada do atuador compromete a
confiabilidade das medidas.

Adotou-se uma janela de 600 ms. Esta é a aplicação de temporização mencionada
com maior frequência em discussões sobre o projeto, e convém registrar que ela
não substitui a anterior, mas a complementa.

#### 5.2.3 Espaçamento entre disparos do sensor

Determina o intervalo mínimo entre disparos consecutivos do transdutor.
Intervalos inferiores permitem que o eco residual do disparo anterior seja
captado como se fosse o eco do disparo corrente, produzindo leituras
incoerentes. Adotou-se 30 ms.

---

## 6. Divergências em relação ao equipamento comercial observado

Duas das três características registradas na observação de campo foram
reavaliadas e não reproduzidas neste projeto. As justificativas seguem.

### 6.1 Posição do sensor

**Observado:** sensor na região superior do equipamento, próximo à abertura.

**Adotado:** sensor fixado no corpo da lixeira, cerca de 4 a 5 cm abaixo da
borda, inclinado aproximadamente 10 graus para baixo.

A hipótese inicial foi posicionar o sensor sobre a própria tampa. Essa
disposição apresenta uma vantagem geométrica legítima: como o sensor se
desloca junto com a tampa, esta jamais pode aparecer em seu campo de visão. A
análise, contudo, identificou três consequências desfavoráveis:

1. **Alteração da mira.** Com a tampa elevada a 35 graus, o sensor gira o
   mesmo ângulo e deixa de observar a mão do usuário. Isso inviabiliza a
   lógica de permanência descrita em 5.2.1, cuja premissa é a detecção
   contínua de presença.
2. **Fadiga da fiação.** Os condutores atravessariam a dobradiça e flexionariam
   a cada acionamento. Estimando quinze acionamentos diários, obtêm-se
   aproximadamente 5.500 ciclos anuais. Condutores de prototipagem apresentam
   ruptura interna do cobre sob esse regime, com falha intermitente e de
   diagnóstico difícil.
3. **Acréscimo de massa.** O conjunto sensor, suporte e cabo adiciona
   aproximadamente 25 g à tampa, incorporados ao braço de alavanca e ao torque
   requerido do atuador.

Registra-se que equipamentos comerciais frequentemente posicionam o sensor na
região superior, porém sobre um **aro fixo**, e não sobre a aba móvel. A tampa
gira sob o sensor. A observação de campo capturou a posição aparente, não a
distinção entre parte fixa e parte móvel.

A solução adotada resolve o problema por argumento geométrico, e não por
calibração empírica: a tampa gira em torno de um eixo traseiro, de modo que
seu deslocamento é sempre ascendente e recuante; o cone de detecção do sensor
aponta para a frente e para baixo. As duas regiões ocupam semiespaços opostos
e não podem se interceptar.

### 6.2 Fonte de energia

**Observado:** operação por bateria, sem cabo de alimentação.

**Adotado:** alimentação pela rede elétrica.

Esta decisão foi tomada em função do objetivo e do contexto de uso do projeto.
Como se trata de um trabalho acadêmico destinado a demonstração e ensaio, e
não de um equipamento a ser instalado permanentemente em uma cozinha, a
mobilidade proporcionada pela bateria não constitui requisito.

Foram avaliadas quatro alternativas:

| Alternativa | Custo | Autonomia | Observação |
|---|---|---|---|
| Cabo USB ligado à rede | R$ 15 | Ilimitada | Exige tomada próxima |
| Duas células 18650 com conversor | R$ 80 | Semanas | Requer circuito de carga |
| Banco de energia USB | Variável | Dias | Desliga sob consumo baixo |
| Quatro pilhas AA | R$ 15 | Horas | Insumo de reposição periódica |

A bateria de 9 V, frequentemente sugerida em projetos didáticos, foi descartada
sem análise comparativa: sua capacidade, da ordem de 500 mAh, e sua elevada
resistência interna são incompatíveis com os picos de corrente do atuador.

Adotou-se a alimentação pela rede, que satisfaz o critério de ausência de
insumos de reposição estabelecido em 2.2 e elimina a variável autonomia do
escopo de ensaio. A quarta alternativa foi rejeitada precisamente por violar
esse critério.

Na configuração final, essa alimentação chega pelo próprio cabo USB da placa,
ligado a um computador ou a uma fonte USB de parede. Não há fonte dedicada nem
conversão adicional.

A migração para alimentação autônoma está prevista como trabalho futuro na
Seção 13.

---

## 7. Arquitetura do sistema

### 7.1 Diagrama de blocos

```
   +------------------+      +------------------+      +------------------+
   |     ENTRADA      |      |  PROCESSAMENTO   |      |      SAÍDA       |
   |                  |      |                  |      |                  |
   |    HC-SR04-P     | ---> |   Arduino Uno    | ---> |   Atuador SG90   |
   |                  | dist |   ATmega328P     | PWM  |                  |
   |  Tempo de voo    |  cm  |  Máq. estados    |      |  Aciona a tampa  |
   +------------------+      +------------------+      +------------------+
            |                         |                         |
            +-------------------------+-------------------------+
                                      |
                        +---------------------------+
                        |       ALIMENTAÇÃO         |
                        |  Cabo USB-B, 5 V          |
                        |  A placa distribui 5 V    |
                        |  e GND aos demais         |
                        +---------------------------+
```

### 7.2 Subsistema mecânico

Adotou-se como base uma **lixeira comercial de tampa articulada**. A dobradiça
traseira já integra o produto, o que dispensa a fabricação de articulação e
mancal — operação cuja tolerância instrumentos domésticos dificilmente
assegurariam.

Registra-se que a hipótese inicial previa uma lixeira com acionamento por
pedal, cuja haste interna seria tracionada pelo atuador. A unidade
efetivamente adquirida não possui pedal, o que exige a fabricação do vínculo
entre o atuador e a tampa.

**Princípio adotado: tração, não compressão.** O atuador é fixado na face
traseira e traciona um vínculo preso à porção da tampa situada atrás do eixo
de rotação. O movimento descendente desse trecho eleva a borda frontal.

A alternativa, empurrar a tampa por baixo, foi preterida: arame delgado sob
compressão está sujeito a flambagem, ao passo que sob tração mantém-se
estável. A solução adotada reproduz o princípio do mecanismo de pedal.

Adaptações requeridas:

| Adaptação | Descrição |
|---|---|
| Fixação do sensor | Face frontal externa, com fita adesiva. Sem perfuração |
| Suporte do atuador | Placa rígida aparafusada ao atuador e colada à lixeira |
| Vínculo atuador-tampa | Arame rígido de 2 mm ou cabo de náilon, sob tração |
| Alojamento da eletrônica | Caixa vedada, afastada do compartimento de resíduos |

A opção por fixação adesiva em vez de perfuração é deliberada: preserva a
integridade da lixeira e permite reposicionamento durante os ensaios. O
atuador constitui exceção, pois exerce esforço de tração cíclico, incompatível
com fixação adesiva direta.

### 7.3 Subsistema elétrico

O sistema possui um único domínio de alimentação. O cabo USB fornece 5 V à
placa, e a própria placa distribui as tensões aos demais componentes:

```
   Computador ou fonte USB
            |
            v
   +-----------------+   pino 5V   [fusível]   +----------------+
   |                 | ----------------------> |  Atuador SG90  |
   |                 |                         |  pico ~700 mA  |
   |                 |   pino 6, sinal PWM     +----------------+
   |  Arduino Uno    | ------------------------------>  |
   |                 |                                  |
   |                 |   pino 5V     +-------------+    |
   |                 | ------------> |  HC-SR04-P  |    |
   |                 |   pinos 4 e 5 +-------------+    |
   |      GND        |                      |           |
   +-----------------+ ---------------------+-----------+
```

A referência de terra é estabelecida por construção: todas as tensões derivam
da mesma placa e retornam aos mesmos pinos GND, dos quais o Arduino Uno dispõe
de três. Não existe necessidade de interligar terras de circuitos distintos,
condição que seria obrigatória caso o atuador possuísse fonte própria.

**Nível lógico único.** Sensor, placa e atuador operam integralmente em 5 V.
Essa uniformidade elimina a adaptação de nível que seria necessária em uma
plataforma de 3,3 V, e reduz o número de erros possíveis na montagem.

**Proteção.** Instala-se um fusível rearmável do tipo PPTC, com 1,1 A de
corrente de retenção, em série na linha de 5 V que alimenta o atuador. O valor
foi definido acima dos 800 mA de consumo normal, para evitar atuação indevida,
e abaixo da corrente de um curto-circuito franco.

A proteção é posicionada nesse trecho por ser a única porção do circuito
montada manualmente. O percurso entre a rede elétrica e a placa dispensa
proteção adicional, uma vez que a fonte USB é dispositivo selado e
certificado, dotado de proteção própria.

**Estabilização.** Emprega-se um capacitor eletrolítico de 1000 uF em paralelo
com a alimentação do atuador, posicionado o mais próximo possível dele. O
componente fornece localmente o transitório de corrente da partida, evitando
que a demanda percorra todo o cabeamento. O posicionamento é relevante: junto
à placa, o benefício se reduz substancialmente.

### 7.4 Subsistema de controle

Implementado em C++ sobre o framework Arduino, arquivo `lixeira_automatica.ino`.
Utiliza a biblioteca de controle de atuadores integrante da distribuição
padrão do ambiente, declarada no código pela diretiva `#include <Servo.h>`.

O desenvolvimento emprega o Arduino IDE. Diferentemente de plataformas
interpretadas, o código-fonte não é transferido para a placa: o ambiente o
compila em linguagem de máquina e transfere apenas o resultado. O arquivo
fonte permanece exclusivamente no computador de desenvolvimento.

---

## 8. Dimensionamento

### 8.1 Torque do atuador

O torque requerido para elevar a tampa é obtido pelo produto entre a massa e a
distância do eixo de rotação ao centro de massa:

```
   T = m x d
```

**Massa.** A unidade adquirida foi pesada e apresentou **410 g no conjunto
completo**. O valor relevante ao cálculo, contudo, é a massa da tampa
isoladamente, uma vez que o corpo permanece estático. Em lixeiras plásticas, a
tampa representa tipicamente de 15 a 25 por cento da massa total, o que situa
a estimativa entre 60 e 100 g.

Considerando o centro de massa a 10 cm do eixo:

| Massa da tampa | Torque requerido |
|---|---|
| 60 g | 0,6 kgf.cm |
| 80 g | 0,8 kgf.cm |
| 100 g | 1,0 kgf.cm |

Comparação entre atuadores disponíveis no mercado nacional, considerando a
hipótese mais desfavorável da faixa:

| Modelo | Torque nominal | Ocupação da capacidade | Engrenagem |
|---|---|---|---|
| SG90 | 1,8 kgf.cm | 56 por cento | Plástico |
| MG90S | 2,2 kgf.cm | 45 por cento | Metal |
| MG996R | 9 a 11 kgf.cm | 11 por cento | Metal |

Adotou-se o **SG90**.

A justificativa exige registro, pois a decisão foi revista no curso do
projeto. A estimativa inicial atribuía 150 g à tampa, o que produziria 1,5
kgf.cm e colocaria o SG90 a 83 por cento da capacidade — regime inadequado
para operação cíclica com engrenagens plásticas. Sob essa premissa, adotou-se
inicialmente o MG996R.

A pesagem posterior revelou que a estimativa era conservadora por ampla
margem. Com a massa efetiva, o SG90 opera abaixo de 60 por cento da
capacidade, faixa em que o desgaste de engrenagens plásticas deixa de ser
crítico.

**Consequência sobre a arquitetura elétrica.** A revisão eliminou peças que
existiam exclusivamente em função do MG996R: fonte dedicada de 5 V / 2 A,
adaptador de terminais e a ligação de terra comum entre dois circuitos. O
SG90, com pico de 700 mA, é alimentado pela própria placa.

**Verificação pendente.** A massa da tampa permanece estimada, não medida. O
procedimento previsto consiste em apoiar uma balança sob a borda frontal,
elevar a tampa até que seu peso repouse sobre o instrumento, e registrar a
leitura junto à distância do eixo. O produto das duas grandezas fornece o
torque diretamente, dispensando a determinação do centro de massa.

Este episódio ilustra um risco metodológico: uma estimativa conservadora
propagou-se por três decisões subsequentes — atuador, arquitetura de
alimentação e lista de materiais — antes de ser confrontada com a medição.
Registra-se como aprendizado que a verificação empírica de premissas deve
anteceder as decisões que delas dependem.

### 8.2 Corrente

O atuador SG90 apresenta corrente de pico da ordem de 700 mA, e valores
bastante inferiores em operação normal com carga reduzida. O microcontrolador
e os circuitos auxiliares da placa consomem cerca de 100 mA.

O consumo total no pior caso situa-se em torno de 800 mA.

**Restrição da alimentação por USB.** O Arduino Uno alimentado pela porta USB
dispõe de aproximadamente 500 mA, limitados por um dispositivo de proteção na
própria placa. O valor é inferior ao pico calculado.

Na prática a montagem opera, uma vez que o pico do atuador é breve e o
capacitor de 1000 uF supre localmente o transitório. Caso se observe
reinicialização da placa no instante da abertura, a solução prevista é
alimentar o Arduino por fonte externa no conector de energia, o que eleva a
corrente disponível.

---

## 9. Lógica de controle

### 9.1 Máquina de estados

```
                    distância < 25 cm
          +--------------------------------->+
          |                                  |
   +-------------+                    +-------------+
   |   FECHADA   |                    |   ABERTA    |
   |             |                    |             |
   | sinal       |                    |  sinal      |
   | desativado  |                    |  ativo      |
   +-------------+                    +-------------+
          ^                                  |
          |                                  |
          +<---------------------------------+
              4 s decorridos desde a última
                   detecção de presença
```

A autotransição no estado ABERTA é o elemento central da lógica: enquanto
houver presença detectada, o registro temporal é renovado e a condição de
saída nunca se satisfaz.

### 9.2 Estrutura do laço

O laço de controle executa quatro etapas por iteração:

1. Verificação da janela de supressão de leitura;
2. Aquisição da medida e avaliação do limiar de distância;
3. Avaliação das transições de estado;
4. Intervalo de repouso.

O ciclo completo dura aproximadamente 120 ms, dos quais cerca de 90 ms
correspondem à aquisição das três amostras da mediana.

### 9.3 Aritmética de tempo

O contador retornado por `millis()` possui largura finita e reinicia após
aproximadamente 49 dias de operação contínua. A subtração entre variáveis do
tipo `unsigned long` produz resultado correto mesmo quando ocorre esse
transbordo, propriedade decorrente da aritmética modular aplicada a inteiros
sem sinal.

A construção `millis() - instanteAnterior` é, portanto, segura. A ordem dos
operandos é obrigatória: invertê-la anula a propriedade.

A consequência de negligenciar esse detalhe é característica dos sistemas
embarcados: a falha não se manifesta em ensaio de curta duração, mas apenas
após semanas de operação contínua.

---

## 10. Lista de materiais

| Item | Componente | Função | Custo |
|---|---|---|---|
| 01 | Arduino Uno | Controlador; conectores soldados de fábrica | R$ 60 |
| 02 | HC-SR04-P | Sensor ultrassônico | R$ 20 |
| 03 | Atuador SG90 | Micro atuador, 1,8 kgf.cm | R$ 20 |
| 04 | Cabo USB-B | Alimentação e transferência do programa | R$ 15 |
| 05 | Fusível rearmável PPTC 1,1 A | Proteção da linha do atuador | R$ 12 |
| 06 | Capacitor 1000 uF / 16 V | Absorção do transitório de partida | R$ 2 |
| 07 | Lixeira de tampa articulada | Base mecânica | R$ 60 |
| 08 | Jumpers e protoboard | Prototipagem e junções auxiliares | R$ 25 |
| | | **Total** | **R$ 214** |

Todos os itens são obtidos no mercado nacional, sem importação. Valores de
referência de setembro de 2026, sujeitos a variação.

---

## 11. Procedimento de montagem e ensaio

O procedimento observa a ordem abaixo. A inversão das etapas 1 e 5 é a
principal causa de dano ao atuador.

**Etapa 1. Verificação do ambiente.** Instalar o Arduino IDE, conectar a placa
e confirmar sua identificação em Ferramentas, Porta. Carregar o exemplo Blink,
disponível em Arquivo, Exemplos, 01.Basics, e confirmar que o LED integrado
pisca. Este ensaio valida a cadeia completa de compilação e transferência.

**Etapa 2. Ligação do sensor.** Conectar exclusivamente o sensor: VCC em 5V,
GND em GND, TRIG no pino 4 e ECHO no pino 5. Carregar o programa e observar as
distâncias no Monitor Serial, ajustado para 9600 baud. Conferir a coerência
com medidas de régua.

**Etapa 3. Ligação do atuador.** Somente após a validação do sensor, conectar
o atuador: fio vermelho em 5V, fio marrom em GND, fio de sinal no pino 6.
Instalar o fusível em série na linha de 5 V e o capacitor em paralelo com a
alimentação do atuador, observada a polaridade.

**Etapa 4. Calibração dos limites de curso.** Com o atuador livre, sem braço
acoplado, ajustar as constantes `ANGULO_FECHADA` e `ANGULO_ABERTA` até
identificar as posições angulares desejadas. A biblioteca trabalha em graus,
de 0 a 180, o que torna o ajuste direto.

**Etapa 5. Acoplamento do braço.** Comandar o atuador para a posição fechada e
somente então encaixar o braço no eixo, no ângulo desejado. O encaixe é
estriado e admite apenas posições discretas; acoplar antes da calibração pode
resultar em posição intermediária inalcançável.

**Etapa 6. Fixação mecânica.** Aparafusar o atuador ao suporte rígido e colar
o conjunto à lixeira. Fixar sensor e eletrônica com fita adesiva, após limpeza
da superfície com álcool.

**Etapa 7. Ajuste dos parâmetros temporais.** Ajustar `DIST_GATILHO_CM` e
`ESPERA_MS` conforme o comportamento observado em uso.

**Etapa 8. Ensaio de regime.** Operar o sistema continuamente por período
prolongado, verificando a ausência de reinicializações e a estabilidade da
temporização.

---

## 12. Riscos identificados e mitigações

| Modo de falha | Severidade | Mitigação adotada |
|---|---|---|
| Reinicialização da placa na abertura | Alta | Capacitor de 1000 uF junto ao atuador; alimentação externa como alternativa |
| Fechamento sobre a mão do usuário | Alta | Temporização não bloqueante a partir da última presença |
| Curto-circuito na fiação montada | Média | Fusível rearmável de 1,1 A na linha do atuador |
| Ruptura das engrenagens do atuador | Média | Calibração dos limites anterior ao acoplamento do braço |
| Detecção da própria tampa pelo sensor | Média | Posicionamento no corpo, inclinado, e supressão de leitura |
| Leitura errática do sensor | Baixa | Mediana de três amostras e espaçamento de 30 ms |
| Ruído e consumo em repouso | Baixa | Desacoplamento do sinal após o fechamento |

---

## 13. Limitações e trabalhos futuros

### 13.1 Limitações reconhecidas

1. **Alimentação pela rede.** O equipamento requer tomada próxima, o que
   restringe seu posicionamento. Trata-se de decisão consciente, justificada
   na Seção 6.2, e não de omissão de projeto.
2. **Corrente limitada pela porta USB.** Conforme 8.2, o pico do atuador
   excede o limite nominal da alimentação por USB. A operação é viável, porém
   sem margem.
3. **Ausência de temporizador de guarda.** Não foi implementado watchdog. A
   ausência é deliberada durante o desenvolvimento, uma vez que a
   reinicialização automática interfere na depuração.
4. **Ausência de detecção de obstrução.** O sistema não identifica se a tampa
   encontrou resistência durante o movimento.
5. **Massa da tampa estimada.** Conforme 8.1, o dimensionamento do atuador
   apoia-se em estimativa, não em medição direta.
6. **Conectores de geração anterior.** O Arduino Uno emprega USB-B, o que
   condicionou o equipamento de desenvolvimento utilizado.

### 13.2 Aprendizado metodológico

O trabalho produziu dois registros que extrapolam o objeto construído.

**A exequibilidade da montagem é critério de seleção.** Comparações entre
plataformas costumam contemplar capacidade de processamento, linguagem, custo
e interface. A competência exigida para montar fisicamente o circuito raramente
aparece nessas comparações, embora possa inviabilizar a construção.

Placas fornecidas sem conectores soldados pressupõem habilidade em soldagem,
competência distinta do projeto eletrônico e da programação. Conforme
documentado em 4.3, esse critério foi incorporado à seleção deste projeto.

**O tempo de ensaio excedeu o tempo de projeto.** As etapas de montagem,
verificação e correção consumiram intervalo superior ao das etapas de
concepção e programação. Um cronograma que contemple apenas a construção não
absorve o primeiro problema não previsto.

### 13.3 Evolução prevista

| Versão | Escopo |
|---|---|
| V1 | Protótipo funcional em bancada e transferência para a lixeira |
| V2 | Registro local de eventos de abertura |
| V3 | Análise dos dados coletados em ambiente de computador |
| V4 | Migração para alimentação autônoma por células recarregáveis |

---

## 14. Registro de decisões

Síntese das decisões de projeto, com as respectivas justificativas.

| # | Decisão | Alternativa preterida | Justificativa |
|---|---|---|---|
| D1 | Lixeira automática como objeto | Proposta inicial anterior | Reúne os três subsistemas robóticos em escala reduzida |
| D2 | Arduino Uno | ESP32, Raspberry Pi | Conectores soldados de fábrica; dispensa soldagem |
| D3 | C++ sobre framework Arduino | — | Decorrência da plataforma adotada em D2 |
| D4 | Sensor HC-SR04 | Sensor infravermelho, laser ToF | Mede distância, tolera qualquer superfície, custo reduzido |
| D5 | Atuador SG90 | MG90S e MG996R | Margem suficiente após pesagem; alimentação pela própria placa |
| D6 | Lixeira de tampa articulada | Fabricação da articulação | Reaproveitamento da dobradiça existente |
| D7 | Sensor no corpo | Sensor na tampa | Preservação da mira, da fiação e do braço de alavanca |
| D8 | Energia da rede elétrica | Bateria, como no equipamento observado | Finalidade acadêmica e ensaio temporário |
| D9 | Temporização não bloqueante | Suspensão por intervalo fixo | Continuidade da aquisição no estado aberto |
| D10 | Mediana de três amostras | Média aritmética | Rejeição integral de medidas espúrias |
| D11 | Fixação adesiva, sem perfuração | Perfuração da lixeira | Preserva a base e permite reposicionamento |

---

## 15. Apêndices

### Apêndice A — Mapa de ligações

| Sinal | Origem | Destino | Observação |
|---|---|---|---|
| VCC sensor | Pino `5V` | HC-SR04-P VCC | Nível lógico único de 5 V |
| GND sensor | Pino `GND` | HC-SR04-P GND | |
| TRIG | Pino `4` | HC-SR04-P TRIG | Saída digital |
| ECHO | HC-SR04-P ECHO | Pino `5` | Ligação direta, sem divisor |
| V+ atuador | Pino `5V` | Atuador, fio vermelho | Em série com o fusível de 1,1 A |
| GND atuador | Pino `GND` | Atuador, fio marrom | |
| PWM | Pino `6` | Atuador, fio de sinal | 50 Hz |
| Capacitor | Junto ao atuador | Entre 5 V e GND | 1000 uF, faixa de polaridade no GND |

O Arduino Uno dispõe de três pinos GND, o que permite distribuir os retornos
sem necessidade de junções externas.

### Apêndice B — Parâmetros de calibração

| Parâmetro | Valor inicial | Faixa sugerida | Efeito |
|---|---|---|---|
| `DIST_GATILHO_CM` | 25 | 15 a 40 | Distância de acionamento |
| `ESPERA_MS` | 4000 | 2000 a 8000 | Permanência após afastamento |
| `TRAVA_ATUADOR_MS` | 600 | 400 a 1000 | Supressão pós-movimento |
| `ESPACO_AMOSTRAS_MS` | 30 | 30 a 60 | Intervalo entre disparos |
| `ANGULO_FECHADA` | 10 | Determinar em bancada | Posição fechada, em graus |
| `ANGULO_ABERTA` | 100 | Determinar em bancada | Posição aberta, em graus |

A biblioteca de atuadores aceita valores de 0 a 180 graus, o que torna a
calibração direta: o parâmetro corresponde ao ângulo observado.

### Apêndice C — Estrutura do repositório

```
.
├── README.md
└── lixeira_automatica/
    └── lixeira_automatica.ino     Firmware de controle, C++
```

O diretório deve possuir o mesmo nome do arquivo, exigência do Arduino IDE.

---

**Setembro de 2026**
