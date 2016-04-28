#include "Core/Debug.h"
#include "Core/Log.h"

namespace Core {
    namespace Debug {

void
ShowAssertMessage(const char *exp, const char *filename, int line)
{
    Log *log = Core::Log::InstanceUnsafe();
    if (nullptr == log)
        Core::Log::Barf("Assertion failed!\n\nExpression: %s\n@ %s(%d)", exp, filename, line);
    else
        log->Write(Log::Error | Log::ShowToUser, "Assertion failed!\n\nExpression: %s\n@ %s(%d)", exp, filename, line);
}

void
ShowAssert2Message(const char *exp, const char *msg, const char *filename, int line)
{
    Log *log = Core::Log::InstanceUnsafe();
    if (nullptr == log)
        Core::Log::Barf("Assertion failed!\n\nMessage: %s\nExpression: %s\n@%s(%d)", msg, exp, filename, line);
    else
        log->Write(Log::Error | Log::ShowToUser, "Assertion failed!\n\nMessage: %s\nExpression: %s\n@%s(%d)", msg, exp, filename, line);
}

    }; // namespace Debug
}; // namespace Core
