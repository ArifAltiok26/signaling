#pragma once
#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>

namespace ygv::signaling
{

namespace utils
{
using LockGuard = std::lock_guard<std::mutex>;
}

template <typename Signature, typename Lock = utils::LockGuard> class Signal;

template <typename Retval, typename... Args, typename Lock> class Signal<Retval(Args...), Lock>
{
  public:
    using Signature = Retval(Args...);
    using SlotType = std::function<Signature>;

    class Connection final
    {
        friend class Signal;

      public:
        Connection() : signal(nullptr)
        {
        }

        Connection(Connection &&other) = default;
        Connection &operator=(Connection &&other) = default;

        void disconnect() noexcept
        {
            if (signal)
            {
                signal->disconnect(this);
            }
        }

      private:
        Signal *signal;
    };

    using ConnectionPtr = std::shared_ptr<Connection>;

  private:
    struct Entity final
    {
        ConnectionPtr conn;
        SlotType slot;
    };

  public:
    Signal() noexcept = default;

    Signal(Signal &&other) noexcept
    {
        Lock lock{other.m_mtx};
        m_entities = std::move(other.m_entities);
        for (auto &entity : m_entities)
        {
            entity.conn->signal = this;
        }
    }

    ~Signal() noexcept
    {
        Lock lock{m_mtx};
        for (auto &entity : m_entities)
        {
            entity.conn->signal = nullptr;
        }
    }

    Signal &operator=(Signal &&other) noexcept
    {

        Lock lock_this{m_mtx};
        for (auto &entity : m_entities)
            entity.conn->signal = nullptr;
        Lock lock_other{other.m_mtx};
        m_entities = std::move(other.m_entities);
        for (auto &entity : m_entities)
            entity.conn->signal = this;
        return *this;
    }

    ConnectionPtr connect(SlotType &&slot) noexcept
    {
        auto conn = std::make_shared<Connection>();
        conn->signal = this;
        Lock lock{m_mtx};
        m_entities.push_back(Entity{conn, std::move(slot)});
        return conn;
    }

    ConnectionPtr operator+=(SlotType &&slot) noexcept
    {
        return connect(std::move(slot));
    }

    void disconnect(ConnectionPtr conn) noexcept
    {
        if (conn)
        {
            disconnect(conn.get());
        }
    }

    size_t size() const noexcept
    {
        Lock lock{m_mtx};
        return m_entities.size();
    }

    bool empty() const noexcept
    {
        return 0 == size();
    }

    void notify(Args... args) const
    {
        if (!m_isBlock)
        {
            Lock lock{m_mtx};
            for (const auto &entity : m_entities)
            {
                entity.slot(args...);
            }
        }
    }

    void operator()(Args... args) const
    {
        notify(std::forward<Args>(args)...);
    }

    bool setBlock(bool value) noexcept
    {
        return m_isBlock.exchange(value);
    }

  private:
    void disconnect(Connection *conn) noexcept
    {
        if (this == conn->signal)
        {
            Lock lock{m_mtx};
            auto iter = std::find_if(std::begin(m_entities), std::end(m_entities),
                                     [conn](auto &item) { return item.conn.get() == conn; });

            if (iter == m_entities.end())
                return;
            conn->signal = nullptr;
            m_entities.erase(iter);
        }
    }

  private:
    std::deque<Entity> m_entities;
    using MutexType = typename Lock::mutex_type;
    mutable MutexType m_mtx;
    std::atomic<bool> m_isBlock = false;
};

namespace utils
{

template <typename Retval, typename... Args, typename Lock>
std::function<Retval(Args...)> make_slot(Signal<Retval(Args...), Lock> &signal)
{
    return [&signal](Args... args) { return signal.notify(args...); };
}

template <typename Instance, typename Class, typename Retval, typename... Args>
std::function<Retval(Args...)> make_slot(Instance *instance, Retval (Class::*memfb)(Args...))
{
    return [instance, memfb](Args... args) { return (instance->*memfb)(args...); };
}

template <typename Instance, typename Class, typename Retval, typename... Args>
std::function<Retval(Args...)> make_slot(Instance *instance, Retval (Class::*memfb)(Args...) const)
{
    return [instance, memfb](Args... args) { return (instance->*memfb)(args...); };
}

template <typename Instance, typename Class, typename Retval, typename... Args>
std::function<Retval(Args...)> make_slot(Instance &instance, Retval (Class::*memfb)(Args...))
{
    return [&instance, memfb](Args... args) { return (instance.*memfb)(args...); };
}

template <typename Instance, typename Class, typename Retval, typename... Args>
std::function<Retval(Args...)> make_slot(Instance &instance, Retval (Class::*memfb)(Args...) const)
{
    return [&instance, memfb](Args... args) { return (instance.*memfb)(args...); };
}
} // namespace utils

} // namespace ygv::signaling