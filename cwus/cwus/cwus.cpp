#include <iostream>
#include <fstream>// for files
#include <vector> // for arrays
#include <algorithm>
#include <cstring>
#include <C:\Users\Asus\source\repos\single_include\nlohmann\json.hpp>
using namespace std;
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
    for (word* Word : *words) {// std find
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

class target{
    friend class Player;
    friend class Enemy;
private:
    int hp;
    bool evade;
    int periodic_hp_change;
    int maxhp;
    string name;
    target* opponent;
public:
    static std::vector<word*> words;
    void take_damage(int wordId) {
        periodic_hp_change += words[wordId]->getPeriodicDmg();
        hp -= words[wordId]->getDmg();
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
    int gethp() {
        return hp;
    }
    int getmaxhp() {
        return maxhp;
    }
    int getperiodicHpChange() {
        return periodic_hp_change;
    }
    bool take_periodicdmg() {
        if (periodic_hp_change != 0) {
            hp -= periodic_hp_change;
            if (periodic_hp_change > 0) {
                periodic_hp_change -= 1;
                cout << name << " recieved " << periodic_hp_change << " periodic damdge!" << '\n';
            }
            else {
                periodic_hp_change += 1;
                cout << name << " recieved " << -periodic_hp_change << " periodic healing!" << '\n';
            }
        }
        return hp > 0;
    }
    void setOpponent(target* Opponent) {
        opponent = Opponent;
    }
    void Battlenotify(int wordid, int target) {
        if (target) {
            opponent->take_damage(wordid);
        }
        else {
            take_damage(wordid);
        }
    }
};
std::vector<word*> target::words;


class Room{
    friend class Enemy_Room;
private:
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
    static void link_rooms() {// не там в какой нибудь геймворлж
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
};
std::vector<Room*> Room::registry;


class Player{
private:
    std::array<bool, 26> letter_inventory{};  // inventory of letters A-Z
    std::vector<int> scroll_inventory;        // inventory of scroll IDs
    Room* current_room = nullptr;             // pointer to current room
    int money = 0;             // player's money amount     
    target* PlayerTarget=new target(); // можно просто target

public:
    target* getTarget() {
        return PlayerTarget;
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
        }
        cout << "your hp:" << PlayerTarget->hp << "\\" << PlayerTarget->maxhp << '\n';
        string playerattack;
        cin >> playerattack;
        bool usingscrolls = false;
        if (playerattack == "scrolls") {
            if (scroll_inventory.size() > 0) {
                usingscrolls = true;
                cout << "you have scrolls of:" << '\n';
                    for (int i = 0; i < scroll_inventory.size();i++) {
                        cout << i+1 << ". " <<PlayerTarget->words[scroll_inventory[i]]->getWordName() << '\n';
                    }
                cout<< "type in the word of a scroll you want" << '\n';
            }
            else {
                cout << "You have no scrolls, type in the word you can cast" << '\n';
            }
            cin >> playerattack;
        }
        int attackid = Ifword(playerattack,&PlayerTarget->words);
        if (attackid>=0) {
            if (usingscrolls) {
                // if using scrolls, check if such one exists
                auto indexx= std::find(scroll_inventory.begin(), scroll_inventory.end(), attackid);
                if (indexx != scroll_inventory.end()) {
                    scroll_inventory.erase(indexx);
                    cout << PlayerTarget->words[attackid]->getDescription()<<'\n';
                    PlayerTarget->Battlenotify(attackid, PlayerTarget->words[attackid]->getTarget());
                }else {
                    cout << "You don't have such scroll" << '\n';
                }
            }else {
                if (usable(attackid, &PlayerTarget->words, &letter_inventory)) {
                    PlayerTarget->Battlenotify(attackid, PlayerTarget->words[attackid]->getTarget());
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
        PlayerTarget->hp = j.value("hp", 20);
        PlayerTarget->maxhp = PlayerTarget->hp;
        PlayerTarget->name = j.value("name", "You");
        letter_inventory=j["letters"].get<std::array<bool, 26>>();
        scroll_inventory=j["scrolls"].get<std::vector<int>>();
    }
};
class Enemy{
private:
    int EnemyId;
    vector<int> attacks;
    //target EnemyTarget;
    // loot later
    target* EnemyTarget = new target();
public:
    target* getTarget() {
        return EnemyTarget;
    }
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
        EnemyTarget->name = j[0].value("enemyName","");
        EnemyTarget->hp = j[0].value("enemyHp", 0);
        EnemyTarget->maxhp = EnemyTarget->hp;
    }
    void attack() {
        int attack = rand() % (attacks.size());
        cout << EnemyTarget->name << EnemyTarget->words[attacks[attack]]->getEnemyDescription()<<'\n';
        EnemyTarget->Battlenotify(attacks[attack], EnemyTarget->words[attacks[attack]]->getTarget());
    }
    void death() {
        cout << EnemyTarget->name<<" is f****** dead" << '\n';
    }
    string getname() {
        return EnemyTarget->name;
    }

};
class Battle {
private:
    Enemy* enemy;
    bool player_turn;
public:
    Battle(Player* player) {
        delete enemy;// убери делет
        enemy = new Enemy();
        player_turn = true;
        cout << "Battle has started! Your enemy:" << '\n';
        cout << enemy->getname() << "(" << enemy->getTarget()->gethp() << "\\" << enemy->getTarget()->getmaxhp() << ")" << '\n';
        int result =battleloop(player);
        if (not(result)) {
            player->death();
            cout << "you lost" << '\n';
        }else {
            enemy->death();
            cout << "you won" << '\n';
        }
        this->~Battle();// не вызывай деструктор руками
    }
    int battleloop(Player* player) {
        player->getTarget()->setOpponent(enemy->getTarget());
        enemy->getTarget()->setOpponent(player->getTarget());
        while (enemy->getTarget()->gethp()>0 and player->getTarget()->gethp() > 0) {
            if (player_turn) {
                if (player->getTarget()->take_periodicdmg()) {
                    cout << enemy->getname() << " has " << enemy->getTarget()->gethp() << " hp left!" << '\n';
                    player->attack();
                    player_turn = not(player_turn);
                }else {break;}
            }
            else {
                if (enemy->getTarget()->take_periodicdmg()){
                    enemy->attack();
                    player_turn = not(player_turn);
                }
            }
        }
        if (enemy->getTarget()->gethp() > 0) {
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
class Enemy_Room{
private:
    Battle* battle;
    Room* EnemyRoom;
public:
    void update(Player* player) {
        if (not(EnemyRoom->visited)) {
            room_event(player);
            EnemyRoom->visited = true;
        }
    }
    void room_event(Player* player) {
        battle = new Battle(player);
    }
};

int main() {
    Player player;
    Enemy_Room room;
    room.room_event(&player);
}
//
//делать что то кроме инициализации в конструкторе no bueno
//порядок методов
//(не спросят НО) класс с cout не круто
//там в Room void int&
//include без полного пути
//инклуд через кавычки #include "..."
// папка проекта папка extern с json и cpp в extern
//std(
// гуглвый стайл гайд
// разделительные пустые строки
// список инициализации через : после объявления заголовка конструктора
// большие штуки по конст ссылкам
//https://google.github.io/styleguide/cppguide.html
// вместо сравнивания строк какая нибудь хэш таблицf(по возможности)
// words можно просто std::vector<word>
// по возможности на каждый new - delete
// указатели медленные? 
// такие ли мы разные со своими врагами?...
// подсистема чтения ресурсов так уж и быть можно global
// прозрачные имена
// управление данными в +- одном месте?