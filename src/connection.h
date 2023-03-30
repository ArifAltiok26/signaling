#pragma once
#include "common.h"
namespace ygv::signals
{

struct Connection
{
    virtual void disconnect() noexcept = 0;
};

using ConnectionPtr = std::shared_ptr<Connection>;
} // namespace ygv::signals