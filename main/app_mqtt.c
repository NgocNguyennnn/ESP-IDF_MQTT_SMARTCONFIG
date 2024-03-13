#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "MQTTS_APP";

extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");


static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "tempota", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
            // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "tempota", "17", 0, 0, 0);
            if(msg_id == -1) ESP_LOGI(TAG, "sent publish ERROR");
            else ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}
static void mqtt_publish_task(void *pvParameters) {
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)pvParameters;
    const char *msg = "17"; // Giả sử đây là dữ liệu bạn muốn publish
    const char *topic = "tempota";

    while (1) {
        int msg_id = esp_mqtt_client_publish(client, topic, msg, 0, 0, 0);
        if(msg_id == -1) {
            ESP_LOGI(TAG, "sent publish ERROR");
        } else {
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        }
        // Đợi trong 2 giây trước khi publish tiếp
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void mqtt_app_start(esp_mqtt_client_handle_t client)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .host = "broker.hivemq.com",
        .port = 1883,
        .event_handle = mqtt_event_handler,
        .client_id = "beiiiuu1",
        .username = "ngocnguyen",
        .password = "123456",
        // .client_cert_pem = (const char *)client_cert_pem_start,
        // .client_key_pem = (const char *)client_key_pem_start,
    };

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
  
    xTaskCreate(mqtt_publish_task, "mqtt_publish_task", 4096, client, 5, NULL);
}

