/* Camera Stream Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>

#include <esp_http_server.h>
#include "esp_camera.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "cam2udp.h"
#include "pwm.h"
#include "udp_receive.h"

static const char *TAG = "example:http_jpg";

static camera_config_t camera_config = {
    .pin_pwdn = 32,
    .pin_reset = -1,
    .pin_xclk = CONFIG_XCLK,
    .pin_sscb_sda = CONFIG_SDA,
    .pin_sscb_scl = CONFIG_SCL,

    .pin_d7 = CONFIG_D7,
    .pin_d6 = CONFIG_D6,
    .pin_d5 = CONFIG_D5,
    .pin_d4 = CONFIG_D4,
    .pin_d3 = CONFIG_D3,
    .pin_d2 = CONFIG_D2,
    .pin_d1 = CONFIG_D1,
    .pin_d0 = CONFIG_D0,
    .pin_vsync = CONFIG_VSYNC,
    .pin_href = CONFIG_HREF,
    .pin_pclk = CONFIG_PCLK,

    //XCLK 20MHz or 10MHz
    .xclk_freq_hz = CONFIG_XCLK_FREQ,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
    //.frame_size = FRAMESIZE_UXGA,   //QQVGA-UXGA Do not use sizes above QVGA when not JPEG
	.frame_size = FRAMESIZE_CIF,
	//.frame_size = FRAMESIZE_SVGA,
	// quality: 32 ist sehr blockig und hat bei cif kaum größenunterschied zu 20
    .jpeg_quality = 17, //0-63 lower number means higher quality
    .fb_count = 2       //if more than one, i2s runs in continuous mode. Use only with JPEG
};

static esp_err_t init_camera()
{
  //initialize the camera
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Camera Init Failed");
    return err;
  }else{
	  sensor_t* sensor =  esp_camera_sensor_get();
	  sensor->set_gainceiling(sensor,GAINCEILING_32X);
	  //sensor->set_denoise(sensor, 4);
	  //sensor->set_agc_gain(sensor,30);
	  //sensor->set_brightness(sensor,1);
	  sensor->set_whitebal(sensor, 1);
	  sensor->set_awb_gain(sensor, 1);
	  sensor->set_wb_mode(sensor, 0);
  }

  return ESP_OK;
}

static esp_err_t event_handler(void* arg, system_event_t* event)
{
  switch (event->event_id)
  {
  case SYSTEM_EVENT_STA_START:
    ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
    ESP_ERROR_CHECK(esp_wifi_connect());
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
    ESP_LOGI(TAG, "Got IP: '%s'",
    ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));

    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
    ESP_ERROR_CHECK(esp_wifi_connect());
    break;
  default:
    break;
  }
  return ESP_OK;
}

static void initialise_wifi(void *arg)
{
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, arg));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  wifi_config_t wifi_config = {};
  memcpy(wifi_config.sta.ssid,CONFIG_WIFI_SSID,sizeof(CONFIG_WIFI_SSID));
  memcpy(wifi_config.sta.password,CONFIG_WIFI_PASSWORD,sizeof(CONFIG_WIFI_PASSWORD));

  ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}

extern "C" void app_main()
{
	sleep(1);
	init_camera();
	init_pwm();

	ESP_ERROR_CHECK(nvs_flash_init());
	initialise_wifi(NULL);
	esp_wifi_set_ps (WIFI_PS_NONE); // disable powersaving for low responsetimes

	TaskHandle_t xHandle = NULL;
	xTaskCreate(cam2udp_task, "cam2udp", 2000, ( void * ) 1,tskIDLE_PRIORITY, &xHandle);
}
