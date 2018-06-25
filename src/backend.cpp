#include "backend.h"

void liborbbec::platform::control_range::populate_raw_data(std::vector<uint8_t>& vec, int32_t value)
{
    vec.resize(sizeof(value));
    auto data = reinterpret_cast<const uint8_t*>(&value);
    std::copy(data, data + sizeof(value),vec.data());
}

double liborbbec::monotomic_to_realtime(double monotonic)
{
    auto realtime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto time_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    return monotonic + (realtime - time_since_epoch);
}
