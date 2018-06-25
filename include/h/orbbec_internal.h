#ifndef ORBBEC_INTERNAL_H
#define ORBBEC_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif
#include "orbbec_types.h"
#include "orbbec_context.h"
#include "orbbec_sensor.h"
#include "orbbec_frame.h"
#include "orbbec_option.h"

typedef enum orbbec_recording_mode
{
    ORBBEC_RECORDING_MODE_BLANK_FRAMES, /* frame metadata will be recorded, but pixel data will be replaced with zeros to save space */
    ORBBEC_RECORDING_MODE_COMPRESSED,   /* frames will be encoded using a proprietary lossy encoding, aiming at x5 compression at some CPU expense */
    ORBBEC_RECORDING_MODE_BEST_QUALITY, /* frames will not be compressed, but rather stored as-is. This gives best quality and low CPU overhead, but you might run out of memory */
    ORBBEC_RECORDING_MODE_COUNT
} orbbec_recording_mode;

typedef struct orbbec_video_stream
{
    orbbec_stream type;
    int index;
    int uid;
    int width;
    int height;
    int fps;
    int bpp;
    orbbec_format fmt;
    orbbec_intrinsics intrinsics;
} orbbec_video_stream;

typedef struct orbbec_software_video_frame
{
    void* pixels;
    void(*deleter)(void*);
    int stride;
    int bpp;
    orbbec_time_t timestamp;
    orbbec_timestamp_domain domain;
    int frame_number;
    const orbbec_stream_profile* profile;
} orbbec_software_video_frame;

orbbec_context* orbbec_create_recording_context(int api_version, const char* filename, const char* section, orbbec_recording_mode mode, orbbec_error** error);
orbbec_context* orbbec_create_mock_context(int api_version, const char* filename, const char* section, orbbec_error** error);
orbbec_device* orbbec_create_software_device(orbbec_error** error);
orbbec_sensor* orbbec_software_device_add_sensor(orbbec_device* dev, const char* sensor_name, orbbec_error** error);
void orbbec_software_sensor_on_video_frame(orbbec_sensor* sensor, orbbec_software_video_frame frame, orbbec_error** error);
void orbbec_software_device_create_matcher(orbbec_device* dev, orbbec_matchers matcher, orbbec_error** error);
orbbec_stream_profile* orbbec_software_sensor_add_video_stream(orbbec_sensor* sensor, orbbec_video_stream video_stream, orbbec_error** error);
void orbbec_software_sensor_add_read_only_option(orbbec_sensor* sensor, orbbec_option option, float val, orbbec_error** error);
void orbbec_software_sensor_update_read_only_option(orbbec_sensor* sensor, orbbec_option option, float val, orbbec_error** error);

#ifdef __cplusplus
}
#endif
#endif // ORBBEC_INTERNAL_H
