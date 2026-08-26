# Harpa laser com 12 sensores VL53L1X

## Objetivo

Controlar 12 sensores VL53L1X no mesmo barramento I2C de um ESP32
NodeMCU-32S. Todos medem continuamente em free-run. O firmware mantém a
leitura mais recente de cada sensor em cache e avalia as 12 cordas sem
bloqueio, deixando a saída futura desacoplada para MIDI, teclado ou OSC.

## Hardware e barramento

- ESP32 NodeMCU-32S.
- I2C a 400 kHz: SDA no GPIO 21 e SCL no GPIO 22.
- Um GPIO `XSHUT` exclusivo para cada sensor.
- Todos os sensores iniciam no endereço de fábrica `0x29`.
- Após a inicialização sequencial, os endereços serão `0x30` a `0x3B`.

Configuração inicial de pinos e endereços:

| Índice | XSHUT | Endereço |
|---:|---:|---:|
| 0 | GPIO 4 | `0x30` |
| 1 | GPIO 13 | `0x31` |
| 2 | GPIO 14 | `0x32` |
| 3 | GPIO 16 | `0x33` |
| 4 | GPIO 17 | `0x34` |
| 5 | GPIO 18 | `0x35` |
| 6 | GPIO 19 | `0x36` |
| 7 | GPIO 23 | `0x37` |
| 8 | GPIO 25 | `0x38` |
| 9 | GPIO 26 | `0x39` |
| 10 | GPIO 27 | `0x3A` |
| 11 | GPIO 32 | `0x3B` |

O GPIO 33 permanece livre como alternativa. GPIOs de serial, flash, entrada
apenas e strapping de boot não serão usados para `XSHUT`.

## Configuração em compile time

Cada sensor terá uma entrada `constexpr` própria:

```cpp
struct LidarConfig {
    uint8_t xshutPin;
    uint8_t i2cAddress;
    RoiConfig roi;
    uint16_t triggerMm;
    uint16_t releaseMm;
};
```

Os valores padrão por sensor serão:

- ROI estreita `4x4`, centralizada em `(8, 7)`, SPAD 199.
- Limiar de ativação: distância menor que 800 mm.
- Limiar de liberação: distância maior que 850 mm.
- Modo de distância curto.
- Orçamento de medição de 20 ms.
- Período de free-run de 25 ms.

Cada entrada poderá alterar pino, endereço, ROI e limiares sem mudar as
classes. Verificações `static_assert` impedirão a compilação quando houver:

- quantidade diferente de 12 sensores;
- pinos `XSHUT` duplicados;
- endereços I2C duplicados ou fora da faixa válida de 7 bits;
- uso de `0x29` como endereço final;
- ROI fora dos limites da matriz SPAD `16x16`;
- `releaseMm` menor ou igual a `triggerMm`.

## Componentes

### `LidarSensor`

Representa um sensor físico e contém:

- uma instância do driver Pololu `VL53L1X`;
- a configuração imutável do sensor;
- estado de inicialização e falha;
- distância mais recente;
- timestamp da última leitura válida;
- status e indicador de validade.

Responsabilidades:

- colocar o sensor em reset e liberá-lo por `XSHUT`;
- inicializar no endereço padrão e trocar para o endereço definitivo;
- aplicar modo, orçamento, ROI e centro SPAD;
- iniciar free-run;
- consultar `dataReady()` e consumir com `read(false)` sem bloqueio;
- atualizar o cache somente após uma leitura pronta.

### `LidarArray`

Possui os 12 objetos `LidarSensor` e coordena:

- reset simultâneo no início;
- inicialização sequencial;
- serviço em round-robin;
- acesso indexado e somente leitura aos caches;
- estado geral e contagem de sensores disponíveis.

Uma chamada a `service()` verifica apenas o próximo sensor. Depois do índice
11, o round-robin volta ao índice 0. O método nunca espera uma nova medição.

### `HarpaController`

Consome os 12 caches e aplica limiar, histerese e validade. Ele não conhece
Serial, MIDI, teclado nem OSC. Sua saída é:

```cpp
struct HarpaFrame {
    uint16_t activeMask;
    uint16_t pressedMask;
    uint16_t releasedMask;
};
```

- `activeMask`: estado atual das 12 cordas.
- `pressedMask`: bits que passaram de inativos para ativos nesta atualização.
- `releasedMask`: bits que passaram de ativos para inativos nesta atualização.

Essa interface permite que uma camada futura traduza as transições para
qualquer protocolo sem alterar aquisição e detecção.

## Inicialização

1. Configurar todos os `XSHUT` como saída baixa.
2. Iniciar o I2C nos GPIOs 21 e 22 a 400 kHz.
3. Para cada sensor, em ordem:
   1. elevar apenas seu `XSHUT` e aguardar a inicialização física;
   2. chamar `init()` no endereço `0x29`;
   3. alterar para o endereço definitivo;
   4. verificar erro de transmissão;
   5. aplicar modo curto, orçamento, ROI e centro;
   6. iniciar free-run de 25 ms.
4. Informar no Serial o resultado individual e o total disponível.

Se um sensor falhar antes de receber seu endereço definitivo, seu `XSHUT`
volta ao nível baixo. Isso remove o dispositivo defeituoso de `0x29` e permite
inicializar os sensores seguintes sem colisão.

## Fluxo em execução

O `loop()` terá a seguinte ordem:

```cpp
void loop() {
    lidars.service();
    HarpaFrame frame = harpa.update(lidars);
    publishDebugTransitions(frame);
}
```

`LidarArray::service()` mantém todos os caches atualizados continuamente,
independentemente de uma corda estar ativa ou de sua leitura ser consultada
por outra camada. `HarpaController::update()` examina as 12 cordas em cada
passagem e retorna imediatamente.

## Validade, latência e falhas

- Uma leitura é utilizável somente quando o driver entrega uma medição pronta
  sem timeout e com status de alcance válido.
- Um cache com mais de 100 ms sem atualização é marcado como inválido.
- Uma corda ativa cuja leitura fique inválida será liberada, evitando uma nota
  travada indefinidamente.
- Um sensor que falhou na inicialização permanece indisponível e não impede o
  funcionamento dos demais.
- O orçamento de 20 ms e período de 25 ms limitam a idade física típica da
  amostra. O round-robin adiciona apenas o tempo de consultar os 12 sensores
  pelo I2C, sem espera deliberada.

## Diagnóstico Serial

O modo normal imprime somente:

- progresso e resultado da inicialização;
- sensores indisponíveis;
- transições `pressed` e `released`.

Uma constante compile time habilitará telemetria de calibração com as 12
distâncias, timestamps e estados. Ela ficará desabilitada por padrão para que
o Serial não prejudique a latência do loop.

## Testes e critérios de aceitação

Testes automáticos de compilação e lógica pura cobrirão:

- unicidade e validade de pinos e endereços;
- limites e conversão de ROI para SPAD;
- validação dos limiares de histerese;
- ativação somente abaixo de `triggerMm`;
- manutenção do estado entre os dois limiares;
- liberação somente acima de `releaseMm`;
- liberação de uma corda quando a leitura expira ou fica inválida;
- geração de `pressedMask` e `releasedMask` somente nas transições.

O firmware completo deve compilar para `nodemcu-32s`. Com o hardware montado,
os critérios físicos são:

1. Os 12 sensores aparecem nos endereços `0x30` a `0x3B`.
2. A falha ou ausência de um sensor não impede os seguintes de inicializar.
3. Cada cache recebe leituras novas continuamente sem bloqueio.
4. Interromper uma região ativa gera um único `pressed` para a corda correta.
5. Retirar o objeto gera um único `released` após cruzar o limiar de liberação.
6. Nenhuma leitura permanece válida por mais de 100 ms sem atualização.

## Riscos e fora de escopo

- Os GPIOs podem ser trocados posteriormente apenas na configuração.
- Interferência óptica entre sensores próximos será avaliada no hardware. Se
  observada, sincronização ou escalonamento das medições será um trabalho
  separado; o primeiro desenho mantém o free-run solicitado.
- MIDI, emulação de teclado e OSC não fazem parte desta implementação. Eles
  consumirão `HarpaFrame` em uma etapa posterior.
