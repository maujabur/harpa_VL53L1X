# Multisensor Laser Harp Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Controlar 12 VL53L1X em free-run no mesmo I2C, manter leituras recentes sem bloqueio e gerar transições de 12 cordas com histerese.

**Architecture:** Um `LidarSensor` encapsula cada dispositivo e seu cache; `LidarArray` controla `XSHUT`, endereçamento sequencial e serviço round-robin. `HarpaController` é lógica pura: consome os 12 caches e produz máscaras `active`, `pressed` e `released`, sem depender da saída futura.

**Tech Stack:** ESP32 NodeMCU-32S, PlatformIO, Arduino framework, `Wire`, Pololu VL53L1X 1.3.1, Unity.

**Spec:** `docs/superpowers/specs/2026-08-26-multisensor-laser-harp-design.md`

## Global Constraints

- Exatamente 12 sensores no mesmo I2C a 400 kHz, SDA GPIO 21 e SCL GPIO 22.
- Endereços finais `0x30` a `0x3B`; endereço de boot `0x29` nunca é endereço final.
- `XSHUT`: GPIOs 4, 13, 14, 16, 17, 18, 19, 23, 25, 26, 27 e 32.
- Cada sensor tem ROI e limiares compile time individuais; padrão ROI `4x4`, centro `(8,7)`, trigger 800 mm e release 850 mm.
- Modo curto, orçamento 20 ms, período free-run 25 ms, cache expirado após 100 ms.
- Aquisição e avaliação de gatilhos não podem bloquear esperando medição.
- Serial normal imprime inicialização, falhas e transições; telemetria completa é compile time e desabilitada por padrão.
- MIDI, teclado e OSC ficam fora deste plano.
- O diretório não possui `.git`; passos de commit são documentados como mensagens sugeridas, mas só podem ser executados depois que o usuário inicializar um repositório.

## File Map

- `include/lidar_config.h`: tipos de configuração, constantes dos 12 sensores, ROI/SPAD e validação compile time.
- `include/lidar_reading.h`: dados de cache e aliases compartilhados.
- `include/harpa_controller.h`: interface e estado do avaliador de cordas.
- `src/harpa_controller.cpp`: histerese, expiração e máscaras de transição.
- `include/lidar_sensor.h`: interface do objeto que representa um VL53L1X.
- `src/lidar_sensor.cpp`: reset, inicialização, endereçamento, free-run e leitura não bloqueante.
- `include/lidar_array.h`: interface do conjunto de 12 objetos.
- `src/lidar_array.cpp`: boot sequencial, round-robin e acesso aos caches.
- `src/main.cpp`: composição, diagnóstico Serial e ponto de extensão para saídas futuras.
- `test/test_lidar_config/test_main.cpp`: testes compile time de configuração.
- `test/test_harpa_controller/test_main.cpp`: testes Unity da máquina de estados.
- `test/test_lidar_sensor/test_main.cpp`: teste de compilação da interface de hardware.
- `README.md`: pinagem, edição dos parâmetros e procedimento de teste físico.
- Remover `include/sensor_config.h` e `test/test_sensor_config/test_main.cpp` após migrar seu conteúdo.

---

### Task 1: Configuração compile time dos 12 sensores

**Files:**
- Create: `include/lidar_config.h`
- Create: `test/test_lidar_config/test_main.cpp`
- Delete after green: `include/sensor_config.h`
- Delete after green: `test/test_sensor_config/test_main.cpp`

**Interfaces:**
- Produces: `RoiConfig`, `LidarConfig`, `LidarConfigArray`, `SENSOR_COUNT`, `SENSOR_CONFIGS`, `spadNumberFromCoordinates()`, `isValidRoi()`, `configsAreValid()`.
- Consumes: only standard C++ headers `<array>`, `<cstddef>`, `<cstdint>`.

- [ ] **Step 1: Write the failing compile-time tests**

Create tests that name the breaks they catch:

```cpp
#include <Arduino.h>
#include "lidar_config.h"

static_assert(SENSOR_COUNT == 12, "A harpa deve ter 12 sensores");
static_assert(spadNumberFromCoordinates(0, 0) == 128);
static_assert(spadNumberFromCoordinates(15, 15) == 0);
static_assert(spadNumberFromCoordinates(8, 7) == 199);
static_assert(isValidRoi({4, 4, 8, 7}));
static_assert(!isValidRoi({3, 4, 8, 7}));
static_assert(configsAreValid(SENSOR_CONFIGS));

void setup() {}
void loop() {}
```

- [ ] **Step 2: Run the test and verify RED**

Run:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" test -e nodemcu-esp32 -f test_lidar_config --without-uploading
```

Expected: compilation fails because `lidar_config.h` does not exist.

- [ ] **Step 3: Implement the configuration types and validation**

Create these exact public definitions:

```cpp
struct RoiConfig {
    uint8_t width;
    uint8_t height;
    uint8_t centerX;
    uint8_t centerY;
};

struct LidarConfig {
    uint8_t xshutPin;
    uint8_t i2cAddress;
    RoiConfig roi;
    uint16_t triggerMm;
    uint16_t releaseMm;
};

constexpr size_t SENSOR_COUNT = 12;
using LidarConfigArray = std::array<LidarConfig, SENSOR_COUNT>;
```

Implement the existing SPAD table conversion and ROI boundary rule in this
file. Add `constexpr` helpers that reject duplicated pins, duplicated
addresses, `0x29`, addresses outside `0x08..0x77`, invalid ROIs, and
`releaseMm <= triggerMm`.

Declare this exact array:

```cpp
constexpr LidarConfigArray SENSOR_CONFIGS{{
    {4,  0x30, {4, 4, 8, 7}, 800, 850},
    {13, 0x31, {4, 4, 8, 7}, 800, 850},
    {14, 0x32, {4, 4, 8, 7}, 800, 850},
    {16, 0x33, {4, 4, 8, 7}, 800, 850},
    {17, 0x34, {4, 4, 8, 7}, 800, 850},
    {18, 0x35, {4, 4, 8, 7}, 800, 850},
    {19, 0x36, {4, 4, 8, 7}, 800, 850},
    {23, 0x37, {4, 4, 8, 7}, 800, 850},
    {25, 0x38, {4, 4, 8, 7}, 800, 850},
    {26, 0x39, {4, 4, 8, 7}, 800, 850},
    {27, 0x3A, {4, 4, 8, 7}, 800, 850},
    {32, 0x3B, {4, 4, 8, 7}, 800, 850},
}};

static_assert(configsAreValid(SENSOR_CONFIGS),
              "Configuracao invalida de pino, endereco, ROI ou histerese");
```

Also define `SDA_PIN=21`, `SCL_PIN=22`, `I2C_CLOCK_HZ=400000`,
`MEASUREMENT_BUDGET_US=20000`, `MEASUREMENT_PERIOD_MS=25`,
`SENSOR_TIMEOUT_MS=100`, `STALE_AFTER_MS=100`, and
`ENABLE_DISTANCE_TELEMETRY=false` in a `LidarDefaults` namespace.

- [ ] **Step 4: Run the config test and verify GREEN**

Run the Step 2 command. Expected: the test firmware compiles and all
`static_assert` checks are accepted.

- [ ] **Step 5: Remove superseded ROI files and rerun all compile tests**

Delete the two superseded files listed above with `apply_patch`, then run:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" test -e nodemcu-esp32 --without-uploading
```

Expected: no missing include for `sensor_config.h`; all test targets compile.

- [ ] **Step 6: Record the suggested commit**

Suggested commit once Git exists: `feat: add compile-time configuration for 12 lidars`.

---

### Task 2: Máquina de estados das 12 cordas

**Files:**
- Create: `include/lidar_reading.h`
- Create: `include/harpa_controller.h`
- Create: `src/harpa_controller.cpp`
- Create: `test/test_harpa_controller/test_main.cpp`

**Interfaces:**
- Consumes: `SENSOR_COUNT`, `SENSOR_CONFIGS`, `LidarDefaults::STALE_AFTER_MS` from `lidar_config.h`.
- Produces: `LidarReading`, `LidarReadingArray`, `HarpaFrame`, `HarpaController::update(const LidarReadingArray&, uint32_t)`.

- [ ] **Step 1: Write failing trigger-transition tests**

Define the wished-for behavior with real objects:

```cpp
#include <Arduino.h>
#include <unity.h>
#include "harpa_controller.h"

HarpaController controller;
LidarReadingArray readings{};

void setUp() {
    controller = HarpaController{};
    readings = {};
}

void test_press_hold_and_release_use_hysteresis() {
    readings[0] = {799, 10, true};
    HarpaFrame frame = controller.update(readings, 10);
    TEST_ASSERT_BITS_HIGH(1u, frame.pressedMask);
    TEST_ASSERT_BITS_HIGH(1u, frame.activeMask);

    readings[0] = {825, 20, true};
    frame = controller.update(readings, 20);
    TEST_ASSERT_EQUAL_UINT16(0, frame.pressedMask);
    TEST_ASSERT_EQUAL_UINT16(0, frame.releasedMask);
    TEST_ASSERT_BITS_HIGH(1u, frame.activeMask);

    readings[0] = {851, 30, true};
    frame = controller.update(readings, 30);
    TEST_ASSERT_BITS_HIGH(1u, frame.releasedMask);
    TEST_ASSERT_BITS_LOW(1u, frame.activeMask);
}

void test_invalid_or_stale_reading_releases_active_string() {
    readings[2] = {700, 10, true};
    controller.update(readings, 10);
    HarpaFrame frame = controller.update(readings, 111);
    TEST_ASSERT_BITS_HIGH(1u << 2, frame.releasedMask);
    TEST_ASSERT_BITS_LOW(1u << 2, frame.activeMask);
}
```

Register both tests in Unity `setup()` and leave `loop()` empty.

- [ ] **Step 2: Run the test and verify RED**

Run:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" test -e nodemcu-esp32 -f test_harpa_controller --without-uploading
```

Expected: compilation fails because `harpa_controller.h` is missing.

- [ ] **Step 3: Implement the shared reading and frame types**

Use these exact definitions:

```cpp
struct LidarReading {
    uint16_t distanceMm = 0;
    uint32_t updatedAtMs = 0;
    bool valid = false;
};

using LidarReadingArray = std::array<LidarReading, SENSOR_COUNT>;

struct HarpaFrame {
    uint16_t activeMask = 0;
    uint16_t pressedMask = 0;
    uint16_t releasedMask = 0;
};
```

`HarpaController` stores only `uint16_t activeMask_`. Its `update()` loops
over all 12 readings. An inactive bit activates only for a fresh valid reading
with `distanceMm < triggerMm`. An active bit releases for invalid/stale data or
`distanceMm > releaseMm`. Equality preserves the current state.

- [ ] **Step 4: Run the trigger test and verify GREEN**

Run the Step 2 command. Expected after upload to an available board: 2 Unity
tests pass. With `--without-uploading`, compilation must succeed; do not report
runtime tests as executed.

- [ ] **Step 5: Add boundary tests**

Add tests proving equality at 800 mm does not press, equality at 850 mm does
not release, and two simultaneous strings set two distinct bits. Run again and
expect all test targets to compile and, when uploaded, all Unity cases to pass.

- [ ] **Step 6: Record the suggested commit**

Suggested commit once Git exists: `feat: add nonblocking harp trigger state machine`.

---

### Task 3: Objeto individual `LidarSensor`

**Files:**
- Create: `include/lidar_sensor.h`
- Create: `src/lidar_sensor.cpp`
- Create: `test/test_lidar_sensor/test_main.cpp`

**Interfaces:**
- Consumes: `LidarConfig`, `LidarReading`, Pololu `VL53L1X`, Arduino `TwoWire`.
- Produces: `holdInReset()`, `begin(TwoWire&)`, `service(uint32_t)`, `reading()`, `available()`.

- [ ] **Step 1: Write the failing interface compilation test**

```cpp
#include <Arduino.h>
#include <type_traits>
#include "lidar_sensor.h"

static_assert(std::is_default_constructible<LidarSensor>::value);

void setup() {
    LidarSensor sensor;
    sensor.holdInReset(SENSOR_CONFIGS[0]);
    (void)sensor.available();
    (void)sensor.reading();
}

void loop() {}
```

- [ ] **Step 2: Run the test and verify RED**

Run:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" test -e nodemcu-esp32 -f test_lidar_sensor --without-uploading
```

Expected: compilation fails because `lidar_sensor.h` is missing.

- [ ] **Step 3: Implement reset and sequential initialization behavior**

Expose this interface:

```cpp
class LidarSensor {
public:
    void holdInReset(const LidarConfig& config);
    bool begin(TwoWire& bus);
    void service(uint32_t nowMs);
    const LidarReading& reading() const;
    bool available() const;

private:
    void failAndReset();
    LidarConfig config_{};
    VL53L1X driver_{};
    LidarReading reading_{};
    bool configured_ = false;
    bool available_ = false;
};
```

`holdInReset()` copies the immutable runtime configuration, configures the pin
as output low and clears reading/state. `begin()` muda o pino para `INPUT`,
deixando o pull-up da placa liberar `XSHUT` sem dirigir 3,3 V diretamente,
waits 10 ms, sets bus and timeout, initializes at `0x29`, changes address, checks
`driver_.last_status`, selects `VL53L1X::Short`, applies 20000 us, calls
`setROISize()` before `setROICenter()`, and starts continuous at 25 ms. Any
failure calls `failAndReset()` and returns false.

- [ ] **Step 4: Implement nonblocking cache update**

`service()` returns immediately unless the object is available and
`driver_.dataReady()` is true. Only then call `driver_.read(false)`. Mark the
reading valid when there is no timeout and
`driver_.ranging_data.range_status == VL53L1X::RangeValid`; otherwise retain
the numeric distance but set `valid=false`. Always set `updatedAtMs=nowMs`
after consuming a ready result.

- [ ] **Step 5: Run the interface test and firmware build**

Run the Step 2 command, then:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e nodemcu-esp32
```

Expected: both commands exit 0; the dependency graph includes `VL53L1X` and
`Wire`.

- [ ] **Step 6: Record the suggested commit**

Suggested commit once Git exists: `feat: encapsulate one vl53l1x sensor`.

---

### Task 4: Orquestração, endereçamento e round-robin

**Files:**
- Create: `include/lidar_array.h`
- Create: `src/lidar_array.cpp`
- Create: `test/test_lidar_array/test_main.cpp`

**Interfaces:**
- Consumes: `LidarSensor`, `SENSOR_CONFIGS`, `LidarReadingArray`, `TwoWire`.
- Produces: `LidarArray::begin(TwoWire&)`, `service(uint32_t)`, `readings()`, `availableCount()`, `sensorAvailable(size_t)`.

- [ ] **Step 1: Write the failing public API test**

```cpp
#include <Arduino.h>
#include "lidar_array.h"

void setup() {
    LidarArray lidars;
    TEST_ASSERT_EQUAL_UINT8(0, lidars.availableCount());
    TEST_ASSERT_EQUAL_UINT32(SENSOR_COUNT, lidars.readings().size());
}

void loop() {}
```

Include Unity and register this as `test_empty_array_starts_unavailable`.

- [ ] **Step 2: Run the test and verify RED**

Run:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" test -e nodemcu-esp32 -f test_lidar_array --without-uploading
```

Expected: compilation fails because `lidar_array.h` is missing.

- [ ] **Step 3: Implement the orchestrator**

Expose:

```cpp
class LidarArray {
public:
    uint8_t begin(TwoWire& bus);
    void service(uint32_t nowMs);
    const LidarReadingArray& readings() const;
    uint8_t availableCount() const;
    bool sensorAvailable(size_t index) const;

private:
    std::array<LidarSensor, SENSOR_COUNT> sensors_{};
    LidarReadingArray readings_{};
    size_t nextSensor_ = 0;
    uint8_t availableCount_ = 0;
};
```

`begin()` first calls `holdInReset(SENSOR_CONFIGS[i])` for all 12, waits 2 ms,
starts the supplied bus on GPIO 21/22 at 400 kHz, then calls each `begin()` in
order. Count successes and keep going after failures. `service()` services
exactly `nextSensor_`, copies its reading into the same array index, and
advances modulo 12. `sensorAvailable()` returns false for an out-of-range index
and otherwise delegates to the corresponding `LidarSensor`.

- [ ] **Step 4: Run compile tests and build**

Run the Step 2 command and the normal firmware build. Expected: exit 0 for
both; the Unity case runs only when the board/COM port is available.

- [ ] **Step 5: Record the suggested commit**

Suggested commit once Git exists: `feat: initialize and service 12 lidars`.

---

### Task 5: Integrar o firmware e diagnóstico de comissionamento

**Files:**
- Modify: `src/main.cpp`
- Create: `README.md`
- Modify: `platformio.ini` only if Unity test discovery requires an explicit test dependency.

**Interfaces:**
- Consumes: `LidarArray`, `HarpaController`, `HarpaFrame`, `ENABLE_DISTANCE_TELEMETRY`.
- Produces: running firmware and documented wiring/calibration workflow.

- [ ] **Step 1: Replace the single-sensor composition in `main.cpp`**

Use global instances `LidarArray lidars;` and `HarpaController harpa;`.
`setup()` starts Serial, calls `lidars.begin(Wire)`, prints the available
count, and loops over `sensorAvailable(i)` to print the result and final address
of every index. `loop()` uses:

```cpp
void loop() {
    const uint32_t nowMs = millis();
    lidars.service(nowMs);
    const HarpaFrame frame = harpa.update(lidars.readings(), nowMs);
    printTransitions(frame);
    printTelemetryIfEnabled(lidars.readings(), nowMs);
}
```

`printTransitions()` prints one line per set bit, with index and `PRESSED` or
`RELEASED`. Telemetry, when enabled, is throttled to one frame every 100 ms and
prints all distances plus `X` for invalid readings.

- [ ] **Step 2: Write commissioning documentation**

Document the 12-row pin/address table, common SDA/SCL wiring, common ground,
power requirements, editing `SENSOR_CONFIGS`, expected boot output, enabling
telemetry, and the physical checks from the spec. Explicitly state that all
carrier boards must expose `XSHUT` and that power capacity must be checked for
12 devices.

- [ ] **Step 3: Run the complete verification suite**

Run:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" test -e nodemcu-esp32 --without-uploading
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e nodemcu-esp32
```

Expected: every test target compiles, final firmware build exits 0, and the
dependency graph contains `VL53L1X 1.3.1` and `Wire`. Report separately that
Unity runtime cases require the board and an available COM port.

- [ ] **Step 4: Perform the physical acceptance test when hardware is connected**

Upload, open Serial at 115200, and verify: 12 successful unique addresses;
removing one device does not block later indices; every sensor refreshes under
100 ms; interrupting each optical path toggles only its bit; every transition
prints once. If optical interference appears, record measurements and stop
before introducing synchronization, because that is explicitly outside this
plan.

- [ ] **Step 5: Record the suggested commit**

Suggested commit once Git exists: `feat: integrate 12-sensor laser harp core`.

---

## Final Review Gate

- Re-read the spec and check every hardware, configuration, latency, error,
  diagnostic and test requirement against Tasks 1–5.
- Scan this plan for placeholder markers and remove any vague implementation
  instruction before execution.
- Confirm signatures match across files: `LidarArray::readings()` returns
  `const LidarReadingArray&`; `HarpaController::update()` consumes that exact
  type; all time values use `uint32_t` milliseconds.
- Do not claim physical success from build-only tests or from a blocked COM
  port.
