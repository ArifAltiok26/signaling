#pragma once
#include "connection.h"
#include "connector_fwd.h"
#include "signal_fwd.h"
namespace ygv::signals
{

template <typename Retval, typename... Args, typename Callable>
ConnectionPtr operator+=(Connector<Retval(Args...)> &connector, Callable &&callbable)
{
    return connector.connect(std::move(callbable));
}

template <typename Instance, typename Class, typename Retval, typename... Args>
std::function<Retval(Args...)> make_slot(Instance &instance, Retval (Class::*memfb)(Args...))
{
    return [&instance, memfb](Args... args) { return (instance.*memfb)(args...); };
}

template <typename Instance, typename Class, typename Retval, typename... Args>
std::function<Retval(Args...)> make_slot(Instance *instance, Retval (Class::*memfb)(Args...))
{

    return [instance, memfb](Args... args) { return (instance->*memfb)(args...); };
}

template <typename Retval, typename... Args, typename Lock>
std::function<Retval(Args...)> make_slot(Signal<Retval(Args...), Lock> &signal)
{
    return [&signal](Args... args) { return signal.notify(args...); };
}
} // namespace ygv::signals