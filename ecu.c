#include <stdio.h>
#include <stdlib.h>
#include "ecu.h"

int init_ecu(ECUState *ecu, int id, float version, int history_size, AlarmCallback callback){
	if (ecu == NULL) return 0;

	ecu->id = id;
	ecu->firmware_version = version;
	ecu->mode = STATE_ACTIVE;
	ecu->current_temp = 25.0f;
	ecu->hw_reg.fan = 0;
	ecu->hw_reg.heater = 0;
	ecu->hw_reg.alarm = 0;
	ecu->hw_reg.reserved = 0;
	ecu->history_capacity = history_size;
	ecu->history_count = 0;
    ecu->on_overheat = callback;

	ecu->temp_history = (float*) malloc(history_size * sizeof(float));

	if(ecu->temp_history == NULL){
		printf("[ERROR] Memory allocation failed for temp_history!\n");
		return 0;
	}

	for (int i = 0; i<history_size; i++){
		ecu->temp_history[i] = 0.0f;
	}

	printf("[SYSTEM] Dynamic memory allocated successfully (%d slots).\n", history_size);
    return 1;
}

void cleanup_ecu(ECUState *ecu){
	if (ecu == NULL) return;

	if (ecu->temp_history != NULL){
		free(ecu->temp_history);
		ecu->temp_history = NULL;
		printf("[SYSTEM] ECU dynamic memory deallocated safely.\n");

	}
}
void print_header(const ECUState *ecu)
{
	if(ecu == NULL) return;
	    printf("\n========================================\n");
        printf("   SYSTEM: MicroCore-OS v%.1f          \n", ecu->firmware_version);
        printf("   ECU ID: %d | MODE: [%d]           \n", ecu->id, ecu->mode);
        printf("========================================\n");
}

void print_menu(void)
{
	printf("1. View System Telemetry & History\n");
    printf("2. Log New Temperature & Triggers Actuator\n");
    printf("3. Change ECU Operational Mode\n");
	printf("4. Expand History Buffer Size (realloc)\n");
    printf("5. Save ECU State to Flash (fwrite)\n");
    printf("6. Load ECU State from Flash (fread)\n");
    printf("0. Shutdown System\n");	
}

void display_telemetry(const ECUState *ecu)
{
	if (ecu == NULL){
		printf("[ERROR] NUll pointer passed to display_telemetry!\n");
		return;
	}
	printf("\n[TELEMETRY REPORT]\n");
	printf("-> Internal Temperature: %.2f oC\n", ecu->current_temp);

	printf("!-> Dynamic History Buffer (%d/%d used):\n", ecu->history_count, ecu->history_capacity);
	for (int i = 0; i < ecu->history_capacity; i++) {
        printf("[%.1f oC] ", ecu->temp_history[i]);
    }
    printf("\n");

	printf("-> Actuator States:\n");
	printf("[1] Cooling Fan: %s\n", (ecu->hw_reg.fan) ? "ON" : "OFF");	
	printf("[2] Heater: %s\n", (ecu->hw_reg.heater) ? "ON" : "OFF");
	printf("[3] Alarm: %s\n", (ecu->hw_reg.alarm) ? "ON" : "OFF");		
}

void log_temperature(ECUState *ecu)
{
	if (ecu == NULL || ecu->temp_history == NULL) return;

	float new_temp = 0.0f;
	printf("\n[New temperature Log]\n");
	printf("Enter reading value (oC): ");
	scanf("%f", &new_temp);
	
	ecu->current_temp = new_temp;

	for (int i = ecu->history_capacity - 1; i > 0; i--)
	{
		ecu->temp_history[i] = ecu->temp_history[i - 1];
	}

	ecu->temp_history[0] = new_temp;

	if (ecu->history_count < ecu->history_capacity)
	{
		ecu->history_count++;
	}
}

void update_hardware_actuators(ECUState *ecu)
{
	if (ecu == NULL) return;
	if(ecu->current_temp > 60.0f)
	{
		//High temp: Turn ON Fan and Alarm, OFF Heater
		ecu->hw_reg.fan = 1;
		ecu->hw_reg.alarm = 1;
		ecu->hw_reg.heater = 0;

        if(ecu->on_overheat != NULL)
        {
            ecu->on_overheat(ecu->current_temp);
        }
	} else if ( ecu->current_temp < 10.0f){
		//Cold temp: Turn on heater, off fan and alarm
		ecu->hw_reg.heater = 1;
		ecu->hw_reg.fan = 0;
		ecu->hw_reg.alarm = 0;
	} else {
		//Normal temp
		ecu->hw_reg.fan = 0;
		ecu->hw_reg.heater = 0;
		ecu->hw_reg.alarm = 0;
	}

	printf("-> Actuators Bit-Field updated: [FAN:%d | HEATER:%d | ALARM:%d]\n",
            ecu->hw_reg.fan, ecu->hw_reg.heater, ecu->hw_reg.alarm);
}

void process_ecu_state_machine(ECUState *ecu)
{
	if(ecu == NULL) return;

	switch(ecu->mode)
	{
		case STATE_BOOT:
			printf("[FSM] ECU in BOOT state. Initializing system check...\n");
			ecu->hw_reg.fan = 0;
            ecu->hw_reg.heater = 0;
            ecu->hw_reg.alarm = 0;
			ecu->mode = STATE_STANDBY;
			printf("[FSM] Transition -> STANDBY Mode\n");
			break;

		case STATE_STANDBY:
			ecu->hw_reg.fan = 0;
            ecu->hw_reg.heater = 0;
            ecu->hw_reg.alarm = 0;
			break;
		
		case STATE_ACTIVE:
			update_hardware_actuators(ecu);//normal operation

			if(ecu->current_temp > 80.0f) //critical overheat
			{
				printf("\n[FSM CRITICAL] Temperature exceeded 80 oC! Triggering STATE_ERROR.\n");
                ecu->mode = STATE_ERROR;
			}
			break;
		
		case STATE_ERROR:
			//Heater OFF, Fand and Alarm ON
			ecu->hw_reg.fan = 1;
            ecu->hw_reg.alarm = 1;
            ecu->hw_reg.heater = 0;
			printf("[FSM EMERGENCY] System locked in ERROR State! Manual reset required.\n");
            break;
		
		default:
			ecu->mode = STATE_ERROR;
			break;
	}
}
void expand_history_buffer(ECUState *ecu, int new_capacity)
{
	if(ecu == NULL || new_capacity <= ecu->history_capacity)
	{
		printf("[WARNING] New capacity must be greater then current capacity (%d)\n", ecu->history_capacity);
		return;
	}

	float *temp_ptr = (float*) realloc(ecu->temp_history, new_capacity*sizeof(float));

	if(temp_ptr == NULL)
	{
		printf("[ERROR] Reallocation failed!");
		return;
	}

	ecu->temp_history = temp_ptr;

	for (int i = ecu->history_capacity; i < new_capacity; i++) {
        ecu->temp_history[i] = 0.0f;
    }

	ecu->history_capacity = new_capacity;
    printf("[SYSTEM] Buffer successfully expanded to %d slots.\n", new_capacity);
}

int save_ecu_to_flash(const ECUState *ecu, const char *filename)
{
	if(ecu == NULL || filename == NULL) return 1;

	FILE *file = fopen(filename, "wb");
	if(file == NULL)
	{
		printf("[ERROR] Could not open file %s for writing\n", filename);
		return 0;
	}
fwrite(&ecu->id, sizeof(int), 1, file);
fwrite(&ecu->firmware_version, sizeof(float), 1, file);
    fwrite(&ecu->mode, sizeof(ECUStateMode), 1, file);
    fwrite(&ecu->current_temp, sizeof(float), 1, file);
    fwrite(&ecu->history_capacity, sizeof(int), 1, file);
    fwrite(&ecu->history_count, sizeof(int), 1, file);
    fwrite(&ecu->hw_reg, sizeof(unsigned char), 1, file);

	fwrite(ecu->temp_history, sizeof(float), ecu->history_capacity, file);
	fclose(file);
	printf("[FLASH] ECU state successfully serialized and saved to '%s'.\n", filename);
    return 1;
}

int load_ecu_from_flash(ECUState *ecu, const char *filename)
{
	if(ecu == NULL || filename == NULL) return 0;

	FILE *file = fopen(filename, "rb");
	if (file == NULL) {
        printf("[ERROR] Could not open file %s for reading!\n", filename);
        return 0;
    }

	// 1. Read scalar fields
    fread(&ecu->id, sizeof(int), 1, file);
    fread(&ecu->firmware_version, sizeof(float), 1, file);
    fread(&ecu->mode, sizeof(ECUStateMode), 1, file);
    fread(&ecu->current_temp, sizeof(float), 1, file);
    
    int loaded_capacity = 0;
    fread(&loaded_capacity, sizeof(int), 1, file);
    fread(&ecu->history_count, sizeof(int), 1, file);
    fread(&ecu->hw_reg, sizeof(unsigned char), 1, file);

    // 2. Reallocate memory if loaded capacity differs
    if (loaded_capacity != ecu->history_capacity) {
        float *temp_ptr = (float*) realloc(ecu->temp_history, loaded_capacity * sizeof(float));
        if (temp_ptr != NULL) {
            ecu->temp_history = temp_ptr;
            ecu->history_capacity = loaded_capacity;
        }
    }

    // 3. Read dynamic array elements back into memory
    fread(ecu->temp_history, sizeof(float), ecu->history_capacity, file);

    fclose(file);
    printf("[FLASH] ECU state restored successfully from '%s'.\n", filename);
    return 1;

}