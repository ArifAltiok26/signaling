#pragma once
#include "connection.h"
#include "connector_fwd.h"
namespace ygv::signals
{

template <typename Retval, typename... Args> struct Connector<Retval(Args...)>
{
    using SlotType = std::function<Retval(Args...)>;

    virtual ~Connector() = default;
    virtual ConnectionPtr connect(SlotType &&slot) noexcept = 0;
    virtual void disconnect(ConnectionPtr) noexcept = 0;
};

} // namespace ygv::signals