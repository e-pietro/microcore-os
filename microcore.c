#include <stdio.h>
#include "ecu.h"

void default_overheat_handler(float temp)
{
	printf("\n>>> [CRITICAL CALLBACK] OVERHEAT EVENT DETECTED (%.1f oC) <<<\n", temp);
    printf(">>> Emergency protocols active! <<<\n");
}

int main(void) {
	ECUState my_ecu;

	if(!init_ecu(&my_ecu, 101, 1.0f, 3, default_overheat_handler)){
		printf("[FATAL] ECU Initialization failed. Exiting...\n");
		return 1;
	}

    int option = -1;
    int is_running = 1; // Control flag for the firmware loop

    // Embedded Super-Loop
    while (is_running) {

		process_ecu_state_machine(&my_ecu);

		print_header(&my_ecu);
		print_menu();

        printf("Select an option: ");
        scanf("%d", &option);

        switch (option) {
            case 1:
                display_telemetry(&my_ecu);
                break;

            case 2:
                log_temperature(&my_ecu);
                break;

            case 3:
                printf("\nSelect State Mode (0=Boot, 1=Standby, 2=Active, 3=Error): ");
				int mode_input;
                scanf(" %d", &mode_input);
				my_ecu.mode = (ECUStateMode)mode_input;
                printf("-> ECU Mode Update\n");
                break;

			case 4:
				printf("\nEnter new capacity for history buffer: ");
				int new_cap;
				scanf("%d", &new_cap);
				expand_history_buffer(&my_ecu, new_cap);
				break;
			
			case 5:
				save_ecu_to_flash(&my_ecu, "ecu_flash.bin");
				break;
			
			case 6:
				load_ecu_from_flash(&my_ecu, "ecu_flash.bin");
				break;

            case 0:
                printf("\n[SHUTDOWN] Stopping MicroCore-OS...\n");
				is_running = 0;
                break;

            default:
                printf("\n[ERROR] Invalid menu option!\n");
                break;
        }
    }

	cleanup_ecu(&my_ecu);

    return 0; // Return success code to Linux kernel
}

