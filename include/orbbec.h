#ifndef ORBBEC_H
#define ORBBEC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "h/types.h"
#include "h/context.h"
#include "h/device.h"
#include "h/frame.h"
#include "h/option.h"
#include "h/processing.h"
#include "h/record_playback.h"
#include "h/sensor.h"

#define ORBBEC_API_MAJOR_VERSION    1
#define ORBBEC_API_MINOR_VERSION    1
#define ORBBEC_API_PATCH_VERSION    0
#define ORBBEC_API_BUILD_VERSION    0

#define STRINGIFY(arg) #arg
#define VAR_ARG_STRING(arg) STRINGIFY(arg)

#define ORBBEC_API_VERSION  (((ORBBEC_API_MAJOR_VERSION) * 10000) + ((ORBBEC_API_MINOR_VERSION) * 100) + (ORBBEC_API_PATCH_VERSION))
#define ORBBEC_API_VERSION_STR (VAR_ARG_STRING(ORBBEC_API_MAJOR_VERSION.ORBBEC_API_MINOR_VERSION.ORBBEC_API_PATCH_VERSION))


int orbbec_get_raw_data_size(const orbbec_raw_data_buffer* buffer, orbbec_error** error);
void orbbec_delete_raw_data(const orbbec_raw_data_buffer* buffer);
const unsigned char* orbbec_get_raw_data(const orbbec_raw_data_buffer* buffer, orbbec_error** error);
int orbbec_get_api_version(orbbec_error** error);
void orbbec_log_to_console(orbbec_log_severity min_severity, orbbec_error ** error);
void orbbec_log_to_file(orbbec_log_severity min_severity, const char * file_path, orbbec_error ** error);
void orbbec_log(orbbec_log_severity severity, const char * message, orbbec_error ** error);
float orbbec_depth_frame_get_distance(const orbbec_frame* frame_ref, int x, int y, orbbec_error** error);
orbbec_time_t orbbec_get_time( orbbec_error** error);

#ifdef __cplusplus
}
#endif
#endif // ORBBEC_H
