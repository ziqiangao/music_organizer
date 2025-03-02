#include "songItem.hpp"
#include <sstream>  // For stringstream
#include <string>   // For string operations
#include <algorithm> // For trimming
#include <iterator>  // For joining elements
#include <cctype>

// Constructor that takes individual fields
songItem::songItem(int id, const std::string& name, const std::deque<std::string> artists)
    : ID(id), Name(name), Artists(artists) {}

// Constructor that takes a SongInfoObject
songItem::songItem(const SongInfoObject& info)
    : ID(info.m_songID), Name(info.m_songName) {
    // Add the main artist
    Artists.push_back(info.m_artistName);

    // Split the m_extraArtistNames string by commas and add each to the deque
    std::stringstream ss(info.m_extraArtistNames);
    std::string artist;

    while (std::getline(ss, artist, ',')) {
        // Trim any extra spaces (optional, based on your data format)
        artist = std::string(artist.begin(), std::find_if(artist.rbegin(), artist.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base());

        if (!artist.empty() && !std::all_of(artist.begin(), artist.end(), ::isdigit)) {
            Artists.push_back(artist);
        }
    }
}

// Member function to convert to a string with format: {artists} - {name}, ID: {id}
std::string songItem::toString() const {
    std::ostringstream oss;
    
    // Join all artists in the deque into a single string
    for (auto it = Artists.begin(); it != Artists.end(); ++it) {
        if (it != Artists.begin()) {
            oss << ", ";  // Add a comma separator between artists
        }
        oss << *it;  // Append the artist name
    }
    
    // Format the string as {artists} - {name}, ID: {id}
    oss << " - " << Name << ", ID: " << ID;
    
    return oss.str();  // Return the formatted string
}

std::string songItem::ArtistsAsString() const {
    std::ostringstream oss;
    
    // Join all artists in the deque into a single string
    for (auto it = Artists.begin(); it != Artists.end(); ++it) {
        if (it != Artists.begin()) {
            oss << ", ";  // Add a comma separator between artists
        }
        oss << *it;  // Append the artist name
    }
    return oss.str();  // Return the formatted string
}