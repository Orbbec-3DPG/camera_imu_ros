
#pragma once

#include <map>
#include "../include/h/orbbec_option.h"
#include "extension.h"
#include "types.h"

namespace liborbbec
{
    struct option_range
    {
        float min;
        float max;
        float step;
        float def;
    };

    class option : public recordable<option>
    {
    public:
        virtual void set(float value) = 0;
        virtual float query() const = 0;
        virtual option_range get_range() const = 0;
        virtual bool is_enabled() const = 0;
        virtual bool is_read_only() const { return false; }
        virtual const char* get_description() const = 0;
        virtual const char* get_value_description(float) const { return nullptr; }
        virtual void create_snapshot(std::shared_ptr<option>& snapshot) const override;

        virtual ~option() = default;
    };


    class options_interface : public recordable<options_interface>
    {
    public:
        virtual option& get_option(orbbec_option id) = 0;
        virtual const option& get_option(orbbec_option id) const = 0;
        virtual bool supports_option(orbbec_option id) const = 0;

        virtual ~options_interface() = default;
    };

    MAP_EXTENSION(ORBBEC_EXTENSION_OPTIONS, liborbbec::options_interface);

    class options_container : public virtual options_interface, public extension_snapshot
    {
    public:
        bool supports_option(orbbec_option id) const override
        {
            auto it = _options.find(id);
            if (it == _options.end()) return false;
            return it->second->is_enabled();
        }

        option& get_option(orbbec_option id) override
        {
            return const_cast<option&>(const_cast<const options_container*>(this)->get_option(id));
        }

        const option& get_option(orbbec_option id) const override
        {
            auto it = _options.find(id);
            if (it == _options.end())
            {
                throw invalid_value_exception(to_string()
                    << "Device does not support option "
                    << orbbec_option_to_string(id) << "!");
            }
            return *it->second;
        }

        void register_option(orbbec_option id, std::shared_ptr<option> option)
        {
            _options[id] = option;
            _recording_function(*this);
        }

        void unregister_option(orbbec_option id)
        {
            _options.erase(id);
        }

        void create_snapshot(std::shared_ptr<options_interface>& snapshot) const override
        {
            snapshot = std::make_shared<options_container>(*this);
        }

        void enable_recording(std::function<void(const options_interface&)> record_action) override
        {
            _recording_function = record_action;
        }

        void update(std::shared_ptr<extension_snapshot> ext) override
        {
            auto ctr = As<options_container>(ext);
            if (!ctr) return;
            for(auto&& opt : ctr->_options)
            {
                _options[opt.first] = opt.second;
            }
        }

    private:
        std::map<orbbec_option, std::shared_ptr<option>> _options;
        std::function<void(const options_interface&)> _recording_function = [](const options_interface&) {};
    };
}
