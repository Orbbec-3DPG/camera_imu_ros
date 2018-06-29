#pragma once

#include "types.h"
#include "core/streaming.h"

#include <atomic>
#include <array>
#include <math.h>

namespace liborbbec
{
    class archive_interface;
    class md_attribute_parser_base;
    class frame;
}

struct frame_additional_data
{
    orbbec_time_t           timestamp = 0;
    unsigned long long      frame_number = 0;
    orbbec_timestamp_domain timestamp_domain = ORBBEC_TIMESTAMP_DOMAIN_HARDWARE_CLOCK;
    orbbec_time_t           system_time = 0;
    orbbec_time_t           frame_callback_started = 0;
    uint32_t                metadata_size = 0;
    bool                    fisheye_ae_mode = false;
    std::array<uint8_t, MAX_META_DATA_SIZE> metadata_blob;
    orbbec_time_t           backend_timestamp = 0;
    orbbec_time_t           last_timestamp = 0;
    unsigned long long      lst_frame_number = 0;

    frame_additional_data() {}

    frame_additional_data(double                in_timestamp,
                          unsigned long long    in_frame_number,
                          double                in_system_time,
                          uint8_t               md_size,
                          const uint8_t*        md_buf,
                          double                backend_time,
                          orbbec_time_t         last_timestamp,
                          unsigned long long    last_frame_number,):
        timestamp(in_timestamp),
        frame_number(in_frame_number),
        system_time(in_system_time),
        metadata_size(md_size),
        backend_timestamp(backend_time),
        last_timestamp(last_timestamp),
        last_frame_number(last_frame_number)
    {
        if(metadata_size)
            std::copy(md_buf, md_buf + std::min(md_size, MAX_META_DATA_SIZE), metadata_blob.begin());
    }
};

namespace liborbbec
{
    typedef std::map<orbbec_frame_metadata_value, std::shared_ptr<md_attribute_parser_base>> metadata_parser_map;

    class frame : public frame_interface
    {
    public:
        std::vector<byte> data;
        frame_additional_data additional_data;
        explicit frame() : ref_count(0), _kept(false), owner(nullptr), on_release(){}
        frame(const frame& r) = delete;
        frame(frame&& r)
            : ref_count(r.ref_count.exchange(false)),
              _kept(r._kept.exchange(false)),
              owner(r.owner),
              on_release()
        {
            *this = std::move(r);
        }

        frame& operator = (const frame& r) = delete;
        frame& operator = (frame&& r)
        {
            data = move(r.data);
            owner = r.owner;
            ref_count = r.ref_count.exchange(false);
            _kept = r._kept.exchange(false);
            on_release = std::move(r.on_release);
            additional_data = std::move(r.additional_data);
            r.owner.reset();
            return *this;
        }

        virtual ~frame() {on_release.reset();}
        orbbec_metadata_type get_frame_metadata(const orbbec_frame_metadata_value &frame_metadata) const override;
        bool supports_frame_metadata(const orbbec_frame_metadata_value &frame_metadata) const override;
        const byte* get_frame_data() const override;
        orbbec_time_t get_frame_timestamp() const override;
        orbbec_timestamp_domain get_frame_timestamp_domain() const override;
        void set_timestamp(double new_ts) override {additional_data.timestamp = new_ts;}
        unsigned long long get_frame_number() const override;
        void set_timestamp_domain(orbbec_timestamp_domain timestamp_domain) override
        {
            additional_data.timestamp_domain = timestamp_domain;
        }
        orbbec_time_t get_frame_system_time() const override;

        std::shared_ptr<stream_profile_interface> get_stream() const override { return stream;}
        void set_stream(std::shared_ptr<stream_profile_interface> sp) override { stream = std::move(sp);}

        orbbec_time_t get_frame_callback_start_time_point() const override;
        void update_frame_callback_start_ts(orbbec_time_t ts) override;

        void acquire() override { ref_count.fetch_add(1);}
        void release() override;
        void keep()    override;

        frame_interface* publish(std::shared_ptr<archive_interface> new_owner) override;
        void attach_continuation(frame_continuation&& continuation) override {on_realease = std::move(continuation);}
        archive_interface* get_owner() const override {return owner.get();}

        std::shared_ptr<sensor_interface> get_sensor() const override;
        void set_sensor(std::shared_ptr<sensor_interface> s) override;
        void log_callback_start(orbbec_time_t timestamp) override;
        void log_callback_end(orbbec_time_t timestamp) override;

        void mark_fixed() override {_fixed = true;}
        bool is_fixed() const override {return _fixed;}

    private:
        std::atomic<int> ref_count;
        std::shared_ptr<archive_interface> owner;
        std::weak_ptr<sensor_interface> sensor;
        frame_continuation on_release;
        bool _fixed = false;
        std::atomic_bool _kept;
        std::shared_ptr<stream_profile_interface> stream;
    };

    class points : public frame
    {
    public:
        float3* get_vertices();
        void export_to_ply(const std::string& fname, const frame_holder& texture);
        size_t  get_vertex_count() const;
        float2* get_texture_coordinates();
    };

    MAP_EXTENSION(ORBBEC_EXTENSION_POINTS, liborbbec::points);

    class composite_frame : public frame
    {
    public:
        composite_frame() : frame() {}
        frame_interface* get_frame(int i) const
        {
            auto frames = get_frames();
            return frames[i];
        }

        frame_interface** get_frames() const {return (frame_interface**)data.data();}

        const frame_interface* first() const
        {
            return get_frame(0);
        }
        frame_interface* first()
        {
            return get_frame(0);
        }

        void keep() override
        {
            auto frames = get_frames();
            for (int i = 0; i < get_embedded_frames_count(); i++)
                if (frame[i]) frames[i]->keep();
            frame::keep();
        }

        size_t get_embedded_frames_count() const {return data.size() / sizeof(orbbec_frame*);}

        orbbec_metadata_type get_frame_metadata(const orbbec_frame_metadata_value& frame_metadata) const override
        {
            return first()->get_frame_metadata(frame_metadata);
        }
        bool supports_frame_metadata(const orbbec_frame_metadata_value& frame_metadata) const override
        {
            return first()->supports_frame_metadata(frame_metadata);
        }
        const byte* get_frame_data() const override
        {
            return first()->get_frame_data();
        }
        orbbec_time_t get_frame_timestamp() const override
        {
            return first()->get_frame_timestamp();
        }
        orbbec_timestamp_domain get_frame_timestamp_domain() const override
        {
            return first()->get_frame_timestamp_domain();
        }
        unsigned long long get_frame_number() const override
        {
            if (first())
                return first()->get_frame_number();
            else
                return frame::get_frame_number();
        }
        orbbec_time_t get_frame_system_time() const override
        {
            return first()->get_frame_system_time();
        }
        std::shared_ptr<sensor_interface> get_sensor() const override
        {
            return first()->get_sensor();
        }
    };
    MAP_EXTENSION(ORBBEC_EXTENSION_COMPOSITE_FRAME, liborbbec::composite_frame);

    class video_frame: public frame
    {
    public:
        video_frame():
            frame(),
            _width(0),
            _height(0),
            _bpp(0),
            _stride(0)
        {}

        int get_width() const { return _width; }
        int get_height() const { return _height; }
        int get_stride() const { return _stride; }
        int get_bpp() const { return _bpp; }

        void assign(int width, int height, int bpp, int stride)
        {
            _width = width;
            _height = height;
            _bpp = bpp;
            _stride = stride;
        }
    private:
        int _width, _height, _bpp, _stride;
    };
    MAP_EXTENSION(ORBBEC_EXTENSION_VIDEO_FRAME, liborbbec::video_frame);

    class depth_frame: public video_frame
    {
    public:
        depth_frame(): video_frame(), _depth_units()
        {}
        frame_interface* publish(std::shared_ptr<archive_interface> new_owner) override
        {
            _depth_units = optional_value<float>();
            return video_frame::publish(new_owner);
        }

        void keep() override
        {
            if(_original) _original->keep();
            video_frame::keep();
        }

        float get_distance(int x, int y) const
        {
            if(_original && get_stream()->get_format() != ORBBEC_FORMAT_Z16)
                return ((depth_frame*)_original.frame)->get_distance(x,y);

            uint64_t pixel = 0;
            switch (get_bpp()/8) //bits for each pixel
            {
            case 1: pixel = get_frame_data()[y*get_width() + x];                                    break;
            case 2: pixel = reinterpret_cast<const uint16_t*>(get_frame_data())[y*get_width() + x]; break;
            case 4: pixel = reinterpret_cast<const uint32_t*>(get_frame_data())[y*get_width() + x]; break;
            case 8: pixel = reinterpret_cast<const uint64_t*>(get_frame_data())[y*get_width() + x]; break;
            default: throw std::runtime_error(to_string() << "Unrecognized depth format " << int(get_bpp() / 8) << " bytes per pixel");
            }
            return pixel * get_units();
        }

        float get_units() const
        {
            if (!_depth_units)
                _depth_units = query_units(get_sensor());
            return _depth_units.value();
        }

        const frame_interface* get_original_depth() const
        {
            auto res = _original.frame;
            auto df = dynamic_cast<depth_frame*>(res);
            if(df)
            {
                auto prev = df->get_original_depth();
                if(prev) return prev;
            }
            return res;
        }

        void set_original(frame_holder h)
        {
            _original = std::move(h);
            attach_continuation(frame_continuation(
                                    [this](){if(_original) _original = {};},get_frame_data()));
        }

    protected:
        static float query_units(const std::shared_ptr<sensor_interface>& sensor)
        {
            if (sensor != nullptr)
            {
                try
                {
                    auto depth_sensor = As<librealsense::depth_sensor>(sensor);
                    if(depth_sensor != nullptr)
                    {
                        return depth_sensor->get_depth_scale();
                    }
                    else
                    {
                        //For playback sensors
                        auto extendable = As<librealsense::extendable_interface>(sensor);
                        if (extendable && extendable->extend_to(TypeToExtension<librealsense::depth_sensor>::value, (void**)(&depth_sensor)))
                        {
                            return depth_sensor->get_depth_scale();
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR("Failed to query depth units from sensor. " << e.what());
                }
                catch (...)
                {
                    LOG_ERROR("Failed to query depth units from sensor");
                }
            }
            else
            {
                LOG_WARNING("sensor was nullptr");
            }

            return 0;
        }

        frame_holder _original;
        mutable optional_value<float> _depth_units;
    };

    MAP_EXTENSION(ORBBEC_EXTENSION_DEPTH_FRAME, liborbbec::depth_frame);

    class disparity_frame: public depth_frame
    {
    public:
        disparity_frame():depth_frame()
        {}

        float get_stereo_baseline(void) const
        {return query_stereo_baseline(this->get_sensor());}

    protected:

        static float query_stereo_baseline(const std::shared_ptr<sensor_interface>& sensor)
        {
            if (sensor != nullptr)
            {
                try
                {
                    auto stereo_sensor = As<liborbbec::depth_stereo_sensor>(sensor);
                    if (stereo_sensor != nullptr)
                    {
                        return stereo_sensor->get_stereo_baseline_mm();
                    }
                    else
                    {
                        //For playback sensors
                        auto extendable = As<liborbbec::extendable_interface>(sensor);
                        if (extendable && extendable->extend_to(TypeToExtension<liborbbec::depth_stereo_sensor>::value, (void**)(&stereo_sensor)))
                        {
                            return stereo_sensor->get_stereo_baseline_mm();
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR("Failed to query stereo baseline from sensor. " << e.what());
                }
                catch (...)
                {
                    LOG_ERROR("Failed to query stereo baseline from sensor");
                }
            }
            else
            {
                LOG_WARNING("sensor was nullptr");
            }

            return 0;
        }
    };
    MAP_EXTENSION(ORBBEC_EXTENSION_DISPARITY_FRAME, liborbbec::disparity_frame);

    class motion_frame : public frame
    {
    public:
        motion_frame() : frame()
        {}
    };

    MAP_EXTENSION(ORBBEC_EXTENSION_MOTION_FRAME, liborbbec::motion_frame);

    class pose_frame: public frame
    {
    public:
        struct pose_info
        {
            float3      translation;
            float3      velocity;
            float3      acceleration;
            float3      rotation;
            float3      angular_velocity;
            float3      angular_acceleration;
            uint32_t    tracker_confidence;
            uint32_t    mapper_confidence;
        };

        pose_frame() : frame() {}

        float3   get_translation()          const { return reinterpret_cast<const pose_info*>(data.data())->translation; }
        float3   get_velocity()             const { return reinterpret_cast<const pose_info*>(data.data())->velocity; }
        float3   get_acceleration()         const { return reinterpret_cast<const pose_info*>(data.data())->acceleration; }
        float4   get_rotation()             const { return reinterpret_cast<const pose_info*>(data.data())->rotation; }
        float3   get_angular_velocity()     const { return reinterpret_cast<const pose_info*>(data.data())->angular_velocity; }
        float3   get_angular_acceleration() const { return reinterpret_cast<const pose_info*>(data.data())->angular_acceleration; }
        uint32_t get_tracker_confidence()   const { return reinterpret_cast<const pose_info*>(data.data())->tracker_confidence; }
        uint32_t get_mapper_confidence()    const { return reinterpret_cast<const pose_info*>(data.data())->mapper_confidence; }
    };

    MAP_EXTENSION(ORBBEC_EXTENSION_POSE_FRAME, liborbbec::pose_frame);

    class archive_interface : public sensor_part
    {
    public:
        virtual callback_invocation_holder begin_callback() = 0;

        virtual frame_interface* alloc_and_track(const size_t size, const frame_additional_data& additional_data, bool requires_memory) = 0;

        virtual std::shared_ptr<metadata_parser_map> get_md_parsers() const = 0;

        virtual void flush() = 0;

        virtual frame_interface* publish_frame(frame_interface* frame) = 0;
        virtual void unpublish_frame(frame_interface* frame) = 0;
        virtual void keep_frame(frame_interface* frame) = 0;

        virtual ~archive_interface() = default;

    };

    std::shared_ptr<archive_interface> make_archive(orbbec_extension type,
                                                    std::atomic<uint32_t>* in_max_frame_queue_size,
                                                    std::shared_ptr<platform::time_service> ts,
                                                    std::shared_ptr<metadata_parser_map> parsers);
}
