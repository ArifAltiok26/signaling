#pragma once
#include "connector.h"
#include "signal_fwd.h"
#include <deque>

namespace ygv::signals
{

template <typename Retval, typename... Args, typename Lock>
class Signal<Retval(Args...), Lock> final : public Connector<Retval(Args...)>
{

  public:
    using ConnectorType = Connector<Retval(Args...)>;
    using SlotType = typename ConnectorType::SlotType;

  private:
    using SignalType = Signal<Retval(Args...), Lock>;

    struct ConnectionImpl final : Connection, std::enable_shared_from_this<ConnectionImpl>
    {
        ConnectionImpl(SignalType *signal) : signal(signal)
        {
        }

        void disconnect() noexcept override final
        {
            if (signal)
            {
                signal->disconnect(this->shared_from_this());
            }
        }

        SignalType *signal;
    };

    struct Entity final
    {
        std::shared_ptr<ConnectionImpl> conn;
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

    ~Signal()
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
        Lock lock_other{other.m_mtx};
        for (auto &entity : m_entities)
            entity.conn->signal = nullptr;
        m_entities = std::move(other.m_entities);
        for (auto &entity : m_entities)
            entity.conn->signal = this;
        return *this;
    }

    ConnectionPtr connect(SlotType &&slot) noexcept override final
    {
        auto conn = std::make_shared<ConnectionImpl>(this);
        Lock lock{m_mtx};
        m_entities.push_back(Entity{conn, std::move(slot)});
        return conn;
    }

    void disconnect(ConnectionPtr conn) noexcept override final
    {
        auto conn_impl = std::dynamic_pointer_cast<ConnectionImpl>(conn);
        if (conn_impl && this == conn_impl->signal)
        {
            Lock lock{m_mtx};
            auto iter = std::find_if(std::begin(m_entities), std::end(m_entities),
                                     [conn_impl](auto &item) { return item.conn == conn_impl; });

            if (iter == m_entities.end())
                return;
            conn_impl->signal = nullptr;
            m_entities.erase(iter);
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

    void notify(Args... args)
    {
        Lock lock{m_mtx};
        for (auto &entity : m_entities)
        {
            entity.slot(args...);
        }
    }

    void operator()(Args... args)
    {
        notify(std::forward<Args>(args)...);
    }

  private:
    std::deque<Entity> m_entities;
    using MutexType = typename Lock::mutex_type;
    mutable MutexType m_mtx;
};

} // namespace ygv::signals