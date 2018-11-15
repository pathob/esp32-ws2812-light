#include "main/isr_gpio.h"

static QueueHandle_t _isr_gpio_queue = NULL;
static unsigned long _isr_gpio_last = 0;

static void IRAM_ATTR ISR_GPIO_handler(
    void* args);

static void IRAM_ATTR ISR_GPIO_task(
    void* pvParams);

void ISR_GPIO_init()
{
    gpio_config_t gpio_conf;
    gpio_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    gpio_conf.pin_bit_mask = BIT(0);
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&gpio_conf);

    _isr_gpio_queue = xQueueCreate(10, sizeof(gpio_num_t));
    xTaskCreate(ISR_GPIO_task, "ISR_GPIO_task", 4096, NULL, (1 | portPRIVILEGE_BIT), NULL);
    gpio_isr_handler_add(0, ISR_GPIO_handler, (void *) 0);
}

static void IRAM_ATTR ISR_GPIO_handler(
    void* args)
{
    gpio_num_t gpio_num = (gpio_num_t) args;
    unsigned long us = micros();

    if (us - _isr_gpio_last > 100000) {
        xQueueSendFromISR(_isr_gpio_queue, &gpio_num, NULL);
        _isr_gpio_last = us;
    }
}

static void IRAM_ATTR ISR_GPIO_task(
    void* pvParams)
{
    gpio_num_t gpio_num;

    while(1) {
        xQueueReceive(_isr_gpio_queue, &gpio_num, portMAX_DELAY);
        STRIPE_toggle();
    }

    vTaskDelete(NULL);
}
