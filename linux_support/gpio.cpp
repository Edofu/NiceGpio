#include "gpio.h"
#include <cstring>

namespace linux_support {
namespace gpio {

namespace detail {
    void setBit(__u64& bits, int pin) {
        bits |= _BITULL(pin);
    }

    void clearBit(__u64& bits, int pin) {
        bits &= ~_BITULL(pin);
    }

    void assignBit(__u64& bits, int pin, TPinState value) {
        (value == TPinState::high) ? setBit(bits, pin) : clearBit(bits, pin);
    }
} // detail

Gpio<GpioType::output>::Gpio(const std::string& dev, uint8_t pinOffset, const TPinState defaultState, const std::string& label) : 
    mPinState{defaultState}
{
    const int fd = open(dev.c_str(), O_RDONLY);

    struct gpio_v2_line_request request;
    std::memset(&request, 0, sizeof(request));

    request.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
    request.num_lines = 1U;
    request.offsets[0] = pinOffset;

    request.config.num_attrs = 1U;
    request.config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES;
    detail::setBit(request.config.attrs[0].mask, 0);
    detail::assignBit(request.config.attrs[0].attr.values, 0, defaultState);

    if (label.length() < GPIO_MAX_NAME_SIZE) {
        std::memcpy(&request.consumer, label.c_str(), label.length());
    }

    const auto ret = ioctl(fd, GPIO_V2_GET_LINE_IOCTL, &request);
    close(fd);
    if (ret < 0) {
        //TODO Error handling
        return;
    }

    mFileDescriptor = request.fd;
}

void Gpio<GpioType::output>::setPin(const TPinState state) {
    struct gpio_v2_line_values value;
    std::memset(&value, 0, sizeof(value));

    detail::setBit(value.mask, 0);
    detail::assignBit(value.bits, 0, state);

    int ret = ioctl(mFileDescriptor, GPIO_V2_LINE_SET_VALUES_IOCTL, &value);
    if (ret < 0) {
        //TODO Error handling
    }

    mPinState = state;
}

void Gpio<GpioType::output>::togglePin() {
    if (mPinState == TPinState::high) {
        setPin(TPinState::low);
    }
    else {
        setPin(TPinState::high);
    }
}

Gpio<GpioType::input>::Gpio(const std::string& dev, uint8_t pinOffset, const std::string& label) {
    const auto fd = open(dev.c_str(), O_RDONLY);

    struct gpio_v2_line_request request;
    std::memset(&request, 0, sizeof(request));

    request.config.flags = GPIO_V2_LINE_FLAG_INPUT;
    request.num_lines = 1U;
    request.offsets[0] = pinOffset;

    if (label.length() < GPIO_MAX_NAME_SIZE) {
        std::memcpy(&request.consumer, label.c_str(), label.length());
    }

    const auto ret = ioctl(fd, GPIO_V2_GET_LINE_IOCTL, &request);
    close(fd);

    if (ret < 0) {
        //TODO Error handling
        return;
    }

    mFileDescriptor = request.fd;
}

std::optional<TPinState> Gpio<GpioType::input>::readPin() {
    struct gpio_v2_line_values values;
    std::memset(&values, 0, sizeof(values));

    detail::setBit(values.mask, 0);
    const auto ret = ioctl(mFileDescriptor, GPIO_V2_LINE_GET_VALUES_IOCTL, &values);
    if (ret < 0) {
        return {};
    }

    return (values.bits > 0) ? TPinState::high : TPinState::low;
}







} // linux_support
} // gpio