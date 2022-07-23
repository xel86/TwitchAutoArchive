#include "Archiver.hpp"
#include "DownloadThread.hpp"
#include "Log.hpp"

#include <iostream>
#include <unistd.h>

std::vector<DownloadThread> threads;

void Archiver::run() {
    while (!m_shutdown) {
        auto changedStatus = m_twitch.setLiveStatus(m_streamers, false);
        if (!changedStatus.empty()) {
            for (Streamer* s : changedStatus) {
                threads.push_back(DownloadThread(*s));
            }
        }

        LOG.write(LogLevel::Always, "Sleeping");

        sleep(m_rate);
    }
}