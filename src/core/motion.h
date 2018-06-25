
#pragma once


#include "streaming.h"

namespace liborbbec
{
    class motion_stream_profile_interface : public virtual stream_profile_interface
    {
    public:
        virtual orbbec_motion_device_intrinsic get_intrinsics() const = 0;
        virtual void set_intrinsics(std::function<orbbec_motion_device_intrinsic()> calc) = 0;
    };

    MAP_EXTENSION(ORBBEC_EXTENSION_MOTION_PROFILE, liborbbec::motion_stream_profile_interface);

    class pose_stream_profile_interface : public virtual stream_profile_interface
    {
        //Empty for now
    };

    MAP_EXTENSION(ORBBEC_EXTENSION_POSE_PROFILE, liborbbec::pose_stream_profile_interface);

    class tm2_extensions
    {
    public:
        //Empty for now
        virtual void enable_loopback(const std::string& input) = 0;
        virtual void disable_loopback() = 0;
        virtual bool is_enabled() const = 0;
        virtual void connect_controller(const std::array<uint8_t, 6>& mac_address) = 0;
        virtual void disconnect_controller(int id) = 0;
        virtual ~tm2_extensions() = default;
    };
    MAP_EXTENSION(ORBBEC_EXTENSION_TM2, liborbbec::tm2_extensions);
}
