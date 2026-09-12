#pragma once

#include <Tez/Core/Types.hxx>
#include <concepts>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Tez
{

///
/// @brief Base for all events.
/// @note Event payloads are constrained to be trivial aggregate types deriving from IEvent.
///
struct IEvent
{
};

///
/// @brief Concept constraining a type to be a valid event payload.
/// @tparam T A type deriving from IEvent and forming a valid aggregate.
///
template <typename T>
concept EventType = std::is_base_of_v<IEvent, T> && std::is_aggregate_v<T>;

class ISubscriberList;

namespace __detail
{
    template <EventType T>
    class SubscriberList;
} // namespace __detail

///
/// @brief Opaque, move-only RAII token identifying a single live event subscription.
///
/// @note Thread-safety contract:
///       - Destroying (or releasing) the token guarantees that no dispatch *started after*
///         the destruction will invoke the callback. Dispatches already in flight on other
///         threads complete first; the callback is never called again afterwards.
///       - Tokens must be released before program termination (the EventSystem outlives all
///         tokens in normal application shutdown).
///
class EventSubscriptionToken
{
public:
    EventSubscriptionToken() = delete;

    EventSubscriptionToken(const EventSubscriptionToken&)            = delete;
    EventSubscriptionToken& operator=(const EventSubscriptionToken&) = delete;

    ///
    /// @brief Transfer constructor. The source token becomes inert.
    ///
    EventSubscriptionToken(EventSubscriptionToken&& other) noexcept
        : _list(other._list)
        , _index(other._index)
        , _generation(other._generation)
    {
        other._list = nullptr;
    }

    ///
    /// @brief Transfer assignment. The previously owned subscription is unsubscribed.
    ///
    EventSubscriptionToken& operator=(EventSubscriptionToken&& other) noexcept;

    ~EventSubscriptionToken();

    ///
    /// @brief Unsubscribes immediately; the token becomes inert and may be destroyed safely.
    ///
    void Release() noexcept;

    ///
    /// @brief Whether this token still owns a live subscription.
    ///
    [[nodiscard]] constexpr bool IsLive() const noexcept { return _list != nullptr; }

private:
    EventSubscriptionToken(ISubscriberList* list, UInt32 index, UInt32 generation)
        : _list(list)
        , _index(index)
        , _generation(generation)
    {
    }

    ISubscriberList* _list       = nullptr;
    UInt32           _index      = 0;
    UInt32           _generation = 0;

    template <EventType T>
    friend class __detail::SubscriberList;
};

///
/// @brief Type-erased base for the per-event-type subscriber table.
///
/// @note All operations are thread-safe. A dispatch pass holds the table's internal lock for
///       its full duration, so subscribers can safely Subscribe/UnSubscribe from inside a
///       callback (re-entrant); external threads block until the pass completes.
///
class ISubscriberList
{
public:
    virtual ~ISubscriberList() = default;

    ///
    /// @brief Delivers the event to every live subscription.
    /// @param event The payload, type-erased (actually const T* for the list's T).
    ///
    virtual void Invoke(const void* event) = 0;

    ///
    /// @brief Registers a type-erased callback and returns its owning token.
    /// @param self Kind-specific payload (owned by the table; may be nullptr).
    /// @param invoke Dispatch thunk; called as invoke(self, event) with no locks held by the caller.
    /// @param destroy Releases the payload state; called exactly once per subscription. May be nullptr.
    ///
    virtual EventSubscriptionToken Subscribe(void* self, void (*invoke)(void* self, const void* event), void (*destroy)(void* self)) = 0;

    ///
    /// @brief Removes the subscription whose entry carries the given generation. The index is
    ///        a fast-path hint; the entry is located by its stable generation, so tokens keep
    ///        working after entries are moved within the table. Idempotent: stale or
    ///        already-removed tokens are silently ignored.
    ///
    virtual void UnSubscribe(UInt32 index, UInt32 generation) = 0;
};

EventSubscriptionToken& EventSubscriptionToken::operator=(EventSubscriptionToken&& other) noexcept
{
    if (_list != nullptr) _list->UnSubscribe(_index, _generation);
    _list       = other._list;
    _index      = other._index;
    _generation = other._generation;
    other._list = nullptr;
    return *this;
}

EventSubscriptionToken::~EventSubscriptionToken()
{
    if (_list != nullptr) _list->UnSubscribe(_index, _generation);
}

void EventSubscriptionToken::Release() noexcept
{
    if (_list != nullptr) _list->UnSubscribe(_index, _generation);
    _list = nullptr;
}

namespace __detail
{
    ///
    /// @brief One dispatch slot: a type-erased callback plus bookkeeping.
    /// @note generation is a unique, stable identifier assigned at subscribe time. It never
    ///        changes for the lifetime of the entry, so tokens keep identifying their
    ///        subscription even after the entry moves within the table.
    ///
    struct CallbackEntry
    {
        void*        self;
        void (*invoke)(void* self, const void* event);
        void (*destroy)(void* self);
        UInt32       generation;
        bool         dead;
    };

    ///
    /// @brief Dispatch thunk for a plain free function. The function pointer itself is stored
    ///        as the entry's self pointer, so no payload allocation is needed.
    ///
    template <typename T>
    void FreeFnInvoke(void* self, const void* event)
    {
        using Fn = void (*)(const T&);
        reinterpret_cast<Fn>(self)(*static_cast<const T*>(event));
    }

    ///
    /// @brief Payload for member-function subscriptions. Owner is one of:
    ///        - S*                  (caller guarantees the owner outlives the token)
    ///        - std::weak_ptr<S>    (dispatch silently skips destroyed owners)
    ///        - std::shared_ptr<S>  (the subscription itself keeps the owner alive)
    ///
    template <typename T, typename Owner, typename S>
    struct MemberPayload
    {
        Owner                    owner;
        void (S::*memberFunc)(const T&);
    };

    ///
    /// @brief Member dispatch thunk with owner lifetime handling per Owner kind.
    ///
    template <typename T, typename Owner, typename S>
    void MemberInvoke(void* self, const void* event)
    {
        const auto* payload = static_cast<const MemberPayload<T, Owner, S>*>(self);
        const T&    evt     = *static_cast<const T*>(event);

        if constexpr (std::is_same_v<Owner, S*>)
        {
            (payload->owner->*payload->memberFunc)(evt);
        }
        else if constexpr (std::is_same_v<Owner, std::weak_ptr<S>>)
        {
            if (std::shared_ptr<S> alive = payload->owner.lock())
                ((*alive).*(payload->memberFunc))(evt);
        }
        else // std::shared_ptr<S>
        {
            ((*payload->owner).*(payload->memberFunc))(evt);
        }
    }

    ///
    /// @brief Generic-callable payload. Small callables (<= 16 bytes, e.g. a couple of
    ///        captured pointers) are stored inline; larger ones go on the heap.
    ///
    template <typename T, typename F>
    struct CallablePayload
    {
        static constexpr size_t kInlineSize  = 16;
        static constexpr size_t kInlineAlign = 16;

        union Storage
        {
            alignas(kInlineAlign) char inlineData[kInlineSize];
            void*                  heapData;
        } storage;

        bool heap = false;
    };

    ///
    /// @brief Moves a callable into a CallablePayload (inline when it fits).
    ///
    template <typename T, typename F>
    void PlaceCallable(CallablePayload<T, F>* payload, F&& value)
    {
        using P = CallablePayload<T, F>;
        if constexpr (sizeof(F) <= P::kInlineSize && alignof(F) <= P::kInlineAlign)
        {
            payload->heap            = false;
            new (payload->storage.inlineData) F(std::forward<F>(value));
        }
        else
        {
            payload->heap            = true;
            payload->storage.heapData = new F(std::forward<F>(value));
        }
    }

    ///
    /// @brief Generic-callable dispatch thunk.
    ///
    template <typename T, typename F>
    void CallableInvoke(void* self, const void* event)
    {
        auto*   payload = static_cast<CallablePayload<T, F>*>(self);
        const T& evt    = *static_cast<const T*>(event);

        F* fn = payload->heap
                    ? static_cast<F*>(payload->storage.heapData)
                    : reinterpret_cast<F*>(payload->storage.inlineData);
        (*fn)(evt);
    }

    ///
    /// @brief Destroys a CallablePayload (its callable first, then the payload itself).
    ///
    template <typename T, typename F>
    void CallableDestroy(void* self)
    {
        auto* payload = static_cast<CallablePayload<T, F>*>(self);
        if (payload->heap)
            delete static_cast<F*>(payload->storage.heapData);
        else
            reinterpret_cast<F*>(payload->storage.inlineData)->~F();
        delete payload;
    }

    ///
    /// @brief Destroys a heap-allocated MemberPayload.
    ///
    template <typename P>
    void DeletePayload(void* self)
    {
        delete static_cast<P*>(self);
    }

    ///
    /// @brief Dense, thread-safe subscriber table for one event type.
    ///
    /// @note Dispatch order is unspecified (removal is O(1) swap-pop). Within a single
    ///        dispatch pass the table is never mutated concurrently, so each pass sees a
    ///        consistent snapshot; re-entrant subscriptions made during a pass may receive
    ///        the in-flight event.
    ///
    template <EventType T>
    class SubscriberList : public ISubscriberList
    {
    public:
        void Invoke(const void* event) override
        {
            std::lock_guard lock(_mutex);
            ++_depth;

            for (UInt32 i = 0; i < static_cast<UInt32>(_entries.size()); ++i)
            {
                CallbackEntry& entry = _entries[i];
                if (!entry.dead) entry.invoke(entry.self, event);
            }

            _compactDeadEntries();
            --_depth;
        }

        EventSubscriptionToken Subscribe(void* self, void (*invoke)(void*, const void*), void (*destroy)(void*)) override
        {
            std::lock_guard lock(_mutex);
            CallbackEntry entry{self, invoke, destroy, _nextGeneration++, false};
            _entries.push_back(entry);
            return EventSubscriptionToken(this, static_cast<UInt32>(_entries.size()) - 1, entry.generation);
        }

        void UnSubscribe(UInt32 index, UInt32 generation) override
        {
            std::lock_guard lock(_mutex);

            UInt32 target = index;
            if (index >= _entries.size() || _entries[index].generation != generation)
            {
                // The entry may have moved within the table; locate it by its stable generation.
                target = static_cast<UInt32>(_entries.size());
                for (UInt32 i = 0; i < static_cast<UInt32>(_entries.size()); ++i)
                {
                    if (_entries[i].generation == generation)
                    {
                        target = i;
                        break;
                    }
                }
            }
            if (target >= _entries.size() || _entries[target].dead) return;

            _entries[target].dead = true;

            // A dispatch pass owns the lock while _depth > 0, so this point is only
            // reachable re-entrantly from that pass: defer removal + destruction to the
            // pass's compaction so the in-flight callback's payload stays alive.
            if (_depth == 0) _removeEntry(target);
        }

    private:
        ///
        /// @brief Swap-pops the entry at index: the last entry moves into its slot (keeping
        ///        its stable generation), and the removed payload is destroyed.
        ///
        void _removeEntry(UInt32 index)
        {
            CallbackEntry removed = _entries[index];
            const UInt32    last   = static_cast<UInt32>(_entries.size()) - 1;

            if (index != last) _entries[index] = _entries[last];
            _entries.pop_back();

            if (removed.destroy != nullptr) removed.destroy(removed.self);
        }

        ///
        /// @brief Forwards live entries to the front (keeping their stable generations) and
        ///        destroys all dead payloads. Called at the end of every dispatch pass.
        ///
        void _compactDeadEntries()
        {
            UInt32 write = 0;
            const  UInt32 total = static_cast<UInt32>(_entries.size());

            for (UInt32 read = 0; read < total; ++read)
            {
                if (_entries[read].dead) continue;
                if (read != write) _entries[write] = _entries[read];
                ++write;
            }

            for (UInt32 read = write; read < total; ++read)
            {
                if (_entries[read].destroy != nullptr) _entries[read].destroy(_entries[read].self);
            }

            _entries.resize(write);
        }

        std::recursive_mutex _mutex;
        std::vector<CallbackEntry> _entries;
        UInt32 _depth           = 0;
        UInt32 _nextGeneration = 1;
    };
} // namespace __detail

///
/// @brief Central, process-wide event bus.
///
/// @note Thread-safety model:
///       - Invoke may be called concurrently from any number of threads; dispatches of
///         different event types never block each other.
///       - Subscribe/UnSubscribe are safe at any time, including from inside a callback
///         of the same event type.
///       - Callbacks of the same event type run concurrently only if dispatched from
///         different threads *between* passes — within one pass they run sequentially.
///         Subscribers must therefore be thread-safe with respect to their own state.
///
class EventSystem
{
public:
    ///
    /// @brief Retrieves the process-wide singleton instance of the EventSystem.
    /// @return EventSystem& A reference to the sole EventSystem instance.
    ///
    static EventSystem& GetInstance()
    {
        static EventSystem instance;
        return instance;
    }

    ///
    /// @brief Subscribes a free function to event type T.
    /// @tparam T A type satisfying EventType.
    /// @param callback The function invoked with const T& for every dispatched event.
    /// @return EventSubscriptionToken Owning token; destruction unsubscribes.
    ///
    template <EventType T>
    [[nodiscard]] EventSubscriptionToken Subscribe(void (*callback)(const T&));

    ///
    /// @brief Subscribes a generic callable (lambda, functor, std::function) to event type T.
    /// @tparam T A type satisfying EventType.
    /// @tparam F A move-constructible, void-returning type invocable with const T&.
    /// @param callback The callable; moved into small-buffer (or heap) storage owned by the bus.
    /// @return EventSubscriptionToken Owning token; destruction unsubscribes.
    ///
    template <EventType T, typename F>
        requires std::invocable<std::remove_cv_t<std::remove_reference_t<F>>, const T&> &&
                 std::is_void_v<std::invoke_result_t<std::remove_cv_t<std::remove_reference_t<F>>, const T&>> &&
                 std::is_move_constructible_v<std::remove_cv_t<std::remove_reference_t<F>>> &&
                 (!std::is_pointer_v<std::remove_reference_t<F>>) &&
                 (!std::is_convertible_v<std::remove_cv_t<std::remove_reference_t<F>>, void (*)(const T&)>)
    [[nodiscard]] EventSubscriptionToken Subscribe(F&& callback);

    ///
    /// @brief Subscribes a member function of a raw-owned instance to event type T.
    /// @tparam T A type satisfying EventType.
    /// @tparam S The owning object type.
    /// @param instance The object whose member function is invoked. Must outlive the token.
    /// @param callback The member function receiving each dispatched event of type T.
    /// @return EventSubscriptionToken Owning token; destruction unsubscribes.
    ///
    template <EventType T, typename S>
    [[nodiscard]] EventSubscriptionToken Subscribe(S* instance, void (S::*callback)(const T&));

    ///
    /// @brief Subscribes a member function of a weakly-owned instance to event type T.
    /// @note Destruction-safe: if the owner has been destroyed, dispatches silently skip
    ///        this subscription until the token is released.
    /// @tparam T A type satisfying EventType.
    /// @tparam S The owning object type (managed by std::shared_ptr elsewhere).
    /// @param owner A weak reference to the owning object.
    /// @param callback The member function receiving each dispatched event of type T.
    /// @return EventSubscriptionToken Owning token; destruction unsubscribes.
    ///
    template <EventType T, typename S>
    [[nodiscard]] EventSubscriptionToken Subscribe(const std::weak_ptr<S>& owner, void (S::*callback)(const T&));

    ///
    /// @brief Subscribes a member function of an owned instance to event type T.
    /// @note The subscription itself holds a shared reference, so the owner stays alive
    ///        until the token is released (release then drops the reference).
    /// @tparam T A type satisfying EventType.
    /// @tparam S The owning object type.
    /// @param owner A shared reference to the owning object; copied into the subscription.
    /// @param callback The member function receiving each dispatched event of type T.
    /// @return EventSubscriptionToken Owning token; destruction unsubscribes.
    ///
    template <EventType T, typename S>
    [[nodiscard]] EventSubscriptionToken Subscribe(const std::shared_ptr<S>& owner, void (S::*callback)(const T&));

    ///
    /// @brief Dispatches an event of type T to all live subscribers.
    /// @tparam T A type satisfying EventType.
    /// @param event The payload delivered to each subscriber as const T&.
    ///
    template <EventType T>
    void Invoke(const T& event);

private:
    EventSystem() = default;

    ~EventSystem()
    {
        for (auto& pair : _lists) delete pair.second;
    }

    EventSystem(const EventSystem&)            = delete;
    EventSystem& operator=(const EventSystem&) = delete;

    template <EventType T>
    ISubscriberList* GetOrCreateList();

    template <EventType T>
    ISubscriberList* FindList() const;

    mutable std::shared_mutex                  _listsMutex;
    std::unordered_map<UInt64, ISubscriberList*> _lists;
};

template <EventType T>
EventSubscriptionToken EventSystem::Subscribe(void (*callback)(const T&))
{
    return GetOrCreateList<T>()->Subscribe(reinterpret_cast<void*>(callback), __detail::FreeFnInvoke<T>, nullptr);
}

template <EventType T, typename F>
    requires std::invocable<std::remove_cv_t<std::remove_reference_t<F>>, const T&> &&
             std::is_void_v<std::invoke_result_t<std::remove_cv_t<std::remove_reference_t<F>>, const T&>> &&
             std::is_move_constructible_v<std::remove_cv_t<std::remove_reference_t<F>>> &&
             (!std::is_pointer_v<std::remove_reference_t<F>>) &&
             (!std::is_convertible_v<std::remove_cv_t<std::remove_reference_t<F>>, void (*)(const T&)>)
EventSubscriptionToken EventSystem::Subscribe(F&& callback)
{
    using Fc      = std::remove_cv_t<std::remove_reference_t<F>>;
    using Payload = __detail::CallablePayload<T, Fc>;

    auto* payload = new Payload();
    __detail::PlaceCallable<T, Fc>(payload, std::forward<F>(callback));

    return GetOrCreateList<T>()->Subscribe(payload, __detail::CallableInvoke<T, Fc>, __detail::CallableDestroy<T, Fc>);
}

template <EventType T, typename S>
EventSubscriptionToken EventSystem::Subscribe(S* instance, void (S::*callback)(const T&))
{
    using Payload = __detail::MemberPayload<T, S*, S>;
    auto*   payload = new Payload{instance, callback};

    return GetOrCreateList<T>()->Subscribe(payload, __detail::MemberInvoke<T, S*, S>, __detail::DeletePayload<Payload>);
}

template <EventType T, typename S>
EventSubscriptionToken EventSystem::Subscribe(const std::weak_ptr<S>& owner, void (S::*callback)(const T&))
{
    using Payload = __detail::MemberPayload<T, std::weak_ptr<S>, S>;
    auto*   payload = new Payload{owner, callback};

    return GetOrCreateList<T>()->Subscribe(payload, __detail::MemberInvoke<T, std::weak_ptr<S>, S>, __detail::DeletePayload<Payload>);
}

template <EventType T, typename S>
EventSubscriptionToken EventSystem::Subscribe(const std::shared_ptr<S>& owner, void (S::*callback)(const T&))
{
    using Payload = __detail::MemberPayload<T, std::shared_ptr<S>, S>;
    auto*   payload = new Payload{owner, callback};

    return GetOrCreateList<T>()->Subscribe(payload, __detail::MemberInvoke<T, std::shared_ptr<S>, S>, __detail::DeletePayload<Payload>);
}

template <EventType T>
void EventSystem::Invoke(const T& event)
{
    if (ISubscriberList* list = FindList<T>()) list->Invoke(&event);
}

template <EventType T>
ISubscriberList* EventSystem::GetOrCreateList()
{
    {
        std::shared_lock lock(_listsMutex);
        if (auto it = _lists.find(TypeID<T>()); it != _lists.end()) return it->second;
    }

    std::unique_lock lock(_listsMutex);
    if (auto it = _lists.find(TypeID<T>()); it != _lists.end()) return it->second;

    ISubscriberList* list = new __detail::SubscriberList<T>();
    _lists.emplace(TypeID<T>(), list);
    return list;
}

template <EventType T>
ISubscriberList* EventSystem::FindList() const
{
    std::shared_lock lock(_listsMutex);
    auto it = _lists.find(TypeID<T>());
    return it == _lists.end() ? nullptr : it->second;
}

} // namespace Tez
