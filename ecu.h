#ifndef ECU_H //nao deixar compilar 2x
#define ECU_H

#define FLAG_FAN (1 << 0) //0x01
#define FLAG_HEATER (1 << 1) // 0x02
#define FLAG_ALARM (1 << 2) //0x04


typedef enum {
	STATE_BOOT = 0,
	STATE_STANDBY = 1,
	STATE_ACTIVE = 2,
	STATE_ERROR = 3
} ECUStateMode;

typedef void(*AlarmCallback)(float current_temp);

//occupies less memory

typedef struct 
{
	unsigned int fan : 1; // 0 = off, 1 = on
	unsigned int heater: 1; // 0 = off, 1 = on
	unsigned int alarm : 1; // 0 = off, 1 =on
	unsigned int reserved: 5; //5 bits reserved for future hardware
} HardwareRegister;


//Dynamic Memory, reorder to optimized alingment / padding
typedef struct
{
	//8-byte
	float *temp_history;
	AlarmCallback on_overheat;

	//4 byte
	int id;
	float firmware_version;
	float current_temp;
	int history_capacity;
	int history_count;

	//enum & bitfield
	ECUStateMode mode;
	HardwareRegister hw_reg;	
} ECUState;

int init_ecu(ECUState *ecu, int id, float version, int history_size, AlarmCallback);
void cleanup_ecu(ECUState *ecu);
void print_header(const ECUState *ecu);
void print_menu(void);
void display_telemetry(const ECUState *ecu);
void log_temperature(ECUState *ecu);
void update_hardware_actuators(ECUState *ecu);
void process_ecu_state_machine(ECUState *ecu);
void expand_history_buffer(ECUState *ecu, int new_capacity);
int save_ecu_to_flash(const ECUState *ecu, const char *filename);
int load_ecu_from_flash(ECUState *ecu, const char *filename);

#endif