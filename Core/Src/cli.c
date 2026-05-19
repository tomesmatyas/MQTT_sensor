#include "cli.h"
#include "main.h"
#include "mqtt_app.h"
#include "lwip.h"
#include "lwip/dhcp.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h> 


extern UART_HandleTypeDef huart3;


uint8_t rx_char;
char rx_buffer[128];
uint8_t rx_idx = 0;
volatile uint8_t cmd_ready = 0;
volatile uint8_t is_monitoring = 0;
char current_monitor_topic[64] = "#";


#define CMD_HISTORY_LEN 5
char cmd_history[CMD_HISTORY_LEN][128];
uint8_t history_head = 0;
uint8_t history_count = 0;
int8_t history_view_idx = -1;
static uint8_t esc_state = 0;


int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart3, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

// Chytrý výpis na pozadí
void CLI_Print_Async(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    printf("\r\033[2K"); 
    printf("%s\r\n", buffer); 

    // Obnovíme prompt
    printf("Nucleo> ");
    if (rx_idx > 0) {
        rx_buffer[rx_idx] = '\0';
        printf("%s", rx_buffer);
    }
    fflush(stdout); 
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART3) {

       
        if (rx_char == 0x03) {
            if (is_monitoring) {
                is_monitoring = 0;
                MQTT_Unsubscribe_Topic(current_monitor_topic);
                CLI_Print_Async("\033[1;31m[Monitor] Ukonceno (Ctrl+C), vracim se k mereni...\033[0m");
                rx_idx = 0;
            }
            HAL_UART_Receive_IT(&huart3, &rx_char, 1);
            return;
        }

        if (is_monitoring) {
            HAL_UART_Receive_IT(&huart3, &rx_char, 1);
            return;
        }

      
        if (rx_char == '\x1B') { esc_state = 1; HAL_UART_Receive_IT(&huart3, &rx_char, 1); return; }
        if (esc_state == 1)    { esc_state = (rx_char == '[') ? 2 : 0; HAL_UART_Receive_IT(&huart3, &rx_char, 1); return; }
        if (esc_state == 2) {
            if (rx_char == 'A') { 
                if (history_count > 0) {
                    history_view_idx = (history_view_idx == -1) ? ((history_head - 1 + CMD_HISTORY_LEN) % CMD_HISTORY_LEN) : ((history_view_idx - 1 + CMD_HISTORY_LEN) % CMD_HISTORY_LEN);
                    strcpy(rx_buffer, cmd_history[history_view_idx]);
                    rx_idx = strlen(rx_buffer);
                    printf("\r\033[2KNucleo> %s", rx_buffer);
                    fflush(stdout); 
                }
            } else if (rx_char == 'B') { 
                if (history_count > 0 && history_view_idx != -1) {
                    history_view_idx = (history_view_idx + 1) % CMD_HISTORY_LEN;
                    if (history_view_idx == history_head) { 
                        history_view_idx = -1;
                        rx_idx = 0;
                        printf("\r\033[2KNucleo> ");
                        fflush(stdout); 
                    } else {
                        strcpy(rx_buffer, cmd_history[history_view_idx]);
                        rx_idx = strlen(rx_buffer);
                        printf("\r\033[2KNucleo> %s", rx_buffer);
                        fflush(stdout); 
                    }
                }
            }
            esc_state = 0;
            HAL_UART_Receive_IT(&huart3, &rx_char, 1);
            return;
        }

    
        if (rx_char == '\b' || rx_char == 127) {
            if (rx_idx > 0) {
                rx_idx--;
                printf("\b \b");
                fflush(stdout);
            }
        }
       
        else if (rx_char == '\r' || rx_char == '\n') {
            rx_buffer[rx_idx] = '\0';
            cmd_ready = 1;
            printf("\r\n");

           
            if (rx_idx > 0) {
                strcpy(cmd_history[history_head], rx_buffer);
                history_head = (history_head + 1) % CMD_HISTORY_LEN;
                if (history_count < CMD_HISTORY_LEN) history_count++;
            }
            history_view_idx = -1; 

        }
       
        else {
            rx_buffer[rx_idx++] = rx_char;
            if (rx_idx >= 127) rx_idx = 0;
            HAL_UART_Transmit(&huart3, &rx_char, 1, 10);
        }

        HAL_UART_Receive_IT(&huart3, &rx_char, 1);
    }
}


void CLI_Init(void) {
    setvbuf(stdout, NULL, _IONBF, 0); 
    printf("\r\n\033[1;36m==================================\033[0m\r\n");
    printf("   Nucleo OS - Zadejte 'help'     \r\n");
    printf("\033[1;36m==================================\033[0m\r\nNucleo> ");
    fflush(stdout);
    HAL_UART_Receive_IT(&huart3, &rx_char, 1);
}

void CLI_Task(void) {
    if (cmd_ready == 1) {
        if (strlen(rx_buffer) > 0) {

           
            if (strncmp(rx_buffer, "help", 4) == 0) {
                printf("Dostupne prikazy:\r\n");
                printf("  help        - Vypise tuto napovedu\r\n");
                printf("  info        - Detailni sitove informace (IP, Maska, Brana)\r\n");
                printf("  uptime      - Doba behu systemu\r\n");
                printf("  led <on/off>- Ovladani modre LED\r\n");
                printf("  send <text> - Odesle zpravu pres MQTT\r\n");
                printf("  renew       - Znovu pozada router o IP adresu (DHCP)\r\n");
                printf("  reboot      - Restartuje celou desku\r\n");
                printf("  clear       - Vymaze obrazovku\r\n");
                printf("  monitor     - naslouchání brokeru (on/off)\r\n");
                printf("  last        - poslední zpráva na brokeru\r\n");
            }
                       
            else if (strncmp(rx_buffer, "ip", 2) == 0) {
                extern struct netif gnetif;
                printf("Moje IP adresa: %s\r\n", ip4addr_ntoa(netif_ip4_addr(&gnetif)));
            }
            else if (strncmp(rx_buffer, "last", 4) == 0) {
                extern char last_rx_topic[];
                extern char last_rx_payload[];
                extern char last_tx_payload[];

                printf("\r\n\033[35m--- Historie MQTT ---\033[0m\r\n");
                printf("Posledni ODESLANA zprava: %s\r\n", last_tx_payload);
                printf("Posledni PRIJATE tema:    %s\r\n", last_rx_topic);
                printf("Posledni PRIJATA zprava:  %s\r\n", last_rx_payload);
            }
            else if (strncmp(rx_buffer, "info", 4) == 0) {
                extern struct netif gnetif;
                printf("\r\n--- Sitove informace ---\r\n");
                if (netif_is_up(&gnetif)) {
                    printf("Stav:       \033[32mPRIPOJENO\033[0m\r\n");
                    printf("IP adresa:  %s\r\n", ip4addr_ntoa(netif_ip4_addr(&gnetif)));
                    printf("Maska:      %s\r\n", ip4addr_ntoa(netif_ip4_netmask(&gnetif)));
                    printf("Brana (GW): %s\r\n", ip4addr_ntoa(netif_ip4_gw(&gnetif)));
                } else {
                    printf("Stav:       \033[31mODPOJENO\033[0m\r\n");
                }
            }
            else if (strncmp(rx_buffer, "monitor", 7) == 0) {
                if (strlen(rx_buffer) > 8 && rx_buffer[7] == ' ') {
                    strncpy(current_monitor_topic, rx_buffer + 8, sizeof(current_monitor_topic) - 1);
                    current_monitor_topic[sizeof(current_monitor_topic) - 1] = '\0';
                } else {
                    strcpy(current_monitor_topic, "#");
                }

                is_monitoring = 1;
                MQTT_Subscribe_Topic(current_monitor_topic);

                printf("\033[1;33m[Monitor] Nasloucham na tematu '%s'...\r\n", current_monitor_topic);
                printf("Pro navrat do menu stisknete Ctrl+C\033[0m\r\n");
            }
            
            else if (strncmp(rx_buffer, "uptime", 6) == 0) {
                uint32_t sec = HAL_GetTick() / 1000;
                printf("Uptime: %lu hodin, %lu minut, %lu vterin\r\n", sec / 3600, (sec % 3600) / 60, sec % 60);
            }
            
            else if (strncmp(rx_buffer, "led ", 4) == 0) {
                char *arg = rx_buffer + 4;
                if (strncmp(arg, "on", 2) == 0) {
                    HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_SET);
                    printf("Modra LED zapnuta.\r\n");
                } else if (strncmp(arg, "off", 3) == 0) {
                    HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_RESET);
                    printf("Modra LED vypnuta.\r\n");
                } else {
                    printf("Pouziti: led on | led off\r\n");
                }
            }
           
            else if (strncmp(rx_buffer, "reboot", 6) == 0) {
                printf("\033[1;31mRestartuji procesor...\033[0m\r\n\n");
                HAL_Delay(100);
                NVIC_SystemReset();
            }
            
            else if (strncmp(rx_buffer, "renew", 5) == 0) {
                extern struct netif gnetif;
                printf("\033[33m[SIT] Zastavuji stare DHCP a zadam o novou IP...\033[0m\r\n");
                dhcp_release(&gnetif);
                dhcp_stop(&gnetif);
                dhcp_start(&gnetif);
                printf("Pozadavek odeslan! Zkuste napsat 'ip' za cca 5 vterin.\r\n");
            }
           
            else if (strncmp(rx_buffer, "clear", 5) == 0) {
                printf("\033[2J\033[H");
            }
           
            else if (strncmp(rx_buffer, "send ", 5) == 0) {
                char *payload = rx_buffer + 5;
                MQTT_Publish_Message(payload);
            }
            
            else {
                printf("Neznamy prikaz: '%s'. Zadejte 'help'.\r\n", rx_buffer);
            }
        }
        rx_idx = 0;
        cmd_ready = 0;
        printf("Nucleo> ");
        fflush(stdout); 
    }
}
