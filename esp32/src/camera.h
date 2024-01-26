
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

enum class CameraSetting
{
  Deinit,
  Default,
  LowRes,
  HighRes,
};

extern "C"
{
  esp_err_t xclk_timer_conf(int ledc_timer, int xclk_freq_hz);
  esp_err_t camera_enable_out_clock(const camera_config_t *config);
  void camera_disable_out_clock(void);
}

CameraSetting _lastCamSetting = CameraSetting::Deinit;
void setupCamera(CameraSetting settingType = CameraSetting::Default)
{
  if(_lastCamSetting == settingType)
  {
    return;
  }

  esp_camera_deinit();

  _lastCamSetting = settingType;

  if(settingType == CameraSetting::Deinit)
  {
    camera_disable_out_clock();
    digitalWrite(PWDN_GPIO_NUM, 1);
    return;
  }
  else
  {
    digitalWrite(PWDN_GPIO_NUM, 0);
  }
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_SXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  if(settingType == CameraSetting::LowRes)
  {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
  }
  else if(settingType == CameraSetting::HighRes)
  {
    config.grab_mode = CAMERA_GRAB_LATEST;
    config.frame_size = FRAMESIZE_SXGA;
    config.jpeg_quality = 6;
  }

  // camera init
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t* s = esp_camera_sensor_get();
  s->set_brightness(s, 1);     // -2 to 2
  s->set_contrast(s,2);       // -2 to 2
  s->set_saturation(s, 1);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 1);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 0);  // 0 = disable , 1 = enable
  s->set_aec2(s, 1);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 800);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 15);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)6);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
	
	if (const auto fb = esp_camera_fb_get())
	{
	  esp_camera_fb_return(fb);
	}
}

bool checkPhoto(const char* path)
{
  File f_pic = LittleFS.open(path);
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}

void capturePhotoSaveSpiffs(const char* path)
{
  camera_fb_t * fb = NULL;
  bool ok = 0;
  
  do
  {
    Serial.println("Taking a photo...");

    fb = esp_camera_fb_get();
    if (!fb)
    {
      Serial.println("Camera capture failed");
      return;
    }

    Serial.printf("Picture file name: %s\n", path);
    File file = LittleFS.open(path, FILE_WRITE);

    if (!file)
    {
      Serial.println("Failed to open file in writing mode");
    }
    else
    {
      for(int x = 0; x < 10; x++)
      {
        Serial.print("Writing ");
        Serial.print(fb->len);
        Serial.println(" bytes");

        const auto count = file.write(fb->buf, fb->len);

        Serial.print("Wrote ");
        Serial.print(count);
        Serial.println(" bytes");

        if(count > 0)
        {
          break;
        }
      }
    }

    file.close();
    esp_camera_fb_return(fb);

    ok = checkPhoto(path);
  } while ( !ok );
}

uint8_t* lastCatpure = nullptr;
size_t lastCatpureLen = -1;

void capture()
{
	if(lastCatpure)
	{
		delete[] lastCatpure;
		lastCatpureLen = -1;
	}

	setupCamera(CameraSetting::HighRes);

	digitalWrite(4, HIGH);
	delay(10);

	if (const auto fb = esp_camera_fb_get())
	{
	  esp_camera_fb_return(fb);
	}

	auto* fb = esp_camera_fb_get();
	digitalWrite(4, LOW);

	if (fb)
	{
		if(lastCatpure = new uint8_t[fb->len])
		{
			lastCatpureLen = fb->len;
			memcpy(lastCatpure, fb->buf, lastCatpureLen);
		}
		else
		{
			Serial.println("Camera buffer allocation failed");
		}
	}
	else
	{
		Serial.println("Capture Camera capture failed");
	}

	esp_camera_fb_return(fb);
}