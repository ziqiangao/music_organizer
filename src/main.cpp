#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>
#include <Geode/binding/MusicDownloadManager.hpp>
#include <km7dev.server_api/include/ServerAPIEvents.hpp>
#include <filesystem> // C++17 for filesystem handling
#include <Geode/binding/GJSongBrowser.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <Geode/binding/DialogObject.hpp>
#include <Geode/modify/OptionsLayer.hpp>
#include "songItem.hpp"
#include <Geode/binding/GameManager.hpp>

void showSSMenu()
{
    auto sb = GJSongBrowser::create();
    CCDirector::sharedDirector()->getRunningScene()->addChild(sb);
    sb->showLayer(true);
}

// Namespaces
namespace fs = std::filesystem;
using namespace geode::prelude;

// Vars
auto p = MusicDownloadManager::sharedState(); // Music Download Manager Singleton
gd::string folder = Mod::get()->getSettingValue<std::filesystem::path>("song-location").string(); // Song Location Setting
bool exportlist = Mod::get()->getSettingValue<bool>("exportlist"); // Export List Setting
std::string listfmt = Mod::get()->getSettingValue<std::string>("listfmt"); // File Format Of The Exported List Setting
std::string listName = Mod::get()->getSettingValue<std::string>("listname"); // Name Of Exported List Setting

/*
* Gets The Current Domanin using km7dev's Server API
*/
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

class $modify(OptionsLayer) {
    void onSoundtracks(CCObject* sender) {
        showSSMenu();
    }
};

int i = 0;
class $modify(MenuLayer)
{
    void onRobTop(CCObject *sender) // Debug Info
    {
        switch (i) {
        case 0:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (Song Path)", p->pathForSong(0), 2, 1.0f, true, ccWHITE), 2
            ));
            i += 1;
            break;
            
        case 1:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (SFX Path)", p->pathForSFX(0), 4, 1.0f, true,ccWHITE), 2
            ));
            i += 1;
            break;
        case 2:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (Settings->Save Songs List)", exportlist ? "True" : "False", 17, 1.0f, true,ccWHITE), 2
            ));
            i += 1;
            break;
        case 3:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (Settings->List Format)", listfmt, 28, 1.0f, true,ccWHITE), 2
            ));
            i += 1;
            break;
        case 4:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (Settings->List Name)", listName, 17, 1.0f, true,ccWHITE), 2
            ));
            i += 1;
            break;
        case 5:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (Song Info Test)", songItem(*static_cast<SongInfoObject*>(p->getDownloadedSongs()->randomObject())).toString(), 28, 1.0f, true,ccWHITE), 2
            ));
            i += 1;
            break;
        case 6:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (Menu Song)", GameManager::get()->m_customMenuSongID ? songItem(*static_cast<SongInfoObject*>(p->getSongInfoObject(GameManager::get()->m_customMenuSongID))).toString() : "NONE", 28, 1.0f, true,ccWHITE), 2
            ));
            i += 1;
            break;
        case 7:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (Practice Song)", GameManager::get()->m_customPracticeSongID ? songItem(*static_cast<SongInfoObject*>(p->getSongInfoObject(GameManager::get()->m_customPracticeSongID))).toString() : "NONE", 28, 1.0f, true,ccWHITE), 2
            ));
            i += 1;
            break;
        default:

            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Key Master", "What are doing here!?!?", 18, 1.0f, true,ccWHITE), 2
            ));
            i = 0;
            break;
        }
    }
};


class $modify(MusicDownloadManager)
{
    gd::string pathForSong(int p0)
    {
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
        geode::log::debug("pathForSong {}", fullPath);
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
        geode::log::debug("pathForSFX {}", fullPath);
        return gd::string(fullPath);
    }

    gd::string pathForSFXFolder(int p0) {
        gd::string ret = folder;
        
        return ret;
    }

    gd::string pathForSongFolder(int p0) {
        gd::string ret = folder;
        
        return ret;
    }

    static MusicDownloadManager *sharedState()
    {
        
        auto p = MusicDownloadManager::sharedState();
        if (!p->m_resourceSfxUnorderedSet.empty()) {
        p->m_resourceSfxUnorderedSet.clear();
        p->m_resourceSongUnorderedSet.clear();
        log::info("Cleared Resource IDS");}
        return p;
    }
};