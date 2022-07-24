#include "Args.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

void printHelp() {
    std::cout << "Usage: TwitchAutoArchive [OPTIONS]...\n";
    std::cout << "Daemon to automatically download twitch streams when they go "
                 "live\n";

    std::cout << "Example: twitchautoarchive --daemon --rate 60\n\n";
    std::cout << "Options:\n";
    std::cout
        << "  -x, --debug    \tDebug mode - print additional info to log\n";
    std::cout
        << "  -r, --rate     \tRate at which to check if any streams have gone "
           "live (in seconds)\n";
    std::cout << "  -c, --config   \tConfig file containing streamers to "
                 "archive and twitch credentials. Sample config can be found "
                 "in project directory.\n";
    std::cout
        << "  --daemon       \tFork program to run as daemon in background\n";
}

std::filesystem::path setupDir() {
    std::filesystem::path dir;
    if (const char* xdg = std::getenv("XDG_CONFIG_HOME")) {
        dir = xdg;
        dir.append("twitchautoarchive");
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directory(dir);
            std::cout << "Created twitchautoarchive directory: " << dir << "\n";
        }
    } else if (const char* home = std::getenv("HOME")) {
        dir = home;
        dir.append(".config");
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directory(dir);
            std::cout << "Created config directory: " << dir << "\n";
        }

        dir.append("twitchautoarchive");
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directory(dir);
            std::cout << "Created twitchautoarchive directory: " << dir << "\n";
        }
    }

    return dir;
}

void parseArgs(int argc, char** argv, Args& args) {
    if (argc > 16) {
        std::cout << "Entered way too many command line arguments.\n";
        std::exit(1);
    }

    auto dir = setupDir();

    /* Set args to defaults */
    args.daemon = false;
    args.debug = false;
    args.rate = 60;
    args.configPath = "";
    args.logPath = dir / "taa.log";

    const std::vector<std::string_view> argList(argv + 1, argv + argc);
    for (auto it = argList.begin(), end = argList.end(); it != end; ++it) {
        std::string_view arg{*it};

        if (arg == "--help" || arg == "-h") {
            printHelp();
            std::exit(0);
        }

        else if (arg == "--daemon") {
            args.daemon = true;
        }

        else if (arg == "--debug" || arg == "-x") {
            args.debug = true;
        }

        else if (arg == "--rate" || arg == "-r") {
            if (it + 1 != end) {
                try {
                    args.rate = std::stoi(std::string(*(it + 1)));
                    ++it;
                } catch (const std::invalid_argument& ia) {
                    std::cerr << "The rate argument (-r, --rate) requires an "
                                 "integer in seconds. Invalid argument: "
                              << ia.what() << "\n";
                    std::exit(1);
                }
            } else {
                std::cerr << "The rate argument (-r, --rate) requires an "
                             "integer in seconds.\n";
                std::exit(1);
            }
        }

        else if (arg == "--config" || arg == "-c") {
            if (it + 1 != end) {
                std::filesystem::path path{*(it + 1)};

                if (std::filesystem::is_regular_file(path)) {
                    args.configPath = path;
                    ++it;
                } else {
                    std::cerr << "File path provided for streamers argument "
                                 "(-c, --config): "
                              << path
                              << " either doesn't exist or "
                                 "isn't a regular file\n";
                    std::exit(1);
                }
            } else {
                std::cerr
                    << "The file argument (-c, --config) requires a path to a "
                       "text file with the list of streamers to archive\n";
                std::exit(1);
            }
        }

        else {
            std::cerr << "Unknown argument: " << arg << "\n\n";
            printHelp();
        }
    }

    /* If no streamer path was specified, attempt to use default location */
    if (args.configPath.empty()) {
        args.configPath = dir / "config";
    }
}