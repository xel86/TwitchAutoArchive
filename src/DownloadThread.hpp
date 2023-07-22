#pragma once

#include "TwitchServer.hpp"

#include <filesystem>
#include <mutex>
#include <unordered_set>

using user_id = std::string;

void streamlinkDownloadFunc(Streamer& streamer, std::unordered_set<user_id>& downloading, std::mutex& downloadingMutex);