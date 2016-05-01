#pragma once

#define NOMINMAX
#include "enet/enet.h"

#include "Core/IO/BitStream.h"

namespace Network {

template <bool IsReader>
class NetStream {
protected:
    ENetPeer *peer;
    Core::IO::BitStream &stream;
public:
    enum {
        IsWriting = IsReader ? 0 : 1
    };

    NetStream(ENetPeer *_peer, Core::IO::BitStream &_stream)
    : peer(_peer),
      stream(_stream)
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

    template <typename T> bool SerializeArray(Core::Collections::Array<T> &array) // ToDo: different sizes (1, 2, 4 bytes)
    {
        if (IsReader)
        {
            if (stream.RemainingBytes() >= 4)
            {
                uint32_t size;
                stream >> size;
                array.Resize(size);

                auto bitsCount = sizeof(T) << 3;
                for (uint32_t i = 0; i < size; ++i)
                {
                    if (stream.RemainingBits() < bitsCount)
                        return false;
                    stream >> array[i];
                }
            }
            else
                return false;
        }
        else
        {
            uint32_t size = array.Count();
            stream << size;

            for (uint32_t i = 0; i < size; ++i)
                stream << array[i];

            return true;
        }
    }

    bool SerializeTimestamp(float &timestamp)
    {
        if (IsReader)
        {
            if (stream.RemainingBytes() >= 4)
            {
                uint32_t t;
                stream >> t;
                t -= enet_peer_clock_differential(peer);
                timestamp = t * 0.001f;
                return true;
            }
            else
                return false;
        }
        else
        {
            uint32_t t = timestamp * 1000.0f;
            //t += enet_peer_clock_differential(peer);
            stream << t;
            return true;
        }
    }
};

class NetReadStream : public NetStream<true> {
public:
    NetReadStream(ENetPeer *_peer, Core::IO::BitStream &_stream)
    : NetStream(_peer, _stream)
    { }

    ~NetReadStream()
    { }
};

class NetWriteStream : public NetStream<false> {
public:
    NetWriteStream(ENetPeer *_peer, Core::IO::BitStream &_stream)
    : NetStream(_peer, _stream)
    { }

    ~NetWriteStream()
    { }
};

} // namespace Network
