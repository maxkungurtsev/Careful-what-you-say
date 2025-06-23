#include "data_manager.h"

#include <random>
#include <numeric>

constexpr int k_shop_stock_size=4;


std::string toLowerCase(std::string& str) {
    std::string newstr="";
    for (char c : str) {
        newstr += std::tolower(c);
    }
    return newstr;
}

int randomIndex(std::vector<int> weights) {
    std::random_device rd;
    //rng
    std::mt19937 gen(rd());
    //choose random with based on weights
    std::discrete_distribution<> dist(weights.begin(), weights.end());
    return dist(gen);
}


class MovementSystem {
private:
    DungeonManager& dungeon_manager_;
    EntitysManager& entitys_manager_;
    PlayerDataAndManager& player_manager_;
public:
    MovementSystem(DungeonManager& dungeon_manager, EntitysManager& entitys_manager, PlayerDataAndManager& player_manager) :
        dungeon_manager_(dungeon_manager), entitys_manager_(entitys_manager), player_manager_(player_manager) {};
    void moveToAnotherRoom(){
        
        //composing pretty text to show and a vector of possible moves
        std::string greeting="where do you want to go now?\n";
        std::vector<std::string> available_directions;
        if (player_manager_.getCurrentRoom()->room_left_id_>=0){
            available_directions.push_back("left");
            greeting = greeting+ std::to_string(available_directions.size())+". left("+ typeByRoomId(player_manager_.getCurrentRoom()->room_left_id_)+")\n";
        }
        if (player_manager_.getCurrentRoom()->room_up_id_ >= 0) {
            available_directions.push_back("forward");
            greeting = greeting + std::to_string(available_directions.size()) + ". forward(" + typeByRoomId(player_manager_.getCurrentRoom()->room_up_id_) + ")\n";
        }
        if (player_manager_.getCurrentRoom()->room_right_id_ >= 0) {
            available_directions.push_back("right");
            greeting = greeting + std::to_string(available_directions.size()) + ". right(" + typeByRoomId(player_manager_.getCurrentRoom()->room_right_id_) + ")\n";
        }
        if (player_manager_.getCurrentRoom()->room_down_id_ >= 0) {
            available_directions.push_back("backward");
            greeting = greeting + std::to_string(available_directions.size()) + ". backward(" + typeByRoomId(player_manager_.getCurrentRoom()->room_down_id_) + ")\n";
        }
        if (player_manager_.getCurrentRoom()->is_last_room_) {
            available_directions.push_back("down");
            greeting = greeting + std::to_string(available_directions.size()) + ". down(next floor) \n";
        }
        std::cout << greeting<<'\n';

        // direction input
        std::string direction;
        std::cin >> direction;
        direction = toLowerCase(direction);
        //check input
        if (std::find(available_directions.begin(), available_directions.end(), direction) == available_directions.end()) {
            std::cout << "Your choice does not correspond with any of the available ones. Random direction shall be chosen." << '\n';
            direction = available_directions[rand() % (available_directions.size() - 1)];
        }
        if (direction != "down") {
            std::cout << "your direction is " << direction << '\n';
        }
        // actually moving
        switch (direction[0]) {
        case 'l': // which means direction=left
            player_manager_.setCurrentRoom(dungeon_manager_.getAllRooms()[player_manager_.getCurrentRoom()->room_left_id_]);
            break;
        case 'u':// which means direction=up
            player_manager_.setCurrentRoom(dungeon_manager_.getAllRooms()[player_manager_.getCurrentRoom()->room_up_id_]);
            break;
        case 'r':// which means direction=right
            player_manager_.setCurrentRoom(dungeon_manager_.getAllRooms()[player_manager_.getCurrentRoom()->room_right_id_]);
            break;
        case 'b':// which means direction=backward
            player_manager_.setCurrentRoom(dungeon_manager_.getAllRooms()[player_manager_.getCurrentRoom()->room_down_id_]);
            break;
        case 'd':// which means direction=right
            player_manager_.setCurrentRoom(dungeon_manager_.getFirstRooms()[player_manager_.getCurrentRoom()->floor_ + 1]);
            std::cout << "You venture down, onto the floor " << player_manager_.getCurrentRoom()->floor_ + 1 << '\n';
            break;
        }
    };
    std::string typeByRoomId(int room_id) {
        int room_type = dungeon_manager_.getAllRooms()[room_id].room_type_;
        switch (room_type) {
        case 0:
            return "regular room";
        case 1:
            return "treasure room";
        case 2:
            return "shop";
        }
    }

};

class BattleSystem {
private:
    WordsManager& words_manager_;
    PlayerDataAndManager& player_manager_;
    EntitysManager& entitys_manager_;
    //default settings for player entity
    EntityData player = entitys_manager_.getEntitys()[0][0];
    // to apply overtime damage/healing
    bool takePeriodicDamage(EntityData& entity) {
        if (entity.getPeriodicHpChange() != 0) {
            if (entity.getPeriodicHpChange() > 0) {
                if (entity.name_ == entitys_manager_.getEntitys()[0][0].name_) {
                    std::cout << "You suffer from ";
                }
                else {
                    std::cout << entity.name_ << " suffers from ";
                }
                std::cout << entity.getPeriodicHpChange() << " overtime damage! \n";
            }
            else {
                std::cout << entity.name_ << " recieved " << entity.getPeriodicHpChange() << " Hp as periodic healing! \n";
            }
            entity.setHp(entity.getHp() - entity.getPeriodicHpChange());
            if (entity.getHp() > entity.max_hp_) {
                entity.setHp(entity.max_hp_);
            }
            entity.setPeriodicHpChange(entity.getPeriodicHpChange() - 1);
            return entity.getHp() > 0;
        }
        return true;
    }
    // check if attack player wants to perform is real
    const int ifWord(std::string player_attack) {
        toLowerCase(player_attack);
        for (const WordData& word : words_manager_.getWords()) {
            if (word.word_name_ == player_attack) {
                return word.word_id_;
            }
        }
        return -1;
    };
    // check if attack can be used with the letter player has
    bool usable(int word_id) {
        std::string player_attack = words_manager_.getWords()[word_id].word_name_;
        for (const char letter : player_attack) {
            if (!player_manager_.getLetterInventory()[letter - 'a']) {
                return false;
            }
        }
        return true;
    }
    // generating random enemy
    EntityData generateEnemy() {
        std::vector<int> weights;
        if (player_manager_.getCurrentRoom()->floor_ < entitys_manager_.getEntitys().size()){

            const std::vector<EntityData>& entity_list = entitys_manager_.getEntitys()[player_manager_.getCurrentRoom()->floor_];
            for (const EntityData& entity_data : entity_list) {
                weights.push_back(entity_data.chance_);
            }
            if (std::accumulate(weights.begin(), weights.end(), 0) == 0) {
                return EntityData(10, "goblin", std::vector<int>{0, 1}, 10);
            }
            std::discrete_distribution<> dist(weights.begin(), weights.end());
            return entity_list[randomIndex(weights)];
        }
        return EntityData(10, "goblin", std::vector<int>{0, 1}, 10);
    }
    // to apply effects of the attack to "victim"
    void takeAttack(EntityData& victim, int word_id) {
        //if entity can evade an attack and attack would deal damage
        if (victim.getEvade() and (words_manager_.getWords()[word_id].dmg_>0)) {
            std::cout << victim.name_ << " dodged an attack! \n";
            if (words_manager_.getWords()[word_id].periodic_dmg_ != 0) {
                std::cout << "( " << words_manager_.getWords()[word_id].periodic_dmg_ << " overtime damage is still applied) \n";
            }
            victim.setEvade(false);
        }
        else {
            victim.setHp(victim.getHp()-(words_manager_.getWords()[word_id].dmg_));
            if (victim.getHp() > victim.max_hp_) {
                victim.setHp(victim.max_hp_);
            }
        }
        victim.setPeriodicHpChange(victim.getPeriodicHpChange()+words_manager_.getWords()[word_id].periodic_dmg_);
        victim.setEvade(words_manager_.getWords()[word_id].add_evade_);
    }
    // to perform players move
    void playerLandAttack(EntityData& enemy) {
        std::cout << enemy.name_ << ": " << enemy.getHp() << "/" << enemy.max_hp_ << " Hp \n";
        std::cout << "Type 'scrolls' to use scrolls or enter your word. Your letters:" << std::endl;
        for (int i = 0; i < 26; ++i) {
            if (player_manager_.getLetterInventory()[i]) {
                std::cout << char('A' + i) << " ";
            }
        }
        std::cout << "\nYour HP: " << player.getHp() << " / " << player.max_hp_ << "\n";

        std::string player_attack;
        std::cin >> player_attack;
        std::vector<int>& scroll_inventory = player_manager_.getScrollInventory();
        bool use_scrolls = false;
        int scroll_id;
        if (player_attack == "scrolls") {
            if (!player_manager_.getScrollInventory().empty()) {
                use_scrolls = true;
                std::cout << "You have scrolls of:" << '\n';
                for (size_t i = 0; i < scroll_inventory.size(); ++i) {
                    scroll_id = scroll_inventory[i];
                    std::cout << i + 1 << ". " << words_manager_.getWords()[scroll_id].word_name_ << '\n';
                }
                std::cout << "Type the word of the scroll you want" << '\n';
            }
            else {
                std::cout << "You have no scrolls; type the word you can cast" << '\n';
            }
            std::cin >> player_attack;
        }

        int attack_id = ifWord(player_attack);
        if (attack_id >= 0) {
            if (use_scrolls) {
                auto used_scroll = std::find(scroll_inventory.begin(), scroll_inventory.end(), attack_id);
                if (used_scroll != scroll_inventory.end()) {
                    scroll_inventory.erase(used_scroll);
                    std::cout << words_manager_.getWords()[attack_id].description_ << '\n';
                    landAttack(player, enemy, attack_id);
                }
                else {
                    std::cout << "You don't have such scroll" << '\n';
                }
            }
            else {
                if (usable(attack_id)) {
                    landAttack(player, enemy, attack_id);
                }
            }
        }
        else {
            std::cout << "That's not a valid word." << '\n';
        }
    }
    //to choose whos gonna be affected by a word
    void landAttack(EntityData& caster, EntityData& opponent, int word_id) {
        if (caster.name_ == entitys_manager_.getEntitys()[0][0].name_) {
            std::cout << words_manager_.getWords()[word_id].description_ << '\n';
        }
        else {
            std::cout << caster.name_<< words_manager_.getWords()[word_id].enemy_description_ << '\n';
        }
        if (words_manager_.getWords()[word_id].cast_on_enemy_) {
            takeAttack(opponent, word_id);
        }else{
            takeAttack(caster, word_id);
        }
    }
    bool battleLoop(EntityData& player, EntityData& enemy) {
        std::cout << "you encounter a " << enemy.name_ << "! \n";
        bool player_turn = true;
        while (player.getHp() > 0 and enemy.getHp() > 0) {
            if (player_turn) {
                if (takePeriodicDamage(player)) {
                    playerLandAttack(enemy);
                }
            }else {
                if (takePeriodicDamage(enemy)) {
                    if (enemy.attack_pool_.size()>0) {
                    landAttack(enemy, player, enemy.attack_pool_[rand() % (enemy.attack_pool_.size())]);
                    }
                    else {
                        std::cout << enemy.name_ << " doesn't attack! \n";
                    }
                }
            }
            player_turn = not(player_turn);
        }
        return player.getHp() > 0;
    }
public:
    BattleSystem(WordsManager& words_manager, PlayerDataAndManager& player_manager, EntitysManager& entitys_manager) :words_manager_(words_manager),
        player_manager_(player_manager), entitys_manager_(entitys_manager) {
    }
    bool startBattle() {
        EntityData enemy=generateEnemy();
        return battleLoop(player, enemy);
    }
};

class EventSystem { 
private:
    PlayerDataAndManager& player_manager_;
    LootManager& loot_manager_;
    WordsManager& words_manager_;
    BattleSystem& battle_system_;
    void giveOutLoot(const LootData& loot) {
        switch (loot.loot_type_) {
        case 0:// money
            std::cout << loot.amount_or_id_ << " gold!" << '\n';
            player_manager_.setMoney(loot.amount_or_id_ + player_manager_.getMoney());
            break;
        case 1:// letter  
            std::cout << "letter " << char('A' + loot.amount_or_id_) << '\n';
            player_manager_.getLetterInventory()[loot.amount_or_id_] = true;
            break;
        case 2:
            std::cout << "scroll of '" << words_manager_.getWords()[loot.amount_or_id_].word_name_ << "'!" << '\n';
            if (player_manager_.getScrollInventory().size()<101){
                player_manager_.getScrollInventory().push_back(loot.amount_or_id_);
            }
            else {
                std::cout << "But your inventory is full, so you won't get it greedy wizard." << '\n';
            }
            break;
        }
    }
    const LootData generateLoot() {
        // first we generate vector containing each items chances to show up
        std::vector<int> weights;
        // vector to track letters generated in this room.( we don't want to give player copies of them)
        std::vector<int> generated_letters;
        const std::vector<LootData>& loot_list = loot_manager_.getLootTable()[player_manager_.getCurrentRoom()->floor_];
        // according to current floor we take loot list of that floor from loot table
        for (const auto& loot_data : loot_list) {
            // if we in the shop, prevent money from generating
            if (player_manager_.getCurrentRoom()->room_type_ == 2 and loot_data.loot_type_ == 0) {
                weights.push_back(0);
                continue;
            }else if (loot_data.loot_type_ == 1) {// if it's a letter
                // and player already has it or we already made this one
                if (player_manager_.getLetterInventory()[loot_data.amount_or_id_] or 
                    (std::find(generated_letters.begin(), generated_letters.end(), loot_data.amount_or_id_)!= generated_letters.end())){
                    weights.push_back(0);
                    continue;
                }
            }
            weights.push_back(loot_data.chance_);
        }
        // if weights is empty or every item has chance = 0 it means nothing could generate
        // we plug this inconvinience with some scroll 
        if (std::accumulate(weights.begin(), weights.end(), 0) == 0) {
            return LootData(2, 0, 1, 10);
        }
        std::discrete_distribution<> dist(weights.begin(), weights.end());
        return loot_list[randomIndex(weights)];
    }
    void treasureEvent(){
        std::cout << "In this treasure room you found:";
        giveOutLoot(generateLoot());
    }
    void shopEvent() {
        //to mark slots as sold after purchase
        static std::vector <bool> sold_items;
        //pretty greeting text
        static std::string greeting;
        std::cout<<"You Enter the shop. \n Shopkeeper: Oi, mate! Bet ya came lookin' for some ancient wisdom, eh? \n Well, have a squiz at this then! \n";
        //loot for sale
        static std::vector<LootData> shop_stock;
        // if player hasn't visited this shop, we stock it.
        if (not((*player_manager_.getCurrentRoom()).getVisited())) {
            shop_stock.clear();
            // to keep letter copies from spawning we firstly copy letter_inventory_ 
            std::array<bool, 26> temporary_letter_inventory = player_manager_.getLetterInventory();
            //stocking the shop
            for (int i = 0; i < k_shop_stock_size; i++) {
                shop_stock.push_back(generateLoot());
                if (shop_stock.back().loot_type_ == 1) {
                    //if letter is generated we tempoarly give it to the player to prevent it from generating again
                    player_manager_.getLetterInventory()[shop_stock.back().amount_or_id_] = true;
                }
            }
            // and lastly, we set letter inventory to its original state
            player_manager_.getLetterInventory() = temporary_letter_inventory;
            sold_items = std::vector<bool>(k_shop_stock_size, false);
        }
        while (true) {
            // composing pretty text
            greeting = "";
            for (int i = 0; i < k_shop_stock_size; i++) {
                if (!sold_items[i]) {
                    if (shop_stock[i].loot_type_ == 1) {
                        greeting = greeting + std::to_string(i + 1) + ". letter " + char('A' + shop_stock[i].amount_or_id_) + " ";
                    }
                    else {
                        greeting = greeting + std::to_string(i + 1) + ". scroll of '" + words_manager_.getWords()[shop_stock.back().amount_or_id_].word_name_ + "' ";
                    }
                    greeting = greeting + "(" + std::to_string(shop_stock[i].price_) + " g.) \n";
                }
                else {
                    greeting = greeting + std::to_string(i + 1) + ". SOLD OUT \n";
                }

            }
            std::cout << greeting;
            std::cout << "(this time use numbers to choose or type 'exit' to go away)" << '\n'<<"your savings:"
                << player_manager_.getMoney()<<" gold"<<'\n';
            std::string choice_str;
            std::cin >> choice_str;
            if (choice_str == "exit") {
                std::cout << "See ya, mate! \n";
                return;
            }
            // if choice is number we can start working with it
            if (std::all_of(choice_str.begin(), choice_str.end(), ::isdigit)) {
                int choice_int = std::stoi(choice_str);
                // throw away unwanted numbers
                if(choice_int>5 or choice_int==0){
                    std::cout << "Oi matey, that's not even on the table, eh! Try again, would ya? \n";
                }else {// actually buying
                    // if player already bought that one
                    if (sold_items[choice_int-1]) {
                        std::cout << "You've already snagged that one, mate. \n";
                    //or he doesn't have enough money we don't sell anything
                    }else if (player_manager_.getMoney() < shop_stock[choice_int - 1].price_) {
                        std::cout << "Ya don't have enough cash for this one, mate. \n";
                    }
                    else {
                        std::cout << "Ere ya go, mate! \n You bought ";
                        giveOutLoot(shop_stock[choice_int - 1]);
                        sold_items[choice_int - 1] = true;
                        player_manager_.setMoney(player_manager_.getMoney() - shop_stock[choice_int - 1].price_);
                    }
                }
            }else {
                std::cout << "Oi matey, that's not even on the table, eh! Try again, would ya? \n";
            }
        }
    }
    bool enemyEvent() {
        bool battle_won = battle_system_.startBattle();
        if (battle_won) {
            LootData enemy_drop = generateLoot();
            std::cout << "You won! Your reward is:";
            giveOutLoot(enemy_drop);
        }
        else {
            std::cout << "You died! \n";
        }
        return battle_won;
    }
public:
    EventSystem(BattleSystem& battle_system, PlayerDataAndManager& player_manager, LootManager& loot_manager, WordsManager& words_manager) :
        battle_system_(battle_system), player_manager_(player_manager), loot_manager_(loot_manager), words_manager_(words_manager) {};
    bool roomEvent() {
        bool player_is_alive = true;
        switch(player_manager_.getCurrentRoom()->room_type_){
            case 0:
                if (not(player_manager_.getCurrentRoom()->getVisited())) {
                    player_is_alive = enemyEvent();
                }
                break;
            case 1:
                if (not(player_manager_.getCurrentRoom()->getVisited())) {
                    treasureEvent();
                }
                break;
            case 2:
                shopEvent();
                break;
            default:
                std::cout << "invalid roomtype while choosing event" << '\n';
        }
        player_manager_.getCurrentRoom()->setVisited();
        return player_is_alive;
    }

};



void gameloop() {
    DungeonManager dungeon_manager = DungeonManager();
    EntitysManager entitys_manager = EntitysManager();
    LootManager loot_manager = LootManager();
    WordsManager words_manager = WordsManager();
    dungeon_manager.loadDungeon();
    entitys_manager.loadEntitys();
    loot_manager.loadLoot();
    words_manager.loadWords();
    PlayerDataAndManager player_manager = PlayerDataAndManager(dungeon_manager.getAllRooms());
    MovementSystem movement_system(dungeon_manager, entitys_manager, player_manager);
    BattleSystem battle_system = BattleSystem(words_manager, player_manager, entitys_manager);
    EventSystem event_system =EventSystem(battle_system, player_manager, loot_manager, words_manager);
    while (event_system.roomEvent()) {
        movement_system.moveToAnotherRoom();
    }
    std::cout << "GAME OVER";
}





int main() {
    gameloop();
    return 0;
}

//делать что то кроме инициализации в конструкторе no bueno
//порядок методов
//(не спросят НО) класс с cout не круто
// папка проекта папка extern с json и cpp в extern
// список инициализации через : после объявления заголовка конструктора
// большие штуки по конст ссылкам
//https://google.github.io/styleguide/cppguide.html               wip
// вместо сравнивания строк какая нибудь хэш таблицf(по возможности)
// words можно просто std::vector<word>
// по возможности на каждый new - delete
// такие ли мы разные со своими врагами?...                   wip
// подсистема чтения ресурсов так уж и быть можно global                wip
// прозрачные имена
// управление данными в +- одном месте?              wip