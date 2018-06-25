#ifndef ORBBEC_TYPES_HPP
#define ORBBEC_TYPES_HPP

#include "../orbbec.h"
#include "../h/orbbec_context.h"
#include "../h/orbbec_device.h"
#include "../h/orbbec_frame.h"
#include "../h/orbbec_processing.h"
#include "../h/orbbec_record_playback.h"
#include "../h/orbbec_sensor.h"
#include "../h/orbbec_pipeline.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <exception>
#include <iterator>
#include <sstream>
#include <chrono>

struct orbbec_frame_callback
{
    virtual void on_frame(orbbec_frame * f) = 0;
    virtual void release() = 0;
    virtual ~orbbec_frame_callback() {}
};

struct orbbec_frame_processor_callback
{
    virtual void on_frame(orbbec_frame * f, orbbec_source * source) = 0;
    virtual void release() = 0;
    virtual ~orbbec_frame_processor_callback() {}
};

struct orbbec_notifications_callback
{
    virtual void on_notification(orbbec_notification* n) = 0;
    virtual void release() = 0;
    virtual ~orbbec_notifications_callback() {}
};

struct orbbec_log_callback
{
    virtual void on_event(orbbec_log_severity severity, const char * message) = 0;
    virtual void release() = 0;
    virtual ~orbbec_log_callback() {}
};

struct orbbec_devices_changed_callback
{
    virtual void on_devices_changed(orbbec_device_list* removed, orbbec_device_list* added) = 0;
    virtual void release() = 0;
    virtual ~orbbec_devices_changed_callback() {}
};

struct orbbec_playback_status_changed_callback
{
    virtual void on_playback_status_changed(orbbec_playback_status status) = 0;
    virtual void release() = 0;
    virtual ~orbbec_playback_status_changed_callback() {}
};

namespace orbbec
{
    class error : public std::runtime_error
    {
        std::string function, args;
        orbbec_exception_type type;
    public:
        explicit error(orbbec_error* err) : runtime_error(orbbec_get_error_message(err))
        {
            function = (nullptr != orbbec_get_failed_function(err)) ? orbbec_get_failed_function(err) : std::string();
            args = (nullptr != orbbec_get_failed_args(err)) ? orbbec_get_failed_args(err) : std::string();
            type = orbbec_get_librealsense_exception_type(err);
            orbbec_free_error(err);
        }

        explicit error(const std::string& message) : runtime_error(message.c_str())
        {
            function = "";
            args = "";
            type = ORBBEC_EXCEPTION_TYPE_UNKNOWN;
        }

        const std::string& get_failed_function() const
        {
            return function;
        }

        const std::string& get_failed_args() const
        {
            return args;
        }

        orbbec_exception_type get_type() const { return type; }

        static void handle(orbbec_error* e);
    };

    #define ORBBEC_ERROR_CLASS(name, base) \
    class name : public base\
    {\
    public:\
        explicit name(orbbec_error* e) noexcept : base(e) {}\
    }

    ORBBEC_ERROR_CLASS(recoverable_error, error);
    ORBBEC_ERROR_CLASS(unrecoverable_error, error);
    ORBBEC_ERROR_CLASS(camera_disconnected_error, unrecoverable_error);
    ORBBEC_ERROR_CLASS(backend_error, unrecoverable_error);
    ORBBEC_ERROR_CLASS(device_in_recovery_mode_error, unrecoverable_error);
    ORBBEC_ERROR_CLASS(invalid_value_error, recoverable_error);
    ORBBEC_ERROR_CLASS(wrong_api_call_sequence_error, recoverable_error);
    ORBBEC_ERROR_CLASS(not_implemented_error, recoverable_error);
    #undef ORBBEC_ERROR_CLASS

    inline void error::handle(orbbec_error* e)
    {
        if (e)
        {
            auto h = orbbec_get_librealsense_exception_type(e);
            switch (h) {
            case ORBBEC_EXCEPTION_TYPE_CAMERA_DISCONNECTED:
                throw camera_disconnected_error(e);
            case ORBBEC_EXCEPTION_TYPE_BACKEND:
                throw backend_error(e);
            case ORBBEC_EXCEPTION_TYPE_INVALID_VALUE:
                throw invalid_value_error(e);
            case ORBBEC_EXCEPTION_TYPE_WRONG_API_CALL_SEQUENCE:
                throw wrong_api_call_sequence_error(e);
            case ORBBEC_EXCEPTION_TYPE_NOT_IMPLEMENTED:
                throw not_implemented_error(e);
            case ORBBEC_EXCEPTION_TYPE_DEVICE_IN_RECOVERY_MODE:
                throw device_in_recovery_mode_error(e);
            default:
                throw error(e);
            }
        }
    }

    class context;
    class device;
    class device_list;
    class syncer;
    class device_base;
    class roi_sensor;
    class frame;

    struct option_range
    {
        float min;
        float max;
        float def;
        float step;
    };

    struct region_of_interest
    {
        int min_x;
        int min_y;
        int max_x;
        int max_y;
    };
}

#endif // ORBBEC_TYPES_HPP
