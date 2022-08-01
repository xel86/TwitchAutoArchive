#include "Archiver.hpp"
#include "Args.hpp"
#include "Config.hpp"
#include "DownloadThread.hpp"
#include "Log.hpp"

#include <iostream>
#include <thread>
#include <unistd.h>

void Archiver::syncStreamersListFromConfig()
{
    auto lastWriteTime = std::filesystem::last_write_time(gArgs.configPath);
    /* If config hasn't been changed, don't waste time reparsing */
    if (lastWriteTime == mLastConfigWriteTime)
    {
        LOG.write(LogLevel::Always, "Didn't change");
        return;
    }

    LOG.write(LogLevel::Always, "changed");
    mLastConfigWriteTime = lastWriteTime;

    Config cfg = parseConfig(gArgs.configPath);

    /* Add any added streamers from the new config */
    for (const auto& [user_id, streamer] : cfg.streamers)
    {
        if (!mStreamers.count(user_id))
        {
            mStreamers.emplace(user_id, streamer);
            LOG.write(LogLevel::Always,
                      "Added new streamer (" + user_id + ") to streamers target list");
        }
    }

    /* Remove any streamers deleted from the previous config */
    for (auto it = mStreamers.cbegin(); it != mStreamers.cend();)
    {
        const std::string& user_id = it->first;
        const std::string& user_login = it->second.user_login;
        if (!cfg.streamers.count(user_id))
        {
            LOG.write(LogLevel::Always, "Removed streamer " + user_login + " (" + user_id + ") " +
                                            "from streamers target list");
            it = mStreamers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Archiver::run()
{
    mLastConfigWriteTime = std::filesystem::last_write_time(gArgs.configPath);

    while (!mShutdown)
    {
        LOG.write(LogLevel::Verbose, "Checking live status for all streams");

        syncStreamersListFromConfig();

        auto changedStatus = mTwitchServer.setLiveStatus(mStreamers, false);
        if (!changedStatus.empty())
        {
            for (Streamer* s : changedStatus)
            {
                /* Streamer went online */
                if (s->live)
                {
                    auto t = std::thread(streamlinkDownloadFunc, s->user_login, s->dir);
                    t.detach();
                }
            }
        }

        sleep(mRate);
    }
}