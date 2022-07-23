#pragma once

#include "TwitchServer.hpp"

#include <filesystem>
#include <unordered_map>
#include <vector>

struct Config {
    TwitchAuth auth;
    std::unordered_map<std::string, Streamer> streamers;
};

/* Parse config file consisting of user ids of streamers and what directory to
 * download the vods too, and twitch authentication information */
Config parseConfig(std::filesystem::path path);

void generateSampleConfig(std::filesystem::path path);