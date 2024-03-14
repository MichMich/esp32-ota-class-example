#include "OTAManager.h"
#include "HTTPClient.h"
#include "esp_log.h"

static const char *TAG = "OTAManager";

/**
 * @brief Construct a new OTAManager::OTAManager object
 */
OTAManager::OTAManager() {
	ArduinoOTA.onStart(std::bind(&OTAManager::onStart, this));
	ArduinoOTA.onProgress(std::bind(&OTAManager::onProgress, this, std::placeholders::_1, std::placeholders::_2));
	ArduinoOTA.onEnd(std::bind(&OTAManager::onEnd, this));
	ArduinoOTA.onError(std::bind(&OTAManager::onError, this, std::placeholders::_1));

	ArduinoOTA.begin();
}

/**
 * @brief Set the progress callback
 *
 * @param callback the callback function
 */
void OTAManager::setProgressCallback(ProgressCallback callback) {
	_progressCallback = callback;
}

/**
 * @brief Update the firmware
 *
 * @param url the url of the firmware
 * @param callback the callback function (optional)
 */
void OTAManager::updateFirmware(const char *url, ProgressCallback callback) {
	if (callback) setProgressCallback(callback);

	_url = strdup(url);
	startUpdate();
}

/* PRIVATE METHODS */

/**
 * @brief Start the update
 */
void OTAManager::startUpdate() {
	ESP_LOGI(TAG, "OTA: Updating firmware: %s\n", _url);

	_lastPercentage = 0;

	if (httpUpdate(_url)) {
		ESP_LOGI(TAG, "Ota done!");
		delay(1000);
		ESP.restart();
	}
}

/**
 * @brief On start of the update
 */
void OTAManager::onStart() {
}

/**
 * @brief On progress of the update
 *
 * @param progress the progress
 * @param total the total
 */
void OTAManager::onProgress(unsigned int progress, unsigned int total) {
	float percent = progress / (total / 100.0);

	if ((u_int8_t)percent > _lastPercentage) {
		_lastPercentage = percent;
		ESP_LOGD(TAG, "Progress: %d%\n", (int)percent);

		if (_progressCallback) _progressCallback((int)percent);
	}
}

/**
 * @brief On end of the update
 */
void OTAManager::onEnd() {
	ESP_LOGI(TAG, "Ota Done!");
}

/**
 * @brief On error of the update
 *
 * @param error the error
 */
void OTAManager::onError(ota_error_t error) {
	ESP_LOGE(TAG, "Error[%u]: ", error);
}

/**
 * @brief Update the firmware via http
 *
 * @param url the url
 * @return true if the update was successful
 * @return false if the update was not successful
 */
bool OTAManager::httpUpdate(const char *url) {
	HTTPClient httpClient;
	httpClient.useHTTP10(true);
	httpClient.setTimeout(5000);

	httpClient.begin(url);

	int result = httpClient.GET();
	if (result != HTTP_CODE_OK) {
		ESP_LOGE(TAG, "HTTP ERROR CODE: %d\n", result);
		return false;
	}
	int httpSize = httpClient.getSize();

	if (!Update.begin(httpSize, U_FLASH)) {
		ESP_LOGE(TAG, "ERROR: %s\n", httpClient.errorToString(httpClient.GET()));
		return false;
	}

	uint8_t buff[1024] = {0};
	size_t sizePack;

	WiFiClient *stream = httpClient.getStreamPtr();
	while (httpClient.connected() && (httpSize > 0 || httpSize == -1)) {
		sizePack = stream->available();
		if (sizePack) {
			int c = stream->readBytes(buff, ((sizePack > sizeof(buff)) ? sizeof(buff) : sizePack));
			Update.write(buff, c);
			if (httpSize > 0) httpSize -= c;
		}
		int percent = int(Update.progress() * 100 / httpClient.getSize());
		if (percent > _lastPercentage) {
			_lastPercentage = percent;
			ESP_LOGI(TAG, "Progress: %d%\n", percent);
			if (_progressCallback) _progressCallback(percent);
		}
	}
	if (!Update.end()) {
		ESP_LOGE(TAG, "ERROR: %s\n", Update.getError());
		return false;
	}
	httpClient.end();
	return true;
}