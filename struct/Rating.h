#ifndef RATING_H
#define RATING_H

#include "../util/Base.h"

class Rating {
public:
    int userID;
    int itemID;
    string rating;

    Rating(int userID, int itemID, string rating):userID(userID),itemID(itemID),rating(rating) {}
    bool operator==(const Rating &b) const {
        if ((userID == b.userID) && (itemID == b.itemID)) {
            return true;
        } else {
            return false;
        }
    }

    bool operator>(const Rating &b) const {
        if (userID > b.userID) {
            return true;
        } else if (userID < b.userID) {
            return false;
        } else {
            return (itemID > b.itemID);
        }
    }

    bool operator<(const Rating &b) const {
        if (userID < b.userID) {
            return true;
        } else if (userID > b.userID) {
            return false;
        } else {
            return (itemID < b.itemID);
        }
    }
};

#endif //RATING_H
