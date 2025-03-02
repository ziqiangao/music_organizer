#ifndef SONGITEM_HPP
#define SONGITEM_HPP

#include <string>
#include <deque>
#include <Geode/binding/SongInfoObject.hpp>
#include <sstream>  // For stringstream

struct songItem {
    int ID;
    std::string Name;
    std::deque<std::string> Artists;  // Now a deque of artists

    // Constructor that takes individual fields
    songItem(int id, const std::string& name, const std::deque<std::string> artists);

    // Constructor that takes a SongInfoObject
    songItem(const SongInfoObject& info);

    // Member function to convert to a string with format: {artists} - {name}, ID: {id}
    std::string toString() const;
    std::string ArtistsAsString() const;
};

#endif // SONGITEM_HPP
