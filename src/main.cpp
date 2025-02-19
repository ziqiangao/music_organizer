#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>
#include <Geode/binding/MusicDownloadManager.hpp>
#include <km7dev.server_api/include/ServerAPIEvents.hpp>
#include <filesystem>  // C++17 for filesystem handling

namespace fs = std::filesystem;

using namespace geode::prelude;

auto p = MusicDownloadManager::sharedState();
gd::string folder = p->pathForSongFolder(0); // Folder already contains the correct base path

gd::string getDomain() {
    // Get the server URL
    std::string url = ServerAPIEvents::getCurrentServer().url;

    if (url == "NONE_REGISTERED") {
        url = ServerAPIEvents::getBaseUrl();
    };

    // Variable to hold the domain
    std::string domain;

    // Check if the URL contains "://"
    size_t domainStart = url.find("://");

    if (domainStart != std::string::npos) {
        // If it has "://", set domainStart to after "://"
        domainStart += 3;
    } else {
        // If there's no "://", start from the beginning of the string
        domainStart = 0;
    }

    // Find the first slash after the domain (if it exists)
    size_t domainEnd = url.find('/', domainStart);
    
    if (domainEnd != std::string::npos) {
        // Extract the domain
        domain = url.substr(domainStart, domainEnd - domainStart);
    } else {
        // If no slash is found, the domain is the rest of the string
        domain = url.substr(domainStart);
    }

    return domain;
}

void createDirectoryIfNeeded(const std::string& path) {
    std::string directoryPath = path.substr(0, path.find_last_of('/'));
    // Check if the directory exists
    if (!fs::exists(directoryPath)) {
        // If it doesn't exist, create the directory
        fs::create_directories(directoryPath); // This creates all intermediate directories as needed
        geode::log::info("Created directory: {}", directoryPath);
    }
}
/*
class $modify(MenuLayer) {
    void onMoreGames(CCObject* target) {
        std::string deb = p->pathForSong(1244); // Assuming the song ID is 1244
        log::info("Song path: {}", deb);  // Logging the full song path

        // Show the domain in the alert
        FLAlertLayer::create(
            "Debug",
            deb,  // Show the domain
            "OK"
        )->show(); 
    }
};
*/
class $modify(MusicDownloadManager) {
    gd::string pathForSong(int p0) {
        geode::log::debug("pathForSong");
    
        // Ensure no extra slash at the end of folder
        std::string fullPath = std::string(folder);
        if (fullPath.back() == '\\' || fullPath.back() == '/') {
            fullPath.pop_back();  // Remove the last slash if it exists
        }
        
        // Corrected concatenation
        fullPath += gd::string("/") + getDomain() + "/Songs/" + gd::string(std::to_string(p0)) + ".ogg";
    
        // Replace backslashes with forward slashes for consistency
        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
    
        // Create the directory up to `Songs/`
        std::string directoryPath = fullPath.substr(0, fullPath.find_last_of('/'));
        createDirectoryIfNeeded(directoryPath);
    
        return gd::string(fullPath);
    }
    

    gd::string pathForSFX(int p0) {
        geode::log::debug("pathForSFX");
    
        std::string fullPath = std::string(folder);
        if (fullPath.back() == '\\' || fullPath.back() == '/') {
            fullPath.pop_back();
        }
    
        fullPath += gd::string("/") + getDomain() + "/SFX/" + gd::string(std::to_string(p0)) + ".ogg";
    
        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
    
        std::string directoryPath = fullPath.substr(0, fullPath.find_last_of('/'));
        createDirectoryIfNeeded(directoryPath);
    
        return gd::string(fullPath);
    }
    
};
