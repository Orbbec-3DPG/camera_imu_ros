#ifndef ORBBEC_SENSOR_HPP
#define ORBBEC_SENSOR_HPP

#include "orbbec_types.hpp"
#include "orbbec_frame.hpp"

namespace orbbec
{
    class notification
    {
    public:
        notification(orbbec_notification* notification)
        {
            orbbec_error* e = nullptr;
            _description = orbbec_get_notification_description(notification, &e);
            error::handle(e);
            _timestamp = orbbec_get_notification_timestamp(notification, &e);
            error::handle(e);
            _severity = orbbec_get_notification_severity(notification, &e);
            error::handle(e);
            _category = orbbec_get_notification_category(notification, &e);
            error::handle(e);
            _serialized_data = orbbec_get_notification_serialized_data(notification, &e);
            error::handle(e);
        }

        notification() = default;

        /**
        * retrieve the notification category
        * \return            the notification category
        */
        orbbec_notification_category get_category() const
        {
            return _category;
        }
        /**
        * retrieve the notification description
        * \return            the notification description
        */
        std::string get_description() const
        {
            return _description;
        }

        /**
        * retrieve the notification arrival timestamp
        * \return            the arrival timestamp
        */
        double get_timestamp() const
        {
            return _timestamp;
        }

        /**
        * retrieve the notification severity
        * \return            the severity
        */
        orbbec_log_severity get_severity() const
        {
            return _severity;
        }

        /**
        * retrieve the notification's serialized data
        * \return            the serialized data (in JSON format)
        */
        std::string get_serialized_data() const
        {
            return _serialized_data;
        }

    private:
        std::string _description;
        double _timestamp = -1;
        orbbec_log_severity _severity = ORBBEC_LOG_SEVERITY_COUNT;
        orbbec_notification_category _category = ORBBEC_NOTIFICATION_CATEGORY_COUNT;
        std::string _serialized_data;
    };

    template<class T>
    class notifications_callback : public orbbec_notifications_callback
    {
        T on_notification_function;
    public:
        explicit notifications_callback(T on_notification) : on_notification_function(on_notification) {}

        void on_notification(orbbec_notification* _notification) override
        {
            on_notification_function(notification{ _notification });
        }

        void release() override { delete this; }
    };

    template<class T>
    class frame_callback : public orbbec_frame_callback
    {
        T on_frame_function;
    public:
        explicit frame_callback(T on_frame) : on_frame_function(on_frame) {}

        void on_frame(orbbec_frame* fref) override
        {
            on_frame_function(frame{ fref });
        }

        void release() override { delete this; }
    };

    class options
    {
    public:
        /**
        * check if particular option is supported
        * \param[in] option     option id to be checked
        * \return true if option is supported
        */
        bool supports(orbbec_option option) const
        {
            orbbec_error* e = nullptr;
            auto res = orbbec_supports_option(_options, option, &e);
            error::handle(e);
            return res > 0;
        }

        /**
        * get option description
        * \param[in] option     option id to be checked
        * \return human-readable option description
        */
        const char* get_option_description(orbbec_option option) const
        {
            orbbec_error* e = nullptr;
            auto res = orbbec_get_option_description(_options, option, &e);
            error::handle(e);
            return res;
        }

        /**
        * get option value description (in case specific option value hold special meaning)
        * \param[in] option     option id to be checked
        * \param[in] val      value of the option
        * \return human-readable description of a specific value of an option or null if no special meaning
        */
        const char* get_option_value_description(orbbec_option option, float val) const
        {
            orbbec_error* e = nullptr;
            auto res = orbbec_get_option_value_description(_options, option, val, &e);
            error::handle(e);
            return res;
        }

        /**
        * read option's value
        * \param[in] option   option id to be queried
        * \return value of the option
        */
        float get_option(orbbec_option option) const
        {
            orbbec_error* e = nullptr;
            auto res = orbbec_get_option(_options, option, &e);
            error::handle(e);
            return res;
        }

        /**
        * retrieve the available range of values of a supported option
        * \return option  range containing minimum and maximum values, step and default value
        */
        option_range get_option_range(orbbec_option option) const
        {
            option_range result;
            orbbec_error* e = nullptr;
            orbbec_get_option_range(_options, option,
                &result.min, &result.max, &result.step, &result.def, &e);
            error::handle(e);
            return result;
        }

        /**
        * write new value to the option
        * \param[in] option     option id to be queried
        * \param[in] value      new value for the option
        */
        void set_option(orbbec_option option, float value) const
        {
            orbbec_error* e = nullptr;
            orbbec_set_option(_options, option, value, &e);
            error::handle(e);
        }

        /**
        * check if particular option is read-only
        * \param[in] option     option id to be checked
        * \return true if option is read-only
        */
        bool is_option_read_only(orbbec_option option) const
        {
            orbbec_error* e = nullptr;
            auto res = orbbec_is_option_read_only(_options, option, &e);
            error::handle(e);
            return res > 0;
        }

        options& operator=(const options& other)
        {
            _options = other._options;
            return *this;
        }

        virtual ~options() = default;
   protected:
       explicit options(orbbec_options* o = nullptr) : _options(o) {}

       template<class T>
       options& operator=(const T& dev)
       {
           _options = (orbbec_options*)(dev.get());
           return *this;
       }

       options(const options& other) : _options(other._options) {}

    private:
        orbbec_options* _options;
    };

    class sensor : public options
    {
    public:

        using options::supports;
        /**
        * open sensor for exclusive access, by committing to a configuration
        * \param[in] profile    configuration committed by the sensor
        */
        void open(const stream_profile& profile) const
        {
            orbbec_error* e = nullptr;
            orbbec_open(_sensor.get(),
                profile.get(),
                &e);
            error::handle(e);
        }

        /**
        * check if specific camera info is supported
        * \param[in] info    the parameter to check for support
        * \return                true if the parameter both exist and well-defined for the specific sensor
        */
        bool supports(orbbec_camera_info info) const
        {
            orbbec_error* e = nullptr;
            auto is_supported = orbbec_supports_sensor_info(_sensor.get(), info, &e);
            error::handle(e);
            return is_supported > 0;
        }

        /**
        * retrieve camera specific information, like versions of various internal components
        * \param[in] info     camera info type to retrieve
        * \return             the requested camera info string, in a format specific to the sensor model
        */
        const char* get_info(orbbec_camera_info info) const
        {
            orbbec_error* e = nullptr;
            auto result = orbbec_get_sensor_info(_sensor.get(), info, &e);
            error::handle(e);
            return result;
        }

        /**
        * open sensor for exclusive access, by committing to composite configuration, specifying one or more stream profiles
        * this method should be used for interdependent  streams, such as depth and infrared, that have to be configured together
        * \param[in] profiles   vector of configurations to be commited by the sensor
        */
        void open(const std::vector<stream_profile>& profiles) const
        {
            orbbec_error* e = nullptr;

            std::vector<const orbbec_stream_profile*> profs;
            profs.reserve(profiles.size());
            for (auto& p : profiles)
            {
                profs.push_back(p.get());
            }

            orbbec_open_multiple(_sensor.get(),
                profs.data(),
                static_cast<int>(profiles.size()),
                &e);
            error::handle(e);
        }

        /**
        * close sensor for exclusive access
        * this method should be used for releasing sensor resource
        */
        void close() const
        {
            orbbec_error* e = nullptr;
            orbbec_close(_sensor.get(), &e);
            error::handle(e);
        }

        /**
        * Start passing frames into user provided callback
        * \param[in] callback   Stream callback, can be any callable object accepting rs2::frame
        */
        template<class T>
        void start(T callback) const
        {
            orbbec_error* e = nullptr;
            orbbec_start_cpp(_sensor.get(), new frame_callback<T>(std::move(callback)), &e);
            error::handle(e);
        }

        /**
        * stop streaming
        */
        void stop() const
        {
            orbbec_error* e = nullptr;
            orbbec_stop(_sensor.get(), &e);
            error::handle(e);
        }

        /**
        * register notifications callback
        * \param[in] callback   notifications callback
        */
        template<class T>
        void set_notifications_callback(T callback) const
        {
            orbbec_error* e = nullptr;
            orbbec_set_notifications_callback_cpp(_sensor.get(),
                new notifications_callback<T>(std::move(callback)), &e);
            error::handle(e);
        }


        /**
        * check if physical sensor is supported
        * \return   list of stream profiles that given sensor can provide, should be released by orbbec_delete_profiles_list
        */
        std::vector<stream_profile> get_stream_profiles() const
        {
            std::vector<stream_profile> results;

            orbbec_error* e = nullptr;
            std::shared_ptr<orbbec_stream_profile_list> list(
                orbbec_get_stream_profiles(_sensor.get(), &e),
                orbbec_delete_stream_profiles_list);
            error::handle(e);

            auto size = orbbec_get_stream_profiles_count(list.get(), &e);
            error::handle(e);

            for (auto i = 0; i < size; i++)
            {
                stream_profile profile(orbbec_get_stream_profile(list.get(), i, &e));
                error::handle(e);
                results.push_back(profile);
            }

            return results;
        }

        sensor& operator=(const std::shared_ptr<orbbec_sensor> other)
        {
            options::operator=(other);
            _sensor.reset();
            _sensor = other;
            return *this;
        }

        sensor& operator=(const sensor& other)
        {
            *this = nullptr;
             options::operator=(other._sensor);
            _sensor = other._sensor;
            return *this;
        }
        sensor() : _sensor(nullptr) {}

        operator bool() const
        {
            return _sensor != nullptr;
        }

        const std::shared_ptr<orbbec_sensor>& get() const
        {
            return _sensor;
        }

        template<class T>
        bool is() const
        {
            T extension(*this);
            return extension;
        }

        template<class T>
        T as() const
        {
            T extension(*this);
            return extension;
        }

    protected:
        friend context;
        friend device_list;
        friend device;
        friend device_base;
        friend roi_sensor;

        std::shared_ptr<orbbec_sensor> _sensor;

        explicit sensor(std::shared_ptr<orbbec_sensor> dev)
            :options((orbbec_options*)dev.get()),  _sensor(dev)
        {
        }
    };

    inline bool operator==(const sensor& lhs, const sensor& rhs)
    {
        if (!(lhs.supports(ORBBEC_CAMERA_INFO_NAME) && lhs.supports(ORBBEC_CAMERA_INFO_SERIAL_NUMBER)
            && rhs.supports(ORBBEC_CAMERA_INFO_NAME) && rhs.supports(ORBBEC_CAMERA_INFO_SERIAL_NUMBER)))
            return false;

        return std::string(lhs.get_info(ORBBEC_CAMERA_INFO_NAME)) == rhs.get_info(ORBBEC_CAMERA_INFO_NAME)
            && std::string(lhs.get_info(ORBBEC_CAMERA_INFO_SERIAL_NUMBER)) == rhs.get_info(ORBBEC_CAMERA_INFO_SERIAL_NUMBER);
    }

    class roi_sensor : public sensor
    {
    public:
        roi_sensor(sensor s)
            : sensor(s.get())
        {
            orbbec_error* e = nullptr;
            if(orbbec_is_sensor_extendable_to(_sensor.get(), ORBBEC_EXTENSION_ROI, &e) == 0 && !e)
            {
                _sensor.reset();
            }
            error::handle(e);
        }

        void set_region_of_interest(const region_of_interest& roi)
        {
            orbbec_error* e = nullptr;
            orbbec_set_region_of_interest(_sensor.get(), roi.min_x, roi.min_y, roi.max_x, roi.max_y, &e);
            error::handle(e);
        }

        region_of_interest get_region_of_interest() const
        {
            region_of_interest roi {};
            orbbec_error* e = nullptr;
            orbbec_get_region_of_interest(_sensor.get(), &roi.min_x, &roi.min_y, &roi.max_x, &roi.max_y, &e);
            error::handle(e);
            return roi;
        }

        operator bool() const { return _sensor.get() != nullptr; }
    };

    class depth_sensor : public sensor
    {
    public:
        depth_sensor(sensor s)
            : sensor(s.get())
        {
            orbbec_error* e = nullptr;
            if (orbbec_is_sensor_extendable_to(_sensor.get(), ORBBEC_EXTENSION_DEPTH_SENSOR, &e) == 0 && !e)
            {
                _sensor.reset();
            }
            error::handle(e);
        }

        /** Retrieves mapping between the units of the depth image and meters
        * \return depth in meters corresponding to a depth value of 1
        */
        float get_depth_scale() const
        {
            orbbec_error* e = nullptr;
            auto res = orbbec_get_depth_scale(_sensor.get(), &e);
            error::handle(e);
            return res;
        }

        operator bool() const { return _sensor.get() != nullptr; }
    };

    class depth_stereo_sensor : public depth_sensor
    {
    public:
        depth_stereo_sensor(sensor s): depth_sensor(s)
        {
            orbbec_error* e = nullptr;
            if (_sensor && orbbec_is_sensor_extendable_to(_sensor.get(), ORBBEC_EXTENSION_DEPTH_STEREO_SENSOR, &e) == 0 && !e)
            {
                _sensor.reset();
            }
            error::handle(e);
        }

        /** Retrieves mapping between the units of the depth image and meters
        * \return depth in meters corresponding to a depth value of 1
        */
        float get_stereo_baseline() const
        {
            orbbec_error* e = nullptr;
            auto res = orbbec_get_depth_scale(_sensor.get(), &e);
            error::handle(e);
            return res;
        }

        operator bool() const { return _sensor.get() != nullptr; }
    };
}

#endif // ORBBEC_SENSOR_HPP
