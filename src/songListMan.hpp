#ifndef SONGLISTMAN_HPP
#define SONGLISTMAN_HPP

#include "songItem.hpp"
#include <vector>
#include <string>



class songListMan
{
public:
    // Delete constructor to prevent instantiation
    songListMan() = delete;

    // Static Save Function
    static void Save(const std::vector<songItem>& songs);
    static void SaveTXT(const std::vector<songItem>& songs);
    static void SaveJSON(const std::vector<songItem>& songs);
    static void SaveM3U(const std::vector<songItem>& songs);
    static void SaveHTML(const std::vector<songItem>& songs);
    static std::vector<songItem> compileIntoVector(cocos2d::CCArray* List);
    static std::string getDomain();
    static std::ofstream openFile(std::string File);
};

#endif // SONGLISTMAN_HPP
