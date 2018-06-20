#ifndef RECORD_PLAYBACK_HPP
#define RECORD_PLAYBACK_HPP

#include "types.hpp"
#include "device.hpp"

namespace orbbec
{
    template<class T>
    class status_changed_callback : public orbbec_playback_status_changed_callback
    {
        T on_status_changed_function;
    public:
        explicit status_changed_callback(T on_status_changed) : on_status_changed_function(on_status_changed) {}

        void on_playback_status_changed(orbbec_playback_status status) override
        {
            on_status_changed_function(status);
        }

        void release() override { delete this; }
    };

    class playback : public device
    {
    public:
        playback(device d) : playback(d.get()) {}

        /**
        * Pauses the playback
        * Calling pause() in "Paused" status does nothing
        * If pause() is called while playback status is "Playing" or "Stopped", the playback will not play until resume() is called
        */
        void pause()
        {
            orbbec_error* e = nullptr;
            orbbec_playback_device_pause(_dev.get(), &e);
            error::handle(e);
        }

        /**
        * Un-pauses the playback
        * Calling resume() while playback status is "Playing" or "Stopped" does nothing
        */
        void resume()
        {
            orbbec_error* e = nullptr;
            orbbec_playback_device_resume(_dev.get(), &e);
            error::handle(e);
        }

        /**
         * Retrieves the name of the playback file
         * \return Name of the playback file
         */
        std::string file_name() const
        {
            return m_file; //cached in construction
        }

        /**
        * Retrieves the current position of the playback in the file in terms of time. Units are expressed in nanoseconds
        * \return Current position of the playback in the file in terms of time. Units are expressed in nanoseconds
        */
        uint64_t get_position() const
        {
            orbbec_error* e = nullptr;
            uint64_t pos = orbbec_playback_get_position(_dev.get(), &e);
            error::handle(e);
            return pos;
        }

        /**
        * Retrieves the total duration of the file
        * \return Total duration of the file
        */
        std::chrono::nanoseconds get_duration() const
        {
            orbbec_error* e = nullptr;
            std::chrono::nanoseconds duration(orbbec_playback_get_duration(_dev.get(), &e));
            error::handle(e);
            return duration;
        }

        /**
        * Sets the playback to a specified time point of the played data
        * \param[in] time  The time point to which playback should seek, expressed in units of nanoseconds (zero value = start)
        */
        void seek(std::chrono::nanoseconds time)
        {
            orbbec_error* e = nullptr;
            orbbec_playback_seek(_dev.get(), time.count(), &e);
            error::handle(e);
        }

        /**
        * Indicates if playback is in real time mode or non real time
        * \return True iff playback is in real time mode
        */
        bool is_real_time() const
        {
            orbbec_error* e = nullptr;
            bool real_time = orbbec_playback_device_is_real_time(_dev.get(), &e) != 0;
            error::handle(e);
            return real_time;
        }

        /**
        * Set the playback to work in real time or non real time
        *
        * In real time mode, playback will play the same way the file was recorded.
        * In real time mode if the application takes too long to handle the callback, frames may be dropped.
        * In non real time mode, playback will wait for each callback to finish handling the data before
        * reading the next frame. In this mode no frames will be dropped, and the application controls the
        * frame rate of the playback (according to the callback handler duration).
        * \param[in] real_time  Indicates if real time is requested, 0 means false, otherwise true
        * \return True on successfully setting the requested mode
        */
        void set_real_time(bool real_time) const
        {
            orbbec_error* e = nullptr;
            orbbec_playback_device_set_real_time(_dev.get(), (real_time ? 1 : 0), &e);
            error::handle(e);
        }

        /**
        * Set the playing speed
        * \param[in] speed  Indicates a multiplication of the speed to play (e.g: 1 = normal, 0.5 twice as slow)
        */
        void set_playback_speed(float speed) const
        {
            orbbec_error* e = nullptr;
            orbbec_playback_device_set_playback_speed(_dev.get(), speed, &e);
            error::handle(e);
        }

        /**
        * Start passing frames into user provided callback
        * \param[in] callback   Stream callback, can be any callable object accepting rs2::frame
        */

        /**
        * Register to receive callback from playback device upon its status changes
        *
        * Callbacks are invoked from the reading thread, any heavy processing in the callback handler will affect
        * the reading thread and may cause frame drops\ high latency
        * \param[in] callback   A callback handler that will be invoked when the playback status changes, can be any callable object accepting orbbec_playback_status
        */
        template <typename T>
        void set_status_changed_callback(T callback)
        {
            orbbec_error* e = nullptr;
            orbbec_playback_device_set_status_changed_callback(_dev.get(), new status_changed_callback<T>(std::move(callback)), &e);
            error::handle(e);
        }

        /**
        * Returns the current state of the playback device
        * \return Current state of the playback
        */
        orbbec_playback_status current_status() const
        {
            orbbec_error* e = nullptr;
            orbbec_playback_status sts = orbbec_playback_device_get_current_status(_dev.get(), &e);
            error::handle(e);
            return sts;
        }

        /**
        * Stops the playback, effectively stopping all streaming playback sensors, and resetting the playback.
        *
        */
        void stop()
        {
            orbbec_error* e = nullptr;
            orbbec_playback_device_stop(_dev.get(), &e);
            error::handle(e);
        }
    protected:
        friend context;
        explicit playback(std::shared_ptr<orbbec_device> dev) : device(dev)
        {
            orbbec_error* e = nullptr;
            if(orbbec_is_device_extendable_to(_dev.get(), ORBBEC_EXTENSION_PLAYBACK, &e) == 0 && !e)
            {
                _dev.reset();
            }
            error::handle(e);

            if(_dev)
            {
                e = nullptr;
                m_file = orbbec_playback_device_get_file_path(_dev.get(), &e);
                error::handle(e);
            }
        }
    private:
        std::string m_file;
    };
    class recorder : public device
    {
    public:
        recorder(device d) : recorder(d.get()) {}

        /**
        * Creates a recording device to record the given device and save it to the given file as rosbag format
        * \param[in]  file      The desired path to which the recorder should save the data
        * \param[in]  device    The device to record
        */
        recorder(const std::string& file, rs2::device device)
        {
            orbbec_error* e = nullptr;
            _dev = std::shared_ptr<orbbec_device>(
                orbbec_create_record_device(device.get().get(), file.c_str(), &e),
                orbbec_delete_device);
            rs2::error::handle(e);
        }

        /**
        * Pause the recording device without stopping the actual device from streaming.
        */
        void pause()
        {
            orbbec_error* e = nullptr;
            orbbec_record_device_pause(_dev.get(), &e);
            error::handle(e);
        }

        /**
        * Unpauses the recording device, making it resume recording
        */
        void resume()
        {
            orbbec_error* e = nullptr;
            orbbec_record_device_resume(_dev.get(), &e);
            error::handle(e);
        }

        /**
        * Gets the name of the file to which the recorder is writing
        * \return The  name of the file to which the recorder is writing
        */
        std::string filename() const
        {
            orbbec_error* e = nullptr;
            std::string filename = orbbec_record_device_filename(_dev.get(), &e);
            error::handle(e);
            return filename;
        }
    protected:
        explicit recorder(std::shared_ptr<orbbec_device> dev) : device(dev)
        {
            orbbec_error* e = nullptr;
            if (orbbec_is_device_extendable_to(_dev.get(), ORBBEC_EXTENSION_RECORD, &e) == 0 && !e)
            {
                _dev.reset();
            }
            error::handle(e);
        }
    };
}

#endif // RECORD_PLAYBACK_HPP
