#ifndef ORBBEC_TYPES_H
#define ORBBEC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum orbbec_notification_category
{
    ORBBEC_NOTIFICATION_CATEGORY_FRAMES_TIMEOUT,
    ORBBEC_NOTIFICATION_CATEGORY_FRAME_CORRUPTED,
    ORBBEC_NOTIFICATION_CATEGORY_HARDWARE_ERROR,
    ORBBEC_NOTIFICATION_CATEGORY_HARDWARE_EVENT,
    ORBBEC_NOTIFICATION_CATEGORY_UNKNOWN_ERROR,
    ORBBEC_NOTIFICATION_CATEGORY_FIRMWARE_UPDATE_RECOMMANDED,
    ORBBEC_NOTIFICATION_CATEGORY_COUNT
} orbbec_notification_category;

const char* orbbec_notification_category_to_string(orbbec_notification_category category);

typedef enum orbbec_exception_type
{
    ORBBEC_EXCEPTION_TYPE_UNKNOWN,
    ORBBEC_EXCEPTION_TYPE_CAMERA_DISCONNECTED,
    ORBBEC_EXCEPTION_TYPE_BACKEND,
    ORBBEC_EXCEPTION_TYPE_INVALID_VALUE,
    ORBBEC_EXCEPTION_TYPE_WRONG_API_CALL_SEQUENCE,
    ORBBEC_EXCEPTION_TYPE_NOT_IMPLEMENTED,
    ORBBEC_EXCEPTION_TYPE_DEVICE_IN_RECOVERY_MODE,
    ORBBEC_EXCEPTION_TYPE_IO,
    ORBBEC_EXCEPTION_TYPE_COUNT
} orbbec_exception_type;

const char* orbbec_exception_type_to_string(orbbec_exception_type type);

typedef enum orbbec_distortion
{
    ORBBEC_DISTORTION_NONE,
    ORBBEC_DISTORTION_MODIFIED_BROWN_CONRADY,
    ORBBEC_DISTORTION_INVERSE_BROWN_CONRADY,
    ORBBEC_DISTORTION_FTHETA,
    ORBBEC_DISTORTION_BROW_CONRADY,
    ORBBEC_DISTORTION_COUNT
} orbbec_distortion;

const char* orbbec_distortion_to_string(orbbec_distortion distortion);

typedef struct orbbec_intrinsics
{
    int                 width;
    int                 height;
    float               ppx;        // principal point of the image
    float               ppy;
    float               fx;
    float               fy;
    float               coeffs[5];  // k1, k2, p1, p2, k3
    orbbec_distortion   model;
} orbbec_instrinsics;

typedef struct orbbec_motion_device_intrinsic
{
    /* \internal
    * Scale X       cross axis  cross axis  Bias X \n
    * cross axis    Scale Y     cross axis  Bias Y \n
    * cross axis    cross axis  Scale Z     Bias Z */
    float data[3][4];          /**< Interpret data array values */

    float noise_variances[3];  /**< Variance of noise for X, Y, and Z axis */
    float bias_variances[3];   /**< Variance of bias for X, Y, and Z axis */
} orbbec_motion_device_intrinsic;

typedef enum orbbec_log_severity
{
    ORBBEC_LOG_SEVERITY_DEBUG,
    ORBBEC_LOG_SEVERITY_INFO,
    ORBBEC_LOG_SEVERITY_WARN,
    ORBBEC_LOG_SEVERITY_ERROR,
    ORBBEC_LOG_SEVERITY_FATAL,
    ORBBEC_LOG_SEVERITY_NONE,
    ORBBEC_LOG_SEVERITY_COUNT
} orbbec_log_severity;

const char* orbbec_log_severity_to_string(orbbec_log_severity log);

typedef enum orbbec_extension
{
    ORBBEC_EXTENSION_UNKNOWN,
    ORBBEC_EXTENSION_DEBUG,
    ORBBEC_EXTENSION_INFO,
    ORBBEC_EXTENSION_MOTION,
    ORBBEC_EXTENSION_OPTIONS,
    ORBBEC_EXTENSION_VIDEO,
    ORBBEC_EXTENSION_ROI,
    ORBBEC_EXTENSION_DEPTH_SENSOR,
    ORBBEC_EXTENSION_VIDEO_FRAME,
    ORBBEC_EXTENSION_MOTION_FRAME,
    ORBBEC_EXTENSION_COMPOSITE_FRAME,
    ORBBEC_EXTENSION_POINTS,
    ORBBEC_EXTENSION_DEPTH_FRAME,
    ORBBEC_EXTENSION_ADVANCED_MODE,
    ORBBEC_EXTENSION_RECORD,
    ORBBEC_EXTENSION_VIDEO_PROFILE,
    ORBBEC_EXTENSION_PLAYBACK,
    ORBBEC_EXTENSION_DEPTH_STEREO_SENSOR,
    ORBBEC_EXTENSION_MOTION_PROFILE,
    ORBBEC_EXTENSION_DISPARITY_FRAME,
    ORBBEC_EXTENSION_POSE_FRAME,
    ORBBEC_EXTENSION_POSE_PROFILE,
    ORBBEC_EXTENSION_TM2,
    ORBBEC_EXTENSION_SOFTWARE_DEVICE,
    ORBBEC_EXTENSION_SOFTWARE_SENSOR,
    ORBBEC_EXTENSION_COUNT
} orbbec_extension;

const char* orbbec_extension_to_string(orbbec_extension type);

//typedef enum orbbec_matchers
//{
//   ORBBEC_MATCHER_DI,      //compare depth and ir based on frame number

//   ORBBEC_MATCHER_DI_C,    //compare depth and ir based on frame number,
//                        //compare the pair of corresponding depth and ir with color based on closest timestamp,
//                        //commonlly used by SR300

//   ORBBEC_MATCHER_DLR_C,   //compare depth, left and right ir based on frame number,
//                        //compare the set of corresponding depth, left and right with color based on closest timestamp,
//                        //commonlly used by RS415, RS435

//   ORBBEC_MATCHER_DLR,     //compare depth, left and right ir based on frame number,
//                        //commonlly used by RS400, RS405, RS410, RS420, RS430

//   ORBBEC_MATCHER_DEFAULT, //the default matcher compare all the streams based on closest timestamp

//   ORBBEC_MATCHER_COUNT
//}orbbec_matchers;

typedef struct orbbec_device_info orbbec_device_info;
typedef struct orbbec_device orbbec_device;
typedef struct orbbec_error orbbec_error;
typedef struct orbbec_raw_data_buffer orbbec_raw_data_buffer;
typedef struct orbbec_frame orbbec_frame;
typedef struct orbbec_frame_queue orbbec_frame_queue;
typedef struct orbbec_pipeline orbbec_pipeline;
typedef struct orbbec_pipeline_profile orbbec_pipeline_profile;
typedef struct orbbec_config orbbec_config;
typedef struct orbbec_device_list orbbec_device_list;
typedef struct orbbec_stream_profile_list orbbec_stream_profile_list;
typedef struct orbbec_stream_profile orbbec_stream_profile;
typedef struct orbbec_frame_callback orbbec_frame_callback;
typedef struct orbbec_log_callback orbbec_log_callback;
typedef struct orbbec_syncer orbbec_syncer;
typedef struct orbbec_device_serializer orbbec_device_serializer;
typedef struct orbbec_source orbbec_source;
typedef struct orbbec_processing_block orbbec_processing_block;
typedef struct orbbec_frame_processor_callback orbbec_frame_processor_callback;
typedef struct orbbec_playback_status_changed_callback orbbec_playback_status_changed_callback;
typedef struct orbbec_context orbbec_context;
typedef struct orbbec_device_hub orbbec_device_hub;
typedef struct orbbec_sensor_list orbbec_sensor_list;
typedef struct orbbec_sensor orbbec_sensor;
typedef struct orbbec_options orbbec_options;
typedef struct orbbec_devices_changed_callback orbbec_devices_changed_callback;
typedef struct orbbec_notification orbbec_notification;
typedef struct orbbec_notifications_callback orbbec_notifications_callback;
typedef void (*orbbec_notification_callback_ptr)(orbbec_notification*,void*);
typedef void (*orbbec_devices_changed_callback_ptr)(orbbec_device_list*, orbbec_device_list*, void*);
typedef void (*orbbec_frame_callback_ptr)(orbbec_frame*, void*);
typedef void (*orbbec_frame_processor_callback_ptr)(orbbec_frame*, orbbec_frame*, void*);

typedef double orbbec_time_t;               //timestamp ms
typedef long long orbbec_metadata_type;     // metadata attribute type

orbbec_exception_type orbbec_get_liborbbec_exception_type(const orbbec_error* error);
const char* orbbec_get_failed_function (const orbbec_error* error);
const char* orbbec_get_failed_args (const orbbec_error* error);
const char* orbbec_get_error_message (const orbbec_error* error);
void orbbec_free_error (orbbec_error* error);

#ifdef __cplusplus
}
#endif
#endif // ORBBEC_TYPES_H
