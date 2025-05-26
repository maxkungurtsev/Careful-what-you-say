#pragma once
#include <vector>
#include <algorithm>
using namespace std;
class Player;// forward declaration
class RoomObserver {
public:
    virtual void update(Player* player) = 0;
};
class RoomSubject {
public:
    void registerRoomObserver(RoomObserver* observer) {
        roomObservers_.push_back(observer);
    }
    void removeRoomObserver(RoomObserver* observer) {
        roomObservers_.erase(std::remove(roomObservers_.begin(), roomObservers_.end(), observer), roomObservers_.end());
    }
    void Roomnotify(Player* player) {
        for (auto* obs : roomObservers_) {
            obs->update(player);
        }
    }
private:
    std::vector<RoomObserver*> roomObservers_;
};
// specifically for battles
class BattleObserver {
public:
    virtual void update(int wordId) = 0;
};
class BattleSubject {
public:
    void registerBattleObserver(BattleObserver* observer) {
        battleObservers_.push_back(observer);
    }
    void removeBattleObserver(BattleObserver* observer) {
        battleObservers_.erase(std::remove(battleObservers_.begin(), battleObservers_.end(), observer),battleObservers_.end());
    }
    void Battlenotify(int wordId, int target) {
        if (target >= 0 and target <= static_cast<int>(battleObservers_.size())) {
            battleObservers_[target]->update(wordId);
        }else {
            cout << " target(" << target << ")<0 or observers list size(" << battleObservers_.size() << ")<0" << '\n';
        }
    }
private:
    std::vector<BattleObserver*> battleObservers_;
};
