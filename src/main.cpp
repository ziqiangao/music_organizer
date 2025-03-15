#include <Geode/modify/MusicDownloadManager.hpp>
#include <km7dev.server_api/include/ServerAPIEvents.hpp>
#include <filesystem> // C++17 for filesystem handling
#include <Geode/modify/GJSongBrowser.hpp>
#include <Geode/modify/OptionsLayer.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>

// Namespaces
namespace fs = std::filesystem;
using namespace geode::prelude;

// Vars
auto p = MusicDownloadManager::sharedState();                                                     // Music Download Manager Singleton
gd::string folder = Mod::get()->getSettingValue<std::filesystem::path>("song-location").string(); // Song Location Setting
gd::string music0 = "";

std::string getFilenameWithoutExtension(const std::string &fullPath)
{
    // Use std::filesystem to get the filename and remove the extension
    std::filesystem::path p(fullPath);
    return p.stem().string(); // Get the filename without the extension
}


/*
* Gets The Current Domanin using km7dev's Server API
*/
std::string getDomain()
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

void deleteAudioFiles()
{
    fs::path saveDir = dirs::getSaveDir();
    geode::log::info("Starting Clean Up");

    if (!fs::exists(saveDir) || !fs::is_directory(saveDir))
    {
        geode::log::warn("Directory does not exist: {}", saveDir.string());
        return;
    }

    try
    {
        for (const auto &entry : fs::directory_iterator(saveDir))
        {
            if (entry.is_regular_file())
            {
                std::string ext = entry.path().extension().string();
                if (ext == ".mp3" || ext == ".ogg")
                {
                    fs::remove(entry.path());
                    geode::log::info("Deleted: {}", entry.path().string());
                }
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
        geode::log::error("Filesystem error: {}", e.what());
    }
}

$on_mod(Loaded)
{
    deleteAudioFiles();
    listenForSettingChanges("song-location", [](std::filesystem::path value)
                            {folder = value.string(); log::info("Setting Changed {}", value.string()); });
}

void showSSMenu()
{
    auto sb = GJSongBrowser::create();
    CCDirector::sharedDirector()->getRunningScene()->addChild(sb);
    sb->showLayer(true);
    sb->setZOrder(CCDirector::sharedDirector()->getRunningScene()->getHighestChildZ() + 1);
}

class $modify(GJSongBrowser)
{
    void exitLayer(CCObject *p0)
    {
        log::debug("GJSongBrowser::exitLayer");
        if (FMODAudioEngine::get()->isMusicPlaying(0) && MenuLayer::get() != nullptr)
        {
            log::debug("{}", music0);
            log::debug("{}", getFilenameWithoutExtension(music0));
            if (getFilenameWithoutExtension(music0) != "menuLoop")
            {
                GameManager::get()->m_customMenuSongID = std::stoi(getFilenameWithoutExtension(music0));
            }
            else
            {
                GameManager::get()->m_customMenuSongID = 0;
            }
        }
        if (LevelEditorLayer::get() == nullptr)
        {
            int timeMS = FMODAudioEngine::get()->getMusicTimeMS(0);
            GJSongBrowser::exitLayer(p0);
            FMODAudioEngine::get()->playMusic(GameManager::get()->m_customMenuSongID ? p->pathForSong(GameManager::get()->m_customMenuSongID) : "menuLoop.mp3", true, timeMS != 0 ? 0 : 0.5, 0);
            FMODAudioEngine::get()->setMusicTimeMS(timeMS, true, 0);
        }
        else
        {
            log::info("In Editor, Skipping");
            GJSongBrowser::exitLayer(p0);
        }
    };

};

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

class $modify(OptionsLayer)
{
    void onSoundtracks(CCObject *sender)
    {
        showSSMenu();
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
        // geode::log::debug("pathForSong {}", fullPath);
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
        // geode::log::debug("pathForSFX {}", fullPath);
        return gd::string(fullPath);
    }

    static MusicDownloadManager *sharedState()
    {

        auto p = MusicDownloadManager::sharedState();
        if (!p->m_resourceSfxUnorderedSet.empty())
        {
            p->m_resourceSfxUnorderedSet.clear();
            p->m_resourceSongUnorderedSet.clear();
            log::info("Cleared Resource IDS");
        }
        return p;
    }

    void downloadCustomSong(int p0) {
        log::info("{}",p0);
        this->getSongInfo(p0,false);
        MusicDownloadManager::downloadCustomSong(p0);
    }
};

class $modify(FMODAudioEngine) {
    void playMusic(gd::string path, bool shouldLoop, float fadeInTime, int channel) {
        music0 = path;
        FMODAudioEngine::playMusic(path, shouldLoop, fadeInTime, channel);
    }
};