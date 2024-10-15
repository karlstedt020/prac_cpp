class BaseRole {
public:
    int id;
    Logger logger;
    std::string role;
    int night_vote;
    int day_vote;
    bool alive;

    BaseRole(int i, Logger& l, std::string r) : id(i), logger(l), role(r), night_vote{-1}, day_vote{-1}, alive{true} {}

    // night action
    virtual void act(std::vector<std::shared_ptr<BaseRole>> &alive_ids, std::vector<int> &votes, bool boss, int round) {};

    // returns ID of voted player
    virtual void vote(std::vector<std::shared_ptr<BaseRole>> &alive_ids, int round) {};

    //greeting player
    virtual void greet(std::vector<int> &mafia, int boss_id) {};

    virtual ~BaseRole() = default;
};


std::shared_ptr<BaseRole>& random_choice(std::vector<std::shared_ptr<BaseRole>>& vec) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, vec.size() - 1);
    return vec[distrib(gen)];
}


class Civilian : public BaseRole {

public:
    Civilian(int i, Logger& l) : BaseRole(i, l, "civilian") {}

    void act(std::vector<std::shared_ptr<BaseRole>> &alive_ids, std::vector<int> &votes, bool boss, int round) override {
        return;
    }

    void vote(std::vector<std::shared_ptr<BaseRole>> &alive_ids, int round) override {
        int voted_id = this->id;
        while (voted_id == this->id) {
            voted_id = random_choice(alive_ids).get()->id;
        }
        day_vote = voted_id;
        logger.log_round(round, "Civilian " + std::to_string(this->id) + " decided to vote off " + std::to_string(voted_id) + "\n");
    }

    void greet(std::vector<int> &mafia, int boss_id) {
        std::cout << "\n======================================================\n";
        std::cout << "\nYou are a civilian. Try to survive and figure out, who is mafia.\n";
        std::cout << "\n======================================================\n\n";
    }
};


class Mafia : public BaseRole {

public:
    Mafia(int i, Logger& l) : BaseRole(i, l, "mafia") {}

    void act(std::vector<std::shared_ptr<BaseRole>> &alive_ids, std::vector<int> &votes, bool boss, int round) override {
        int voted_id = this->id;
        bool voted_mafia = true;
        while (voted_id == this->id || voted_mafia) {
            auto voted = random_choice(alive_ids).get();
            voted_id = voted->id;
            voted_mafia = true ? voted->role == "mafia" : false;
        }
        logger.log_round(round, "At night mafia " + std::to_string(this->id) + " wanted to kill " + std::to_string(voted_id) + "\n");
        votes[voted_id]++;
        night_vote = voted_id;
        if (boss) {
            int argmax = 0, ma = 0;
            for (int i = 0; i < votes.size(); ++i) {
                if (votes[i] > ma) {
                    ma = votes[i];
                    argmax = i;
                }
            }
            if (votes[argmax] == votes[voted_id]) {
                night_vote = voted_id;
            }
            else {
                night_vote = argmax;
            }
            logger.log_round(round, "Boss " + std::to_string(this->id) + " decided to kill " + std::to_string(voted_id) + "\n");
        }
    }

    void vote(std::vector<std::shared_ptr<BaseRole>> &alive_ids, int round) override {
        int voted_id = this->id;
        bool voted_mafia = true;
        while (voted_id == this->id || voted_mafia) {
            auto voted = random_choice(alive_ids).get();
            voted_id = voted->id;
            voted_mafia = true ? voted->role == "mafia" : false;
        }
        day_vote = voted_id;
        logger.log_round(round, "Vafia " + std::to_string(this->id) + " decided to vote off " + std::to_string(voted_id) + "\n");
    }

    void greet(std::vector<int> &mafia, int boss_id) {
        std::cout << "\n======================================================\n";
        if (boss_id == this->id)
            std::cout << "\nYou are boss of the mafia. Kill other players at night, and remember, that your vote at night is decisive.\n";
        else
            std::cout << "\nYou are mafia. Decide, which player you want to kill at night, and try to get rid of all civilians.\n";
        std::cout << "Your teammates are: ";
        for (int ind : mafia)
            std::cout << ind << ' ';
        std::cout << "\n\n======================================================\n\n";
    }
};


class Doctor : public BaseRole {
public:
    int prev_id = -1;

    Doctor(int i, Logger& l) : BaseRole(i, l, "doctor") {}

    void act(std::vector<std::shared_ptr<BaseRole>> &alive_ids, std::vector<int> &votes, bool boss, int round) override {
        int voted_id = prev_id;
        while (voted_id == prev_id) {
            voted_id = random_choice(alive_ids).get()->id;
        }
        night_vote = voted_id;
        logger.log_round(round, "Doctor " + std::to_string(this->id) + " decided to heal " + std::to_string(voted_id) + "\n");
    }

    void vote(std::vector<std::shared_ptr<BaseRole>> &alive_ids, int round) override {
        int voted_id = this->id;
        while (voted_id == this->id) {
            voted_id = random_choice(alive_ids).get()->id;
        }
        day_vote = voted_id;
        logger.log_round(round, "Doctor " + std::to_string(this->id) + " decided to vote off " + std::to_string(voted_id) + "\n");
    }

    void greet(std::vector<int> &mafia, int boss_id) {
        std::cout << "\n======================================================\n";
        std::cout << "\nYou are a doctor. Try to heal civilians at night and figure out, who is mafia.\n";
        std::cout << "\n======================================================\n\n";
    }
};


class Chief : public BaseRole {
public:
    int n;
    std::vector<int> checked_ids;
    int action;
    // 0 means wasn't checked, 1 means civilian, 2 means mafia

    Chief(int i, Logger& l, int n_players) : BaseRole(i, l, "chief"), n(n_players) {
        checked_ids.resize(n_players+1, 0);
        checked_ids[n_players] = 1;
    }

    void act(std::vector<std::shared_ptr<BaseRole>> &alive_ids, std::vector<int> &votes, bool boss, int round) override {
        int voted_id = n;
        for (auto& x : alive_ids) {
            if (checked_ids[x->id] == 2) {
                action = 2;
                night_vote = x->id;
                logger.log_round(round, "Chief " + std::to_string(this->id) + " decided to kill " + std::to_string(night_vote) + "\n");
                return;
            }
        }
        while (checked_ids[voted_id]) {
            voted_id = random_choice(alive_ids).get()->id;
        }
        action = 1;
        night_vote = voted_id;
        logger.log_round(round, "Chief " + std::to_string(this->id) + " decided to check " + std::to_string(voted_id) + "\n");
    }

    void vote(std::vector<std::shared_ptr<BaseRole>> &alive_ids, int round) override {
        int voted_id = this->id;
        for (auto& x : alive_ids) {
            if (checked_ids[x->id] == 2) {
                day_vote = x->id;
                logger.log_round(round, "Chief " + std::to_string(this->id) + " decided to vote off " + std::to_string(day_vote) + "\n");
                return;
            }
        }
        while (voted_id == this->id) {
            voted_id = random_choice(alive_ids).get()->id;
        }
        day_vote = voted_id;
        logger.log_round(round, "Chief " + std::to_string(this->id) + " decided to vote off " + std::to_string(voted_id) + "\n");
    }

    void set_status(int id, int status) {
        checked_ids[id] = status;
    }

    void greet(std::vector<int> &mafia, int boss_id) {
        std::cout << "\n======================================================\n";
        std::cout << "\nYou are a chief. You can check people at night and shoot, if you know exactly, who is mafia.\n";
        std::cout << "\n======================================================\n\n";
    }
};


class Maniac : public BaseRole {

public:
    Maniac(int i, Logger& l) : BaseRole(i, l, "maniac") {}

    void act(std::vector<std::shared_ptr<BaseRole>> &alive_ids, std::vector<int> &votes, bool boss, int round) override {
        int voted_id = this->id;
        while (voted_id == this->id) {
            voted_id = random_choice(alive_ids).get()->id;
        }
        night_vote = voted_id;
        logger.log_round(round, "Maniac " + std::to_string(this->id) + " decided to kill " + std::to_string(voted_id) + "\n");
    }

    void vote(std::vector<std::shared_ptr<BaseRole>> &alive_ids, int round) override {
        int voted_id = this->id;
        while (voted_id == this->id) {
            voted_id = random_choice(alive_ids).get()->id;
        }
        day_vote = voted_id;
        logger.log_round(round, "Maniac " + std::to_string(this->id) + " decided to vote off " + std::to_string(voted_id) + "\n");
    }

    void greet(std::vector<int> &mafia, int boss_id, int round) {
        std::cout << "\n======================================================\n";
        std::cout << "\nYou are a maniac. Try to become the only one remaining.\n";
        std::cout << "\n======================================================\n\n";
    }
};
