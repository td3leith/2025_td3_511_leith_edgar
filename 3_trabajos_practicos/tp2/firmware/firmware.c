/*ADC:Primera version con interrupciones y el freeRTOS*/

#include <stdio.h>    // Para printf
#include <stdlib.h>   // Para funciones generales (aunque no muy usada aquí directamente)
#include <math.h>     // Para funciones matemáticas (aunque la fórmula es simple)
#include "pico/stdlib.h" // Para funciones básicas de la Pico SDK (stdio_init_all)
#include "hardware/adc.h" // Para el ADC
#include "FreeRTOS.h" // Para FreeRTOS (tipos de datos, scheduler)
#include "task.h"     // Para la creación y manejo de tareas FreeRTOS
#include "queue.h"    // Para la creación y manejo de colas FreeRTOS
#include "hardware/irq.h" // Para el manejo de interrupciones (ADC_IRQ_FIFO)

/* Definición del canal del sensor. Usamos 4 para el sensor interno */
#define TEMP_SENSOR_CH 4

// Declaración global de la cola para enviar datos del ADC
// Usamos 'QueueHandle_t' que es el tipo correcto en FreeRTOS
QueueHandle_t q_adc; // Nombre más claro y consistente con sugerencias previas

// Función Handler de la interrupción del ADC
void adc_isr(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint16_t value_adc; // Declaración correcta de la variable para el valor ADC raw
    float temp_c; // Variable para almacenar la temperatura convertida

    // Siempre es bueno verificar el nivel del FIFO, aunque con una conversión única no es tan crítico
    if (adc_fifo_get_level() > 0) {
        value_adc = adc_fifo_get(); // Obtener el valor del ADC del FIFO

        // Conversión a temperatura: se usa la fórmula que propusiste en el código base.
        // Asegúrate que esta fórmula es la correcta para tu sensor interno si no estás seguro.
        // (Nota: la mía era 27 - (voltaje - 0.706f) / 0.001721f; la tuya es 24 - (0.706f-voltaje)/0.001721)
        float voltaje = value_adc * (3.3f / 4095.0f); // Usar 3.3f y 4095.0f para float
        temp_c = 24.0f - (0.706f - voltaje) / 0.001721f; // Asegurar que todos los literales sean float

        // Enviar la temperatura (float) a la cola desde la ISR
        xQueueSendFromISR(q_adc, &temp_c, &xHigherPriorityTaskWoken);
    }

    // Esta línea le indica al sistema operativo FreeRTOS que, si durante la ejecución
    // de esta interrupción se despertó una tarea de mayor prioridad,
    // debe conmutar a esa tarea inmediatamente después de salir de la ISR.
    // Esto asegura que la tarea de mayor prioridad se ejecute lo antes posible.
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    irq_clear(ADC_IRQ_FIFO); // Limpiar la bandera de interrupción del ADC
}

// Función para inicializar y configurar el ADC para el sensor interno
void set_adc(void) {
    adc_init(); // Inicializar el módulo ADC (corregido: quitado ':')

    // Como usamos el sensor de temperatura interno (canal 4), no necesitamos adc_gpio_init().
    // Solo habilitamos el sensor y seleccionamos su canal.
    adc_set_temp_sensor_enabled(true); // Habilitar el sensor de temperatura interno (corregido: typo 'sest' a 'set')
    adc_select_input(TEMP_SENSOR_CH); // Seleccionar el canal 4 para el sensor interno

    /* Configuración y activación de las interrupciones del ADC */
    // setup(true, true, 1, false, false):
    // - true (habilitar FIFO)
    // - true (habilitar interrupción cuando el FIFO tiene 1 elemento)
    // - 1 (número de elementos en el FIFO para disparar la interrupción)
    // - false (no redondear)
    // - false (no errores de sobre-lectura)
    adc_fifo_setup(true, true, 1, false, false); // Corregido: 'ture' a 'true'
    adc_irq_set_enabled(true); // Habilitar la interrupción del ADC
    irq_set_exclusive_handler(ADC_IRQ_FIFO, adc_isr); // Asignar el handler de interrupción (adc_isr)
    irq_set_enabled(ADC_IRQ_FIFO, true); // Habilitar la interrupción globalmente
    // La función adc_run(true) se moverá a una tarea separada para disparar periódicamente.
}

// Tarea para imprimir la temperatura recibida de la cola
void Impresion_temp(void *pvParameters) { // Renombrado a pvParameters por convención FreeRTOS
    float current_temp_value; // Variable para almacenar la temperatura recibida

    while (1) {
        // Intentar recibir un valor de la cola con un tiempo de espera de 100ms
        // Corregido: 'wQueueReceive' a 'xQueueReceive'
        if (xQueueReceive(q_adc, &current_temp_value, pdMS_TO_TICKS(100)) == pdTRUE) {
            // Imprimir el valor recibido y convertido
            printf("Temperatura: %.2f C\n", current_temp_value);
        }
        vTaskDelay(pdMS_TO_TICKS(1500)); // Esperar 1.5 segundos antes de intentar leer de nuevo
    }
}

// Nueva tarea para disparar periódicamente las conversiones del ADC
void Tarea_disparo_adc(void *pvParameters) {
    while (1) {
        adc_run(true); // Iniciar una conversión ADC
        vTaskDelay(pdMS_TO_TICKS(1000)); // Disparar una conversión cada 1 segundo
    }
}

int main(void) {
    stdio_init_all(); // Inicializar stdio para printf (corregido: quitado ':')
    printf("Iniciando ADC con interrupciones y FreeRTOS para sensor interno...\n");

    // Crear la cola donde se almacenarán los datos del ADC
    // La cola almacenará valores de tipo float (sizeof(float))
    q_adc = xQueueCreate(5, sizeof(float)); // Corregido: 'lec_adc' a 'q_adc', tamaño a 'float'

    if (q_adc == NULL) { // Verificar si la cola se creó correctamente
        while (1) {
            printf("Error: No se pudo crear la cola del ADC.\n"); // Corregido: añadir ';'
            // Un delay para no spamear la consola y dar tiempo a leer el error
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // Configurar el ADC y sus interrupciones
    set_adc();

    // Crear las tareas necesarias
    // Impresion_temp (prioridad 1)
    xTaskCreate(Impresion_temp, "Receptora", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL); // Aumento el stack por seguridad
    // Tarea_disparo_adc (prioridad 2, para que dispare el ADC)
    xTaskCreate(Tarea_disparo_adc, "DisparadorADC", configMINIMAL_STACK_SIZE, NULL, 2, NULL);

    // Iniciar el scheduler de FreeRTOS
    vTaskStartScheduler();

    // Este bucle infinito solo se alcanzará si el scheduler no inicia
    while (1);
}