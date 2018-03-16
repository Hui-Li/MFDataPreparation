#ifndef USER_H
#define USER_H

#include "../util/Base.h"

class User {
public:
    mutex locker;
    int userID;
    vector<int> ratedItemIDs;
    unordered_map<int, float> ratings;

    User() {}
};

#endif //USER_H
