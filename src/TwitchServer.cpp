#include "TwitchServer.hpp"
#include "Log.hpp"

#include "json.hpp"

#include <curl/curl.h>
#include <sstream>

using json = nlohmann::json;

TwitchServer::TwitchServer(TwitchAuth&& auth) : mAuth(auth)
{
    mCurl = curl_easy_init();
    if (!mCurl)
    {
        LOG.write(LogLevel::Error, "Failed to create a CURL easy handle.");
        std::exit(1);
    }

    if (auth.accessToken.empty())
    {
        LOG.write(LogLevel::Error, "Access token not provided in config.");
        std::exit(1);
    }

    curl_easy_setopt(mCurl, CURLOPT_HTTPGET, 1L);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Client-Id: " + auth.clientID).c_str());
    headers = curl_slist_append(headers, ("Authorization: Bearer " + auth.accessToken).c_str());

    curl_easy_setopt(mCurl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(mCurl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(mCurl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(mCurl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_3);

    curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, curlWriteDataCallback);
}

std::string
TwitchServer::appendUserIdsToUrl(std::string url,
                                 const std::unordered_map<std::string, Streamer>& streamers)
{
    if (url[url.size() - 1] == '/')
    {
        url.pop_back();
    }

    /* Append user_id query parameter to string:
     * helix/streams?user_id=123&user_id=321&... */
    url += "?";

    for (const auto& [user_id, _] : streamers)
    {
        url += "user_id=";
        url += user_id;
        url += "&";
    }

    /* Delete dangling & */
    url.pop_back();

    return url;
}

std::vector<Streamer*>
TwitchServer::setLiveStatus(std::unordered_map<std::string, Streamer>& streamers,
                            bool streamersAdded)
{
    std::string url = appendUserIdsToUrl(helixStreamsUrl, streamers);
    curl_easy_setopt(mCurl, CURLOPT_URL, url.c_str());

    std::stringstream data;
    curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, (void*)&data);

    CURLcode res = curl_easy_perform(mCurl);

    if (res != CURLE_OK)
    {
        LOG.write(LogLevel::Error, "curl failed to send get request with error: " +
                                       std::string(curl_easy_strerror(res)));
        return {};
    }

    int code{};
    curl_easy_getinfo(mCurl, CURLINFO_RESPONSE_CODE, &code);

    if (code != 200)
    {
        LOG.write(LogLevel::Error,
                  "Twitch GetStreams api call failed with error code: " + std::to_string(code));
        return {};
    }

    std::vector<Streamer*> changedStatus;
    try
    {
        std::vector<json> liveIds;
        json j = json::parse(data.str());
        for (const auto& obj : j["data"])
        {
            try
            {
                if (obj["type"] == "live")
                {
                    liveIds.push_back(obj);
                }
            }
            catch (...)
            {}
        }

        for (auto& [user_id, streamer] : streamers)
        {
            const std::string& tmp = user_id;
            auto it = std::find_if(liveIds.begin(), liveIds.end(),
                                   [&](json& obj) { return obj["user_id"] == tmp; });

            if (it != liveIds.end())
            {
                /* Streamer went online */
                if (streamer.live == false)
                {
                    streamer.live = true;
                    streamer.user_login = (*it)["user_login"];
                    changedStatus.push_back(&streamer);
                }
            }
            else
            {
                /* Streamer went offline */
                if (streamer.live == true)
                {
                    streamer.live = false;
                    changedStatus.push_back(&streamer);
                }
            }
        }
    }
    catch (std::exception& e)
    {
        LOG.write(LogLevel::Error,
                  "Failed to parse twitch api GetStreams json response with error: " +
                      std::string(e.what()));
        return {};
    }

    return changedStatus;
}

size_t TwitchServer::curlWriteDataCallback(const char* contents, size_t size, size_t nmemb,
                                           void* userp)
{
    size_t realsize = size * nmemb;
    std::string data(contents, realsize);

    auto returnData = reinterpret_cast<std::stringstream*>(userp);
    *returnData << data;

    return realsize;
}
