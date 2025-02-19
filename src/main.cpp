#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>
#include <Geode/binding/MusicDownloadManager.hpp>
#include <km7dev.server_api/include/ServerAPIEvents.hpp>
#include <filesystem> // C++17 for filesystem handling
#include <Geode/binding/GJSongBrowser.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

void showSSMenu()
{
    auto sb = GJSongBrowser::create();
    CCDirector::sharedDirector()->getRunningScene()->addChild(sb);
    sb->showLayer(true);
}


namespace fs = std::filesystem;

using namespace geode::prelude;

auto p = MusicDownloadManager::sharedState();
gd::string folder = p->pathForSongFolder(0); // Folder already contains the correct base path

gd::string getDomain()
{
    // Get the server URL
    std::string url = ServerAPIEvents::getCurrentServer().url;

    if (url == "NONE_REGISTERED")
    {
        url = ServerAPIEvents::getBaseUrl();
    };

    // Variable to hold the domain
    std::string domain;

    // Check if the URL contains "://"
    size_t domainStart = url.find("://");

    if (domainStart != std::string::npos)
    {
        // If it has "://", set domainStart to after "://"
        domainStart += 3;
    }
    else
    {
        // If there's no "://", start from the beginning of the string
        domainStart = 0;
    }

    // Find the first slash after the domain (if it exists)
    size_t domainEnd = url.find('/', domainStart);

    if (domainEnd != std::string::npos)
    {
        // Extract the domain
        domain = url.substr(domainStart, domainEnd - domainStart);
    }
    else
    {
        // If no slash is found, the domain is the rest of the string
        domain = url.substr(domainStart);
    }

    return domain;
}

void createDirectoryIfNeeded(const std::string &path)
{
    std::string directoryPath = path.substr(0, path.find_last_of('/'));
    // Check if the directory exists
    if (!fs::exists(directoryPath))
    {
        // If it doesn't exist, create the directory
        fs::create_directories(directoryPath); // This creates all intermediate directories as needed
        geode::log::info("Created directory: {}", directoryPath);
    }
}

class $modify(MenuLayer)
{
    void onMoreGames(CCObject *target)
    {
        showSSMenu();
    };
};

class $modify(MusicDownloadManager)
{
    gd::string pathForSong(int p0)
    {
        geode::log::debug("pathForSong");

        std::string fullPath = std::string(folder);
        if (!fullPath.empty() && (fullPath.back() == '\\' || fullPath.back() == '/'))
        {
            fullPath.pop_back(); // Remove trailing slash
        }

        // Fix concatenation issues
        fullPath.append("/").append(getDomain().c_str()).append("/Songs/").append(std::to_string(p0)).append(".ogg");

        // Replace backslashes with forward slashes for consistency
        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

        // Ensure directories exist
        createDirectoryIfNeeded(fullPath);

        return gd::string(fullPath);
    }

    gd::string pathForSFX(int p0)
    {
        

        std::string fullPath = std::string(folder);
        if (!fullPath.empty() && (fullPath.back() == '\\' || fullPath.back() == '/'))
        {
            fullPath.pop_back();
        }

        fullPath.append("/").append(getDomain().c_str()).append("/SFX/s").append(std::to_string(p0)).append(".ogg");

        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

        createDirectoryIfNeeded(fullPath);
    geode::log::debug("pathForSFX {}",fullPath);
        return gd::string(fullPath);
    }

    bool isSongDownloaded(int p0)
    {
        log::info("Checked {}", std::to_string(p0));
        std::string songPath = std::string(pathForSong(p0)); // Convert gd::string to std::string

        // Check if the file exists in the specified path
        if (std::filesystem::exists(songPath))
        {
            return true;
        }

        return false;
    }
    
    gd::string pathForSFXFolder(int p0) {
        std::string fullPath = std::string(folder);
        if (!fullPath.empty() && (fullPath.back() == '\\' || fullPath.back() == '/'))
        {
            fullPath.pop_back();
        }
        
        fullPath.append("/").append(getDomain().c_str()).append("/SFX/");

        createDirectoryIfNeeded(fullPath);
        log::info("{}",fullPath);
        return gd::string(fullPath);
    }
};
