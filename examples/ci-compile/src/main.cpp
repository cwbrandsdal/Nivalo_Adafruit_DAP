// Compile-only consumer: links the STM32 target code the way the Nivalo
// ESP32 bridge uses it. CI builds this sketch; it is never flashed.

#include <Arduino.h>
#include <Adafruit_DAP.h>

namespace {

constexpr int kSwclkPin = 13;
constexpr int kSwdioPin = 12;
constexpr int kResetPin = 27;
constexpr uint32_t kFlashBase = 0x08000000;

Adafruit_DAP_STM32 dap;
uint8_t image[256];

void onDapError(const char *message) { Serial.println(message); }

}  // namespace

void setup() {
  Serial.begin(115200);
  if (!dap.begin(kSwclkPin, kSwdioPin, kResetPin, &onDapError) || !dap.targetConnect()) {
    return;
  }

  uint32_t id = 0;
  if (dap.select(&id)) {
    Serial.printf("STM32 %s id=0x%08lx flash=%lu\n", dap.target_device.name, static_cast<unsigned long>(id),
                  static_cast<unsigned long>(dap.target_device.flash_size));
    dap.unprotectBoot();
    dap.program_start(kFlashBase, sizeof(image));
    if (dap.programFlash(kFlashBase, image, sizeof(image), true) && dap.verifyFlash(kFlashBase, image, sizeof(image))) {
      Serial.printf("crc32=0x%08lx\n", static_cast<unsigned long>(dap.computeFlashCRC32(kFlashBase, sizeof(image))));
    }
    dap.protectBoot();
    dap.deselect();
  }
  dap.dap_disconnect();
}

void loop() {}
