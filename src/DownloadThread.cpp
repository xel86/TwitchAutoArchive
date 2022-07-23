#include "DownloadThread.hpp"
#include "Log.hpp"

#include <thread>

DownloadThread::DownloadThread(Streamer target) : m_target(target) {
    m_thread =
        std::thread(streamlinkDownloadFunc, target.user_login, target.dir);
}

bool DownloadThread::streamlinkDownloadFunc(std::string user_login,
                                            std::filesystem::path dir) {
    std::string url = "https://twitch.tv/" + user_login;
    std::time_t result = std::time(nullptr);
    dir /= (user_login + "-" + std::to_string(result) + ".mkv");

    LOG.write(LogLevel::Always, "Opening stream " + url + " to download.");

    std::string cmd = "streamlink " + url + " best -o " + dir.string();
    std::system(cmd.c_str());

    LOG.write(LogLevel::Always,
              "Closed stream " + url + ", finished downloading.");

    return true;
}