/* USER CODE BEGIN Includes */
#include "lcd_i2c.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* USER CODE BEGIN PV  (Private Variables) */
volatile uint32_t ic_rise  = 0;
volatile uint32_t ic_fall  = 0;
volatile uint8_t  ic_flag  = 0;   /* 0=wait-rise, 1=wait-fall, 2=done */
volatile uint32_t pulse_us = 0;
/* USER CODE END PV */

/* ── Input capture callback (called from TIM3 IRQ) ── */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance != TIM3) return;

    if (ic_flag == 0) {                         /* rising edge captured */
        ic_rise = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
        ic_flag = 1;
        /* switch to falling edge */
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2,
                                      TIM_INPUTCHANNELPOLARITY_FALLING);
    } else if (ic_flag == 1) {                   /* falling edge captured */
        ic_fall  = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
        pulse_us = (ic_fall >= ic_rise)
                   ? (ic_fall - ic_rise)
                   : (0xFFFF - ic_rise + ic_fall + 1); /* overflow wrap */
        ic_flag  = 2;
        /* restore rising edge for next measurement */
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2,
                                      TIM_INPUTCHANNELPOLARITY_RISING);
    }
}

/* ── Trigger one measurement ── */
static uint32_t hcsr04_measure(void) {
    ic_flag = 0;
    /* 10 µs trigger pulse */
    HAL_GPIO_WritePin(HC_TRIG_GPIO_Port, HC_TRIG_Pin, GPIO_PIN_SET);
    DWT_Delay_us(10);          /* see note below for DWT init */
    HAL_GPIO_WritePin(HC_TRIG_GPIO_Port, HC_TRIG_Pin, GPIO_PIN_RESET);

    uint32_t t0 = HAL_GetTick();
    while (ic_flag != 2) {
        if (HAL_GetTick() - t0 > 30) return 0;  /* 30 ms timeout */
    }
    /* distance (cm) = pulse_us / 58  (speed of sound ÷ 2) */
    return pulse_us / 58;
}

/* ── DWT microsecond delay (add to SystemClock_Config or USER CODE BEGIN 2) ──
   CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
   DWT->CYCCNT = 0;
   DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
   ── then use: ──
static inline void DWT_Delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    us *= (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < us);
} */

/* ── Main loop ── */
/* USER CODE BEGIN 2 */
    /* Enable DWT cycle counter for µs delays */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;

    LCD_Init(&hi2c1);
    LCD_SetCursor(&hi2c1, 0, 0);
    LCD_WriteString(&hi2c1, " Distance Meter ");
    LCD_SetCursor(&hi2c1, 0, 1);
    LCD_WriteString(&hi2c1, "  STM32F429ZI   ");
    HAL_Delay(1500);
    LCD_Clear(&hi2c1);

    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
    char buf[17];
    while (1) {
        uint32_t dist_cm = hcsr04_measure();

        LCD_SetCursor(&hi2c1, 0, 0);
        LCD_WriteString(&hi2c1, "Distance:       ");

        LCD_SetCursor(&hi2c1, 0, 1);
        if (dist_cm == 0 || dist_cm > 400) {
            snprintf(buf, sizeof(buf), "Out of range    ");
        } else {
            snprintf(buf, sizeof(buf), "%3lu cm         ", dist_cm);
        }
        LCD_WriteString(&hi2c1, buf);

        HAL_Delay(200);   /* ~5 Hz refresh */
    }
/* USER CODE END WHILE */
