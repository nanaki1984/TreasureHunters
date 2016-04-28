#pragma once

#include "Core/IO/BitStream.h"

namespace Network {

template <bool IsReader>
class NetStream {
protected:
    Core::IO::BitStream &stream;
public:
    NetStream(Core::IO::BitStream &_stream)
    : stream(_stream)
    {
        if (IsReader)
            stream.Rewind();
        else
            stream.Reset();
    }

    ~NetStream()
    { }

    template <typename T> bool Serialize(T &value)
    {
        if (IsReader)
        {
            auto bitsCount = sizeof(T) << 3;
            if (stream.RemainingBits() >= bitsCount)
            {
                stream >> value;
                return true;
            }
            else
                return false;
        }
        else
        {
            stream << value;
            return true;
        }
    }
};

class NetReadStream : public NetStream<true> {
public:
    NetReadStream(Core::IO::BitStream &_stream)
    : NetStream(_stream)
    { }

    ~NetReadStream()
    { }
};

class NetWriteStream : public NetStream<false> {
public:
    NetWriteStream(Core::IO::BitStream &_stream)
    : NetStream(_stream)
    { }

    ~NetWriteStream()
    { }
};

} // namespace Network
