#ifndef ORBBEC_RECORD_PLAYBACK_H
#define ORBBEC_RECORD_PLAYBACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

typedef enum orbbec_playback_status
{
    ORBBEC_PLAYBACK_STATUS_UNKNOWN, /**< Unknown state */
    ORBBEC_PLAYBACK_STATUS_PLAYING, /**< One or more sensors were started, playback is reading and raising data */
    ORBBEC_PLAYBACK_STATUS_PAUSED,  /**< One or more sensors were started, but playback paused reading and paused raising data*/
    ORBBEC_PLAYBACK_STATUS_STOPPED, /**< All sensors were stopped, or playback has ended (all data was read). This is the initial playback status*/
    ORBBEC_PLAYBACK_STATUS_COUNT
} orbbec_playback_status;
const char* orbbec_playback_status_to_string(orbbec_playback_status status);

typedef void (*orbbec_playback_status_changed_callback_ptr)(orbbec_playback_status);
orbbec_device* orbbec_create_record_device(const orbbec_device* device, const char* file, orbbec_error** error);
void orbbec_record_device_pause(const orbbec_device* device, orbbec_error** error);
void orbbec_record_device_resume(const orbbec_device* device, orbbec_error** error);
const char* orbbec_record_device_filename(const orbbec_device* device, orbbec_error** error);
orbbec_device* orbbec_create_playback_device(const char* file, orbbec_error** error);
const char* orbbec_playback_device_get_file_path(const orbbec_device* device, orbbec_error** error);
unsigned long long int orbbec_playback_get_duration(const orbbec_device* device, orbbec_error** error);
void orbbec_playback_seek(const orbbec_device* device, long long int time, orbbec_error** error);
unsigned long long int orbbec_playback_get_position(const orbbec_device* device, orbbec_error** error);
void orbbec_playback_device_resume(const orbbec_device* device, orbbec_error** error);
void orbbec_playback_device_pause(const orbbec_device* device, orbbec_error** error);
void orbbec_playback_device_set_real_time(const orbbec_device* device, int real_time, orbbec_error** error);
int orbbec_playback_device_is_real_time(const orbbec_device* device, orbbec_error** error);
void orbbec_playback_device_set_status_changed_callback(const orbbec_device* device, orbbec_playback_status_changed_callback* callback, orbbec_error** error);
orbbec_playback_status orbbec_playback_device_get_current_status(const orbbec_device* device, orbbec_error** error);
void orbbec_playback_device_set_playback_speed(const orbbec_device* device, float speed, orbbec_error** error);
void orbbec_playback_device_stop(const orbbec_device* device, orbbec_error** error);

#ifdef __cplusplus
}
#endif
#endif // ORBBEC_RECORD_PLAYBACK_H
