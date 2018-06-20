#ifndef ORBBEC_CONTEXT_H
#define ORBBEC_CONTEXT_H
#ifdef __cplusplus
extern "C" {
#endif
#include "types.h"

orbbec_context* orbbec_create_context(int api_version, orbbec_error** error);
void orbbec_delete_context(orbbec_context* context);
void orbbec_set_devices_changed_callback_cpp(orbbec_context* context, orbbec_devices_changed_callback* callback, orbbec_error** error);
void orbbec_set_devices_changed_callback(const orbbec_context* context, orbbec_devices_changed_callback_ptr callback, void* user, orbbec_error** error);
orbbec_device* orbbec_context_add_device(orbbec_context* ctx, const char* file, orbbec_error** error);
void orbbec_context_remove_device(orbbec_context* ctx, const char* file, orbbec_error** error);
orbbec_device_list* orbbec_query_devices(const orbbec_context* context, orbbec_error** error);
orbbec_device_hub* orbbec_create_device_hub(const orbbec_context* context, orbbec_error** error);
void orbbec_delete_device_hub(const orbbec_device_hub* hub);
orbbec_device* orbbec_device_hub_wait_for_device(orbbec_context* ctx, const orbbec_device_hub* hub, orbbec_error** error);
int orbbec_device_hub_is_device_connected(const orbbec_device_hub* hub, const orbbec_device* device, orbbec_error** error);

#ifdef __cplusplus
}
#endif
#endif // ORBBEC_CONTEXT_H
