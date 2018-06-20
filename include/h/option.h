#ifndef ORBBEC_OPTION_H
#define ORBBEC_OPTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

typedef enum orbbec_option
{
    ORBBEC_OPTION_BACKLIGHT_COMPENSATION                     , /**< Enable / disable color backlight compensation*/
    ORBBEC_OPTION_BRIGHTNESS                                 , /**< Color image brightness*/
    ORBBEC_OPTION_CONTRAST                                   , /**< Color image contrast*/
    ORBBEC_OPTION_EXPOSURE                                   , /**< Controls exposure time of color camera. Setting any value will disable auto exposure*/
    ORBBEC_OPTION_GAIN                                       , /**< Color image gain*/
    ORBBEC_OPTION_GAMMA                                      , /**< Color image gamma setting*/
    ORBBEC_OPTION_HUE                                        , /**< Color image hue*/
    ORBBEC_OPTION_SATURATION                                 , /**< Color image saturation setting*/
    ORBBEC_OPTION_SHARPNESS                                  , /**< Color image sharpness setting*/
    ORBBEC_OPTION_WHITE_BALANCE                              , /**< Controls white balance of color image. Setting any value will disable auto white balance*/
    ORBBEC_OPTION_ENABLE_AUTO_EXPOSURE                       , /**< Enable / disable color image auto-exposure*/
    ORBBEC_OPTION_ENABLE_AUTO_WHITE_BALANCE                  , /**< Enable / disable color image auto-white-balance*/
    ORBBEC_OPTION_VISUAL_PRESET                              , /**< Provide access to several recommend sets of option presets for the depth camera */
    ORBBEC_OPTION_LASER_POWER                                , /**< Power of the F200 / SR300 projector, with 0 meaning projector off*/
    ORBBEC_OPTION_ACCURACY                                   , /**< Set the number of patterns projected per frame. The higher the accuracy value the more patterns projected. Increasing the number of patterns help to achieve better accuracy. Note that this control is affecting the Depth FPS */
    ORBBEC_OPTION_MOTION_RANGE                               , /**< Motion vs. Range trade-off, with lower values allowing for better motion sensitivity and higher values allowing for better depth range*/
    ORBBEC_OPTION_FILTER_OPTION                              , /**< Set the filter to apply to each depth frame. Each one of the filter is optimized per the application requirements*/
    ORBBEC_OPTION_CONFIDENCE_THRESHOLD                       , /**< The confidence level threshold used by the Depth algorithm pipe to set whether a pixel will get a valid range or will be marked with invalid range*/
    ORBBEC_OPTION_EMITTER_ENABLED                            , /**< Laser Emitter enabled */
    ORBBEC_OPTION_FRAMES_QUEUE_SIZE                          , /**< Number of frames the user is allowed to keep per stream. Trying to hold-on to more frames will cause frame-drops.*/
    ORBBEC_OPTION_TOTAL_FRAME_DROPS                          , /**< Total number of detected frame drops from all streams */
    ORBBEC_OPTION_AUTO_EXPOSURE_MODE                         , /**< Auto-Exposure modes: Static, Anti-Flicker and Hybrid */
    ORBBEC_OPTION_POWER_LINE_FREQUENCY                       , /**< Power Line Frequency control for anti-flickering Off/50Hz/60Hz/Auto */
    ORBBEC_OPTION_ASIC_TEMPERATURE                           , /**< Current Asic Temperature */
    ORBBEC_OPTION_ERROR_POLLING_ENABLED                      , /**< disable error handling */
    ORBBEC_OPTION_PROJECTOR_TEMPERATURE                      , /**< Current Projector Temperature */
    ORBBEC_OPTION_OUTPUT_TRIGGER_ENABLED                     , /**< Enable / disable trigger to be outputed from the camera to any external device on every depth frame */
    ORBBEC_OPTION_MOTION_MODULE_TEMPERATURE                  , /**< Current Motion-Module Temperature */
    ORBBEC_OPTION_DEPTH_UNITS                                , /**< Number of meters represented by a single depth unit */
    ORBBEC_OPTION_ENABLE_MOTION_CORRECTION                   , /**< Enable/Disable automatic correction of the motion data */
    ORBBEC_OPTION_AUTO_EXPOSURE_PRIORITY                     , /**< Allows sensor to dynamically ajust the frame rate depending on lighting conditions */
    ORBBEC_OPTION_COLOR_SCHEME                               , /**< Color scheme for data visualization */
    ORBBEC_OPTION_HISTOGRAM_EQUALIZATION_ENABLED             , /**< Perform histogram equalization post-processing on the depth data */
    ORBBEC_OPTION_MIN_DISTANCE                               , /**< Minimal distance to the target */
    ORBBEC_OPTION_MAX_DISTANCE                               , /**< Maximum distance to the target */
    ORBBEC_OPTION_TEXTURE_SOURCE                             , /**< Texture mapping stream unique ID */
    ORBBEC_OPTION_FILTER_MAGNITUDE                           , /**< The 2D-filter effect. The specific interpretation is given within the context of the filter */
    ORBBEC_OPTION_FILTER_SMOOTH_ALPHA                        , /**< 2D-filter parameter controls the weight/radius for smoothing.*/
    ORBBEC_OPTION_FILTER_SMOOTH_DELTA                        , /**< 2D-filter range/validity threshold*/
    ORBBEC_OPTION_HOLES_FILL                                 , /**< Enhance depth data post-processing with holes filling where appropriate*/
    ORBBEC_OPTION_STEREO_BASELINE                            , /**< The distance in mm between the first and the second imagers in stereo-based depth cameras*/
    ORBBEC_OPTION_AUTO_EXPOSURE_CONVERGE_STEP                , /**< Allows dynamically ajust the converge step value of the target exposure in Auto-Exposure algorithm*/
    ORBBEC_OPTION_COUNT                                        /**< Number of enumeration values. Not a valid input: intended to be used in for-loops. */
} orbbec_option;
const char* orbbec_option_to_string(orbbec_option option);

int orbbec_is_option_read_only(const orbbec_options* options, orbbec_option option, orbbec_error** error);
float orbbec_get_option(const orbbec_options* options, orbbec_option option, orbbec_error** error);
void orbbec_set_option(const orbbec_options* options, orbbec_option option, float value, orbbec_error** error);
int orbbec_supports_option(const orbbec_options* options, orbbec_option option, orbbec_error** error);
void orbbec_get_option_range(const orbbec_options* sensor, orbbec_option option, float* min, float* max, float* step, float* def, orbbec_error** error);
const char* orbbec_get_option_description(const orbbec_options* options, orbbec_option option, orbbec_error ** error);
const char* orbbec_get_option_value_description(const orbbec_options* options, orbbec_option option, float value, orbbec_error ** error);

#ifdef __cplusplus
}
#endif
#endif // ORBBEC_OPTION_H
