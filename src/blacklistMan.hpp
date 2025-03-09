#include <deque>


class blacklistMan {
public:
    static std::unordered_set<int> blacklist; // Declare the static deque

    static void loadblacklist();
    static void saveBlacklist();
    static bool isblacklisted(int n);
    static void setblacklist(int n, bool bl);
};
