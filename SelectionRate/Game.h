#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <string>

class Game
{
private:
    enum class State
    {
        Menu,
        Settings,
        Playing,
        GameOver
    };

    enum class Mode
    {
        Standard,
        Endless
    };

    struct Target
    {
        sf::Vector2f position;
        float radius;
        bool moving;
        float speedX;
        float speedY;
        float difficultyMultiplier;
        sf::Color outlineColor;
    };

    struct BulletMark
    {
        sf::CircleShape shape;
        sf::Clock lifeClock;
    };

    struct HitEffect
    {
        sf::CircleShape shape;
        sf::Clock lifeClock;
        float growthSpeed;
    };

    struct ShotFlash
    {
        sf::CircleShape shape;
        sf::Clock lifeClock;
        float growthSpeed;
    };

    sf::RenderWindow window;
    std::vector<Target> targets;
    std::vector<BulletMark> bulletMarks;
    std::vector<HitEffect> hitEffects;
    std::vector<ShotFlash> shotFlashes;
    sf::Font font;

    // Main menu
    std::optional<sf::Text> titleText;
    std::optional<sf::Text> standardText;
    std::optional<sf::Text> endlessText;
    std::optional<sf::Text> settingsText;
    std::optional<sf::Text> exitText;

    // Settings menu
    std::optional<sf::Text> settingsTitleText;
    std::optional<sf::Text> spawnRateLabelText;
    std::optional<sf::Text> spawnRateValueText;
    std::optional<sf::Text> spawnRateMinusText;
    std::optional<sf::Text> spawnRatePlusText;
    std::optional<sf::Text> resetRecordsText;
    std::optional<sf::Text> backText;
    std::optional<sf::Text> settingsInfoText;

    sf::RectangleShape spawnMinusButton;
    sf::RectangleShape spawnPlusButton;
    sf::RectangleShape resetRecordsButton;
    sf::RectangleShape backButton;

    // HUD
    std::optional<sf::Text> scoreText;
    std::optional<sf::Text> timerText;
    std::optional<sf::Text> missesText;
    std::optional<sf::Text> idleText;
    std::optional<sf::Text> comboText;

    // Game over
    std::optional<sf::Text> gameOverText;
    std::optional<sf::Text> finalScoreText;
    std::optional<sf::Text> restartText;
    std::optional<sf::Text> menuText;
    std::optional<sf::Text> shareRecordText;
    std::optional<sf::Text> exitGameOverText;
    std::optional<sf::Text> shareStatusText;

    // Shared info
    std::optional<sf::Text> hintText;
    std::optional<sf::Text> bestScoreStandardMenuText;
    std::optional<sf::Text> bestScoreEndlessMenuText;
    std::optional<sf::Text> bestScoreGameOverText;
    std::optional<sf::Text> currentModeText;

    int score;
    int timeLeft;
    int comboHits;

    int bestScoreStandard;
    int bestScoreEndless;
    int bestTimeEndless;

    int misses;
    int maxMisses;
    int endlessTimeSeconds;

    float comboMultiplier;

    State gameState;
    Mode currentMode;

    sf::Clock gameClock;
    sf::Clock spawnClock;
    sf::Clock inactivityClock;
    sf::Clock shareStatusClock;

    float spawnInterval;
    int maxTargets;
    float maxInactivityTime;
    bool showShareStatus;

    void processEvents();
    void update();
    void render();

    void setupText();
    void setupTarget();
    void spawnTarget();
    void updateTargets();
    void updateBulletMarks();
    void updateHitEffects();
    void updateShotFlashes();

    void addBulletMark(float x, float y);
    void addHitEffect(float x, float y, float radius);
    void addShotFlash(float x, float y);

    void updateGameplayText();
    void updateTextPositions();
    void updateButtonHover();

    void startGame(Mode mode);
    void resetGame();
    void returnToMenu();
    void openSettings();

    int getClickedTargetIndex(int mouseX, int mouseY) const;
    bool isTextClicked(const sf::Text& text, int mouseX, int mouseY) const;
    int getBasePointsByRadius(float radius) const;
    std::string formatTime(int totalSeconds) const;

    void drawTarget(const Target& target);
    void drawBulletMark(const BulletMark& mark);
    void drawHitEffect(const HitEffect& effect);
    void drawShotFlash(const ShotFlash& flash);

    void loadBestScore();
    void saveBestScore();
    void resetRecords();
    void checkBestScore();

    std::string getCurrentRecordString() const;
    bool copyToClipboard(const std::string& text);
    void showShareCopiedMessage(const std::string& message);

public:
    Game();
    void run();
};