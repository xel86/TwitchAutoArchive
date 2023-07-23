#include "DownloadThread.hpp"
#include "Log.hpp"

#include <thread>

const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};

std::string bytesToHuman(uint64_t bytes)
{
    double b = static_cast<double>(bytes);

    int i = 0;
    while (b > 1000)
    {
        b /= 1000;
        i++;
    }

    char str[16];
    sprintf(str, "%.2f %s", b, units[i]);

    return std::string(str);
}

void streamlinkDownloadFunc(Streamer streamer,
                            std::unordered_set<user_id>& downloading,
                            std::mutex& downloadingMutex)
{
    LOG.write(LogLevel::Always, "Opening stream " + streamer.user_login + " to download.");

    /* Construct streamlink download command */
    std::string url = "https://twitch.tv/" + streamer.user_login;
    std::time_t result = std::time(nullptr);
    std::string filename = streamer.user_login + "-" + std::to_string(result) + ".mkv";
    std::filesystem::path path = streamer.dir / filename;
    std::string cmd = "streamlink " + url + " best -o " + path.string();

    std::system(cmd.c_str());

    /* Remove streamer user_id from the downloading set once streamlink is finished */
    {
        std::unique_lock<std::mutex> lock(downloadingMutex);
        downloading.erase(streamer.user_id);
    }

    std::string size = "0.0 B";
    try
    {
        if (std::filesystem::exists(path))
        {
            size = bytesToHuman(std::filesystem::file_size(path));
        }
    }
    catch (const std::filesystem::filesystem_error& ex)
    {
        LOG.write(LogLevel::Warning,
                  "Error getting file size after downloading stream "
                  + streamer.user_login
                  + ": " + ex.what());
    }

    LOG.write(LogLevel::Always,
              "Closed stream "
              + streamer.user_login
              + ", finished downloading "
              + size);
}