#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/*ADC Y PORT SERIAL*/

#define TEMP_SENSOR_CH 4

QueueHandle_t q_adc;

void task_print(void *params){

    float temp_c;
     while(1){
         xQueueReceive(q_adc, &temp_c, portMAX_DELAY);
        printf("t =%2fC\n", temp_c);
        vTaskDelay(pdMS_TO_TICKS(3000));/*aqui introduzco unos 3 segundos entre impresion e impresion*/
    }
}

void task_adc(void *params){

    uint16_t raw;
    float voltaje;
    float temp_c;

    while(1){
        raw=adc_read();
        float voltaje = raw * 3.3f / 4096.0f;
        float temp_c = 27.0 - ((voltaje - 0.706) / 0.001721);       
         xQueueSend(q_adc, &temp_c, portMAX_DELAY);
    }
}

int main(){

    stdio_init_all();

    adc_init();
     adc_set_temp_sensor_enabled(true);
   
    adc_select_input(TEMP_SENSOR_CH);

    q_adc=xQueueCreate(1, sizeof(float));
    xTaskCreate(task_print,"Print", 2*configMINIMAL_STACK_SIZE, NULL,1,NULL);
    xTaskCreate(task_adc,"ADC", 2*configMINIMAL_STACK_SIZE, NULL,1,NULL);

     vTaskStartScheduler();
     while(true);
}
