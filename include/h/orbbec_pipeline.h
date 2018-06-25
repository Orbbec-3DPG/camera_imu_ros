#ifndef ORBBEC_PIPELINE_H
#define ORBBEC_PIPELINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "orbbec_types.h"
#include "orbbec_sensor.h"

orbbec_pipeline* orbbec_create_pipeline(orbbec_context* ctx, orbbec_error ** error);
void orbbec_pipeline_stop(orbbec_pipeline* pipe, orbbec_error ** error);
orbbec_frame* orbbec_pipeline_wait_for_frames(orbbec_pipeline* pipe, unsigned int timeout_ms, orbbec_error ** error);
int orbbec_pipeline_poll_for_frames(orbbec_pipeline* pipe, orbbec_frame** output_frame, orbbec_error ** error);
void orbbec_delete_pipeline(orbbec_pipeline* pipe);
orbbec_pipeline_profile* orbbec_pipeline_start(orbbec_pipeline* pipe, orbbec_error ** error);
orbbec_pipeline_profile* orbbec_pipeline_start_with_config(orbbec_pipeline* pipe, orbbec_config* config, orbbec_error ** error);
orbbec_pipeline_profile* orbbec_pipeline_get_active_profile(orbbec_pipeline* pipe, orbbec_error ** error);
orbbec_device* orbbec_pipeline_profile_get_device(orbbec_pipeline_profile* profile, orbbec_error ** error);
orbbec_stream_profile_list* orbbec_pipeline_profile_get_streams(orbbec_pipeline_profile* profile, orbbec_error** error);
void orbbec_delete_pipeline_profile(orbbec_pipeline_profile* profile);
orbbec_config* orbbec_create_config(orbbec_error** error);
void orbbec_delete_config(orbbec_config* config);
void orbbec_config_enable_stream(orbbec_config* config, orbbec_stream stream,
    int index, int width, int height, orbbec_format format, int framerate, orbbec_error** error);
void orbbec_config_enable_all_stream(orbbec_config* config, orbbec_error ** error);
void orbbec_config_enable_device(orbbec_config* config, const char* serial, orbbec_error ** error);
void orbbec_config_enable_device_from_file(orbbec_config* config, const char* file, orbbec_error ** error);
void orbbec_config_enable_device_from_file_repeat_option(orbbec_config* config, const char* file, int repeat_playback, orbbec_error ** error);
void orbbec_config_enable_record_to_file(orbbec_config* config, const char* file, orbbec_error ** error);
void orbbec_config_disable_stream(orbbec_config* config, orbbec_stream stream, orbbec_error ** error);
void orbbec_config_disable_indexed_stream(orbbec_config* config, orbbec_stream stream, int index, orbbec_error ** error);
void orbbec_config_disable_all_streams(orbbec_config* config, orbbec_error ** error);
orbbec_pipeline_profile* orbbec_config_resolve(orbbec_config* config, orbbec_pipeline* pipe, orbbec_error ** error);
int orbbec_config_can_resolve(orbbec_config* config, orbbec_pipeline* pipe, orbbec_error ** error);

#ifdef __cplusplus
}
#endif
#endif // ORBBEC_PIPELINE_H
