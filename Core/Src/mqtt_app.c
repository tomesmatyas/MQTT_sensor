#include "stm32f4xx_hal.h"
#include "main.h"
#include "mqtt_app.h"
#include "lwip/apps/mqtt.h"
#include "lwip.h"
#include <stdio.h>
#include <string.h>
#include "bmp180.h"
#include "ili9341.h"
#include "cli.h"


#define BMP180_ADDR 0xEE
extern I2C_HandleTypeDef hi2c1; 
extern TIM_HandleTypeDef htim3;
extern uint8_t is_monitoring;

volatile uint8_t timer_chce_merit = 0;

mqtt_client_t *my_mqtt_client;
ip_addr_t broker_ipaddr;

char last_rx_topic[64] = "Zatim nic neprijato";
char last_rx_payload[512] = "Zatim nic neprijato"; 
char last_tx_payload[128] = "Zatim nic neodeslano";


static uint16_t rx_payload_offset = 0;


void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    strncpy(last_rx_topic, topic, sizeof(last_rx_topic) - 1);
    last_rx_topic[sizeof(last_rx_topic) - 1] = '\0';


    rx_payload_offset = 0;

    printf("\r\n\033[32m[MQTT RX] Tema: %s\033[0m\r\n", topic);
}



void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
   
    uint16_t space_left = sizeof(last_rx_payload) - rx_payload_offset - 1;
    uint16_t copy_len = len < space_left ? len : space_left;

    memcpy(last_rx_payload + rx_payload_offset, data, copy_len);
    rx_payload_offset += copy_len;

  
    if (flags & MQTT_DATA_FLAG_LAST) {
        last_rx_payload[rx_payload_offset] = '\0';

       printf("\033[32m[MQTT RX] Zprava: %s\033[0m\r\nNucleo> ", last_rx_payload);

        if (strcmp(last_rx_topic, "mqtt/led") == 0) {
            
            if (strcmp(last_rx_payload, "ON") == 0) {
                HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_SET); 
                printf("\r\n[AKCE] LED zapnuta na dálku!\r\nNucleo> ");
            } else if (strcmp(last_rx_payload, "OFF") == 0) {
                HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_RESET);
                printf("\r\n[AKCE] LED vypnuta na dálku!\r\nNucleo> ");
            }
        }
        else if (strcmp(last_rx_topic, "mqtt/displej") == 0) {
			
		}
		else if (strcmp(last_rx_topic, "mqtt/teplota") == 0) {
			
		}
		else if (strcmp(last_rx_topic, "xtichy28_mqtt/status") == 0) {
			float rx_temp = 0.0;
			            float rx_press = 0.0;

			           
			            if (sscanf(last_rx_payload, "temp: %f; press: %f", &rx_temp, &rx_press) == 2) {

			                
			                float rx_press_hPa = rx_press;

			                char text_temp[32];
			                char text_press[32];
			                sprintf(text_temp, "Teplota: %.1f C", rx_temp);
			                sprintf(text_press, "Tlak: %.0f hPa", rx_press_hPa);

			              

			               
			                ILI9341_DrawText(10, 40, text_temp, ILI9341_GREEN, ILI9341_BLACK, 2);
			                ILI9341_DrawText(10, 65, text_press, ILI9341_YELLOW, ILI9341_BLACK, 2);

			            } else {
			                
			                ILI9341_FillRectangle(10, 40, 300, 20, ILI9341_BLACK);
			                ILI9341_DrawText(10, 40, last_rx_payload, ILI9341_WHITE, ILI9341_BLACK, 2);
			            }

		}
    }
}

void MQTT_Publish_Message(const char *message) {
    if (my_mqtt_client != NULL && mqtt_client_is_connected(my_mqtt_client)) {

      
        strncpy(last_tx_payload, message, sizeof(last_tx_payload) - 1);
        last_tx_payload[sizeof(last_tx_payload) - 1] = '\0';

       
        err_t err = mqtt_publish(my_mqtt_client, "mqtt/1", message, strlen(message), 0, 0, NULL, NULL);

        if (err == 0) { 
        	CLI_Print_Async("\033[36m[Odeslano] %s\033[0m", message);
        } else {
           
            printf("\033[31m[Chyba LwIP] Neodeslano! Kod chyby: %d\033[0m\r\n", err);
        }
    } else {
        printf("\033[31m[Chyba] Nejsi pripojen k MQTT!\033[0m\r\n");
    }
}


void MQTT_Subscribe_Topic(const char *topic) {
    if (my_mqtt_client != NULL && mqtt_client_is_connected(my_mqtt_client)) {
        mqtt_subscribe(my_mqtt_client, topic, 0, NULL, NULL);
        printf("\r\n\033[1;33m[Monitor] Odebírám téma: '%s'\033[0m\r\n", topic);
    }
}

void MQTT_Unsubscribe_Topic(const char *topic) {
    if (my_mqtt_client != NULL && mqtt_client_is_connected(my_mqtt_client)) {
        mqtt_unsubscribe(my_mqtt_client, topic, NULL, NULL);
        printf("\r\n\033[1;33m[Monitor] Odhlášeno z tématu: '%s'\033[0m\r\n", topic);
    }
}

void MQTT_Toggle_Monitor(uint8_t state) {
    if (my_mqtt_client != NULL && mqtt_client_is_connected(my_mqtt_client)) {
        if (state == 1) {
           
            mqtt_subscribe(my_mqtt_client, "#", 0, NULL, NULL);
            printf("\r\n\033[1;33m[Monitor] Sledovani VSECH zprav (topic '#') ZAPNUTO.\033[0m\r\n");
        } else {
           
            mqtt_unsubscribe(my_mqtt_client, "#", NULL, NULL);
            printf("\r\n\033[1;33m[Monitor] Sledovani VSECH zprav VYPNUTO.\033[0m\r\n");
        }
    } else {
        printf("\r\n\033[31m[Chyba] Nejsi pripojen k MQTT!\033[0m\r\n");
    }
}
static void mqtt_sub_request_cb(void *arg, err_t result) {
    if (result == ERR_OK) {
        printf("[MQTT] Odber tematu uspesne potvrzen brokerem.\r\n");
    } else {
        printf("[MQTT] CHYBA: Broker odmitl odber (kod: %d)\r\n", result);
    }
}


void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if(status == MQTT_CONNECT_ACCEPTED) {
        HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET); 
        BMP180_Init(&hi2c1);

        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);

        mqtt_subscribe(client, "xtichy28_mqtt/status", 0, mqtt_sub_request_cb, NULL);


        mqtt_subscribe(client, "testtopic/test", 0, NULL, arg);

        mqtt_subscribe(client, "mqtt/led", 0, NULL, arg);
        mqtt_subscribe(client, "mqtt/displej", 0, NULL, arg);
        mqtt_subscribe(client, "mqtt/teplota", 0, NULL, arg);
        mqtt_subscribe(client, "mqtt/1", 0, NULL, arg);

        printf("\r\n\033[1;32m--- Uspesne pripojeno k MQTT Brokeru! ---\033[0m\r\nNucleo> ");
    } else {
    	HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET); 
    	printf("\r\n\033[31m[Chyba] MQTT spojeni zamitnuto/spadlo. Kod chyby: %d\033[0m\r\nNucleo> ", status);
    }
}


void connect_to_mqtt(void) {
    struct mqtt_connect_client_info_t ci;
    memset(&ci, 0, sizeof(ci));
    ci.client_id = "nucleo_f439_klient_v2";

    
    if (my_mqtt_client == NULL) {
        my_mqtt_client = mqtt_client_new();
        if (my_mqtt_client == NULL) {
            printf("\r\n\033[1;31m[Chyba] Malo RAM pro MQTT klienta!\033[0m\r\n");
            return;
        }
    }

    mqtt_client_connect(my_mqtt_client, &broker_ipaddr, 1883, mqtt_connection_cb, 0, &ci);
}




extern struct netif gnetif;




void MQTT_Network_Task(void) {
    MX_LWIP_Process();
    static uint32_t last_check = 0;


    if (netif_is_link_up(&gnetif) && !ip4_addr_isany_val(*netif_ip4_addr(&gnetif))) {

      
        if (HAL_GetTick() - last_check > 10000) {
            last_check = HAL_GetTick();

            if (my_mqtt_client == NULL || !mqtt_client_is_connected(my_mqtt_client)) {
                printf("\033[33m[MQTT] Pokus o pripojeni...\033[0m\r\n");

                IP4_ADDR(&broker_ipaddr, 192, 168, 53, 15);
                connect_to_mqtt();
            }
        }


        if (my_mqtt_client != NULL && mqtt_client_is_connected(my_mqtt_client)) {

       
            if (is_monitoring == 0) {

             
                if (timer_chce_merit == 1) {
                    timer_chce_merit = 0; 

                    float teplota = 0.0f;
                    float tlak = 0.0f;

                    
                    if (BMP180_Read_All(&hi2c1, &teplota, &tlak) == 1) {
                        char zprava[128];

                        
                        snprintf(zprava, sizeof(zprava), "{\"teplota\": \"%.1f\", \"tlak\": \"%.1f\"}", teplota, tlak);
                        MQTT_Publish_Message(zprava);
                    } else {
                        MQTT_Publish_Message("{\"chyba\": \"Senzor na I2C neodpovida!\"}");
                    }
                }
            }
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM3) {
        static uint16_t ms_counter = 0;

        ms_counter++; 

        if (ms_counter >= 5000) { 
            timer_chce_merit = 1; 
            ms_counter = 0;

        }
    }
}


