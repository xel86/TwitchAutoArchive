#pragma once

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

enum LogLevel {
    Always = -1,
    Error = 0,
    Warning = 1,
    Verbose = 2,
};

class Log {
  public:
    Log(){};

    void init(std::filesystem::path path, bool debug) {
        if (debug)
            m_level = LogLevel::Verbose;
        else
            m_level = LogLevel::Error;

        if (!path.has_parent_path()) {
            std::cerr << "Path to log file doesn't exist!\n";
        }

        m_log.open(path, std::ofstream::out | std::ofstream::trunc);

        if (!m_log.is_open()) {
            std::cerr << "Error opening log file\n";
        }
    }

    void write(LogLevel level, const std::string& message) {
        const std::lock_guard<std::mutex> lock(m_mutex);

        if (!m_log.is_open()) {
            return;
        }

        if (m_level >= level) {
            std::string stamp;
            std::time_t time = std::time(nullptr);

            stamp = std::asctime(std::localtime(&time));

            /* Remove newline from timestamp string */
            stamp.pop_back();

            switch (level) {
            case Always:
                stamp += " |> ";
                break;
            case Error:
                stamp += " | ERROR |> ";
                break;
            case Warning:
                stamp += " | WARNING |> ";
                break;
            case Verbose:
                stamp += " | VERBOSE |> ";
                break;
            };

            m_log << stamp << message << std::endl;
        }
    }

  private:
    std::ofstream m_log;
    std::mutex m_mutex;
    LogLevel m_level;
};

extern Log LOG;