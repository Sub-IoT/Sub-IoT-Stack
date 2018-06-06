#include "hwleds.h"
#include "hwsystem.h"
#include "scheduler.h"
#include "timer.h"
#include "button.h"

#include "assert.h"

#include "log.h"
#include "led.h"

#include "d7ap_stack.h"
#include "lorawan_stack.h"

#define DEBUG_PRINTF(...) 							log_print_string(__VA_ARGS__)

#define LORAWAN_APP_PORT                10
#define LORAWAN_MTU                     51
#define LORAWAN_DEVICE_EUI              {0xBE, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x24, 0x24}
#define LORAWAN_APPLICATION_EUI         {0xBE, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x9F}
#define LORAWAN_APPLICATION_KEY         {0x4D, 0x0D, 0x9C, 0x6D, 0x8F, 0x2C, 0xF7, 0xF4, 0xB0, 0x20, 0x78, 0xD7, 0x6F, 0x42, 0xB9, 0x10}

#define SENSOR_FILE_ID                  0x40
#define SENSOR_FILE_SIZE                2
#define SENSOR_INTERVAL_LORAWAN_SEC     TIMER_TICKS_PER_SEC * 60
#define SENSOR_INTERVAL_D7AP_SEC        TIMER_TICKS_PER_SEC * 20

typedef struct {
  char name[4];
  uint16_t mtu;
  void (*init)(void);
  void (*stop)(void);
  uint8_t (*send)(uint8_t* buffer, uint16_t length);
} network_driver_t;

network_driver_t lora;
network_driver_t d7;
network_driver_t* current_network_driver;

static alp_init_args_t alp_init_args;

// Define the D7 interface configuration used for sending the ALP command on
static d7ap_master_session_config_t session_config = {
  .qos = {
    .qos_resp_mode = SESSION_RESP_MODE_ANY,
    .qos_retry_mode = SESSION_RETRY_MODE_NO,
    .qos_stop_on_error       = false,
    .qos_record              = false
  },
  .dormant_timeout = 0,
  .addressee = {
    .ctrl = {
      .nls_method = AES_NONE,
      .id_type = ID_TYPE_NOID,
    },
    .access_class = 0x01,
    .id = 0
  }
};

uint8_t network_driver_send(uint8_t *data, uint16_t length) {
  DEBUG_PRINTF("network_driver_send(): sending %d bytes of data", length);
  return current_network_driver->send(data, length);
}

void on_alp_command_completed_cb(uint8_t tag_id, bool success) {
  if (success)
    log_print_string("Command completed successfully");
  else
    log_print_string("Command failed, no ack received");
}

void on_alp_command_result_cb(d7ap_session_result_t result, uint8_t* payload, uint8_t payload_length) {
  log_print_string("recv response @ %i dB link budget from:", result.link_budget);
  log_print_data(result.addressee.id, 8);
}

static uint8_t transmit_d7ap(uint8_t* alp, uint16_t len) {
  alp_layer_execute_command(alp, len, &session_config);
  return 0;
}

static void init_d7ap() {
  alp_init_args.alp_command_completed_cb = &on_alp_command_completed_cb;
  alp_init_args.alp_command_result_cb = &on_alp_command_result_cb;

  dae_access_profile_t access_classes[1] = {
    {
      .channel_header = {
        .ch_coding = PHY_CODING_FEC_PN9,
        .ch_class = PHY_CLASS_LO_RATE,
        .ch_freq_band = PHY_BAND_868
      },
      .subprofiles[0] = {
        .subband_bitmap = 0x00, // void scan automation channel list
        .scan_automation_period = 0,
      },
      .subbands[0] = (subband_t){
        .channel_index_start = 200,
        .channel_index_end = 200,
        .eirp = 10,
        .cca = 86,
        .duty = 0,
      }
    }
  };

  fs_init_args_t fs_init_args = (fs_init_args_t){
      .access_profiles_count = 1,
      .access_profiles = access_classes,
      .access_class = 0x01
  };

  d7ap_stack_init(&fs_init_args, &alp_init_args, false, NULL);
  DEBUG_PRINTF("DASH7 init");
}

static void lora_init(void) {
  uint8_t devEui[8]  = LORAWAN_DEVICE_EUI;
  uint8_t appEui[8]  = LORAWAN_APPLICATION_EUI;
  uint8_t appKey[16] = LORAWAN_APPLICATION_KEY;

  lorawan_stack_init(devEui, appEui, appKey, NULL, NULL);
  DEBUG_PRINTF("LoRa init");
}

static uint8_t lora_send(uint8_t* buffer, uint16_t length) {
  lorawan_stack_error_t err;
  err = lorawan_stack_send(buffer, length, LORAWAN_APP_PORT, false);

  DEBUG_PRINTF("network_driver_send(): lorawan_stack_send returned %d, ", err);
  if (err == LORAWAN_STACK_ERROR_OK) {
    led_toggle(0);
    DEBUG_PRINTF("Ok!");
    return 1;
  } else {
    DEBUG_PRINTF("Packet not sent!");
    return 0;
  }
}

void network_drivers_init() {
  lora.mtu = LORAWAN_MTU;
  lora.init = &lora_init;
  lora.send = &lora_send;
  lora.stop = &lorawan_stack_deinit;
  memcpy(lora.name, "LoRa", 4);

  d7.init = &init_d7ap;
  d7.stop = &d7ap_stack_stop;
  d7.send = &transmit_d7ap;
  memcpy(d7.name, "DSH7", 4);
}

static void on_button_pressed(button_id_t button_id) {
  network_driver_t* nd = &d7;
  if(current_network_driver == &d7)
    nd = &lora;

  DEBUG_PRINTF("Switching from %s to %s", current_network_driver->name, nd->name);
  // stop previous network driver
  current_network_driver->stop();

  // init new network driver
  current_network_driver = nd;
  current_network_driver->init();
  DEBUG_PRINTF("Switching done");
}

void execute_sensor_measurement()
{
  // first get the sensor reading ...
  int16_t temperature = 0; // in decicelsius. When there is no sensor, we just transmit 0 degrees

#if defined USE_HTS221
  HTS221_Get_Temperature(hts221_handle, &temperature);
#endif

  // Generate ALP command. We do this manually for now (until we have an API for this).
  // We will be sending a return file data action, without a preceding file read request.
  // This is an unsolicited message, where we push the sensor data to the gateway(s).
  // Please refer to the spec for the format

  uint8_t alp_command[4 + SENSOR_FILE_SIZE] = {
    // ALP Control byte
    ALP_OP_RETURN_FILE_DATA,
    // File Data Request operand:
    SENSOR_FILE_ID, // the file ID
    0, // offset in file
    SENSOR_FILE_SIZE // data length
    // the sensor data, see below
  };

  temperature = __builtin_bswap16(temperature); // convert to big endian before transmission
  memcpy(alp_command + 4, (uint8_t*)&temperature, SENSOR_FILE_SIZE);

  current_network_driver->send(alp_command, sizeof(alp_command));

  timer_tick_t delay = SENSOR_INTERVAL_D7AP_SEC;
  if(current_network_driver == &lora)
    delay = SENSOR_INTERVAL_LORAWAN_SEC;

  timer_post_task_delay(&execute_sensor_measurement, delay);
}

void bootstrap() {
  DEBUG_PRINTF("Device booted\n");

  network_drivers_init();
  current_network_driver = &d7;
  current_network_driver->init();

  ubutton_register_callback(0, &on_button_pressed);

  sched_register_task(&execute_sensor_measurement);
  sched_post_task(&execute_sensor_measurement);
}
