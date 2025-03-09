#include "blacklistMan.hpp"
#include <Geode/modify/CustomSongWidget.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <Geode/utils/cocos.hpp>
#include "songListMan.hpp"

using namespace geode::prelude;
namespace fs = std::filesystem;


// Define the static member outside the class
std::unordered_set<int> blacklistMan::blacklist; 

void blacklistMan::loadblacklist() {
    log::info("blacklistMan::loadblacklist");
    blacklist = Mod::get()->getSavedValue<std::unordered_set<int>>(std::string("blacklist@").append(songListMan::getDomain()),{});
}

void blacklistMan::saveBlacklist() {
    log::info("{}",blacklistMan::blacklist);
    Mod::get()->setSavedValue(std::string("blacklist@").append(songListMan::getDomain()),blacklistMan::blacklist);
}

bool blacklistMan::isblacklisted(int number) {
    return blacklist.find(number) != blacklist.end();  // Using find to check existence in unordered_set
}

void blacklistMan::setblacklist(int number, bool bl) {
    if (bl) {
        // Add the number to the blacklist if it's not already present
        auto result = blacklist.insert(number);
        if (result.second) {  // Check if the insertion was successful (i.e., the number wasn't already in the set)
            log::info("Added {} to the blacklist.", number);
        } else {
            log::info("{} is already in the blacklist.", number);
        }
    } else {
        // Remove the number from the blacklist
        size_t erased = blacklist.erase(number);
        if (erased > 0) {
            log::info("Removed {} from the blacklist.", number);
        } else {
            log::info("{} was not found in the blacklist.", number);
        }
    }
}


class $modify(MyCustomSongWidget, CustomSongWidget)
{
    bool init(SongInfoObject *songInfo, CustomSongDelegate *songDelegate, bool showSongSelect, bool showPlayMusic, bool showDownload, bool isRobtopSong, bool unkBool, bool isMusicLibrary, int unk)
    {
        if (!CustomSongWidget::init(songInfo, songDelegate, showSongSelect, showPlayMusic, showDownload, isRobtopSong, unkBool, isMusicLibrary, unk))
            return false;

        log::debug("bool init(songInfo, songDelegate, bool showSongSelect {}, bool showPlayMusic {}, bool showDownload {}, bool isRobtopSong {}, bool unkBool {}, bool isMusicLibrary {}, int unk {})", showSongSelect, showPlayMusic, showDownload, isRobtopSong, unkBool, isMusicLibrary, unk);
        auto blacklistbtn = ButtonSprite::create("Blacklist", "bigFont.fnt", "GJ_button_01.png");
        auto whitelistbtn = ButtonSprite::create("Whitelist", "bigFont.fnt", "GJ_button_06.png");

        auto blbtncon = CCMenuItemSpriteExtra::create(blacklistbtn, this, menu_selector(MyCustomSongWidget::onBlacklistButton));
        auto wlbtncon = CCMenuItemSpriteExtra::create(whitelistbtn, this, menu_selector(MyCustomSongWidget::onWhitelistButton));


        blbtncon->setScale(.4);
        wlbtncon->setScale(.4);
        blbtncon->setID("blacklist-button"_spr);
        wlbtncon->setID("whitelist-button"_spr);

        blbtncon->m_baseScale = 0.4;
        wlbtncon->m_baseScale = 0.4;
        blbtncon->setSizeMult(1.04);
        wlbtncon->setSizeMult(1.04);

        blbtncon->setPosition(isMusicLibrary ? -285 : -385, isMusicLibrary ? -170 : -175);
        wlbtncon->setPosition(isMusicLibrary ? -285 : -385, isMusicLibrary ? -170 : -175);

        this->getChildByID("buttons-menu")->addChild(blbtncon);
        this->getChildByID("buttons-menu")->addChild(wlbtncon);
        updateBlacklistButton();
        return true;
    };
    void updateBlacklistButton()
    {
        bool isBlacklisted = blacklistMan::isblacklisted(m_customSongID);
        bool isdownloaded = MusicDownloadManager::sharedState()->isSongDownloaded(this->m_customSongID);
        this->getChildByID("buttons-menu")->getChildByID("blacklist-button"_spr)->setVisible(isdownloaded && !isBlacklisted);
        this->getChildByID("buttons-menu")->getChildByID("whitelist-button"_spr)->setVisible(isdownloaded && isBlacklisted);
    }
    void onBlacklistButton(CCObject *sender)
    {
        setBlacklist(true);
    }
    void onWhitelistButton(CCObject *sender)
    {
        setBlacklist(false);
    }
    void setBlacklist(bool blacklist)
    {
        blacklistMan::setblacklist(m_customSongID, blacklist);
        updateBlacklistButton();
    }
    void downloadSongFinished(int p0) {
        updateBlacklistButton();
        CustomSongWidget::downloadSongFinished(p0);
    }
    void onPlayback(CCObject* sender) {
        updateBlacklistButton();
        CustomSongWidget::onPlayback(sender);
    }
    void deleteSong() {
        setBlacklist(false);
        CustomSongWidget::deleteSong();
        this->getChildByID("buttons-menu")->getChildByID("blacklist-button"_spr)->setVisible(false);
        this->getChildByID("buttons-menu")->getChildByID("whitelist-button"_spr)->setVisible(false);
    }
};