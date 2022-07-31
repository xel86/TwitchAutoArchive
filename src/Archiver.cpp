#include "Archiver.hpp"
#include "Args.hpp"
#include "Config.hpp"
#include "DownloadThread.hpp"
#include "Log.hpp"

#include <iostream>
#include <thread>
#include <unistd.h>

void Archiver::refreshStreamersFromConfig() {
    Config cfg = parseConfig(gArgs.configPath);

    /* Add any added streamers from the new config */
    for (const auto& [user_id, streamer] : cfg.streamers) {
        if (!m_streamers.count(user_id)) {
            m_streamers.emplace(user_id, streamer);
            LOG.write(LogLevel::Always, "Added new streamer (" + user_id +
                                            ") to streamers target list");
        }
    }

    /* Remove any streamers deleted from the previous config */
    for (auto it = m_streamers.cbegin(); it != m_streamers.cend();) {
        const std::string& user_id = it->first;
        const std::string& user_login = it->second.user_login;
        if (!cfg.streamers.count(user_id)) {
            LOG.write(LogLevel::Always, "Removed streamer " + user_login +
                                            " (" + user_id + ") " +
                                            "from streamers target list");
            it = m_streamers.erase(it);
        } else {
            ++it;
        }
    }
}

void Archiver::run() {
    while (!m_shutdown) {
        LOG.write(LogLevel::Verbose, "Checking live status for all streams");

        refreshStreamersFromConfig();

        auto changedStatus = m_twitch.setLiveStatus(m_streamers, false);
        if (!changedStatus.empty()) {
            for (Streamer* s : changedStatus) {
                /* Streamer went online */
                if (s->live) {
                    auto t = std::thread(streamlinkDownloadFunc, s->user_login,
                                         s->dir);
                    t.detach();
                }
            }
        }

        sleep(m_rate);
    }
}