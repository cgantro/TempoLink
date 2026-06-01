#pragma once
#include <string>

enum class ConnectionState { Disconnected, Connecting, Connected };

struct AppState
{
    ConnectionState connection  = ConnectionState::Disconnected;
    std::string     roomId;
    std::string     localPeerId;
    int             sampleRate  = 48000;
    int             bufferSize  = 256;
};
