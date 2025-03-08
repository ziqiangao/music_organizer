#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>
#include <Geode/binding/MusicDownloadManager.hpp>
#include <km7dev.server_api/include/ServerAPIEvents.hpp>
#include <filesystem> // C++17 for filesystem handling
#include <Geode/binding/GJSongBrowser.hpp>
#include <Geode/modify/GJSongBrowser.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <Geode/binding/DialogObject.hpp>
#include <Geode/modify/OptionsLayer.hpp>
#include "songItem.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/modify/AppDelegate.hpp>
#include "songListMan.hpp"
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/fmod/fmod.hpp>
#include "MusicOrganizer.h"
#include <Geode/loader/Dirs.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>
#include <Geode/binding/MenuLayer.hpp>

// Namespaces
namespace fs = std::filesystem;
using namespace geode::prelude;

// Vars
auto p = MusicDownloadManager::sharedState();                                                     // Music Download Manager Singleton
gd::string folder = Mod::get()->getSettingValue<std::filesystem::path>("song-location").string(); // Song Location Setting
bool exportlist = Mod::get()->getSettingValue<bool>("exportlist");                                // Export List Setting
std::string listfmt = Mod::get()->getSettingValue<std::string>("listfmt");                        // File Format Of The Exported List Setting
std::string listName = Mod::get()->getSettingValue<std::string>("listname");                      // Name Of Exported List Setting
std::string listLoc = Mod::get()->getSettingValue<std::filesystem::path>("listLocation").string();
gd::string music0 = "";

std::string getFilenameWithoutExtension(const std::string &fullPath)
{
    // Use std::filesystem to get the filename and remove the extension
    std::filesystem::path p(fullPath);
    return p.stem().string(); // Get the filename without the extension
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

void moveAllFolders(const fs::path &from, const fs::path &to)
{
    try
    {
        if (!fs::exists(to))
        {
            fs::create_directories(to);
            geode::log::info("Created destination directory: {}", to.string());
        }

        for (const auto &entry : fs::directory_iterator(from))
        {
            if (fs::is_directory(entry.path()))
            {
                fs::path destination = to / entry.path().filename();
                fs::rename(entry.path(), destination);
                geode::log::info("Moved folder: {} -> {}", entry.path().string(), destination.string());
            }
        }
    }
    catch (const std::exception &e)
    {
        geode::log::warn("Could Not move folders: {}", e.what());
    }
}

$execute
{
    deleteAudioFiles();
    listenForSettingChanges("song-location", [](std::filesystem::path value)
                            {folder = value.string(); log::info("Setting Changed {}", value.string()); });
    listenForSettingChanges("exportlist", [](bool value)
                            { exportlist = value; log::info("Setting Changed {}", value ? "True" : "False"); });
    listenForSettingChanges("listfmt", [](std::string value)
                            { listfmt = value; log::info("Setting Changed {}", value); });
    listenForSettingChanges("listname", [](std::string value)
                            { listName = value; log::info("Setting Changed {}", value); });
    listenForSettingChanges("listLocation", [](std::filesystem::path value)
                            { listLoc = value.string(); log::info("Setting Changed {}", value.string()); });
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
    }
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

#ifdef DEBUG_MODE
int i = 0;
class $modify(MenuLayer)
{
    void onRobTop(CCObject *sender) // Debug Info
    {
        switch (i)
        {
        case 0:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (Song Path)", "<i000>" + p->pathForSong(0) + "</i>", 2, 1.0f, true, ccWHITE), 2));
            i += 1;
            break;

        case 1:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (SFX Path)", "<i000>" + p->pathForSFX(0) + "</i>", 4, 1.0f, true, ccWHITE), 2));
            i += 1;
            break;
        case 2:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (Settings->Save Songs List)", exportlist ? "True" : "False", 17, 1.0f, true, ccWHITE), 2));
            i += 1;
            break;
        case 3:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (Settings->List Format)", listfmt, 28, 1.0f, true, ccWHITE), 2));
            i += 1;
            break;
        case 4:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (Settings->List Name)", listName, 17, 1.0f, true, ccWHITE), 2));
            i += 1;
            break;
        case 5:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(DialogObject::create("Debug (Song Info Test)", p->getDownloadedSongs()->count() > 0 ? songItem(*static_cast<SongInfoObject *>(p->getDownloadedSongs()->randomObject())).toString() : "Erm, You Don't Seem To have Any Songs Downloaded, <s100> So We Can't Pick a Song</s>", 28, 1.0f, true, ccWHITE), 2));
            i += 1;
            break;
        case 6:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (Menu Song)", GameManager::get()->m_customMenuSongID ? songItem(*static_cast<SongInfoObject *>(p->getSongInfoObject(GameManager::get()->m_customMenuSongID))).toString() : "<cg><s200>No Song Selected</s></c>", 28, 1.0f, true, ccWHITE), 2));
            i += 1;
            break;
        case 7:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (Practice Song)", GameManager::get()->m_customPracticeSongID ? songItem(*static_cast<SongInfoObject *>(p->getSongInfoObject(GameManager::get()->m_customPracticeSongID))).toString() : "<cf><s200>No Song Selected</s></c>", 28, 1.0f, true, ccWHITE), 2));
            i += 1;
            break;

        case 8:
        {
            bool suc = p->m_songObjects->writeToFile("songs.txt");
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Debug (Songs)", suc ? "<cj><s200>Check File songs.txt</s></c>" : "<co><s200>Could Not Write</s></c>", 28, 1.0f, true, ccWHITE), 2));
            i += 1;
            break;
        }
        default:
            CCDirector::sharedDirector()->getRunningScene()->addChild(DialogLayer::create(
                DialogObject::create("Key Master", "What are doing <s200><cr>here!?!?</c></s>", 18, 1.0f, true, ccWHITE), 2));
            i = 0;
            break;
        }
    }
};
#endif

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
        fullPath.append("/").append(songListMan::getDomain().c_str()).append("/Songs/").append(std::to_string(p0)).append(".ogg");

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

        fullPath.append("/").append(songListMan::getDomain().c_str()).append("/SFX/s").append(std::to_string(p0)).append(".ogg");

        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

        createDirectoryIfNeeded(fullPath);
        // geode::log::debug("pathForSFX {}", fullPath);
        return gd::string(fullPath);
    }

    gd::string pathForSFXFolder(int p0)
    {
        gd::string ret = folder;

        return ret;
    }

    gd::string pathForSongFolder(int p0)
    {
        gd::string ret = folder;

        return ret;
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
};

class $modify(AppDelegate)
{
    void trySaveGame(bool p0)
    {
        std::vector<songItem> SongList = songListMan::compileIntoVector(p->getDownloadedSongs());
        songListMan::Save(SongList);
        AppDelegate::trySaveGame(p0);
    }
};

class $modify(FMODAudioEngine)
{
    void playMusic(gd::string path, bool shouldLoop, float fadeInTime, int channel)
    {
        log::debug("music channel: {}", channel);
        if (1)
        {
            music0 = path;
            log::debug("music0: {}", path);
        }
        FMODAudioEngine::playMusic(path, shouldLoop, fadeInTime, channel);
    }
};