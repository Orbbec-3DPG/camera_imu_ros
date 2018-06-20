#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "types.hpp"
#include "sensor.hpp"
#include <array>

namespace orbbec
{
    class context;
    class device_list;
    class pipeline_profile;
    class device_hub;

    class device
    {
    public:
        /**
        * returns the list of adjacent devices, sharing the same physical parent composite device
        * \return            the list of adjacent devices
        */
        std::vector<sensor> query_sensors() const
        {
            orbbec_error* e = nullptr;
            std::shared_ptr<orbbec_sensor_list> list(
                orbbec_query_sensors(_dev.get(), &e),
                orbbec_delete_sensor_list);
            error::handle(e);

            auto size = orbbec_get_sensors_count(list.get(), &e);
            error::handle(e);

            std::vector<sensor> results;
            for (auto i = 0; i < size; i++)
            {
                std::shared_ptr<orbbec_sensor> dev(
                    orbbec_create_sensor(list.get(), i, &e),
                    orbbec_delete_sensor);
                error::handle(e);

                sensor orbbec_dev(dev);
                results.push_back(orbbec_dev);
            }

            return results;
        }

        template<class T>
        T first() const
        {
            for (auto&& s : query_sensors())
            {
                if (auto t = s.as<T>()) return t;
            }
            throw rs2::error("Could not find requested sensor type!");
        }

        /**
        * check if specific camera info is supported
        * \param[in] info    the parameter to check for support
        * \return                true if the parameter both exist and well-defined for the specific device
        */
        bool supports(orbbec_camera_info info) const
        {
            orbbec_error* e = nullptr;
            auto is_supported = orbbec_supports_device_info(_dev.get(), info, &e);
            error::handle(e);
            return is_supported > 0;
        }

        /**
        * retrieve camera specific information, like versions of various internal components
        * \param[in] info     camera info type to retrieve
        * \return             the requested camera info string, in a format specific to the device model
        */
        const char* get_info(orbbec_camera_info info) const
        {
            orbbec_error* e = nullptr;
            auto result = orbbec_get_device_info(_dev.get(), info, &e);
            error::handle(e);
            return result;
        }

        /**
        * send hardware reset request to the device
        */
        void hardware_reset()
        {
            orbbec_error* e = nullptr;

            orbbec_hardware_reset(_dev.get(), &e);
            error::handle(e);
        }

        device& operator=(const std::shared_ptr<orbbec_device> dev)
        {
            _dev.reset();
            _dev = dev;
            return *this;
        }
        device& operator=(const device& dev)
        {
            *this = nullptr;
            _dev = dev._dev;
            return *this;
        }
        device() : _dev(nullptr) {}

        operator bool() const
        {
            return _dev != nullptr;
        }
        const std::shared_ptr<orbbec_device>& get() const
        {
            return _dev;
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
        virtual ~device()
        {
        }
    protected:
        friend class rs2::context;
        friend class rs2::device_list;
        friend class rs2::pipeline_profile;
        friend class rs2::device_hub;

        std::shared_ptr<orbbec_device> _dev;
        explicit device(std::shared_ptr<orbbec_device> dev) : _dev(dev)
        {
        }
    };

    class debug_protocol : public device
    {
    public:
        debug_protocol(device d)
                : device(d.get())
        {
            orbbec_error* e = nullptr;
            if(orbbec_is_device_extendable_to(_dev.get(), ORBBEC_EXTENSION_DEBUG, &e) == 0 && !e)
            {
                _dev.reset();
            }
            error::handle(e);
        }

        std::vector<uint8_t> send_and_receive_raw_data(const std::vector<uint8_t>& input) const
        {
            std::vector<uint8_t> results;

            orbbec_error* e = nullptr;
            std::shared_ptr<const orbbec_raw_data_buffer> list(
                    orbbec_send_and_receive_raw_data(_dev.get(), (void*)input.data(), (uint32_t)input.size(), &e),
                    orbbec_delete_raw_data);
            error::handle(e);

            auto size = orbbec_get_raw_data_size(list.get(), &e);
            error::handle(e);

            auto start = orbbec_get_raw_data(list.get(), &e);

            results.insert(results.begin(), start, start + size);

            return results;
        }
    };

    class device_list
    {
    public:
        explicit device_list(std::shared_ptr<orbbec_device_list> list)
            : _list(move(list)) {}

        device_list()
            : _list(nullptr) {}

        operator std::vector<device>() const
        {
            std::vector<device> res;
            for (auto&& dev : *this) res.push_back(dev);
            return res;
        }

        bool contains(const device& dev) const
        {
            orbbec_error* e = nullptr;
            auto res = !!(orbbec_device_list_contains(_list.get(), dev.get().get(), &e));
            error::handle(e);
            return res;
        }

        device_list& operator=(std::shared_ptr<orbbec_device_list> list)
        {
            _list = move(list);
            return *this;
        }

        device operator[](uint32_t index) const
        {
            orbbec_error* e = nullptr;
            std::shared_ptr<orbbec_device> dev(
                orbbec_create_device(_list.get(), index, &e),
                orbbec_delete_device);
            error::handle(e);

            return device(dev);
        }

        uint32_t size() const
        {
            orbbec_error* e = nullptr;
            auto size = orbbec_get_device_count(_list.get(), &e);
            error::handle(e);
            return size;
        }

        device front() const { return std::move((*this)[0]); }
        device back() const
        {
            return std::move((*this)[size() - 1]);
        }

        class device_list_iterator
        {
            device_list_iterator(
                const device_list& device_list,
                uint32_t uint32_t)
                : _list(device_list),
                  _index(uint32_t)
            {
            }

        public:
            device operator*() const
            {
                return _list[_index];
            }
            bool operator!=(const device_list_iterator& other) const
            {
                return other._index != _index || &other._list != &_list;
            }
            bool operator==(const device_list_iterator& other) const
            {
                return !(*this != other);
            }
            device_list_iterator& operator++()
            {
                _index++;
                return *this;
            }
        private:
            friend device_list;
            const device_list& _list;
            uint32_t _index;
        };

        device_list_iterator begin() const
        {
            return device_list_iterator(*this, 0);
        }
        device_list_iterator end() const
        {
            return device_list_iterator(*this, size());
        }
        const orbbec_device_list* get_list() const
        {
            return _list.get();
        }

    private:
        std::shared_ptr<orbbec_device_list> _list;
    };

    class tm2 : public device //TODO: add to wrappers
    {
    public:
        tm2(device d)
            : device(d.get())
        {
            orbbec_error* e = nullptr;
            if (orbbec_is_device_extendable_to(_dev.get(), ORBBEC_EXTENSION_TM2, &e) == 0 && !e)
            {
                _dev.reset();
            }
            error::handle(e);
        }

        /**
        * Enter the given device into loopback operation mode that uses the given file as input for raw data
        * \param[in]  from_file  Path to bag file with raw data for loopback
        */
        void enable_loopback(const std::string& from_file)
        {
            orbbec_error* e = nullptr;
            orbbec_loopback_enable(_dev.get(), from_file.c_str(), &e);
            error::handle(e);
        }

        /**
        * Restores the given device into normal operation mode
        */
        void disable_loopback()
        {
            orbbec_error* e = nullptr;
            orbbec_loopback_disable(_dev.get(), &e);
            error::handle(e);
        }

        /**
        * Checks if the device is in loopback mode or not
        * \return true if the device is in loopback operation mode
        */
        bool is_loopback_enabled() const
        {
            orbbec_error* e = nullptr;
            int is_enabled = orbbec_loopback_is_enabled(_dev.get(), &e);
            error::handle(e);
            return is_enabled != 0;
        }

        /**
        * Connects to a given tm2 controller
        * \param[in]  mac_addr   The MAC address of the desired controller
        */
        void connect_controller(const std::array<uint8_t, 6>& mac_addr)
        {
            orbbec_error* e = nullptr;
            orbbec_connect_tm2_controller(_dev.get(), mac_addr.data(), &e);
            error::handle(e);
        }

        /**
        * Disconnects a given tm2 controller
        * \param[in]  id         The ID of the desired controller
        */
        void disconnect_controller(int id)
        {
            orbbec_error* e = nullptr;
            orbbec_disconnect_tm2_controller(_dev.get(), id, &e);
            error::handle(e);
        }
    };
}

#endif // DEVICE_HPP
