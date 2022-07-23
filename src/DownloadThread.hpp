#pragma once

#include "TwitchServer.hpp"

#include <filesystem>
#include <thread>

class DownloadThread {
  public:
    DownloadThread(Streamer target);

  private:
    static bool streamlinkDownloadFunc(std::string user_login,
                                       std::filesystem::path dir);

    std::thread m_thread;
    Streamer m_target;
    bool m_shutdown;
    bool m_finished;
};