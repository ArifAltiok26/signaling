#pragma once
#include <mutex>
namespace ygv::signals
{

using LockGuard = std::lock_guard<std::mutex>;

template <typename Signature, typename Lock = LockGuard> class Signal;
} // namespace ygv::signals