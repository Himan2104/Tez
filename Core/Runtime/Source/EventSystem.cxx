#include "Tez/Core/Types.hxx"
#include <Tez/Core/EventSystem.hxx>

namespace Tez
{

EventSubscriptionToken::EventSubscriptionToken(UInt32 index)
    : _index{index}
{
}

EventSubscriptionToken::~EventSubscriptionToken() { EventSystem::GetInstance().Unsubscribe(*this); }

} // namespace Tez
