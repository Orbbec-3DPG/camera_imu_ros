#ifndef INTERNAL_HPP
#define INTERNAL_HPP

#include "types.hpp"
#include "device.hpp"
#include "context.hpp"
#include "../h/internal.h"

namespace orbbec
{
    class recording_context : public context
    {
    public:
        /**
        * create librealsense context that will try to record all operations over librealsense into a file
        * \param[in] filename string representing the name of the file to record
        */
        recording_context(const std::string& filename,
                          const std::string& section = "",
                          orbbec_recording_mode mode = ORBBEC_RECORDING_MODE_BLANK_FRAMES)
        {
            orbbec_error* e = nullptr;
            _context = std::shared_ptr<orbbec_context>(
                orbbec_create_recording_context(ORBBEC_API_VERSION, filename.c_str(), section.c_str(), mode, &e),
                orbbec_delete_context);
            error::handle(e);
        }

        recording_context() = delete;
    };

    class mock_context : public context
    {
    public:
        /**
        * create librealsense context that given a file will respond to calls exactly as the recording did
        * if the user calls a method that was either not called during recording or violates causality of the recording error will be thrown
        * \param[in] filename string of the name of the file
        */
        mock_context(const std::string& filename,
                     const std::string& section = "")
        {
            orbbec_error* e = nullptr;
            _context = std::shared_ptr<orbbec_context>(
                orbbec_create_mock_context(ORBBEC_API_VERSION, filename.c_str(), section.c_str(), &e),
                orbbec_delete_context);
            error::handle(e);
        }

        mock_context() = delete;
    };

    namespace internal
    {
        /**
        * \return            the time at specific time point, in live and redord contextes it will return the system time and in playback contextes it will return the recorded time
        */
        inline double get_time()
        {
            orbbec_error* e = nullptr;
            auto time = orbbec_get_time( &e);

            error::handle(e);

            return time;
        }
    }

    class software_sensor : public sensor
    {
    public:
        /**
        * Add video stream to software sensor
        *
        * \param[in] video_stream   all the parameters that required to defind video stream
        */
        stream_profile add_video_stream(orbbec_video_stream video_stream)
        {
            orbbec_error* e = nullptr;

            stream_profile stream(orbbec_software_sensor_add_video_stream(_sensor.get(),video_stream, &e));
            error::handle(e);

            return stream;
        }

        /**
        * Inject frame into the sensor
        *
        * \param[in] frame   all the parameters that required to define video frame
        */
        void on_video_frame(orbbec_software_video_frame frame)
        {
            orbbec_error* e = nullptr;
            orbbec_software_sensor_on_video_frame(_sensor.get(), frame, &e);
            error::handle(e);
        }

        /**
        * Register option that will be supported by the sensor
        *
        * \param[in] option  the option
        * \param[in] val  the initial value
        */
        void add_read_only_option(orbbec_option option, float val)
        {
            orbbec_error* e = nullptr;
            orbbec_software_sensor_add_read_only_option(_sensor.get(), option, val, &e);
            error::handle(e);
        }

        /**
        * Update value of registered option
        *
        * \param[in] option  the option
        * \param[in] val  the initial value
        */
        void set_read_only_option(orbbec_option option, float val)
        {
            orbbec_error* e = nullptr;
            orbbec_software_sensor_update_read_only_option(_sensor.get(), option, val, &e);
            error::handle(e);
        }
    private:
        friend class software_device;

        software_sensor(std::shared_ptr<orbbec_sensor> s)
            : rs2::sensor(s)
        {
            orbbec_error* e = nullptr;
            if (orbbec_is_sensor_extendable_to(_sensor.get(), ORBBEC_EXTENSION_SOFTWARE_SENSOR, &e) == 0 && !e)
            {
                _sensor = nullptr;
            }
            rs2::error::handle(e);
        }
    };


    class software_device : public device
    {
        std::shared_ptr<orbbec_device> create_device_ptr()
        {
            orbbec_error* e = nullptr;
            std::shared_ptr<orbbec_device> dev(
                orbbec_create_software_device(&e),
                orbbec_delete_device);
            error::handle(e);
            return dev;
        }

    public:
        software_device()
            : device(create_device_ptr())
        {}

        /**
        * Add sensor stream to software sensor
        *
        * \param[in] name   the name of the sensor
        */
        software_sensor add_sensor(std::string name)
        {
            orbbec_error* e = nullptr;
            std::shared_ptr<orbbec_sensor> sensor(
                orbbec_software_device_add_sensor(_dev.get(), name.c_str(), &e),
                orbbec_delete_sensor);
            error::handle(e);

            return software_sensor(sensor);
        }

        /**
        * Set the wanted matcher type that will be used by the syncer
        * \param[in] matcher matcher type
        */
        void create_matcher(orbbec_matchers matcher)
        {
            orbbec_error* e = nullptr;
            orbbec_software_device_create_matcher(_dev.get(), matcher, &e);
            error::handle(e);
        }
    };

}
#endif // INTERNAL_HPP
