#pragma once

#include <filesystem>

struct Args {
    bool debug;  /* Flag to enable debug mode */
    bool daemon; /* Flag to determine whether to daemonize process */
    int rate;    /* Rate at which to check for live status (seconds) */
    std::filesystem::path configPath; /* Config file path */
    std::filesystem::path logPath;    /* File path for log */
};

extern Args gArgs;

/* Prints help menu */
void printHelp();

/* Parses args
 * if successful all arguments will be placed in args struct
 * if failed program will exit */
void parseArgs(int argc, char** argv, Args& args);

/* Setup directory for streamer list and log file */
std::filesystem::path setupDir();
