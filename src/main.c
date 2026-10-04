#include "driver/gptimer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h" 
#include "esp_attr.h"
#include "esp_log.h"

uint8_t led_1=0;
uint8_t led_2=0;
uint8_t led_3=0;
TaskHandle_t task_func_200_handle;
TaskHandle_t task_func_300_handle;
TaskHandle_t task_func_500_handle;
gptimer_alarm_config_t alarm_for_200;
gptimer_alarm_config_t alarm_for_300;
gptimer_alarm_config_t alarm_for_500;
gptimer_handle_t timer_for_itr_handle_1;
gptimer_handle_t timer_for_itr_handle_2;
gptimer_handle_t timer_for_itr_handle_3;
uint8_t isr_status=0;

void gpio_setup_f(){
    gpio_config_t gpio_200_300_500={
        .pin_bit_mask=(1ULL<<46)|(1ULL<<14)|(1ULL<<3),
        .mode=GPIO_MODE_OUTPUT,
        .pull_down_en=GPIO_PULLDOWN_ENABLE
    };
    gpio_config(&gpio_200_300_500);
    gpio_set_level(46,1);
    gpio_set_level(14,1);
    gpio_set_level(3,1);
}

bool IRAM_ATTR timer_handler_200(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
    BaseType_t is_awoken=pdFALSE;
    vTaskNotifyGiveFromISR(task_func_200_handle, &is_awoken);
    return is_awoken==pdTRUE;
}

bool IRAM_ATTR timer_handler_300(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
    BaseType_t is_awoken=pdFALSE;
    vTaskNotifyGiveFromISR(task_func_300_handle, &is_awoken);
    return is_awoken==pdTRUE;
}

bool IRAM_ATTR timer_handler_500(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
    BaseType_t is_awoken=pdFALSE;
    vTaskNotifyGiveFromISR(task_func_500_handle, &is_awoken);
    return is_awoken==pdTRUE;
}

void task_func_200(void *pvParameters){
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        led_1=!led_1;
        gpio_set_level(46,led_1);
    }
}

void task_func_300(void *pvParameters){
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        led_2=!led_2;
        gpio_set_level(14,led_2);
    }
}

void task_func_500(void *pvParameters){
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        led_3=!led_3;
        gpio_set_level(3,led_3);
    }
}

void task_setup(){
    xTaskCreate(task_func_200, "task_func_200", 4096, NULL, 1, &task_func_200_handle);
    xTaskCreate(task_func_300, "task_func_300", 4096, NULL, 1, &task_func_300_handle);
    xTaskCreate(task_func_500, "task_func_500", 4096, NULL, 1, &task_func_500_handle);
}

void timer_setup_f(){
    gptimer_config_t timer_for_itr={
        .clk_src=GPTIMER_CLK_SRC_DEFAULT,
        .resolution_hz=1000000,
        .direction=GPTIMER_COUNT_UP,
        .intr_priority=0
    };
    gptimer_new_timer(&timer_for_itr, &timer_for_itr_handle_1);
    gptimer_new_timer(&timer_for_itr, &timer_for_itr_handle_2);
    gptimer_new_timer(&timer_for_itr, &timer_for_itr_handle_3);

    alarm_for_200.alarm_count=200000;
    alarm_for_200.reload_count=0;
    alarm_for_200.flags.auto_reload_on_alarm=true;

    alarm_for_300.alarm_count=300000;
    alarm_for_300.reload_count=0;
    alarm_for_300.flags.auto_reload_on_alarm=true;

    alarm_for_500.alarm_count=500000;
    alarm_for_500.reload_count=0;
    alarm_for_500.flags.auto_reload_on_alarm=true;
    
    
    gptimer_event_callbacks_t timer_for_isr_callbacks_200 = {.on_alarm=timer_handler_200};
    gptimer_event_callbacks_t timer_for_isr_callbacks_300 = {.on_alarm=timer_handler_300};
    gptimer_event_callbacks_t timer_for_isr_callbacks_500 = {.on_alarm=timer_handler_500};
    gptimer_set_alarm_action(timer_for_itr_handle_1, &alarm_for_200);
    gptimer_set_alarm_action(timer_for_itr_handle_2, &alarm_for_300);
    gptimer_set_alarm_action(timer_for_itr_handle_3, &alarm_for_500);
    gptimer_register_event_callbacks(timer_for_itr_handle_1, &timer_for_isr_callbacks_200, NULL);
    gptimer_register_event_callbacks(timer_for_itr_handle_2, &timer_for_isr_callbacks_300, NULL);
    gptimer_register_event_callbacks(timer_for_itr_handle_3, &timer_for_isr_callbacks_500, NULL);
    gptimer_enable(timer_for_itr_handle_1);
    gptimer_enable(timer_for_itr_handle_2);
    gptimer_enable(timer_for_itr_handle_3);
    gptimer_start(timer_for_itr_handle_1);
    gptimer_start(timer_for_itr_handle_2);
    gptimer_start(timer_for_itr_handle_3);
}

void app_main() {
    gpio_setup_f();
    task_setup();
    timer_setup_f();
    while(1){
        vTaskDelay(100);
    }
}