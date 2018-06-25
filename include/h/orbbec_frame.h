#ifndef ORBBEC_FRAME_H
#define ORBBEC_FRAME_H

#ifdef __cplusplus
extern "C" {
#endif
#include "orbbec_types.h"

typedef enum orbbec_timestamp_domain
{
    ORBBEC_TIMESTAMP_DOMAIN_HARDWARE_CLOCK, /**< Frame timestamp was measured in relation to the camera clock */
    ORBBEC_TIMESTAMP_DOMAIN_SYSTEM_TIME,    /**< Frame timestamp was measured in relation to the OS system clock */
    ORBBEC_TIMESTAMP_DOMAIN_COUNT           /**< Number of enumeration values. Not a valid input: intended to be used in for-loops. */
} orbbec_timestamp_domain;
const char* orbbec_timestamp_domain_to_string(orbbec_timestamp_domain info);

typedef enum orbbec_frame_metadata_value
{
    ORBBEC_FRAME_METADATA_FRAME_COUNTER        , /**< A sequential index managed per-stream. Integer value*/
    ORBBEC_FRAME_METADATA_FRAME_TIMESTAMP      , /**< Timestamp set by device clock when data readout and transmit commence. usec*/
    ORBBEC_FRAME_METADATA_SENSOR_TIMESTAMP     , /**< Timestamp of the middle of sensor's exposure calculated by device. usec*/
    ORBBEC_FRAME_METADATA_ACTUAL_EXPOSURE      , /**< Sensor's exposure width. When Auto Exposure (AE) is on the value is controlled by firmware. usec*/
    ORBBEC_FRAME_METADATA_GAIN_LEVEL           , /**< A relative value increasing which will increase the Sensor's gain factor. \
                                              When AE is set On, the value is controlled by firmware. Integer value*/
    ORBBEC_FRAME_METADATA_AUTO_EXPOSURE        , /**< Auto Exposure Mode indicator. Zero corresponds to AE switched off. */
    ORBBEC_FRAME_METADATA_WHITE_BALANCE        , /**< White Balance setting as a color temperature. Kelvin degrees*/
    ORBBEC_FRAME_METADATA_TIME_OF_ARRIVAL      , /**< Time of arrival in system clock */
    ORBBEC_FRAME_METADATA_TEMPERATURE          , /**< Temperature of the device, measured at the time of the frame capture. Celsius degrees */
    ORBBEC_FRAME_METADATA_BACKEND_TIMESTAMP    , /**< Timestamp get from uvc driver. usec*/
    ORBBEC_FRAME_METADATA_ACTUAL_FPS           , /**< Actual fps */
    ORBBEC_FRAME_METADATA_COUNT
} orbbec_frame_metadata_value;

const char* orbbec_frame_metadata_to_string(orbbec_frame_metadata_value metadata);
const char* orbbec_frame_metadata_value_to_string(orbbec_frame_metadata_value metadata);

typedef struct orbbec_vertex
{
    float xyz[3];
} orbbec_vertex;

typedef struct orbbec_pixel
{
    int ij[2];
} orbbec_pixel;

typedef struct orbbec_vector
{
    float x, y, z;
}orbbec_vector;

typedef struct orbbec_quaternion
{
    float x, y, z, w;
}orbbec_quaternion;

typedef struct orbbec_pose
{
    orbbec_vector      translation;          /**< X, Y, Z values of translation, in meters (relative to initial position)                                    */
    orbbec_vector      velocity;             /**< X, Y, Z values of velocity, in meter/sec                                                                   */
    orbbec_vector      acceleration;         /**< X, Y, Z values of acceleration, in meter/sec^2                                                             */
    orbbec_quaternion  rotation;             /**< Qi, Qj, Qk, Qr components of rotation as represented in quaternion rotation (relative to initial position) */
    orbbec_vector      angular_velocity;     /**< X, Y, Z values of angular velocity, in radians/sec                                                         */
    orbbec_vector      angular_acceleration; /**< X, Y, Z values of angular acceleration, in radians/sec^2                                                   */
    unsigned int    tracker_confidence;   /**< pose data confidence 0x0 - Failed, 0x1 - Low, 0x2 - Medium, 0x3 - High                                     */
    unsigned int    mapper_confidence;    /**< pose data confidence 0x0 - Failed, 0x1 - Low, 0x2 - Medium, 0x3 - High                                     */
} orbbec_pose;

orbbec_metadata_type orbbec_get_frame_metadata(const orbbec_frame* frame, orbbec_frame_metadata_value frame_metadata, orbbec_error** error);
int orbbec_supports_frame_metadata(const orbbec_frame* frame, orbbec_frame_metadata_value frame_metadata, orbbec_error** error);
orbbec_timestamp_domain orbbec_get_frame_timestamp_domain(const orbbec_frame* frameset, orbbec_error** error);
orbbec_time_t orbbec_get_frame_timestamp(const orbbec_frame* frame, orbbec_error** error);
unsigned long long orbbec_get_frame_number(const orbbec_frame* frame, orbbec_error** error);
const void* orbbec_get_frame_data(const orbbec_frame* frame, orbbec_error** error);
int orbbec_get_frame_width(const orbbec_frame* frame, orbbec_error** error);
int orbbec_get_frame_height(const orbbec_frame* frame, orbbec_error** error);
int orbbec_get_frame_stride_in_bytes(const orbbec_frame* frame, orbbec_error** error);
int orbbec_get_frame_bits_per_pixel(const orbbec_frame* frame, orbbec_error** error);
void orbbec_frame_add_ref(orbbec_frame* frame, orbbec_error ** error);
void orbbec_release_frame(orbbec_frame* frame);
void orbbec_keep_frame(orbbec_frame* frame);
orbbec_vertex* orbbec_get_frame_vertices(const orbbec_frame* frame, orbbec_error** error);
void orbbec_export_to_ply(const orbbec_frame* frame, const char* fname, orbbec_frame* texture, orbbec_error** error);
orbbec_pixel* orbbec_get_frame_texture_coordinates(const orbbec_frame* frame, orbbec_error** error);
int orbbec_get_frame_points_count(const orbbec_frame* frame, orbbec_error** error);
const orbbec_stream_profile* orbbec_get_frame_stream_profile(const orbbec_frame* frame, orbbec_error** error);
int orbbec_is_frame_extendable_to(const orbbec_frame* frame, orbbec_extension extension_type, orbbec_error ** error);
orbbec_frame* orbbec_allocate_synthetic_video_frame(orbbec_source* source, const orbbec_stream_profile* new_stream, orbbec_frame* original,
    int new_bpp, int new_width, int new_height, int new_stride, orbbec_extension frame_type, orbbec_error** error);
orbbec_frame* orbbec_allocate_composite_frame(orbbec_source* source, orbbec_frame** frames, int count, orbbec_error** error);
orbbec_frame* orbbec_extract_frame(orbbec_frame* composite, int index, orbbec_error** error);
int orbbec_embedded_frames_count(orbbec_frame* composite, orbbec_error** error);
void orbbec_synthetic_frame_ready(orbbec_source* source, orbbec_frame* frame, orbbec_error** error);
void orbbec_pose_frame_get_pose_data(const orbbec_frame* frame, orbbec_pose* pose, orbbec_error** error);

#ifdef __cplusplus
}
#endif
#endif // ORBBEC_FRAME_H
