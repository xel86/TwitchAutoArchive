#pragma once

#include <curl/curl.h>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

const std::string helixStreamsUrl = "https://api.twitch.tv/helix/streams";

struct TwitchAuth
{
    std::string clientID;
    std::string clientSecret;
    std::string accessToken;
    std::string personalOAuth;
};

struct Streamer
{
    std::string user_id;       /* User id of streamer */
    std::string user_login;    /* User login of streamer */
    std::filesystem::path dir; /* Directory to download streams too */
    bool live{false};
};

class TwitchServer
{
  public:
    TwitchServer(TwitchAuth&& auth);

    /* Calls twitch api GetStreams and sets the live bool for every streamer
     * struct to true or false accordingly. Returns a vector of pointers to the
     * Streamer elements in the map passed in that have changed live status.
     * Either from live to offline or offline to live. */
    bool setLiveStatus(std::unordered_map<std::string, Streamer>& streamers);

  private:
    std::string appendUserIdsToUrl(std::string url,
                                   const std::unordered_map<std::string, Streamer>& streamers);

    static size_t curlWriteDataCallback(const char* contents, size_t size, size_t nmemb,
                                        void* userp);
    TwitchAuth mAuth;
    CURL* mCurl;
};