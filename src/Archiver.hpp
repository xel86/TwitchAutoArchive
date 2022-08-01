#pragma once

#include "TwitchServer.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

class Archiver
{
  public:
    using user_id = std::string;

    Archiver(TwitchAuth&& auth, std::unordered_map<user_id, Streamer>&& streamers, int rate) :
        mTwitchServer(std::move(auth)), mStreamers(std::move(streamers)), mRate(rate){};

    void run();

    void syncStreamersListFromConfig();

  private:
    TwitchServer mTwitchServer;
    std::unordered_map<user_id, Streamer> mStreamers;
    std::filesystem::file_time_type mLastConfigWriteTime;
    int mRate;
    bool mShutdown{false};
};