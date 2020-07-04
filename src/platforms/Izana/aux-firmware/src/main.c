#include "main.h"

void main(void)
{
	uint8_t cmd = 0;
	bool status = false;

	device_init();

	while(1) {
		do {
			status = (!(cmd = device_read_cmd())) && 
						(!device_get_over_current_status()) && 
						(!device_get_adc_status());
		} while(status);

		if(device_get_adc_status()) {
			device_process_adc();
		}

		if(cmd) {
			device_process_cmd(cmd);
		}
		
		if(device_get_over_current_status()) {
			device_process_over_current();
		}
	}
}