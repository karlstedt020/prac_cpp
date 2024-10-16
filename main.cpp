#include <iostream>
#include <vector>
#include <string>
#include <future>
#include <memory>
#include <random>
#include <clocale>
#include <unistd.h>
#include "shared_ptr.cpp"
#include "logger.cpp"
#include "roles.cpp"


Logger logger;

int alive_mafia = 0;

int random(int n) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(0, n-1);
    return dist6(rng);
}


void init_roles(std::vector<std::shared_ptr<BaseRole>> &roles, int n) {
    std::vector<bool> has_role(n);
    int doc_id = random(n);
    roles[doc_id] = std::shared_ptr<BaseRole> (new Doctor(doc_id, logger));
    has_role[doc_id] = true;

    int chief_id = doc_id;
    while (has_role[chief_id])
        chief_id = random(n);
    roles[chief_id] = std::shared_ptr<BaseRole> (new Chief(chief_id, logger, n));
    has_role[chief_id] = true;

    int man_id = chief_id;
    while (has_role[man_id])
        man_id = random(n);
    roles[man_id] = std::shared_ptr<BaseRole> (new Maniac(man_id, logger));
    has_role[man_id] = true;

    for (int i = 0; i < 2; ++i) {
        int mafia_id = chief_id;
        while (has_role[mafia_id])
            mafia_id = random(n);
        roles[mafia_id] = std::shared_ptr<BaseRole> (new Mafia(mafia_id, logger));
        has_role[mafia_id] = true;
    }

    for (int i = 0; i < n; ++i) {
        if (!has_role[i]) {
            roles[i] = std::shared_ptr<BaseRole> (new Civilian(i, logger));
        }
    }
}


bool check_alive(std::vector<std::shared_ptr<BaseRole>> &alive, int id) {
    for (auto x : alive) {
        if (x->id == id)
            return true;
    }
    return false;
}


void night(std::vector<std::shared_ptr<BaseRole>> &players, std::vector<std::shared_ptr<BaseRole>> &alive, int n, bool user, int boss_id, int round) {
    std::vector<std::future<void>> actions;
    std::vector<int> votes(n);
    for (auto& x : alive) {
        if (x->id == 0 && user)
            continue;
        actions.push_back(std::async(std::launch::async, [&]() {
            if (boss_id != x->id)
                x->act(alive, votes, false, round);
        }));
    }
    if (user) {
        // user is playing
        if (!check_alive(alive, 0)) {
            std::cout << "You are dead" << std::endl;
        }
        else {
            std::string role = players[0]->role;

            if (role == "civilian") {
                std::cout << "Civilian has no night activities" << std::endl;
            }
            if (role == "doctor") {
                std::cout << "Input ID of a player, who you want to heal: ";
                int healed = -1;
                while (true) {
                    std::cin >> healed;
                    if (!check_alive(alive, healed)) {
                        std::cout << "This player is dead!" << std::endl;
                        continue;
                    }
                    if (dynamic_cast<Doctor*>(players[0].get())->prev_id == healed) {
                        std::cout << "You can not heal one player two nights in a row!" << std::endl;
                        continue;
                    }
                    std::cout << "The choice was made" << std::endl;
                    logger.log_round(round, "Player (doctor) " + std::to_string(0) + " decided to healed " + std::to_string(healed) + "\n");
                    dynamic_cast<Doctor*>(players[0].get())->prev_id = healed;
                    players[0]->night_vote = healed;
                    break;
                }
            }
            if (role == "chief") {
                int act = -1;
                while (act != 1 && act != 2) {
                    std::cout << "Choose an action (1 - check, 2 - shoot): ";
                    std::cin >> act;
                }
                int checked = -1;
                while (true) {
                    std::cout << "Input ID of a player to commit the action: ";
                    std::cin >> checked;
                    if (!check_alive(alive, checked)) {
                        std::cout << "This player is dead!" << std::endl;
                        continue;
                    }
                    if (act == 1 && dynamic_cast<Chief*>(players[0].get())->checked_ids[checked]) {
                        std::cout << "This player was already checked!" << std::endl;
                        continue;
                    }
                    if (act == 1 && (players[checked]->role == "civilian" || players[checked]->role == "doctor")) {
                        dynamic_cast<Chief*>(players[0].get())->checked_ids[checked] = 1;
                        std::cout << "Player " << checked << " is not mafia" << std::endl;
                        logger.log_round(round, "Player (chief) " + std::to_string(0) + " decided to check " + std::to_string(checked) + "\n");
                        dynamic_cast<Chief*>(players[0].get())->action = 1;
                    }
                    else if (act == 1) {
                        dynamic_cast<Chief*>(players[0].get())->checked_ids[checked] = 2;
                        std::cout << "Player " << checked << " is mafia" << std::endl;
                        logger.log_round(round, "Player (chief) " + std::to_string(0) + " decided to check " + std::to_string(checked) + "\n");
                        dynamic_cast<Chief*>(players[0].get())->action = 1;
                    }
                    else {
                        dynamic_cast<Chief*>(players[0].get())->action = 2;
                        logger.log_round(round, "Player (chief) " + std::to_string(0) + " decided to shoot " + std::to_string(checked) + "\n");
                    }
                    players[0]->night_vote = checked;
                    break;
                }
            }
            if (role == "mafia" && boss_id != 0) {
                int killed = -1;
                while (true) {
                    std::cout << "Input ID of a player, who you want to kill: ";
                    std::cin >> killed;
                    if (!check_alive(alive, killed)) {
                        std::cout << "This player is dead!" << std::endl;
                        continue;
                    }
                    std::cout << "The choice was made" << std::endl;
                    players[0]->night_vote = killed;
                    logger.log_round(round, "Player (mafia) " + std::to_string(0) + " decided to kill " + std::to_string(killed) + "\n");
                    votes[killed]++;
                    break;
                }
            }
            if (role == "maniac") {
                int killed = -1;
                while (true) {
                    std::cout << "Input ID of a player, that you want to kill: ";
                    std::cin >> killed;
                    if (!check_alive(alive, killed)) {
                        std::cout << "--This player is dead!" << std::endl;
                        continue;
                    }
                    std::cout << "--The choice was made" << std::endl;
                    players[0]->night_vote = killed;
                    logger.log_round(round, "Player (maniac) " + std::to_string(0) + " decided to kill " + std::to_string(killed) + "\n");
                    break;
                }
            }
        }
    }
    for (auto& action: actions) {
        action.get();
    }
    if (boss_id == 0) {
        std::cout << "The rest of mafia decided to kill these players: \n" << std::endl;
        for (int i = 0; i < n; ++i) {
            if (votes[i]) {
                std::cout << "Player ID " << i << ": " << votes[i] << " voices" << std::endl;
            }
        }
        std::cout << "In controversial situations your vote will be decisive\n";
        int killed = -1;
        while (true) {
            std::cout << "Input ID of a player, who you want to kill: ";
            std::cin >> killed;
            if (!check_alive(alive, killed)) {
                std::cout << "This player is dead!" << std::endl;
                continue;
            }
            std::cout << "The choice was made" << std::endl;
            votes[killed]++;
            int ma = votes[killed], argmax = killed;
            for (int i = 0; i < n; ++i) {
                if (votes[i] > ma) {
                    ma = votes[i];
                    argmax = i;
                }
            }
            players[0]->night_vote = argmax;
            logger.log_round(round, "Player (mafia boss) " + std::to_string(0) + " decided to kill " + std::to_string(argmax) + "\n");
            break;
        }
    }
    else {
        if (players[boss_id]->alive)
            players[boss_id]->act(alive, votes, true, round);
    }
    std::string str = "Mafia's votes:\n";
    for (int i = 0; i < n; ++i) {
        if (votes[i] > 0) {
            str += "Player ID " + std::to_string(i) + " has " + std::to_string(votes[i]) + " votes\n";
        }
    }
    logger.log_round(round, str);
}


void pop_by_id(std::vector<std::shared_ptr<BaseRole>> &alive, int id) {
    int pop = -1;
    for (int i = 0; i < alive.size(); ++i) {
        if (alive[i]->id == id) {
            pop = i;
            break;
        }
    }
    if (alive[pop]->role == "mafia")
        alive_mafia--;
    alive.erase(alive.begin()+pop);
}


void summarize(std::vector<std::shared_ptr<BaseRole>> &players, std::vector<std::shared_ptr<BaseRole>> &alive, int n, int boss_id, int round) {
    int healed = -1;
    int checked = -1, shot = -1;
    int killed_maf = -1, killed_man = -1;

    for (auto &x : alive) {
        if (x->role == "doctor")
            healed = x->night_vote;
        if (x->role == "chief" && static_cast<Chief*>(x.get())->action == 1) {
            checked = x->night_vote;
            if (players[checked]->role == "mafia")
                static_cast<Chief*>(x.get())->set_status(checked, 2);
            else
                static_cast<Chief*>(x.get())->set_status(checked, 1);
        }
        if (x->role == "chief" && static_cast<Chief*>(x.get())->action == 2)
            shot = x->night_vote;
        if (x->role == "mafia" && x->id == boss_id)
            killed_maf = x->night_vote;
        if (x->role == "maniac")
            killed_man = x->night_vote;
    }
    if (killed_maf != -1 && healed != killed_maf) {
        std::cout << "--At night player " << killed_maf << " was killed\n";
        logger.log_round(round, "--At night player " + std::to_string(killed_maf) + " was killed\n");
        pop_by_id(alive, killed_maf);
        players[killed_maf]->alive = false;
    }
    if (killed_man != -1 && healed != killed_man && killed_man != killed_maf) {
        std::cout << "--At night player " << killed_man << " was killed\n";
        logger.log_round(round, "--At night player " + std::to_string(killed_man) + " was killed\n");
        pop_by_id(alive, killed_man);
        players[killed_man]->alive = false;
    }
    if (shot != -1 && healed != shot && shot != killed_man && shot != killed_maf) {
        std::cout << "--At night player " << shot << " was killed\n";
        logger.log_round(round, "--At night player " + std::to_string(shot) + " was killed\n");
        pop_by_id(alive, shot);
        players[shot]->alive = false;
    }
}


void vote(std::vector<std::shared_ptr<BaseRole>> &players, std::vector<std::shared_ptr<BaseRole>> &alive, int n, bool user, int round) {
    std::vector<int> counter(n);
    std::vector<std::future<void>> actions;
    int argmax = 0, ma = 0;
    for (auto &x : alive) {
        if ((x.get())->id == 0 && user)
            continue;
        actions.push_back(std::async(std::launch::async, [&]() {
            (x.get())->vote(alive, round);
            if ((x.get())->day_vote != -1) {
                counter[(x.get())->day_vote]++;
                if (counter[(x.get())->day_vote] > ma) {
                    argmax = (x.get())->day_vote;
                    ma = counter[(x.get())->day_vote];
                }
            }
        }));
    }
    if ((players[0].get())->alive && user) {
        int vote = -1;
        while (vote == -1 || vote >= players.size() || vote < 0 || !((players[vote].get())->alive)) {
            std::cout << "Please enter ID of a player that you want to kick: ";
            std::cin >> vote;
        }
        counter[vote]++;
        if (counter[vote] > ma) {
            argmax = vote;
            ma = counter[vote];
        }
    }
    for (auto& action : actions) {
        action.get();
    }
    int coun = 0;
    for (int i = 0; i < n; ++i) {
        if (counter[i] == ma)
            coun++;
    }
    if (coun > 1) {
        std::cout << "--The city couldn't decide, who has to be expelled. So no one would be.\n";
        logger.log_round(round, "--The city couldn't decide, who has to be expelled. So no one would be.\n");
    }
    else {
        std::cout << "--Due to the majority of the votes player " << argmax << " was voted off\n";
        logger.log_round(round, "--Due to the majority of the votes player " + std::to_string(argmax) + " was voted off\n");
        pop_by_id(alive, argmax);
        players[argmax]->alive = false;
    }

}


int main() {
    int n;
    bool play = false;
    std::cout << "Please enter the amount of players: ";
    std::cin >> n;
    std::cout << "Do you want to play? (y/n) ";
    char ans;
    std::cin >> ans;
    if (ans == 'y')
        play = true;
    auto x = std::shared_ptr<BaseRole> (new Civilian(-1, logger));
    std::vector<std::shared_ptr<BaseRole>> players(n, x);
    init_roles(players, n);
    std::vector<std::shared_ptr<BaseRole>> alive{players};
    std::vector<int> mafia_ids;
    int com_id, doc_id, man_id;
    alive_mafia = 2;
    for (auto& x : players) {
        if (x->role == "chief")
            com_id = x->id;
        if (x->role == "mafia")
            mafia_ids.push_back(x->id);
        if (x->role == "doctor")
            doc_id = x->id;
        if (x->role == "maniac")
            man_id = x->id;
    }
    int boss_id = mafia_ids[random(2)];

    if (play)
        (players[0].get())->greet(mafia_ids, boss_id);
    int round = 1;
    while (players[man_id]->alive && alive.size() > 1 || alive_mafia != 0 && alive_mafia < alive.size()) {
        std::cout << "============= Round " << round << " =============\n\n";
        std::cout << "=== Players alive: ";
        for (auto& x : alive) {
            std::cout << (x.get())->id << ' ';
        }
        if (play && players[0]->role == "mafia") {
            std::cout << "\nYour team: ";
            for (int ind : mafia_ids) {
                std::cout << ind << ' ';
            }
            std::cout << "The boss is " << boss_id;
        }
        std::cout << "\n\n--The night has fallen...\n\n" << std::endl;
        night(players, alive, n, play, boss_id, round);
        std::cout << "\n\n--Good morning! Or not so good...\n\n";
        summarize(players, alive, n, boss_id, round);
        if (!players[boss_id]->alive) {
            boss_id = -1;
            for (auto id : mafia_ids) {
                if (players[id]->alive) {
                    boss_id = id;
                    break;
                }
            }
        }
        if (!(players[man_id]->alive && alive.size() > 1 || alive_mafia != 0 && alive_mafia < alive.size())) {
            break;
        }
        std::cout << "\n=== Players alive: ";
        for (auto& x : alive) {
            std::cout << (x.get())->id << ' ';
        }
        std::cout << "\n\n=== Voting phase\n\n";
        logger.log_round(round, "\n\n=== Voting phase\n\n");
        vote(players, alive, n, play, round);
        usleep(3e6);
        std::cout << "\n\n\n";
        round++;
        if (!players[boss_id]->alive) {
            boss_id = -1;
            for (auto id : mafia_ids) {
                if (players[id]->alive) {
                    boss_id = id;
                    break;
                }
            }
        }
    }
    std::cout << "============ Game finished! ============\n\n";
    if (players[man_id]->alive) {
        std::cout << "Maniac won!\n\n";
        logger.log_results("Maniac won!\n");
    }
    else {
        if (alive_mafia != 0) {
            std::cout << "Mafia won!\nMafia team: ";
            logger.log_results("Mafia won!\nMafia team: ");
            std::string str = "";
            for (int ind : mafia_ids) {
                std::cout << ind << ' ';
                str += std::to_string(ind) + " ";
            }
            logger.log_results(str);
        }
        else {
            logger.log_results("Civilians won!\n");
            std::cout << "Civilians won!\n\n";
        }
    }
    std::cout << "Roles: \n";
    logger.log_results("Roles: ");
    for (auto& x : players) {
        std::cout << x->id << ": " << x->role << std::endl;
        logger.log_results(std::to_string(x->id) + ": " + x->role);
    }
    return 0;
}
