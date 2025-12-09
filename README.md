🎯 Advanced Number Guessing Game — C++17

A fully–featured, industry-style number guessing game built using modern C++17, focusing on clean design, modular structure, and real-world software engineering principles.

🚀 Features

🧠 Smart Gameplay

Random number guessing with dynamic difficulty

Live updating hint range (too high, too low)

Fully validated player input

Optional attempt limits

Time-based scoring

⚙️ Software Engineering Quality

Modern C++17 design (MT19937_64 RNG, chrono timers, robust input handling)

Clean modular functions

Clear UX with instant prompt display

Cross-platform Windows/Linux compatibility

No dependency on external libraries

🏆 Leaderboard System

Stores player name, timestamp, score, attempts, and time taken

Persistent leaderboard.csv file

CSV-safe formatting

Automatic parsing and display of top entries

📂 Project Structure

numberGuessing.cpp        → Main source file with complete game logic

leaderboard.csv           → Auto-created on game completion (optional)

🧩 How to Build & Run
🔧 Compile (g++ recommended)

g++ -std=c++17 -O2 -o numberGuessing numberGuessing.cpp

▶️ Run

./numberGuessing

🛠 Technologies Used

C++17 (Modern STL, chrono, mt19937_64 RNG)

File I/O (CSV logging)

Input validation & error handling

Cross-platform console I/O

🖼 Demo (Console Preview)

=== Advanced Number Guessing Game ===

Choose difficulty:

  1) Easy   (1 - 20)
  2) Medium (1 - 100)
  3) Hard   (1 - 1000)
  4) Custom
Enter choice [1-4]:

🧮 Scoring System

Score is dynamically calculated based on:

Range size

Attempts used

Time taken

Accuracy and efficiency

Rewards faster and more accurate guesses!

📊 Leaderboard Example
Time                 Player         Diff        Att   Sec    Score
----------------------------------------------------------------------
2025-01-10 18:21:33  Harsh          Medium      7     12.3   82.14
2025-01-10 18:23:50  Anonymous      Easy        3      5.1   91.02

💡 Future Improvements

GUI version (SFML / Qt)

Online leaderboard sync

Difficulty auto-adjustment

Unit tests (GoogleTest)

🤝 Contributing

Pull requests and suggestions are always welcome!

If you'd like to improve UX, scoring, or add new features—feel free to contribute.

⭐ Show Your Support

If you like this project, consider giving it a ⭐ on GitHub!
