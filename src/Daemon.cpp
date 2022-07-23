#include "Archiver.hpp"
#include "Args.hpp"
#include "Config.hpp"
#include "Log.hpp"
#include "TwitchServer.hpp"

#include <filesystem>
#include <signal.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

Log LOG;

void daemonize() {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Failed to fork into daemon process. Exiting.");
        exit(EXIT_FAILURE);
    }

    /* Terminate parent process. */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    if (setsid() < 0) {
        fprintf(stderr,
                "Failed to set child process as session leader. Exiting.");
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Failed to fork into daemon process. Exiting.");
        exit(EXIT_FAILURE);
    }

    /* Terminate parent process. */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);

    /* Change the working directory to the root directory */
    chdir("/");

    /* Close open file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    pid_t new_pid = getpid();

    LOG.write(LogLevel::Always, "TwitchAutoArchive daemon started with PID " +
                                    std::to_string(new_pid));

    /* Don't block context switch */
    sleep(1);
}

int main(int argc, char** argv) {
    Args args;
    parseArgs(argc, argv, args);

    Config cfg = parseConfig(args.configPath);

    LOG.init(args.logPath, args.debug);
    LOG.write(LogLevel::Always, "Log initalized!");

    if (args.daemon) {
        daemonize();
    }

    auto archiver =
        Archiver(std::move(cfg.auth), std::move(cfg.streamers), args.rate);

    archiver.run();
}