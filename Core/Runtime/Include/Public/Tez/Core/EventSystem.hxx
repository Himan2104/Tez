#pragma once

#include "Tez/Core/Array.hxx"
#include "Tez/Core/Types.hxx"
#include "Tez/Core/UIDProvider.hxx"
#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>

namespace Tez
{
/// \brief Base for all Events
struct IEvent
{
};

class EventSubscriptionToken
{
public:
    EventSubscriptionToken() = delete;
    ~EventSubscriptionToken();

private:
    EventSubscriptionToken(UInt32 index)
        : _index(index)
    {
    }

private:
    UInt32 _index;
};

template <typename T>
concept EventType = std::is_base_of_v<IEvent, T> && std::is_aggregate_v<T>;

namespace __detail
{
    class ISubscriberList
    {
    };

    template <typename T>
    class SubscriberList : public ISubscriberList
    {
    public:
        DynamicArray<std::function<void(const T&)>> subscribers;
    };
} // namespace __detail

class EventSystem
{
public:
    static EventSystem& GetInstance();

    template <EventType T>
    [[nodiscard]] EventSubscriptionToken Subscribe(std::function<void(const T&)> callback);

    template <EventType T, typename S>
    [[nodiscard]] EventSubscriptionToken Subscribe(S* instance, void (S::*memberFunc)(const T&));

    template <EventType T, typename... Args>
    void Invoke(Args&&... args);

    void UnSubscribe(EventSubscriptionToken token);

private:
    EventSystem() = default;

    std::unordered_map<UInt64, __detail::ISubscriberList*> _subscriberMappings;
    UIDProvider<UInt64> _uidProvider;
};

template <EventType T, typename... Args>
void EventSystem::Invoke(Args&&... args)
{
}
} // namespace Tez
