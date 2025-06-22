#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <array>

#include "json.hpp"
using json = nlohmann::json;


class LootData {
public:
    const int loot_type_;
    const int amount_or_id_;
    const int chance_;
    const int price_;
    LootData(const int loot_type, const int amount_or_id, const int chance, const int price):
        loot_type_(loot_type), amount_or_id_(amount_or_id), chance_(chance), price_(price){}
};

class LootManager {
private:
    std::vector<std::vector<LootData>> loot_table_;
public:
    void loadLoot() {
        std::ifstream in_file("loot.json");
        if (!in_file.is_open()) {
            std::cerr << "Не удалось открыть room.json" << std::endl;
            return;
        }
        json j;
        in_file >> j;
        for (const auto& floor_data : j) {

            std::vector<LootData> floor;
            for (const auto& loot_data : floor_data["loot_list"]) {
                LootData loot(loot_data.value("loot_type", 0), loot_data.value("amount_or_id", 0), loot_data.value("chance", 0), loot_data.value("price", 0));
                floor.push_back(loot);
            }
            loot_table_.push_back(floor);
        }
    }
    std::vector<std::vector<LootData>>* getLootTable() {
        return &loot_table_;
    }
};


class RoomData {// class holds data about 1 room
private:
    bool visited_;
public:
    const int room_id_;// all const and public since it won't change and i'd like to avoid making 10 getters but still keep encapsulation
    const int floor_;  // "visited_" has to change at some point so it's an exception
    const int room_type_;
    const int room_left_id_;
    const int room_right_id_;
    const int room_up_id_;
    const int room_down_id_;
    const bool is_first_room_;
    const bool is_last_room_;
    RoomData(int room_id, int floor, int room_type, int left_id, int right_id, int up_id, int down_id, bool is_first, bool is_last):
        room_id_(room_id), floor_(floor), room_type_(room_type), room_left_id_(left_id), room_right_id_(right_id), room_up_id_(up_id), 
        room_down_id_(down_id), visited_(false), is_first_room_(is_first), is_last_room_(is_last){}
    void setVisited() {
        visited_ = true;
    }
    int getVisited() {
        return visited_;
    }
};

class DungeonManager {
private:
    std::vector<RoomData> first_rooms_;// for player to teleport in when going to next floor
    std::vector<RoomData> all_rooms_;  // for linking rooms together
public:
    void loadDungeon(){
        std::ifstream in_file("room.json");
        if (!in_file.is_open()) {
            std::cerr << "Не удалось открыть room.json" << std::endl;
            return;
        }
        json j;
        in_file >> j;
        for (const auto& room_data : j) {

            RoomData room(room_data.value("roomid", 0), room_data.value("floor", 0), room_data.value("room_type", 0),
                          room_data.value("room_left_id", -1), room_data.value("room_right_id", -1), room_data.value("room_up_id", -1), 
                          room_data.value("room_down_id", -1), room_data.value("is_first", false), room_data.value("is_last", false));
            if (room_data.value("is_first", false)) {
                first_rooms_.push_back(room);
            }
            all_rooms_.push_back(room);
        }
        in_file.close();
    }
    std::vector<RoomData>* getAllRooms() {
        return &all_rooms_;
    }
    std::vector<RoomData>* getFirstRooms() {
        return &first_rooms_;
    }
};


// get ready this class is... "unique"
class PlayerDataAndManager {
private:// due to encapsulation fields must be private with setters and getters 
    std::array<bool, 26> letter_inventory_;// in that case the most of PlayerManager functionality would be in PlayerData 
    std::vector<int> scroll_inventory_;//so it makes no sence to make "PlayerManager" that only initializes player
    RoomData* current_room_;           //and "PlayerData" that MANAGES players data half of the time
    int money_;
public:
    PlayerDataAndManager(std::vector<RoomData>& rooms){
        std::ifstream in_file("starter_pack.json");
        if (!in_file.is_open()) {
            std::cerr << "Не удалось открыть starter_pack.json" << std::endl;
            return;
        }
        json j;
        in_file >> j;
        //copy first 26 elements from j.value... to letterinventory its cause json library doesn't work with std::array
        std::copy_n(j.value("letters", std::vector<bool>(26, false)).begin(), 26, letter_inventory_.begin());
        // but it works with vectors
        scroll_inventory_ = j.value("scrolls", std::vector<int>{});
        money_ = j.value("money", 0);
        current_room_ = &rooms[j.value("starter_room", 0)];
        in_file.close();
    }
    std::array<bool, 26>& getLetterInventory(){
        return letter_inventory_;        
    }
    std::vector<int>& getScrollInventory() {
        return scroll_inventory_;
    }
    void setLetterInventory(const std::vector<int>& new_scroll_inventory) {
        scroll_inventory_ = new_scroll_inventory;
    }
    RoomData* getCurrentRoom() {
        return current_room_;
    }
    void setCurrentRoom(RoomData& new_room) {
        current_room_ = &new_room;
    }
    void setMoney(int money) {
        money_ = money;
        if (money_ < 0) {
            money = 0;
        }
    }
    int getMoney() {
        return money_;
    }
};


class WordData {
public:// same situation as with RoomData
    const int word_id_;
    const std::string word_name_;
    const int target_;
    const int dmg_;
    const int periodic_dmg_;
    const bool add_evade_;
    const std::string description_;
    const std::string enemy_description_;
    WordData(int word_id, std::string word_name, int target, int dmg, int periodic_dmg,
        std::string description, std::string enemy_description, bool add_evade) :
        word_id_(word_id), word_name_(word_name), target_(target), dmg_(dmg), periodic_dmg_(periodic_dmg),
        description_(description), enemy_description_(enemy_description), add_evade_(add_evade){}
    
};
class WordsManager {
private:
    std::vector<WordData> words_;
public:
    void loadWords() {
        std::ifstream in_file;
        in_file.open("words.json");
        if (!in_file.is_open()) {
            std::cerr << "Не удалось открыть words.json" << std::endl;
            return;
        }
        json j;
        in_file >> j;
        for (const auto& word_data : j) {
            WordData word(word_data.value("word_id", 0),
                word_data.value("word_name", "heal"),
                word_data.value("target", 0),
                word_data.value("dmg", -2),
                word_data.value("periodic_dmg", 0),
                word_data.value("description", "you healed yourself for 2 hp!"),
                word_data.value("enemy_description", "healed themselves for 2 hp!"),
                word_data.value("add_evade", false));
            words_.push_back(word);
        }
        in_file.close();
    }
    std::vector<WordData>* getWords() {
        return &words_;
    }
};


class EntityData {
private:
    int hp_;
    bool evade_;
    int periodic_hp_change_;
public:
    const int max_hp_;
    const std::vector<int> attack_pool_;
    const std::string name_;
    const int entity_id_;
    const int chance_;
    EntityData(int entity_id, int max_hp, const std::string& name, const std::vector<int>& attack_pool, int chance) :
        entity_id_(entity_id), hp_(max_hp), evade_(false), periodic_hp_change_(0), max_hp_(max_hp), name_(name),
        attack_pool_(attack_pool), chance_(chance) {}
    void setHp(int new_hp){
        hp_ = new_hp;
    }
    int getHp(){
        return hp_; 
    }
    void setEvade(bool new_evade) {
        evade_ = new_evade;
    }
    bool getEvade() { 
        return evade_; 
    }
    void setPeriodicHpChange(int new_periodic_hp_change) {
        periodic_hp_change_ = new_periodic_hp_change;
    }
    int getPeriodicHpChange() { 
        return periodic_hp_change_; 
    }
};

class EntitysManager {
private:
    std::vector<std::vector<EntityData>> entitys_;
public:
    void loadEntitys() {
        std::ifstream in_file("entitys.json");
        if (!in_file.is_open()) { 
            std::cerr << "Не удалось открыть room.json" << std::endl;
            return;
        }
        json j;
        in_file >> j;
        for (const auto& floor_data : j) {
            std::vector<EntityData> floor;
            for (const auto& entity_data : floor_data["spawn_list"]) {
                EntityData entity = EntityData(entity_data.value("entity_id", 0), entity_data.value("max_hp", 0),
                                               entity_data.value("name", ""), entity_data.value("attack_pool",std::vector<int>{}), entity_data.value("chance",1));
                floor.push_back(entity);
            }
            entitys_.push_back(floor);
        }
    }
    const std::vector<std::vector<EntityData>>& getEntitys(){
        return entitys_;
    }
};