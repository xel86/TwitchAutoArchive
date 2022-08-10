# TwitchAutoArchive (WIP)
C++ Daemon that watches twitch users and will download their streams as they go live.

## Requirements
[streamlink](https://github.com/streamlink/streamlink) accessible via path

## Installation
Clone the repository: `git clone https://github.com/xel86/TwitchAutoArchive.git`

cd into the directory and build the binary: `make`

You can then either run the binary from `bin/twitchautoarchive` or move the binary to a more convienient folder like `/usr/bin` 

## Usage
Move the `sample_config` provided into `~/.config/twitchautoarchive`, make the folder if it doesn't already exist.

Rename the sample_config file to just `config` and fill out the required fields. Currently you will have to provided your twitch auth token yourself, either by using a third party service or by using curl using the official [twitch api instructions](https://dev.twitch.tv/docs/api/get-started#get-an-oauth-token)

Finally, fill out the streamers section at the bottom of the config with the user id of the streamer you would like to track and the absolute path to the directory to download their streams too. This directory must be made before running twitchautoarchive.

Then start the daemon using `twitchautoarchive --daemon`

Logs from the daemon will be deposited to `~/.config/twitchautoarchive/taa.log`

### Arguments
You can specify the rate at which the archiver will use the api to check for the live status of streamers by using the `--rate` argument.

## WIP
Currently twitchautoarchive calls streamlink using `system`, this is ugly and prevents the streamlink downloading threads to be killed when you kill the main daemon process. This means you will have to manually kill each individual ongoinging downloading streamlink processes if you want to stop them.

Eventually I hope to use ffmpeg directly to download the streams for better control, but this seems like quite a task.
