#include "OTAManager.h"
#include <Arduino.h>

void setup() {
	Serial.begin(115200);

	/* setup */
	OTAManager otaManager;

	// Shorthand
	otaManager.updateFirmware("http://example.com/firmware.bin", [](u_int8_t progress, OTATarget target) {
		Serial.printf("Progress: %d%%\n", progress);
	});

	// Separate calls
	otaManager.setProgressCallback([](u_int8_t progress, OTATarget target) {
		Serial.printf("Progress: %d%%\n", progress);
	});
	otaManager.updateFirmware("http://example.com/firmware.bin");
}

void loop() { /* loop */
}
