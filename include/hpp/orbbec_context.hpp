#ifndef ORBBEC_CONTEXT_HPP
#define ORBBEC_CONTEXT_HPP

#include "orbbec_types.hpp"
#include "orbbec_record_playback.hpp"
#include "orbbec_processing.hpp"

namespace rs2
{
    class event_information
    {
    public:
        event_information(device_list removed, device_list added)
            :_removed(removed), _added(added) {}

        /**
        * check if specific device was disconnected
        * \return            true if device disconnected, false if device connected
        */
        bool was_removed(const rs2::device& dev) const
        {
            orbbec_error* e = nullptr;

            if (!dev)
                return false;

            auto res = orbbec_device_list_contains(_removed.get_list(), dev.get().get(), &e);
            error::handle(e);

            return res > 0;
        }

        /**
        * returns a list of all newly connected devices
        * \return            the list of all new connected devices
        */
        device_list get_new_devices()  const
        {
            return _added;
        }

    private:
        device_list _removed;
        device_list _added;
    };

    template<class T>
    class devices_changed_callback : public orbbec_devices_changed_callback
    {
        T _callback;

    public:
        explicit devices_changed_callback(T callback) : _callback(callback) {}

        void on_devices_changed(orbbec_device_list* removed, orbbec_device_list* added) override
        {
            std::shared_ptr<orbbec_device_list> old(removed, orbbec_delete_device_list);
            std::shared_ptr<orbbec_device_list> news(added, orbbec_delete_device_list);


            event_information info({ device_list(old), device_list(news) });
            _callback(info);
        }

        void release() override { delete this; }
    };

    class pipeline;
    class device_hub;

    /**
    * default librealsense context class
    * includes realsense API version as provided by ORBBEC_API_VERSION macro
    */
    class context
    {
    public:
        context()
        {
            orbbec_error* e = nullptr;
            _context = std::shared_ptr<orbbec_context>(
                orbbec_create_context(ORBBEC_API_VERSION, &e),
                orbbec_delete_context);
            error::handle(e);
        }

        /**
        * create a static snapshot of all connected devices at the time of the call
        * \return            the list of devices connected devices at the time of the call
        */
        device_list query_devices() const
        {
            orbbec_error* e = nullptr;
            std::shared_ptr<orbbec_device_list> list(
                orbbec_query_devices(_context.get(), &e),
                orbbec_delete_device_list);
            error::handle(e);

            return device_list(list);
        }

        /**
         * @brief Generate a flat list of all available sensors from all RealSense devices
         * @return List of sensors
         */
        std::vector<sensor> query_all_sensors() const
        {
            std::vector<sensor> results;
            for (auto&& dev : query_devices())
            {
                auto sensors = dev.query_sensors();
                std::copy(sensors.begin(), sensors.end(), std::back_inserter(results));
            }
            return results;
        }


        device get_sensor_parent(const sensor& s) const
        {
            orbbec_error* e = nullptr;
            std::shared_ptr<orbbec_device> dev(
                orbbec_create_device_from_sensor(s._sensor.get(), &e),
                orbbec_delete_device);
            error::handle(e);
            return device{ dev };
        }

        /**
        * register devices changed callback
        * \param[in] callback   devices changed callback
        */
        template<class T>
        void set_devices_changed_callback(T callback)
        {
            orbbec_error* e = nullptr;
            orbbec_set_devices_changed_callback_cpp(_context.get(),
                new devices_changed_callback<T>(std::move(callback)), &e);
            error::handle(e);
        }

        /**
         * Creates a device from a RealSense file
         *
         * On successful load, the device will be appended to the context and a devices_changed event triggered
         * @param file  Path to a RealSense File
         * @return A playback device matching the given file
         */
        playback load_device(const std::string& file)
        {
            orbbec_error* e = nullptr;
            auto device = std::shared_ptr<orbbec_device>(
                orbbec_context_add_device(_context.get(), file.c_str(), &e),
                orbbec_delete_device);
            rs2::error::handle(e);

            return playback { device };
        }

        void unload_device(const std::string& file)
        {
            orbbec_error* e = nullptr;
            orbbec_context_remove_device(_context.get(), file.c_str(), &e);
            rs2::error::handle(e);
        }

protected:
        friend class rs2::pipeline;
        friend class rs2::device_hub;

        context(std::shared_ptr<orbbec_context> ctx)
            : _context(ctx)
        {}
        std::shared_ptr<orbbec_context> _context;
    };

    /**
    * device_hub class - encapsulate the handling of connect and disconnect events
    */
    class device_hub
    {
    public:
        explicit device_hub(context ctx)
            : _ctx(std::move(ctx))
        {
            orbbec_error* e = nullptr;
            _device_hub = std::shared_ptr<orbbec_device_hub>(
                orbbec_create_device_hub(_ctx._context.get(), &e),
                orbbec_delete_device_hub);
            error::handle(e);
        }

        /**
        * If any device is connected return it, otherwise wait until next RealSense device connects.
        * Calling this method multiple times will cycle through connected devices
        */
        device wait_for_device() const
        {
            orbbec_error* e = nullptr;
            std::shared_ptr<orbbec_device> dev(
                orbbec_device_hub_wait_for_device(_ctx._context.get(), _device_hub.get(), &e),
                orbbec_delete_device);

            error::handle(e);

            return device(dev);

        }

        /**
        * Checks if device is still connected
        */
        bool is_connected(const device& dev) const
        {
            orbbec_error* e = nullptr;
            auto res = orbbec_device_hub_is_device_connected(_device_hub.get(), dev._dev.get(), &e);
            error::handle(e);

            return res > 0 ? true : false;

        }
    private:
        context _ctx;
        std::shared_ptr<orbbec_device_hub> _device_hub;
    };

}

#endif // ORBBEC_CONTEXT_HPP

