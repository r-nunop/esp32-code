/* LIBRERÍAS ESTÁNDAR DE C*/
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

/* LIBRERÍAS ESPECÍFICAS DEL ESP32 */
#include <driver/gpio.h> 		//ES
#include "esp_err.h" 			//manejo de errores
#include "soc/gpio_num.h" 		//referencia a números de pines
#include <freertos/FreeRTOS.h> 	//delay, interrupciones y manejo de tareas.
#include "esp_log.h" 			//logs

/* Puerto de la ESP32 que conecta con el LED interno de la placa 
(el verde, hay 3 puertos para referenciar al RGB, el 0, el 2 y el 4) */
#define LED_GPIO GPIO_NUM_2 

// Puerto del switch
#define SWITCH_GPIO GPIO_NUM_5 

static const char* TAG = "R1";


static void configure_led()  {
	//1º: reseteamos el pin antes de configurarlo
	//Nota: TODAS las funciones se llaman con captura de errores para capturar los errores que puedan llegar a darnos
	ESP_LOGI(TAG, "Reseteando pin del LED");
	ESP_ERROR_CHECK(gpio_reset_pin(LED_GPIO));
	
	//2º: declaramos la funcionalidad (como es un LED, solamente es salida)
	ESP_LOGI(TAG, "Configurando pin del LED");
	ESP_ERROR_CHECK(gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT));
}

static void configure_switch() {
	//1ª: reseteamos el pin antes de configurarlo
	ESP_LOGI(TAG, "Reseteando pin del switch");
	ESP_ERROR_CHECK(gpio_reset_pin(SWITCH_GPIO));
	
	//2º: configuramos la funcionalidad (entrada)
	ESP_LOGI(TAG, "Configurando pin del switch");
	ESP_ERROR_CHECK(gpio_set_direction(SWITCH_GPIO, GPIO_MODE_INPUT));
	
	/* 3º: como estamos configurando un interruptor, tendremos que configurar una resistencia pull para la correcta funcionalidad del cirtuito 
	(en este caso, mejor pull-down) */
	ESP_LOGI(TAG, "Definiendo resistencia del switch");
	ESP_ERROR_CHECK(gpio_set_pull_mode(SWITCH_GPIO, GPIO_PULLDOWN_ONLY));
}

static void blink_led(gpio_num_t led, u_int32_t ticks, u_int32_t *tick) {
	//1º: cambiamos el valor de la variable
	ESP_LOGI(TAG, "Estado del led: %s", *tick ? "Encendido" : "Apagado");
	ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(LED_GPIO, *tick));
	
	//2º: cambiamos el valor de la variable del tick
	*tick = !(*tick);
	
	//3º: generamos interrupción para el parpadeo del led
	ESP_LOGI(TAG, "Tasa de refresco actual: %u", (unsigned)ticks);
	vTaskDelay(ticks / portTICK_PERIOD_MS );
}

void app_main(void)
{
	bool estado = 0;
	u_int32_t ticks;
	u_int32_t tick = 0;

	configure_led();
	configure_switch();
		
    while (true) {
		//1º: recogemos estado del LED
		estado = gpio_get_level(SWITCH_GPIO);
		
		//2º: establecemos frecuencia según estado
		if(estado == 0) {
			ticks = 1000; //1000ms, 1Hz o 1s
		} else {
			ticks = 500; //500ms, 2Hz o 0.5s 
		}
			
		//3º: ejecutamos el blink según los estados
		blink_led(LED_GPIO, ticks, &tick);
    }
}
