#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <array>
#include "json.hpp"
using json = nlohmann::json;


class RoomData {
private:
    int room_id_;
    int floor_;
    int room_type_;
    int room_left_id_;
    int room_right_id_;
    int room_up_id_;
    int room_down_id_;
    bool visited_;
    bool is_first_room_;
    bool is_last_room_;
public:
    RoomData(int room_id, int floor, int room_type, int left_id, int right_id, int up_id, int down_id, bool is_first, bool is_last):
        room_id_(room_id), floor_(floor), room_type_(room_type), room_left_id_(left_id), room_right_id_(right_id), room_up_id_(up_id), 
        room_down_id_(down_id), visited_(false), is_first_room_(is_first), is_last_room_(is_last){}
};


class DungeonInitializer {
private:
    std::vector<RoomData> first_rooms_;// for player to teleport in when going to next floor
    std::vector<RoomData> all_rooms_;  // for linking rooms together
public:
    void createDungeoun(){
        std::ifstream in_file("room.json");
        if (!in_file.is_open()) {
            std::cerr << "Не удалось открыть room.json" << std::endl;
            return;
        }
        json j;
        in_file >> j;
        for (const auto& room_data : j) {
            RoomData room(room_data.value("roomid", 0), room_data.value("floor", 0), room_data.value("room_type", 0),
                          room_data.value("left_id", 0), room_data.value("right_id", 0), room_data.value("up_id", 0), 
                          room_data.value("down_id", 0), room_data.value("is_first", false), room_data.value("is_last", false));
            if (room_data.value("is_first", false)) {
                first_rooms_.push_back(room);
            }
            all_rooms_.push_back(room);
        }
    }
    const std::vector<RoomData>& getAllRooms() {
        return all_rooms_;
    }
    const std::vector<RoomData>& getFirstRooms() {
        return first_rooms_;
    }
};


class PlayerData {
private:
    std::array<bool, 26> letter_inventory_;
    std::vector<int> scroll_inventory_;
    RoomData* current_room_;
    int money_;
public:
    PlayerData(const std::array<bool, 26>& letter_inventory, const std::vector<int>& scroll_inventory, RoomData* current_room, int money):
        letter_inventory_(letter_inventory), scroll_inventory_(scroll_inventory), current_room_(current_room), money_(money){}
};


class EntityData {
private:
    int entity_id_;
    int hp_;
    bool evade_;
    int periodic_hp_change_;
    int max_hp_;
    std::string name_;
    std::vector<int> attack_pool_;
    static std::vector<Word*> words_;
public:
    EntityData(int entity_id, int max_hp, const std::string& name, const std::vector<int>& attack_pool) :
        entity_id_(entity_id), hp_(max_hp), evade_(false), periodic_hp_change_(0), max_hp_(max_hp), name_(name), attack_pool_(attack_pool){}
};


class AllEntitysInitializer {
private:
    std::vector<EntityData> entitys_;
public:
    void loadEntitys() {
        std::ifstream in_file("enemys.json");
        if (!in_file.is_open()) {
            std::cerr << "Не удалось открыть enemys.json" << std::endl;
            return;
        }
        json j;
        in_file >> j;
        for (const auto& entity_data : j) {
            EntityData entity(entity_data.value("entity_id", 0), entity_data.value("max_hp", 0), entity_data.value("name", "entity"),
                entity_data.value("attack_pool", std::vector<int>{}));
            entitys_.push_back(entity);
        }
    }
    const std::vector<EntityData>& getEntitys(){
        return entitys_;
    }
};

