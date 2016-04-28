#include "Core/Debug.h"
#include "Core/Time/TimeSource.h"

namespace Core {
    namespace Time {

DefineClassInfo(Core::Time::TimeSource, Core::RefCounted);

TimeSource::TimeSource()
: pauseCounter(0),
  deltaTime(0.0f),
  timeFactor(1.0f),
  totalTime(0.0f)
{ }

TimeSource::~TimeSource()
{ }

void
TimeSource::Play()
{
    assert(pauseCounter > 0);
    --pauseCounter;
}

void
TimeSource::Pause()
{
    ++pauseCounter;
}

void
TimeSource::Reset()
{
    totalTime = 0.0f;
}

void
TimeSource::Update(float dt)
{
    deltaTime = (0 == pauseCounter ? dt * timeFactor : 0.0f);
    totalTime += deltaTime;
}

    }; // namespace Time
}; // namespace Core
