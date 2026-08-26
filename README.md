# Harpa laser com 12 VL53L1X

Este firmware controla 12 sensores VL53L1X em free-run no mesmo barramento
I2C de um ESP32 NodeMCU-32S. Ele mantém uma leitura em cache por sensor e
emite transições `PRESSED` e `RELEASED`; MIDI, teclado e OSC não fazem parte
deste projeto.

## Ligações

Todos os sensores compartilham alimentação, GND, SDA e SCL. Conecte SDA de
todos os módulos ao GPIO 21 e SCL ao GPIO 22; conecte todos os GNDs ao GND do
ESP32. O barramento opera a 400 kHz. Cada sensor precisa de uma linha XSHUT
independente:

| Índice | XSHUT no ESP32 | Endereço final I2C |
| ---: | --- | --- |
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

Todos os módulos iniciam em `0x29`. No boot, o firmware mantém todos em
reset, libera um por vez, atribui o endereço final da tabela e só então segue
para o próximo. Portanto, todos os carrier boards devem expor o pino `XSHUT`.
Não conecte `XSHUT` diretamente a 3,3 V: o firmware o mantém baixo para reset
e o libera como entrada, deixando o pull-up do módulo conduzir a liberação.

Confira no datasheet dos módulos se a alimentação é 3,3 V ou se o carrier
aceita 5 V. Nunca aplique nível lógico de 5 V aos GPIOs do ESP32. Dimensione a
fonte, regulador e cabeamento para a corrente simultânea de todos os 12
dispositivos, incluindo o ESP32; não presuma que a alimentação USB da placa
tenha margem suficiente. Use aterramento comum e mantenha SDA/SCL curtos,
com os pull-ups adequados para um único barramento.

## Configuração e telemetria

Edite `SENSOR_CONFIGS` em `include/lidar_config.h` para alterar por sensor o
GPIO `XSHUT`, endereço, ROI, distância de trigger e distância de release. Os
valores padrão usam ROI `4x4`, centro `(8, 7)`, ativação abaixo de 800 mm e
liberação acima de 850 mm. As verificações em compile time rejeitam pinos ou
endereços duplicados, `0x29` como endereço final, ROI inválida e histerese
inválida.

A telemetria de calibração fica desabilitada por padrão. Para habilitá-la,
altere `LidarDefaults::ENABLE_DISTANCE_TELEMETRY` para `true` em
`include/lidar_config.h` e recompile. A Serial, a 115200 baud, emitirá no
máximo um quadro `DIST` a cada 100 ms, com 12 campos na ordem da tabela: uma
distância em milímetros ou `X` quando a leitura não é válida. Desabilite-a
depois da calibração para reduzir a carga da Serial no loop.

## Boot e diagnóstico

Abra a Serial a 115200 baud. Um boot com todos os módulos encontrados terá a
forma abaixo:

```text
Lidars available: 12/12
Lidar 0: OK at 0x30
...
Lidar 11: OK at 0x3B
```

Uma ausência aparece como `FAILED` para aquele índice, mas os índices
seguintes continuam a inicialização. Durante a execução normal, cada mudança
de estado imprime uma única linha, por exemplo `String 3 PRESSED` ou
`String 3 RELEASED`.

## Aceitação física

Após montar o hardware, faça upload e verifique:

1. Os 12 módulos inicializam com êxito, cada um no endereço único `0x30` a
   `0x3B`.
2. Remover ou desligar um módulo não bloqueia a inicialização dos índices
   posteriores.
3. Com telemetria habilitada, cada um dos 12 caches recebe amostras novas em
   menos de 100 ms; o loop nunca espera uma amostra.
4. Interromper cada caminho óptico altera somente o bit/índice correspondente
   e produz um único `PRESSED`; retirar o objeto e cruzar o limiar de release
   produz um único `RELEASED`.
5. Se houver interferência óptica entre sensores, registre as distâncias e
   condições medidas e pare. Sincronização ou escalonamento das medições é
   deliberadamente fora de escopo desta versão.

Para verificação sem hardware, execute:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" test -e nodemcu-esp32 --without-uploading
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e nodemcu-esp32
```

Esses comandos comprovam compilação e link. Casos Unity em execução e a
aceitação física exigem a placa conectada e uma porta serial disponível.
