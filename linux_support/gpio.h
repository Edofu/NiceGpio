#ifndef LINUX_SUPPORT_GPIO_H
#define LINUX_SUPPORT_GPIO_H

#include <string>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <sys/poll.h>
#include <optional>

namespace linux_support {
namespace gpio {

enum class TFlag : uint8_t { 
    input = GPIO_V2_LINE_FLAG_INPUT,
    output = GPIO_V2_LINE_FLAG_OUTPUT,
    activeLow = GPIO_V2_LINE_FLAG_ACTIVE_LOW
};

enum class TEdge : uint8_t {
    risingEdge = GPIO_V2_LINE_FLAG_EDGE_RISING,
    fallingEdge = GPIO_V2_LINE_FLAG_EDGE_FALLING,
    bothEdge = risingEdge | fallingEdge
};

enum class TPinState : bool {
    low = false,
    high = true
};

enum class GpioType {
    input,
    output,
    irq
};

template<GpioType Type>
class Gpio;

template<>
class Gpio<GpioType::output> {
public:
    Gpio(const std::string& dev, const uint8_t pinOffset, const TPinState defaultState, const std::string& label);
    void setPin(const TPinState state);
    void togglePin();

private:
    TPinState mPinState;
    int mFileDescriptor{-1};
};

template<>
class Gpio<GpioType::input> {
public:
    Gpio(const std::string& dev, const uint8_t pinOffset, const std::string& label);
    std::optional<TPinState> readPin();

private:
    int mFileDescriptor{-1};
};



} // gpio
} // linux_support



#endif
