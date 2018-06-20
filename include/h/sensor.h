#ifndef ORBBEC_SENSOR_H
#define ORBBEC_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

typedef enum orbbec_camera_info {
    ORBBEC_CAMERA_INFO_NAME                           , /**< Friendly name */
    ORBBEC_CAMERA_INFO_SERIAL_NUMBER                  , /**< Device serial number */
    ORBBEC_CAMERA_INFO_FIRMWARE_VERSION               , /**< Primary firmware version */
    ORBBEC_CAMERA_INFO_RECOMMENDED_FIRMWARE_VERSION   , /**< Recommended firmware version */
    ORBBEC_CAMERA_INFO_PHYSICAL_PORT                  , /**< Unique identifier of the port the device is connected to (platform specific) */
    ORBBEC_CAMERA_INFO_DEBUG_OP_CODE                  , /**< If device supports firmware logging, this is the command to send to get logs from firmware */
    ORBBEC_CAMERA_INFO_ADVANCED_MODE                  , /**< True iff the device is in advanced mode */
    ORBBEC_CAMERA_INFO_PRODUCT_ID                     , /**< Product ID as reported in the USB descriptor */
    ORBBEC_CAMERA_INFO_CAMERA_LOCKED                  , /**< True iff EEPROM is locked */
    ORBBEC_CAMERA_INFO_USB_TYPE_DESCRIPTOR            , /**< Designated USB specification: USB2/USB3 */
    ORBBEC_CAMERA_INFO_COUNT                            /**< Number of enumeration values. Not a valid input: intended to be used in for-loops. */
} orbbec_camera_info;
const char* orbbec_camera_info_to_string(orbbec_camera_info info);

typedef enum orbbec_stream
{
    ORBBEC_STREAM_ANY,
    ORBBEC_STREAM_DEPTH                            , /**< Native stream of depth data produced by RealSense device */
    ORBBEC_STREAM_COLOR                            , /**< Native stream of color data captured by RealSense device */
    ORBBEC_STREAM_INFRARED                         , /**< Native stream of infrared data captured by RealSense device */
    ORBBEC_STREAM_FISHEYE                          , /**< Native stream of fish-eye (wide) data captured from the dedicate motion camera */
    ORBBEC_STREAM_GYRO                             , /**< Native stream of gyroscope motion data produced by RealSense device */
    ORBBEC_STREAM_ACCEL                            , /**< Native stream of accelerometer motion data produced by RealSense device */
    ORBBEC_STREAM_GPIO                             , /**< Signals from external device connected through GPIO */
    ORBBEC_STREAM_POSE                             , /**< 6 Degrees of Freedom pose data, calculated by RealSense device */
    ORBBEC_STREAM_CONFIDENCE,
    ORBBEC_STREAM_COUNT
} orbbec_stream;
const char* orbbec_stream_to_string(orbbec_stream stream);

typedef enum orbbec_format
{
    ORBBEC_FORMAT_ANY             , /**< When passed to enable stream, librealsense will try to provide best suited format */
    ORBBEC_FORMAT_Z16             , /**< 16-bit linear depth values. The depth is meters is equal to depth scale * pixel value. */
    ORBBEC_FORMAT_DISPARITY16     , /**< 16-bit linear disparity values. The depth in meters is equal to depth scale / pixel value. */
    ORBBEC_FORMAT_XYZ32F          , /**< 32-bit floating point 3D coordinates. */
    ORBBEC_FORMAT_YUYV            , /**< Standard YUV pixel format as described in https://en.wikipedia.org/wiki/YUV */
    ORBBEC_FORMAT_RGB8            , /**< 8-bit red, green and blue channels */
    ORBBEC_FORMAT_BGR8            , /**< 8-bit blue, green, and red channels -- suitable for OpenCV */
    ORBBEC_FORMAT_RGBA8           , /**< 8-bit red, green and blue channels + constant alpha channel equal to FF */
    ORBBEC_FORMAT_BGRA8           , /**< 8-bit blue, green, and red channels + constant alpha channel equal to FF */
    ORBBEC_FORMAT_Y8              , /**< 8-bit per-pixel grayscale image */
    ORBBEC_FORMAT_Y16             , /**< 16-bit per-pixel grayscale image */
    ORBBEC_FORMAT_RAW10           , /**< Four 10-bit luminance values encoded into a 5-byte macropixel */
    ORBBEC_FORMAT_RAW16           , /**< 16-bit raw image */
    ORBBEC_FORMAT_RAW8            , /**< 8-bit raw image */
    ORBBEC_FORMAT_UYVY            , /**< Similar to the standard YUYV pixel format, but packed in a different order */
    ORBBEC_FORMAT_MOTION_RAW      , /**< Raw data from the motion sensor */
    ORBBEC_FORMAT_MOTION_XYZ32F   , /**< Motion data packed as 3 32-bit float values, for X, Y, and Z axis */
    ORBBEC_FORMAT_GPIO_RAW        , /**< Raw data from the external sensors hooked to one of the GPIO's */
    ORBBEC_FORMAT_6DOF            , /**< Pose data packed as floats array, containing translation vector, rotation quaternion and prediction velocities and accelerations vectors */
    ORBBEC_FORMAT_DISPARITY32     , /**< 32-bit float-point disparity values. Depth->Disparity conversion : Disparity = Baseline*FocalLength/Depth */
    ORBBEC_FORMAT_COUNT             /**< Number of enumeration values. Not a valid input: intended to be used in for-loops. */
} orbbec_format;

const char* orbbec_format_to_string(orbbec_format format);

typedef struct orbbec_extrinsics
{
    float rotation[9];    /**< Column-major 3x3 rotation matrix */
    float translation[3]; /**< Three-element translation vector, in meters */
} orbbec_extrinsics;

void orbbec_delete_sensor_list(orbbec_sensor_list* info_list);
int orbbec_get_sensors_count(const orbbec_sensor_list* info_list, orbbec_error** error);
void orbbec_delete_sensor(orbbec_sensor* sensor);
orbbec_sensor* orbbec_create_sensor(const orbbec_sensor_list* list, int index, orbbec_error** error);
orbbec_device* orbbec_create_device_from_sensor(const orbbec_sensor* sensor, orbbec_error** error);
const char* orbbec_get_sensor_info(const orbbec_sensor* sensor, orbbec_camera_info info, orbbec_error** error);
int orbbec_supports_sensor_info(const orbbec_sensor* sensor, orbbec_camera_info info, orbbec_error** error);
int orbbec_is_sensor_extendable_to(const orbbec_sensor* sensor, orbbec_extension extension, orbbec_error** error);
float orbbec_get_depth_scale(orbbec_sensor* sensor, orbbec_error** error);
float orbbec_depth_stereo_frame_get_baseline(const orbbec_frame* frame_ref, orbbec_error** error);
void orbbec_set_region_of_interest(const orbbec_sensor* sensor, int min_x, int min_y, int max_x, int max_y, orbbec_error** error);
void orbbec_get_region_of_interest(const orbbec_sensor* sensor, int* min_x, int* min_y, int* max_x, int* max_y, orbbec_error** error);
void orbbec_open(orbbec_sensor* device, const orbbec_stream_profile* profile, orbbec_error** error);
void orbbec_open_multiple(orbbec_sensor* device, const orbbec_stream_profile** profiles, int count, orbbec_error** error);
void orbbec_close(const orbbec_sensor* sensor, orbbec_error** error);
void orbbec_start(const orbbec_sensor* sensor, orbbec_frame_callback_ptr on_frame, void* user, orbbec_error** error);
void orbbec_start_cpp(const orbbec_sensor* sensor, orbbec_frame_callback* callback, orbbec_error** error);
void orbbec_start_queue(const orbbec_sensor* sensor, orbbec_frame_queue* queue, orbbec_error** error);
void orbbec_stop(const orbbec_sensor* sensor, orbbec_error** error);
void orbbec_set_notifications_callback(const orbbec_sensor* sensor, orbbec_notification_callback_ptr on_notification, void* user, orbbec_error** error);
void orbbec_set_notifications_callback_cpp(const orbbec_sensor* sensor, orbbec_notifications_callback* callback, orbbec_error** error);
const char* orbbec_get_notification_description(orbbec_notification* notification, orbbec_error** error);
orbbec_time_t orbbec_get_notification_timestamp(orbbec_notification* notification, orbbec_error** error);
orbbec_log_severity orbbec_get_notification_severity(orbbec_notification* notification, orbbec_error** error);
orbbec_notification_category orbbec_get_notification_category(orbbec_notification* notification, orbbec_error** error);
const char* orbbec_get_notification_serialized_data(orbbec_notification* notification, orbbec_error** error);
orbbec_stream_profile_list* orbbec_get_stream_profiles(orbbec_sensor* device, orbbec_error** error);
const orbbec_stream_profile* orbbec_get_stream_profile(const orbbec_stream_profile_list* list, int index, orbbec_error** error);
void orbbec_get_stream_profile_data(const orbbec_stream_profile* mode, orbbec_stream* stream, orbbec_format* format, int* index, int* unique_id, int* framerate, orbbec_error** error);
void orbbec_set_stream_profile_data(orbbec_stream_profile* mode, orbbec_stream stream, int index, orbbec_format format, orbbec_error** error);
orbbec_stream_profile* orbbec_clone_stream_profile(const orbbec_stream_profile* mode, orbbec_stream stream, int index, orbbec_format format, orbbec_error** error);
void orbbec_delete_stream_profile(orbbec_stream_profile* mode);
int orbbec_stream_profile_is(const orbbec_stream_profile* mode, orbbec_extension type, orbbec_error** error);
void orbbec_get_video_stream_resolution(const orbbec_stream_profile* mode, int* width, int* height, orbbec_error** error);
void orbbec_get_motion_intrinsics(const orbbec_stream_profile* mode, orbbec_motion_device_intrinsic * intrinsics, orbbec_error ** error);
int orbbec_is_stream_profile_default(const orbbec_stream_profile* mode, orbbec_error** error);
int orbbec_get_stream_profiles_count(const orbbec_stream_profile_list* list, orbbec_error** error);
void orbbec_delete_stream_profiles_list(orbbec_stream_profile_list* list);
void orbbec_get_extrinsics(const orbbec_stream_profile* from,
                        const orbbec_stream_profile* to,
                        orbbec_extrinsics* extrin, orbbec_error** error);
void orbbec_register_extrinsics(const orbbec_stream_profile* from,
    const orbbec_stream_profile* to,
    orbbec_extrinsics extrin, orbbec_error** error);
void orbbec_get_video_stream_intrinsics(const orbbec_stream_profile* mode, orbbec_intrinsics* intrinsics, orbbec_error** error);


#ifdef __cplusplus
}
#endif
#endif // ORBBEC_SENSOR_H
