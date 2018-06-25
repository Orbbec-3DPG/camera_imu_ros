#ifndef ORBBEC_DEVICE_H
#define ORBBEC_DEVICE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "orbbec_types.h"
#include "orbbec_sensor.h"

int orbbec_get_device_count(const orbbec_device_list* info_list, orbbec_error** error);
void orbbec_delete_device_list(orbbec_device_list* info_list);
int orbbec_device_list_contains(const orbbec_device_list* info_list, const orbbec_device* device, orbbec_error** error);
orbbec_device* orbbec_create_device(const orbbec_device_list* info_list, int index, orbbec_error** error);
void orbbec_delete_device(orbbec_device* device);
const char* orbbec_get_device_info(const orbbec_device* device, orbbec_camera_info info, orbbec_error** error);
int orbbec_supports_device_info(const orbbec_device* device, orbbec_camera_info info, orbbec_error** error);
void orbbec_hardware_reset(const orbbec_device * device, orbbec_error ** error);
const orbbec_raw_data_buffer* orbbec_send_and_receive_raw_data(orbbec_device* device, void* raw_data_to_send, unsigned size_of_raw_data_to_send, orbbec_error** error);
int orbbec_is_device_extendable_to(const orbbec_device* device, orbbec_extension extension, orbbec_error ** error);
orbbec_sensor_list* orbbec_query_sensors(const orbbec_device* device, orbbec_error** error);
void orbbec_loopback_enable(const orbbec_device* device, const char* from_file, orbbec_error** error);
void orbbec_loopback_disable(const orbbec_device* device, orbbec_error** error);
int orbbec_loopback_is_enabled(const orbbec_device* device, orbbec_error** error);
void orbbec_connect_tm2_controller(const orbbec_device* device, const unsigned char* mac_addr, orbbec_error** error);
void orbbec_disconnect_tm2_controller(const orbbec_device* device, int id, orbbec_error** error);

#ifdef __cplusplus
}
#endif
#endif // ORBBEC_DEVICE_H
