#pragma once

#include "TwitchServer.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

class Archiver {
  public:
    using user_id = std::string;

    Archiver(TwitchAuth&& auth,
             std::unordered_map<user_id, Streamer>&& streamers, int rate)
        : m_twitch(std::move(auth)), m_streamers(std::move(streamers)),
          m_rate(rate){};

    void run();

    void refreshStreamersFromConfig();

  private:
    TwitchServer m_twitch;
    std::unordered_map<user_id, Streamer> m_streamers;
    int m_rate;
    bool m_shutdown{false};
};