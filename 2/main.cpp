#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;


int random(int start, int end) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(start, end-1);
    return dist(rng);
}

bool choice(double p) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::binomial_distribution<> d(1, p);
    return d(rng);
}


class SolutionABC {
public:
    virtual long long int count_metrics() = 0;

    virtual SolutionABC* move_process(int process_id, int processor_id) = 0;

    virtual string dumps() = 0;

    virtual ~SolutionABC() {};
};


class Solution : public SolutionABC {
    vector<set<int>> pu_to_p;
    vector<int> p_to_pu;
    vector<pair<bool, long long int>> recalc; // not recalculating metrics for processors where nothing changed
    vector<pair<int, int>> processes;
public:
    int n, m;
    Solution(int n, int m, vector<pair<int, int>> procs) : n(n), m(m), processes{procs} {
        pu_to_p.resize(m);
        p_to_pu.resize(n, 0);
        recalc.resize(m, {true, 0});
        for (int i = 0; i < n; ++i) {
            pu_to_p[0].insert(i);
        }
    }

    Solution(Solution& other) {
        //cout <<"GOYDA" << endl;
        n = other.n;
        m = other.m;
        recalc.resize(m, {true, 0});
        pu_to_p.resize(m);
        p_to_pu.resize(n, 0);
        processes = other.processes;
        for (int i = 0; i < n; ++i) 
            p_to_pu[i] = other.p_to_pu[i];
        for (int i = 0; i < m; ++i) {
            for (auto x : other.pu_to_p[i]) {
                pu_to_p[i].insert(x);
            }
        }
        //cout << "Finished" << endl;
    }

    Solution(string& s) {
        stringstream ss{s};
        ////cout << "Constructor" << endl;
        ////cout << s << endl;
        ss >> n >> m;
        processes.resize(n);
        recalc.resize(m);
        pu_to_p.resize(m);
        p_to_pu.resize(n);
        for (int i = 0; i < n; ++i) {
            ss >> processes[i].first >> processes[i].second;
        }
        for (int i = 0; i < m; ++i) {
            //std:://cout << "Started " << i << endl;
            recalc[i] = {true, 0};
            int amount = 0;
            ss >> amount;
            while (amount--) {
                int x;
                ss >> x;
                ////cout << x << endl;
                pu_to_p[i].insert(x);
                p_to_pu[x] = i;
            }
            //std:://cout << "Finished " << i << endl;
        }
        //cout << "End constructor" << endl;
    }

    /*Solution operator=(Solution other) {
        //cout << "Called=" << endl;
        n = other.n;
        m = other.m;
        processes = other.processes;
        p_to_pu = other.p_to_pu;
        pu_to_p = other.pu_to_p;
        recalc = other.recalc;
        return *this;
    }*/

    long long int count_metrics() override {
        long long int total = 0;
        for (int i = 0; i < m; ++i) {
            if (recalc[i].first) {
                long long int sum = 0;
                long long int prev = 0;
                for (int x : pu_to_p[i]) {
                    sum += (prev + processes[x].second);
                    prev += processes[x].second;
                }
                recalc[i] = {false, sum};
            }
            ////cout << recalc[i].second << endl;
            total += recalc[i].second;
        }
        return total;
    }

    SolutionABC* move_process(int process_id, int processor_id) override {
        //cout << "Move started" << endl;
        Solution* new_sol = new Solution(*this);
        //cout << "Copied element" << endl;
        new_sol->pu_to_p[new_sol->p_to_pu[process_id]].erase(process_id);
        new_sol->recalc[new_sol->p_to_pu[process_id]].first = true;
        //cout << "AA" << endl;
        new_sol->p_to_pu[process_id] = processor_id;
        new_sol->pu_to_p[processor_id].insert(process_id);
        new_sol->recalc[processor_id].first = true;
        //cout << "BB" << endl;
        return new_sol;
    }

    string dumps() override {
        stringstream ss;
        ss << n << ' ' << m << ' ';
        for (int i = 0; i < n; ++i) {
            ss << processes[i].first << ' ' << processes[i].second << ' ';
        }
        for (int i = 0; i < m; ++i) {
            ss << pu_to_p[i].size() << ' ';
            for (int x : pu_to_p[i])
                ss << x << ' ';
        }
        return ss.str();
    }

    ~Solution() {
        p_to_pu.clear();
        pu_to_p.clear();
        processes.clear();
        recalc.clear();
    }
};


class Mutation {

public:
    static void exec_mutation(SolutionABC* &sol, double temp, SolutionABC* best) {
        int n = static_cast<Solution*>(sol)->n;
        int m = static_cast<Solution*>(sol)->m;
        int process = random(0, n);
        int processor = random(0, m);
        //cout << "process/processor " << process << '/' << processor << endl;
        SolutionABC* new_sol = sol->move_process(process, processor);
        long long int delta = new_sol->count_metrics() - sol->count_metrics();
        if (delta <= 0.0) {
            if (best != sol)
                delete sol;
            sol = new_sol;
            return;
        }
        //cout << "Checking" << endl;
        if (temp > 1e-20 && choice(exp(-delta/temp))) {
            if (best != sol)
                delete sol;
            sol = new_sol;
        }
        else {
            delete new_sol;
        }
        return;
    }
};


class TempDecreaseABC {
public:
    virtual double decrease(double temp, int iter) = 0;
};


class TempBoltz : public TempDecreaseABC {
public:
    double decrease(double temp, int iter) override {
        return temp / log(1+iter);
    }
};


class TempCauchy : public TempDecreaseABC {
public:
    double decrease(double temp, int iter) override {
        return temp / (1+iter);
    }
};


class TempComb : public TempDecreaseABC {
public:
    double decrease(double temp, int iter) override {
        return temp * log(1+iter) / (1+iter);
    }
};
 

class Immitation {
    int n, m, n_proc, k;
    double temp;
    SolutionABC* curr;
    TempDecreaseABC *t;
public:
    Immitation(int n, int m, vector<pair<int, int>> &processes, int n_proc, int k, TempDecreaseABC *tp) : n(n), temp(100.0), m(m), k(k), n_proc(n_proc), t(tp), curr(new Solution(n, m, processes)) {}

    SolutionABC* main_loop() {
        int curr_steps = 0;
        // have to create a queue
        // need to decrease temperature in the main thread also
        long long int best_metrics = curr->count_metrics();
        while (curr_steps < k) {
            ////cout << best_metrics << '\n';
            vector<int> socks;
            //cout << "Current best: " << best_metrics << endl;
            curr_steps++;
            for (int i = 0; i < n_proc; ++i) {
                // fork n_proc times and call single_process method, then collect the results
                int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
                int opt = 1;
                setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
                sockaddr_in serverAddress;
                serverAddress.sin_family = AF_INET;
                serverAddress.sin_port = htons(12365+i);
                serverAddress.sin_addr.s_addr = INADDR_ANY;
                bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
                listen(serverSocket, 2);
                socks.push_back(serverSocket);
                int pid = fork();
                if (pid == 0) {
                    //cout << "Child process\n";
                    ////cout.flush();
                    SolutionABC* best = single_process();
                    //cout << "solution ready\nResulted metrics: " << best->count_metrics();
                    //cout << best->dumps() << endl << endl << endl << endl << endl;
                    ////cout.flush();
                    // подключиться к сокету и отправить туда best.dumps()
                    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
                    //setsockopt(clientSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
                    int x = connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
                    if (x == -1){
                        //cout << "Connection error for " << i << endl;
                    }
                    string res = best->dumps();
                    delete best;
                    char data[res.length() + 1];
                    strcpy(data, res.c_str());
                    write(clientSocket, data, sizeof(data));
                    ////cout << "Message sent " << i << "\n";
                    ////cout.flush();
                    close(clientSocket);
                    close(serverSocket);
                    exit(0);
                }
            }
            for (int i = 0; i < n_proc; ++i) {
                ////cout << "Waiting for " << i << "-th process" << endl;
                int serverSocket = socks[i];
                int clientSocket = accept(serverSocket, nullptr, nullptr);
                ////cout << "Accepted " << i << endl;
                string received;
                char buffer[1000000];
                int recved = 0;
                //cout << "\n=========\n" << endl;
                while ((recved = recv(clientSocket, buffer, 1000000, 0))) {
                    buffer[recved] = 0;
                    string buf{buffer};
                    int j = buf.size()-1;
                    while (j >= 0 && buf[j] != ' ' && (buf[j] < '0' || buf[j] > '9'))
                        j--;
                    if (j < 0)
                        break;
                    buf = buf.substr(0, j+1);
                    //buf += " "; // лишний пробел 
                    received += buf;
                }
                //cout << received;
                //cout << "\n\n=================== \n" << endl;
                SolutionABC* kek{new Solution(received)};
                //cout << "Counting metrics" << endl;
                long long int curr_metrics = kek->count_metrics();
                //cout << "Received metrics: " << curr_metrics << endl;
                if (curr_metrics < best_metrics) {
                    best_metrics = curr_metrics;
                    delete curr;
                    curr = kek;
                    curr_steps = 0;
                }
                else {
                    delete kek;
                }
                close(serverSocket);
                close(clientSocket);
            }
        }
        return curr;
    }

    SolutionABC* single_process() {
        // the best solution of a process is sent, not the latest one
        SolutionABC* best{new Solution(*(static_cast<Solution*>(curr)))};
        long long int best_metrics = best->count_metrics();
        bool flag = true;
        int iteration = 1;
        while (flag) {
            flag = false;
            for (int i = 0; i < 100; ++i) {
                Mutation::exec_mutation(curr, temp, best);
                long long int curr_metrics = curr->count_metrics();
                if (curr_metrics < best_metrics) {
                    delete best;
                    best = curr;
                    best_metrics = curr_metrics;
                    //cout << "New local best metrics: " << best_metrics << endl;
                    //cout.flush();
                    flag = true;
                }
            }
            //cout.flush();
            temp = t->decrease(temp, iteration);
            if (temp < 1.0)
                break;
            iteration++;
        }
        //cout.flush();
        return best;
    }
};

int main() {
    int n, m, n_proc, temp_type;
    cin >> n >> m >> n_proc >> temp_type;
    vector<pair<int, int>> vect(n);
    for (int i = 0; i < n; ++i) {
        cin >> vect[i].second;
        vect[i].first = i;
    }
    TempDecreaseABC* t = nullptr;
    sort(vect.begin(), vect.end(), [](pair<int, int> &a, pair<int, int> &b) {return a.second < b.second;});
    switch (temp_type) {
        case 1:
            t = new TempBoltz();
        case 2:
            t = new TempCauchy();
        case 3:
            t = new TempComb();
    }
    Immitation imit = Immitation(n, m, vect, n_proc, 10, t);
    Solution* res = static_cast<Solution*>(imit.main_loop());
    cout << res->count_metrics() << endl;
    return 0;
}