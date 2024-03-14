#include <Arduino.h>
#include <ArduinoOTA.h>

enum OTATarget { OTATargetFirmware, OTATargetSpiffs };

typedef std::function<void(u_int8_t progress, OTATarget target)> ProgressCallback;

class OTAManager {
  public:
	OTAManager();
	void setProgressCallback(ProgressCallback callback);
	void updateFirmware(const char *url, ProgressCallback callback = nullptr);

  private:
	int _lastOtaCommand;
	ProgressCallback _progressCallback;
	u_int8_t _lastPercentage;
	const char *_url;
	void startUpdate();
	void onStart();
	void onProgress(unsigned int progress, unsigned int total);
	void onEnd();
	void onError(ota_error_t error);
	bool httpUpdate(OTATarget target, const char *url);
};