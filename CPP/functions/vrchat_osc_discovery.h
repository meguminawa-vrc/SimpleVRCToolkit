#ifndef VRCHAT_OSC_DISCOVERY_H
#define VRCHAT_OSC_DISCOVERY_H

#include <string>
#include <utility>

namespace vrchat_osc {

    // 封装外部可执行文件调用
    std::pair<std::string, int> discover_ip(int timeout_seconds = 5);

} // namespace vrchat_osc

#endif // VRCHAT_OSC_DISCOVERY_H