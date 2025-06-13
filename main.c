#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdlib.h"
#include <stdio.h>

// Pinos
#define LED_R 11
#define LED_G 12
#define LED_B 13
#define BUZZER 10
#define BOTAO_A 5
#define BOTAO_B 6

// Handles das tarefas
TaskHandle_t ledTaskHandle = NULL;
TaskHandle_t buzzerTaskHandle = NULL;

// === Tarefa LED RGB ===
void led_task(void *params) {
    const uint LED_PINS[3] = {LED_R, LED_G, LED_B};
    int i = 0;
    while (1) {
        for (int j = 0; j < 3; j++) {
            gpio_put(LED_PINS[j], j == i ? 1 : 0);
        }
        i = (i + 1) % 3;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// === Tarefa Buzzer ===
void buzzer_task(void *params) {
    while (1) {
        gpio_put(BUZZER, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_put(BUZZER, 0);
        vTaskDelay(pdMS_TO_TICKS(900));
    }
}

// === Tarefa de botões ===
void botao_task(void *params) {
    bool ledSuspenso = false;
    bool buzzerSuspenso = false;

    while (1) {
        // Botão A - LED
        if (!gpio_get(BOTAO_A)) {
            vTaskDelay(pdMS_TO_TICKS(200));  // debounce
            if (ledSuspenso) {
                vTaskResume(ledTaskHandle);
                ledSuspenso = false;
            } else {
                vTaskSuspend(ledTaskHandle);
                ledSuspenso = true;
            }
            while (!gpio_get(BOTAO_A));  // espera soltar
        }

        // Botão B - Buzzer
        if (!gpio_get(BOTAO_B)) {
            vTaskDelay(pdMS_TO_TICKS(200));  // debounce
            if (buzzerSuspenso) {
                vTaskResume(buzzerTaskHandle);
                buzzerSuspenso = false;
            } else {
                vTaskSuspend(buzzerTaskHandle);
                buzzerSuspenso = true;
            }
            while (!gpio_get(BOTAO_B));  // espera soltar
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// === Main ===
int main() {
    stdio_init_all();

    // Inicializa GPIOs
    gpio_init(LED_R); gpio_set_dir(LED_R, GPIO_OUT);
    gpio_init(LED_G); gpio_set_dir(LED_G, GPIO_OUT);
    gpio_init(LED_B); gpio_set_dir(LED_B, GPIO_OUT);
    gpio_init(BUZZER); gpio_set_dir(BUZZER, GPIO_OUT);

    gpio_init(BOTAO_A); gpio_set_dir(BOTAO_A, GPIO_IN); gpio_pull_up(BOTAO_A);
    gpio_init(BOTAO_B); gpio_set_dir(BOTAO_B, GPIO_IN); gpio_pull_up(BOTAO_B);

    // Cria tarefas
    xTaskCreate(led_task, "LED RGB", 256, NULL, 1, &ledTaskHandle);
    xTaskCreate(buzzer_task, "Buzzer", 256, NULL, 1, &buzzerTaskHandle);
    xTaskCreate(botao_task, "Botões", 256, NULL, 2, NULL);

    // Inicia o agendador
    vTaskStartScheduler();

    while (1) {}
    return 0;
}
