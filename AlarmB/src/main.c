#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include "fsm.h"

#define PERIOD_TICK 100/portTICK_RATE_MS

portTickType REBOUND_TICK = 100;

#define GPIO_BUTTON_A 0 //D3 "conectado al pin de G"
#define GPIO_BUTTON_P 15 //D8 "conectado al pin de 3.3V"
#define GPIO_LED 2 //LED

uint32 user_rf_cal_sector_set(void){
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;
    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

int armar(fsm_t *this){
  if(!GPIO_INPUT_GET(GPIO_BUTTON_A)){
    static portTickType xLastISRTick0 = 0;
    portTickType now = xTaskGetTickCount ();
    if (now > xLastISRTick0) {
      xLastISRTick0 = now + REBOUND_TICK;
        printf("%s\n","armar" );
      return 1;
    }
    else{ return 0; }
  }
  else{ return 0; }
};

int pres(fsm_t *this){
  if(GPIO_INPUT_GET(GPIO_BUTTON_P)){
    static portTickType xLastISRTick0 = 0;
    portTickType now = xTaskGetTickCount ();
    if (now > xLastISRTick0) {
      xLastISRTick0 = now + REBOUND_TICK;
        printf("%s\n","presencia" );
      return 1;
    }
    else{ return 0; }
  }
  else{ return 0; }
};

void alarm_on (fsm_t *this){
  GPIO_OUTPUT_SET(GPIO_LED, 0); //ON
}

void alarm_off (fsm_t *this){
  GPIO_OUTPUT_SET(GPIO_LED, 1); //OFF
}

enum fsm_state {
    ALARM_ON,
    ALARM_OFF
};

static fsm_trans_t interruptor[] = {
  {ALARM_OFF, armar, ALARM_ON, alarm_off},
  {ALARM_ON, pres, ALARM_ON, alarm_on},
  {ALARM_ON, armar, ALARM_OFF, alarm_off},
  {-1, NULL, -1, NULL}
};

void inter(void* ignore){
  fsm_t* fsm = fsm_new(interruptor);
  alarm_off(fsm);
  portTickType xLastWakeTime;

  PIN_FUNC_SELECT(GPIO_PIN_REG_15, FUNC_GPIO15);

  while(true) {
    xLastWakeTime = xTaskGetTickCount ();
    fsm_fire(fsm);
    vTaskDelayUntil(&xLastWakeTime, PERIOD_TICK);
  }
}

void user_init(void){
    xTaskCreate(&inter, "startup", 2048, NULL, 1, NULL);
}
