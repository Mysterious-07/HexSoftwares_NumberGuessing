// numberGuessing.cpp
// Advanced Number Guessing Game - in a single-file C++17

#include <iostream>
#include <random>
#include <chrono>
#include <string>
#include <limits>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <cctype>

using namespace std;
using Clock = chrono::steady_clock;

struct GameConfig {
    string difficultyName = "Custom";
    int minValue = 1;
    int maxValue = 100;
    int maxAttempts = 0; // 0 means unlimited
};

struct Result {
    string playerName;
    string difficulty;
    int attempts;
    double elapsedSeconds;
    int secretNumber;
    double score;
    string timestamp;
};

// ---------- Utility helpers ----------
string now_iso8601() {
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);

    tm tm_buf{};
    tm *tmp = std::localtime(&t);   // universally supported (not thread-safe)

    if (tmp)
        tm_buf = *tmp;
    else
        tm_buf = tm();  // fallback zero init

    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
    return string(buf);
}

// Read line safely
string safe_getline() {
    string s;
    getline(cin, s);
    return s;
}

// Prompt and get integer with validation
int prompt_int(const string &prompt, int minAllowed = numeric_limits<int>::min(), int maxAllowed = numeric_limits<int>::max()) {
    while (true) {
        cout << prompt;
        cout.flush(); // ensure prompt appears immediately
        string line;
        if (!getline(cin, line)) {
            // EOF or error
            cout << "\nInput error. Exiting.\n";
            exit(0);
        }
        // trim
        auto start = line.find_first_not_of(" \t\r\n");
        if (start == string::npos) { cout << "Please enter a value.\n"; continue; }
        auto end = line.find_last_not_of(" \t\r\n");
        string trimmed = line.substr(start, end - start + 1);
        try {
            size_t idx = 0;
            long long val = stoll(trimmed, &idx);
            if (idx != trimmed.size()) throw invalid_argument("extra chars");
            if (val < minAllowed || val > maxAllowed) {
                cout << "Enter a number between " << minAllowed << " and " << maxAllowed << ".\n";
                continue;
            }
            return static_cast<int>(val);
        } catch (...) {
            cout << "Invalid input. Please enter an integer.\n";
        }
    }
}

// Prompt yes/no
bool prompt_yesno(const string &prompt) {
    while (true) {
        cout << prompt << " (y/n): ";
        cout.flush(); // ensure prompt appears immediately
        string line;
        if (!getline(cin, line)) exit(0);
        if (line.empty()) continue;
        // find first non-space char
        size_t i = line.find_first_not_of(" \t\r\n");
        if (i == string::npos) continue;
        char c = static_cast<char>(tolower(static_cast<unsigned char>(line[i])));
        if (c == 'y') return true;
        if (c == 'n') return false;
        cout << "Please reply with 'y' or 'n'.\n";
    }
}

// ---------- Leaderboard persistence ----------
const string LEADERBOARD_FILE = "leaderboard.csv";

void append_to_leaderboard(const Result &r) {
    ofstream ofs(LEADERBOARD_FILE, ios::app);
    if (!ofs) {
        cerr << "Warning: could not write leaderboard file.\n";
        return;
    }
    // CSV: timestamp,player,difficulty,attempts,seconds,secret,score
    ofs << "\"" << r.timestamp   << "\","
        << "\"" << r.playerName  << "\","
        << "\"" << r.difficulty  << "\","
        << r.attempts            << ","
        << fixed << setprecision(2) << r.elapsedSeconds << ","
        << r.secretNumber        << ","
        << fixed << setprecision(2) << r.score << "\n";
}

vector<Result> read_leaderboard(int limit = 10) {
    vector<Result> out;
    ifstream ifs(LEADERBOARD_FILE);
    if (!ifs) return out;
    string line;
    while (getline(ifs, line)) {
        if (line.empty()) continue;
        // crude CSV parse (we wrote with quotes for strings)
        stringstream ss(line);
        string item;
        Result r;
        // timestamp
        if (!getline(ss, item, ',')) continue;
        if (item.size() >= 2 && item.front() == '"') item = item.substr(1, item.size()-2);
        r.timestamp = item;
        // player
        if (!getline(ss, item, ',')) continue;
        if (item.size() >= 2 && item.front() == '"') item = item.substr(1, item.size()-2);
        r.playerName = item;
        // difficulty
        if (!getline(ss, item, ',')) continue;
        if (item.size() >= 2 && item.front() == '"') item = item.substr(1, item.size()-2);
        r.difficulty = item;
        // attempts
        if (!getline(ss, item, ',')) continue; r.attempts = stoi(item);
        // seconds
        if (!getline(ss, item, ',')) continue; r.elapsedSeconds = stod(item);
        // secret
        if (!getline(ss, item, ',')) continue; r.secretNumber = stoi(item);
        // score
        if (!getline(ss, item, ',')) continue; r.score = stod(item);
        out.push_back(r);
        if ((int)out.size() >= limit) break;
    }
    return out;
}

void show_leaderboard(int n = 10) {
    auto entries = read_leaderboard(n);
    if (entries.empty()) { cout << "No leaderboard entries yet.\n"; return; }
    cout << "\nTop " << entries.size() << " recent games:\n";
    cout << left << setw(20) << "Time" << setw(15) << "Player" << setw(12) << "Diff" << setw(10) << "Att" << setw(10) << "Sec" << setw(10) << "Score" << '\n';
    cout << string(80, '-') << '\n';
    for (auto &e : entries) {
        cout << setw(20) << e.timestamp << setw(15) << e.playerName << setw(12) << e.difficulty << setw(10) << e.attempts << setw(10) << fixed << setprecision(1) << e.elapsedSeconds << setw(10) << fixed << setprecision(2) << e.score << '\n';
    }
    cout << '\n';
}

// ---------- Game logic ----------
// Create a random integer in [minv, maxv]
// Use fast time-seeded mt19937 to avoid slow random_device on some Windows/MinGW setups
int random_int(int minv, int maxv) {
    static uint64_t seed = static_cast<uint64_t>(Clock::now().time_since_epoch().count());
    static mt19937_64 gen(seed);
    uniform_int_distribution<int> dist(minv, maxv);
    return dist(gen);
}

// Score formula
double compute_score(int attempts, double secondsElapsed, const GameConfig &cfg) {
    double rangeSize = static_cast<double>(cfg.maxValue - cfg.minValue + 1);
    double base = 1000.0 / log2(rangeSize + 1.0);
    double attemptPenalty = 20.0 * (attempts - 1);
    double timePenalty = secondsElapsed / 2.0;
    double score = base - attemptPenalty - timePenalty;
    if (cfg.maxAttempts > 0) {
        double frac = static_cast<double>(attempts) / cfg.maxAttempts;
        score *= (1.0 + max(0.0, 0.5 - frac));
    }
    if (score < 0) score = 0;
    return score;
}

GameConfig choose_difficulty() {
    cout << "Choose difficulty:\n";
    cout << "  1) Easy   (1 - 20, unlimited attempts)\n";
    cout << "  2) Medium (1 - 100, 10 attempts)\n";
    cout << "  3) Hard   (1 - 1000, 12 attempts)\n";
    cout << "  4) Custom\n";
    int choice = prompt_int("Enter choice [1-4]: ", 1, 4);
    GameConfig cfg;
    switch (choice) {
        case 1:
            cfg.difficultyName = "Easy";
            cfg.minValue = 1; cfg.maxValue = 20; cfg.maxAttempts = 0;
            break;
        case 2:
            cfg.difficultyName = "Medium";
            cfg.minValue = 1; cfg.maxValue = 100; cfg.maxAttempts = 10;
            break;
        case 3:
            cfg.difficultyName = "Hard";
            cfg.minValue = 1; cfg.maxValue = 1000; cfg.maxAttempts = 12;
            break;
        case 4:
        default:
            cfg.difficultyName = "Custom";
            cout << "Enter minimum value: ";
            cfg.minValue = prompt_int("", -1000000, 1000000);
            cout << "Enter maximum value: ";
            cfg.maxValue = prompt_int("", cfg.minValue+1, 1000000);
            if (prompt_yesno("Would you like to set a maximum attempts limit?")) {
                cfg.maxAttempts = prompt_int("Enter maximum attempts (>=1): ", 1, 1000000);
            } else cfg.maxAttempts = 0;
            break;
    }
    cout << "You selected: " << cfg.difficultyName << " (" << cfg.minValue << " - " << cfg.maxValue << ")";
    if (cfg.maxAttempts > 0) cout << ", max attempts = " << cfg.maxAttempts;
    cout << '\n';
    return cfg;
}

Result play_game(const GameConfig &cfg) {
    int secret = random_int(cfg.minValue, cfg.maxValue);
    int attempts = 0;
    int lowHint = cfg.minValue, highHint = cfg.maxValue;

    cout << "\nI have selected a number between " << cfg.minValue << " and " << cfg.maxValue << ".\n";
    if (cfg.maxAttempts > 0) cout << "You have up to " << cfg.maxAttempts << " attempts.\n";
    cout << "Type your guess and press Enter.\n";

    auto start = Clock::now();
    while (true) {
        cout << "Allowed range: [" << lowHint << " - " << highHint << "] ";
        cout.flush(); // show prompt immediately
        string prompt = "Enter guess (or 0 to give up): ";
        int guess = prompt_int(prompt, numeric_limits<int>::min(), numeric_limits<int>::max());

        if (guess == 0) {
            cout << "You gave up. The number was " << secret << ".\n";
            break;
        }

        attempts++;

        if (guess == secret) {
            cout << "Congratulations! You guessed correctly in " << attempts << " attempts.\n";
            break;
        } else if (guess > secret) {
            cout << "Too high.\n";
            if (guess - 1 < highHint) highHint = guess - 1;
        } else {
            cout << "Too low.\n";
            if (guess + 1 > lowHint) lowHint = guess + 1;
        }

        if (cfg.maxAttempts > 0 && attempts >= cfg.maxAttempts) {
            cout << "Reached maximum attempts (" << cfg.maxAttempts << "). You lose. The number was " << secret << ".\n";
            break;
        }
    }
    auto end = Clock::now();
    double elapsed = chrono::duration_cast<chrono::duration<double>>(end - start).count();

    Result res;
    cout << "\nEnter your name for the leaderboard (leave blank to skip): ";
    cout.flush();
    string name = safe_getline();
    if (name.empty()) name = "Anonymous";
    res.playerName = name;
    res.difficulty = cfg.difficultyName + " (" + to_string(cfg.minValue) + "-" + to_string(cfg.maxValue) + ")";
    res.attempts = attempts;
    res.elapsedSeconds = elapsed;
    res.secretNumber = secret;
    res.timestamp = now_iso8601();
    res.score = compute_score(max(1, attempts), elapsed, cfg);

    if (name != "Anonymous") append_to_leaderboard(res);
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "=== Advanced Number Guessing Game ===\n";
    cout << "(Type CTRL+D or CTRL+Z to exit any time)\n\n";
    cout.flush();

    while (true) {
        GameConfig cfg = choose_difficulty();
        Result r = play_game(cfg);

        cout << "\nGame summary:\n";
        cout << " Player: " << r.playerName << '\n';
        cout << " Difficulty: " << r.difficulty << '\n';
        cout << " Attempts: " << r.attempts << '\n';
        cout << " Time: " << fixed << setprecision(1) << r.elapsedSeconds << " seconds\n";
        cout << " Score: " << fixed << setprecision(2) << r.score << "\n";

        if (prompt_yesno("Would you like to view the recent leaderboard?")) {
            show_leaderboard(10);
        }

        if (!prompt_yesno("Play again?")) break;
        cout << "\n";
    }

    cout << "Thanks for playing! Goodbye.\n";
    return 0;
}
