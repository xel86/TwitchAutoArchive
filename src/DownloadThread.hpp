#pragma once

#include "TwitchServer.hpp"

#include <filesystem>

void streamlinkDownloadFunc(std::string user_login, std::filesystem::path dir);