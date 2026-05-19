#ifndef MQTT_APP_H
#define MQTT_APP_H

#include <stdint.h> 

void MQTT_Network_Task(void);
void MQTT_Publish_Message(const char *message);
void MQTT_Toggle_Monitor(uint8_t state);
void MQTT_Subscribe_Topic(const char *topic);
void MQTT_Unsubscribe_Topic(const char *topic);

#endif
