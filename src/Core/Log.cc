#include "Core/Log.h"

#ifdef _WIN32
#	define NOMINMAX
#   include <windows.h>
#endif

#include <iostream>
#include "Core/Memory/Memory.h"

namespace Core {

DefineClassInfo(Core::Log, RefCounted) // inherited from Singleton

Log::Log()
: callback(nullptr)
{
    appLog.open("log.txt");
}

Log::~Log()
{
    appLog.close();
}

void
Log::SetCallback(LogFunction _callback)
{
    callback = _callback;
}

void
Log::Write(int msgFlags, const char *msg, ...)
{
    assert(appLog.is_open());

    va_list args;
    char    buffer[1024];

    va_start(args, msg);
    vsnprintf(buffer, 1024, msg, args);
    va_end(args);

    if (callback != nullptr)
        callback((msgFlags & 0x3), buffer);

    bool showMsg = (msgFlags & ShowToUser ? true : false);

    switch ((msgFlags & 0x3)) {
        case Info:
            appLog << "(i) " << buffer << "\n";
            if (showMsg) {
#if defined _WIN32
                MessageBox(0, buffer, "Info", MB_ICONINFORMATION | MB_OK);
#elif defined __APPLE__
#endif
            }
            break;
        case Warning:
            appLog << "(w) " << buffer << "\n";
            if (showMsg) {
#if defined _WIN32
                MessageBox(0, buffer, "Warning", MB_ICONWARNING | MB_OK);
#elif defined __APPLE__
#endif
            }
            break;
        case Error:
            appLog << "(e) " << buffer << "\n";
            if (showMsg) {
#if defined _WIN32
                MessageBox(0, buffer, "Error", MB_ICONERROR | MB_OK);
#elif defined __APPLE__
#endif
            }
            break;
    };

#if defined _DEBUG
    appLog.flush();
#endif // _DEBUG
}

void
Log::Barf(const char *msg, ...)
{
    va_list args;
    char    buffer[1024];

    va_start(args, msg);
    vsnprintf(buffer, 1024, msg, args);
    va_end(args);

    printf("%s\n", buffer);
}

}; // namespace Core
