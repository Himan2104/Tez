#pragma once

#include "Tez/Core/Types.hxx"
#include <functional>
#include <map>
#include <memory>
#include <type_traits>

namespace Tez
{
///
/// \brief Base Class for all Events
///
class IEvent
{
};

///
/// \brief A token that will unsubsribe on destruction
///
class EventSubscriptionToken
{
public:
    EventSubscriptionToken()                                       = delete;
    EventSubscriptionToken(const EventSubscriptionToken&)          = delete;
    EventSubscriptionToken operator=(const EventSubscriptionToken) = delete;
    ~EventSubscriptionToken();

    bool IsValid() const;

private:
    EventSubscriptionToken(UInt32 index);

private:
    constexpr static UInt32 _invalidIndex = static_cast<UInt32>(-1);

    UInt32 _index{_invalidIndex};
};

template <typename T>
concept EventType = std::is_base_of_v<IEvent, T> && std::is_aggregate_v<T>;

class EventSystem
{
public:
    template <EventType T>
    using Callback = std::function<void(std::shared_ptr<T>)>;

    static EventSystem& GetInstance();

    template <EventType T>
    [[nodiscard]] EventSubscriptionToken Subscribe(Callback<T>);

    template <EventType T>
    void Unsubscribe(EventSubscriptionToken token);

    void Unsubscribe(EventSubscriptionToken token);

    template <EventType T, typename... Args>
    void Invoke(Args&&... args);

private:
    EventSystem() = default;

private:
    using SubscriptionContainer = std::vector<std::function<void(std::shared_ptr<IEvent>)>>;

    std::map<UInt64, SubscriptionContainer> _subscriberTable;
};

template <EventType T, typename... Args>
void EventSystem::Invoke(Args&&... args)
{
}

} // namespace Tez
