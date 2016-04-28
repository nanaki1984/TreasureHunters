#pragma once

#include <cstdarg>
#include <fstream>

#include "Core/Singleton.h"
#include "Core/ClassInfo.h"

namespace Core {

class Log : public Singleton<Log> {
    DeclareClassInfo;
public:
    typedef void (*LogFunction)(int, const char *);

    enum MsgFlags {
        Info = 0x0,   // 000
        Warning = 0x1,   // 001
        Error = 0x2,   // 010

        ShowToUser = 0x4 // 100
    };
protected:
    std::ofstream appLog;
    LogFunction callback;
public:
    Log();
    virtual ~Log();

    void SetCallback(LogFunction _callback);

    void Write(int msgFlags, const char *msg, ...);

    static void Barf(const char *msg, ...);
};

}; // namespace Core
