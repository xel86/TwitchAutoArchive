#include "Archiver.hpp"
#include "DownloadThread.hpp"
#include "Log.hpp"

#include <iostream>
#include <thread>
#include <unistd.h>

void Archiver::run() {
    while (!m_shutdown) {
        LOG.write(LogLevel::Verbose, "Checking live status for all streams");

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