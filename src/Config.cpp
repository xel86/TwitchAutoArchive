#include "Config.hpp"
#include "Log.hpp"

#include <fstream>
#include <unordered_map>

void generateSampleConfig(std::filesystem::path path)
{
    std::ofstream sampleConfig(path);
    const char* sample =
R"(# You must provide either a client_id and secret, or an access_token.
# Twitch now embeds ads directly into the hls stream, potentially interupting archiving.
# If you have twitch turbo, you can avoid ads while archiving streams by using a personal twitch oauth token
# This is different from any normal 3rd party access tokens, and must be taken manually from a logged in browser.
# Config this in your streamlink config
# More information: https://streamlink.github.io/cli/plugins/twitch.html#authentication
[TwitchAuth]
client_id =
client_secret =
access_token =

# Replace numbers with a twitch user_id and the directory
# with where you want the downloaded vods to go for that user
[Streamers]
131913322 /home/user/vods/user1
131913321 /home/user/vods/user2
)";

    sampleConfig << sample;
}

std::string trim(const std::string& s)
{
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start))
    {
        start++;
    }

    auto end = s.end();
    do
    {
        end--;
    }
    while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}

Config parseConfig(std::filesystem::path path)
{
    std::ifstream configFile(path);
    if (!configFile.is_open())
    {
        std::cerr << "Couldn't open config file " << path << "\n";

        if (!std::filesystem::exists(path))
        {
            std::cerr << "Generating a sample config in this location, please "
                         "customize it for you to use\n";
            generateSampleConfig(path);
        }

        std::exit(1);
    }

    std::unordered_map<std::string, std::string> items;

    int streamerSectionPos = -1;
    std::string line;
    while (std::getline(configFile, line))
    {
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        auto delimPos = line.find("=");
        if (delimPos != std::string::npos)
        {
            // TODO: Spaces between = will be included?
            auto key = trim(line.substr(0, delimPos));
            auto value = trim(line.substr(delimPos + 1));

            items[key] = value;
            continue;
        }

        if (line.find("[Streamers]") != std::string::npos)
        {
            streamerSectionPos = configFile.tellg();
        }
    }

    std::unordered_map<std::string, Streamer> streamers;
    if (streamerSectionPos >= 0)
    {
        /* Seek to [Streamers] section */
        configFile.clear();
        configFile.seekg(streamerSectionPos);

        Streamer s;
        while (configFile >> s.user_id >> s.dir)
        {
            if (std::filesystem::exists(s.dir))
            {
                streamers[s.user_id] = s;
            }
            else
            {
                std::cerr << "Streamer " << s.user_id
                          << "'s download directory does not exist: " << s.dir
                          << "; continuing without this streamer\n";
            }
        }

        if (streamers.empty())
        {
            std::cerr << "No valid streamers were found in streamers file. File "
                         "should be in format: <user_id> <absolute directory path> "
                         "followed by a newline\n";
            std::exit(1);
        }
    }
    else
    {
        std::cerr << "Streamer section '[Streamers]' missing from config file. "
                     "Please see the sample config for an example on how to "
                     "set this up\n";
        std::exit(1);
    }

    TwitchAuth auth;
    bool enoughInfo = false;
    if (items.count("access_token") && items.count("client_id"))
    {
        auth.clientID = items["client_id"];
        auth.accessToken = items["access_token"];
        enoughInfo = true;
    }

    if (items.count("client_id") && items.count("client_secret"))
    {
        auth.clientID = items["client_id"];
        auth.clientSecret = items["client_secret"];
        enoughInfo = true;
    }

    if (!enoughInfo)
    {
        std::cerr << "Not enough twitch credential information in config to "
                     "use api; you must supply either a client_id and "
                     "client_secret, or an access_token and client_id.\nAn "
                     "access_token will "
                     "be generated for you if a client_id and "
                     "client_secret is provided.\n";
        std::exit(1);
    }

    return Config{auth, streamers};
}