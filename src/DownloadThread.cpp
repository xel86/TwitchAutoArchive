#include "DownloadThread.hpp"
#include "Log.hpp"

#include <thread>

void streamlinkDownloadFunc(Streamer& streamer, std::unordered_set<user_id>& downloading, std::mutex& downloadingMutex)
{
    LOG.write(LogLevel::Always, "Opening stream " + streamer.user_login + " to download.");

    /* Construct streamlink download command */
    std::string url = "https://twitch.tv/" + streamer.user_login;
    std::time_t result = std::time(nullptr);
    std::filesystem::path dir = streamer.dir / (streamer.user_login + "-" + std::to_string(result) + ".mkv");
    std::string cmd = "streamlink " + url + " best -o " + dir.string();

    std::system(cmd.c_str());

    /* Once streamlink closes, remove streamer user_id from the downloading set */
    std::unique_lock<std::mutex> lock(downloadingMutex);
    downloading.erase(streamer.user_id);

    LOG.write(LogLevel::Always, "Closed stream " + streamer.user_login + ", finished downloading.");
}