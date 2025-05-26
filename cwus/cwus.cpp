#include <iostream>
#include <fstream>// for files
#include <vector> // for arrays
#include <algorithm>
#include "Observer.h"
#include <C:\Users\Asus\source\repos\single_include\nlohmann\json.hpp>
using json = nlohmann::json;
class word {
private:
    int WordId;
    string WordName;
    int dmg;
    int periodic_dmg;
    int target;
    string desc;
    string enemyDesc;
public:
    word(int wordid, string wordname, int Dmg,  int per_dmg, int Target, string description, string enemyDescription) {
        WordId = wordid;
        WordName = wordname;
        dmg = Dmg;
        periodic_dmg = per_dmg;
        target = Target;
        desc = description;
        enemyDesc = enemyDescription;
    }
    int getWordId() const {
        return WordId;
    }
    const std::string& getWordName() const {
        return WordName;
    }
    const std::string& getDescription() const {
        return desc;
    }
    const std::string& getEnemyDescription() const {
        return enemyDesc;
    }
    int getDmg() const {
        return dmg;
    }
    int getPeriodicDmg() const {
        return periodic_dmg;
    }
    int getTarget() const {
        return target;
    }
};
void toLowerCase(std::string& str) {
    for (char& c : str) {
        c = std::tolower(static_cast<unsigned char>(c)); // важно: каст к unsigned char
    }
}
int Ifword(string attack, std::vector<word*>* const words) {
    for (word* Word : *words) {
        if (Word->getWordName() == attack) {
            return Word->getWordId();
        }
    }return -1;
}
bool usable(int attackId, std::vector<word*>* const words, std::array<bool, 26>* const letter_inventory) {
    for (char letter : (*words)[attackId]->getWordName()) {
        if (not((*letter_inventory)[letter - 'a'])) {
            cout << "You don't have letters required to cast this." << '\n';
            return false;
        }
    }
}

class target :public BattleObserver, public BattleSubject{
protected:
    int hp;
    bool evade;
    int periodic_hp_change;
    int maxhp;
public:
    static std::vector<word*> words;
    void take_damage(int damage) {
        hp -= damage;
    }
    virtual void death() = 0;
    virtual void attack() = 0;
    void update(int wordId){
        periodic_hp_change += words[wordId]->getPeriodicDmg();
        take_dmg(words[wordId]->getDmg());
    }
    target() {
        if (words.size() == 0) {
            std::ifstream in_words("words.json");
            if (!in_words) {
                std::cerr << "Cannot open words.json" << std::endl;
                return;
            }
            json j;
            in_words >> j;
            for (const auto& Word : j) {
                word* w = new word(Word["word_id"], Word["word_name"], Word["dmg"], Word["periodic_dmg"], Word["target"], Word["description"], Word["enemydescription"]);
                words.push_back(w);
            }
        }
    }
    void take_dmg(int dmg) {
        hp -= dmg;
    }
    int gethp() {
        return hp;
    }
    int getmaxhp() {
        return maxhp;
    }
    int getperiodicHpChange() {
        return periodic_hp_change;
    }
    void initiate_attack(int wordId) {
        Battlenotify(wordId, words[wordId]->getTarget());
    };
};
std::vector<word*> target::words;
class Room :public RoomObserver,public RoomSubject{
protected:
    int roomid;
    int floor;
    int roomtype;
    Room* room_left = nullptr;
    Room* room_right = nullptr;
    Room* room_up = nullptr;
    Room* room_down = nullptr;
    int room_left_id;
    int room_right_id;
    int room_up_id;
    int room_down_id;
    bool visited;

public:
    static std::vector<Room*> registry;
    Room(const std::string& filename = "room.json") {
        std::ifstream in(filename);
        if (!in.is_open()) {
            std::cerr << "Не удалось открыть " << filename << std::endl;
            return;
        }
        json j;
        in >> j;
        roomid = j[0].value("roomid", 0);
        floor = j[0].value("floor", 0);
        roomtype = j[0].value("roomtype", 0);
        room_left_id = j[0].value("room_left", -1);
        room_right_id = j[0].value("room_right", -1);
        room_up_id = j[0].value("room_up", -1);
        room_down_id = j[0].value("room_down", -1);
        visited = j[0].value("visited", false);
        registry.push_back(this);
    }
    static void link_rooms() {
        for (auto* room : registry) {
            for (auto* other : registry) {
                if (room->room_left_id == other->roomid)   room->room_left = other;
                if (room->room_right_id == other->roomid)  room->room_right = other;
                if (room->room_up_id == other->roomid)     room->room_up = other;
                if (room->room_down_id == other->roomid)   room->room_down = other;
            }
        }
    }
    int get_floor() const {
        return floor;
    }
    const int& get_type() const {
        return roomtype;
    }
    virtual void room_event(Player* player) = 0;
};
std::vector<Room*> Room::registry;


class Player : public target, public RoomSubject{
private:
    std::array<bool, 26> letter_inventory{};  // inventory of letters A-Z
    std::vector<int> scroll_inventory;        // inventory of scroll IDs
    Room* current_room = nullptr;             // pointer to current room
    int money = 0;                            // player's money amount

public:
    bool take_periodicdmg() {
        if (periodic_hp_change != 0) {
            if (periodic_hp_change > 0) {
                cout << " You recieved " << periodic_hp_change << " periodic damdge!"<<'\n';
            }
            else {
                cout << " You recieved " << -periodic_hp_change << " periodic healing!" << '\n';
            }
            hp -= periodic_hp_change;
            periodic_hp_change -= 1;
        }
        return hp > 0;
    }
    void death(){
        std::cout << "Player died" << std::endl;
    }
    void attack(){
        std::cout << "type in 'scrolls', to use scrolls or your word. There are your letters:" << std::endl;
        for (int i = 0; i < 26;i++) {
            if (letter_inventory[i]) {
                cout << static_cast<char>('A' + i) << " ";
            }
        }cout << "your hp:" << hp << "\\" << maxhp << '\n';
        string playerattack;
        cin >> playerattack;
        bool usingscrolls = false;
        if (playerattack == "scrolls") {
            if (scroll_inventory.size() > 0) {
                usingscrolls = true;
                cout << "you have scrolls of:" << '\n';
                    for (int i = 0; i < scroll_inventory.size();i++) {
                        cout << i+1 << ". " << words[scroll_inventory[i]]->getWordName() << '\n';
                    }
                cout<< "type in the word of a scroll you want" << '\n';
            }
            else {
                cout << "You have no scrolls, type in the word you can cast" << '\n';
            }
            cin >> playerattack;
        }
        int attackid = Ifword(playerattack,&words);
        if (attackid>=0) {
            if (usingscrolls) {
                // if using scrolls, check if such one exists
                auto indexx= std::find(scroll_inventory.begin(), scroll_inventory.end(), attackid);
                if (indexx != scroll_inventory.end()) {
                    scroll_inventory.erase(indexx);
                    cout << words[attackid]->getDescription()<<'\n';
                    Battlenotify(attackid, words[attackid]->getTarget());
                }else {
                    cout << "You don't have such scroll" << '\n';
                }
            }else {
                if (usable(attackid, &words, &letter_inventory)) {
                    Battlenotify(attackid, words[attackid]->getTarget());
                }
            }
        }
        else {
            cout << "That's not the word." << '\n';
        }
    }
    void change_room(const std::string& direction) {// for now empty
    }
    void setroom(Room* room) {
        current_room = room;
    }
    Player() {
        std::ifstream in("starterPack.json");
        if (!in.is_open()) {
            std::cerr << "Не удалось открыть starterpack.json" << std::endl;
            return;
        }
        json j;
        in >> j;
        hp = j.value("hp", 20);
        maxhp = hp;
        letter_inventory=j["letters"].get<std::array<bool, 26>>();
        scroll_inventory=j["scrolls"].get<std::vector<int>>();
    }
};
class Enemy:public target{
private:
    int EnemyId;
    vector<int> attacks;
    string name;
    // loot later
public:
    Enemy() {// death и attack
        std::ifstream in("enemy.json");
        if (!in.is_open()) {
            std::cerr << "Не удалось открыть enemy.json" << std::endl;
            return;
        }
        json j;
        in >> j;
        EnemyId = j[0]["enemyId"];
        attacks = j[0]["enemyAttacks"].get<std::vector<int>>();
        name = j[0].value("enemyName","");
        hp = j[0].value("enemyHp", 0);
        maxhp = hp;
    }
    bool take_periodicdmg() {
        if (periodic_hp_change != 0) {
            hp -= periodic_hp_change;
            if (periodic_hp_change > 0) {
                periodic_hp_change -= 1;
                cout << name <<" recieved " << periodic_hp_change << " periodic damdge!" << '\n';
            }
            else {
                periodic_hp_change += 1;
                cout << name << " recieved " << -periodic_hp_change << " periodic healing!" << '\n';
            }
        }
        return hp > 0;
    }
    void attack() {
        int attack = (rand() * attacks.size()) % (attacks.size());
        cout << name << words[attacks[attack]]->getEnemyDescription()<<'\n';
        Battlenotify(attacks[attack], words[attacks[attack]]->getTarget());
    }
    void death() {
        cout << name<<" is f****** dead" << '\n';
    }
    string getname() {
        return name;
    }

};
class Battle {
private:
    Enemy* enemy;
    bool player_turn;
public:
    Battle(Player* player) {
        delete enemy;
        enemy = new Enemy();
        player_turn = true;
        cout << "Battle has started! Your enemy:" << '\n';
        cout << enemy->getname() << "(" << enemy->gethp() << "\\" << enemy->getmaxhp() <<")" << '\n';
        int result =battleloop(player);
        if (not(result)) {
            player->death();
            cout << "you lost" << '\n';
        }else {
            enemy->death();
            cout << "you won" << '\n';
        }
        this->~Battle();
    }
    int battleloop(Player* player) {
        player->registerBattleObserver(player);
        player->registerBattleObserver(enemy);
        enemy->registerBattleObserver(enemy);
        enemy->registerBattleObserver(player);
        while (enemy->gethp()>0 and player->gethp() > 0) {
            if (player_turn) {
                if (player->take_periodicdmg()) {
                    cout << enemy->getname() << " has " << enemy->gethp() << " hp left!" << '\n';
                    player->attack();
                    player_turn = not(player_turn);
                }else {break;}
            }
            else {
                if (enemy->take_periodicdmg()){
                    enemy->attack();
                    player_turn = not(player_turn);
                }
            }
        }
        if (enemy->gethp() > 0) {
            return 0;
        }
        else {
            return 1;
        }
    }
    ~Battle() {
        delete enemy;
    }
};
class Enemy_Room :public Room{
private:
    Battle* battle;
public:
    void update(Player* player) {
        if (not(visited)) {
            room_event(player);
            visited = true;
        }
    }
    void room_event(Player* player) {
        battle = new Battle(player);
    }
};

int main() {
    Player player;
    Enemy_Room room;
    player.registerRoomObserver(&room);
    player.Roomnotify(&player);
}