#include "songListMan.hpp"
#include "songItem.hpp"
#include <Geode/binding/SongInfoObject.hpp>
#include <fstream>
#include <km7dev.server_api/include/ServerAPIEvents.hpp>
#include "MusicOrganizer.h"

using namespace geode::prelude;

std::string escapeJsonString(const std::string& str) {
    std::string escaped = str;
    size_t pos = 0;
    while ((pos = escaped.find_first_of("\"\\", pos)) != std::string::npos) {
        if (escaped[pos] == '"') {
            escaped.replace(pos, 1, "\\\"");
        } else if (escaped[pos] == '\\') {
            escaped.replace(pos, 1, "\\\\");
        }
        pos += 2; // Move past the escaped character
    }
    return escaped;
}


/*
* Gets The Current Domanin using km7dev's Server API
*/
gd::string songListMan::getDomain()
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

std::ofstream songListMan::openFile(std::string File) {
    std::string fullPath = std::string(listLoc);
    if (!fullPath.empty() && (fullPath.back() == '\\' || fullPath.back() == '/'))
    {
        fullPath.pop_back();
    }

    fullPath.append("/").append(File);

    std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

    std::string filePath = fullPath; // Change path as needed
    log::info("Opening File: {}",fullPath);
    std::ofstream outFile(filePath);


    if (!outFile) {
        log::error("Failed to open file for writing: {}", fullPath);
    }

    return outFile;
 }

void songListMan::Save(const std::vector<songItem>& songs) {
    auto fmt = std::string(listfmt);

    if (!bool(exportlist)) {
        log::info("Skipping Save, User does not have it enabled");
        return;
    }

    if (fmt == "M3U") {
        log::info("Saving List As {}",fmt);
        SaveM3U(songs);
    } else if (fmt == "TXT")
    {
        log::info("Saving List As {}",fmt);
        SaveTXT(songs);
    } else if (fmt == "JSON")
    {
        log::info("Saving List As {}",fmt);
        SaveJSON(songs);
    } else if (fmt == "HTML")
    {
        log::info("Saving List As {}",fmt);
        SaveHTML(songs);
    } else {
        log::warn("Invaild File Type");
    }
    
}

std::vector<songItem> songListMan::compileIntoVector(cocos2d::CCArray* List) {
    std::vector<songItem> SongList;
    log::info("Compiling Songs To Array");
    for (int i = 0; i < List->count();i++) {
        SongList.push_back(songItem(*static_cast<SongInfoObject*>(List->objectAtIndex(i))));
    };
    return SongList;
}

void songListMan::SaveTXT(const std::vector<songItem>& songs)
{
    std::string filePath = std::string(std::string(listName) + "@" + std::string(getDomain())).append(".txt");
    auto outFile = openFile(filePath);

    if (!outFile) {
        log::error("Failed to open file for writing: {}", filePath);
        return;
    }

    log::info("Saving song list to file: {}", filePath);

    for (const auto& song : songs) {
        outFile << song.toString() << std::endl;
    }

    outFile.close();
    log::info("Save complete.");
}

void songListMan::SaveJSON(const std::vector<songItem>& songs) {
    std::string filePath = std::string(std::string(listName) + "@" + std::string(getDomain())).append(".json");
    auto outFile = openFile(filePath);

    if (!outFile) {
        log::error("Failed to open file for writing: {}", filePath);
        return;
    }

    log::info("Saving song list to file: {}", filePath);
    outFile << "["; 
    for (size_t i = 0; i < songs.size(); ++i) {
        const auto& song = songs[i];
        outFile << "{\"Artists\":\"";  // Use double quotes inside the string
        outFile << escapeJsonString(song.ArtistsAsString()) << "\",\"Name\":\""; 
        outFile << escapeJsonString(song.Name) << "\",\"ID\":\""; 
        outFile << song.ID << "\",\"Path\":\""; 
        outFile << std::string(MusicDownloadManager::sharedState()->pathForSong(song.ID)); 
        outFile << "\"}";
    
        // Check if it's not the last item to append a comma
        if (i != songs.size() - 1) {
            outFile << ",";
        }
    }
    
    outFile << "]"; // Close the array
    
    

    outFile.close();
    log::info("Save complete.");
}

void songListMan::SaveM3U(const std::vector<songItem>& songs) {
    std::string filePath = std::string(std::string(listName) + "@" + std::string(getDomain())).append(".m3u");
    auto outFile = openFile(filePath);

    if (!outFile) {
        log::error("Failed to open file for writing: {}", filePath);
        return;
    }

    log::info("Saving song list to file: {}", filePath);

    outFile << "#EXTM3U" << std::endl;
    for (const auto& song : songs) {
        outFile << "#EXTINF:-1,";
        outFile << song.toString() << std::endl;
        outFile << "file:///" << std::string(MusicDownloadManager::sharedState()->pathForSong(song.ID)) << std::endl << std::endl;
    }

    outFile.close();
    log::info("Save complete.");
}

void songListMan::SaveHTML(const std::vector<songItem>& songs) {
    std::string filePath = std::string(std::string(listName) + "@" + std::string(getDomain())).append(".html");
    auto outFile = openFile(filePath);

    if (!outFile) {
        log::error("Failed to open file for writing: {}", filePath);
        return;
    }

    log::info("Saving song list to file: {}", filePath);


    outFile << "<!doctypehtml><html lang=\"en\"><meta charset=\"UTF-8\"><meta content=\"width=device-width,initial-scale=1\"name=\"viewport\"><title>Playlist</title><style>body{font-family:Arial,sans-serif;margin:20px;background:linear-gradient(to bottom,#1e3c72,#2a5298);color:#fff}.playlist{display:grid;grid-template-columns:repeat(auto-fit,minmax(500px,2fr));gap:10px;margin:auto}.song{border-bottom:2px solid gold;padding:15px;background:rgba(255,255,255,.2);color:#fff;border-radius:5px}.song h3,.song p{margin:5px 0;font-size:14px}audio{width:100%;margin-top:5px}.vsc-controller{display:none}</style><div class=\"playlist\"id=\"playlist\"></div><script>const songs=["; 
    for (size_t i = 0; i < songs.size(); ++i) {
        const auto& song = songs[i];
        outFile << "{\"Artists\":\"";  // Use double quotes inside the string
        outFile << escapeJsonString(song.ArtistsAsString()) << "\",\"Name\":\""; 
        outFile << escapeJsonString(song.Name) << "\",\"ID\":\""; 
        outFile << song.ID << "\",\"Path\":\""; 
        outFile << std::string(MusicDownloadManager::sharedState()->pathForSong(song.ID)); 
        outFile << "\"}";
    
        // Check if it's not the last item to append a comma
        if (i != songs.size() - 1) {
            outFile << ",";
        }
    }
    
    outFile << "],playlistContainer=document.getElementById(\"playlist\");songs.forEach((e=>{const n=document.createElement(\"div\");n.classList.add(\"song\"),n.innerHTML=`\\\n        <h3>${e.Name}</h3>\\\n        <p>By: ${e.Artists} | ID: ${e.ID}</p>\\\n        <audio controls>\\\n          <source src=\"${e.Path}\" type=\"audio/ogg\">\\\n          Your browser does not support the audio element.\\\n        </audio>\\\n      `,playlistContainer.appendChild(n)}));const audios=document.querySelectorAll(\"audio\");audios.forEach((e=>{e.addEventListener(\"play\",(function(){e.playbackRate=1,audios.forEach((n=>{n!==e&&(n.pause(),n.playbackRate=1,n.currentTime=0)}))}))}));</script>"; // Close the array
    
    

    outFile.close();
    log::info("Save complete.");
}