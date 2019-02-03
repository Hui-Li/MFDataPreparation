#ifndef ITEM_H
#define ITEM_H

#include "../util/Base.h"

class Item {
public:
    mutex locker;
    int itemID;
    vector<int> raterIDs;
    unordered_map<int, string> ratings;

    Item() {}
};

#endif //ITEM_H
