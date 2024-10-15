#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class Logger {
    fs::path directory;
public:
    Logger() {
        directory = fs::current_path() / "logs";
        std::cout << directory << std::endl;
        if (!fs::exists(directory)) {
            std::cout << "Creating\n";
            fs::create_directory(directory);
        }
        else {
            for (auto& x : fs::directory_iterator(directory)) {
                fs::remove(x.path());
            }
        }
    }

    void log_round(int round, std::string str) {
        fs::path outp = directory / ("round_" + std::to_string(round));
        std::ofstream output(outp, std::ios::app);
        output << str << std::endl;
        output.close();
    }

    void log_results(std::string str) {
        fs::path outp = directory / "results";
        std::ofstream output(outp, std::ios::app);
        output << str << std::endl;
        output.close();
    }
};
