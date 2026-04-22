#include "Game.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <optional>
#include <string>
#include <fstream>
#include <cstdio>

Game::Game()
    : score(0),
    timeLeft(60),
    comboHits(0),
    bestScoreStandard(0),
    bestScoreEndless(0),
    bestTimeEndless(0),
    misses(0),
    maxMisses(10),
    endlessTimeSeconds(0),
    comboMultiplier(1.0f),
    gameState(State::Menu),
    currentMode(Mode::Standard),
    spawnInterval(1.0f),
    maxTargets(5),
    maxInactivityTime(5.0f)
{
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;

    window.create(
        sf::VideoMode::getDesktopMode(),
        "Tir",
        sf::State::Fullscreen,
        settings
    );

    window.setFramerateLimit(60);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    loadBestScore();
    setupTarget();
    setupText();
    updateTextPositions();
    updateGameplayText();
}

std::string Game::formatTime(int totalSeconds) const
{
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    std::string minStr = (minutes < 10 ? "0" : "") + std::to_string(minutes);
    std::string secStr = (seconds < 10 ? "0" : "") + std::to_string(seconds);

    return minStr + ":" + secStr;
}

void Game::loadBestScore()
{
    std::ifstream file("best_score.txt");

    if (file.is_open())
    {
        file >> bestScoreStandard >> bestScoreEndless >> bestTimeEndless;
        file.close();
    }
    else
    {
        bestScoreStandard = 0;
        bestScoreEndless = 0;
        bestTimeEndless = 0;
    }
}

void Game::saveBestScore()
{
    std::ofstream file("best_score.txt");

    if (file.is_open())
    {
        file << bestScoreStandard << " "
            << bestScoreEndless << " "
            << bestTimeEndless;
        file.close();
    }
}

void Game::checkBestScore()
{
    if (currentMode == Mode::Standard)
    {
        if (score > bestScoreStandard)
        {
            bestScoreStandard = score;
            saveBestScore();
        }
    }
    else if (currentMode == Mode::Endless)
    {
        if (score > bestScoreEndless)
        {
            bestScoreEndless = score;
            bestTimeEndless = endlessTimeSeconds;
            saveBestScore();
        }
        else if (score == bestScoreEndless)
        {
            if (bestTimeEndless == 0 || endlessTimeSeconds < bestTimeEndless)
            {
                bestTimeEndless = endlessTimeSeconds;
                saveBestScore();
            }
        }
    }
}

void Game::setupTarget()
{
}

void Game::spawnTarget()
{
    if (targets.size() >= static_cast<size_t>(maxTargets))
        return;

    Target target;
    int type = std::rand() % 3;

    if (type == 0)
        target.radius = 60.f;
    else if (type == 1)
        target.radius = 40.f;
    else
        target.radius = 20.f;

    auto size = window.getSize();

    int maxX = static_cast<int>(size.x - target.radius * 2.f);
    int maxY = static_cast<int>(size.y - target.radius * 2.f);

    if (maxX < 1) maxX = 1;
    if (maxY < 1) maxY = 1;

    float x = static_cast<float>(std::rand() % maxX);
    float y = static_cast<float>(std::rand() % maxY);

    target.position = { x, y };
    target.moving = (std::rand() % 2 == 0);

    if (target.moving)
    {
        int speedType = std::rand() % 3;

        if (speedType == 0)
        {
            target.speedX = (std::rand() % 2 == 0 ? 1.f : -1.f) * 2.0f;
            target.speedY = (std::rand() % 2 == 0 ? 1.f : -1.f) * 2.0f;
            target.difficultyMultiplier = 1.25f;
            target.outlineColor = sf::Color(255, 170, 60);
        }
        else if (speedType == 1)
        {
            target.speedX = (std::rand() % 2 == 0 ? 1.f : -1.f) * 4.0f;
            target.speedY = (std::rand() % 2 == 0 ? 1.f : -1.f) * 4.0f;
            target.difficultyMultiplier = 1.5f;
            target.outlineColor = sf::Color(180, 80, 255);
        }
        else
        {
            target.speedX = (std::rand() % 2 == 0 ? 1.f : -1.f) * 6.0f;
            target.speedY = (std::rand() % 2 == 0 ? 1.f : -1.f) * 6.0f;
            target.difficultyMultiplier = 2.0f;
            target.outlineColor = sf::Color(80, 140, 255);
        }
    }
    else
    {
        target.speedX = 0.f;
        target.speedY = 0.f;
        target.difficultyMultiplier = 1.0f;
        target.outlineColor = sf::Color::Black;
    }

    targets.push_back(target);
}

void Game::setupText()
{
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf"))
        return;

    titleText.emplace(font, "SELECTION RATE", 64);
    standardText.emplace(font, "STANDARD", 40);
    endlessText.emplace(font, "ENDLESS", 40);
    exitText.emplace(font, "EXIT", 40);
    restartText.emplace(font, "RESTART", 40);

    scoreText.emplace(font, "Score: 0", 28);
    timerText.emplace(font, "Time: 60", 28);
    missesText.emplace(font, "Misses: 0/10", 28);
    idleText.emplace(font, "Idle: 5", 28);
    comboText.emplace(font, "Combo x1.00", 28);

    gameOverText.emplace(font, "GAME OVER", 58);
    finalScoreText.emplace(font, "Final score: 0", 34);
    hintText.emplace(font, "ESC - Menu", 24);
    bestScoreStandardMenuText.emplace(font, "Best Standard: 0", 28);
    bestScoreEndlessMenuText.emplace(font, "Best Endless: 0 pts | 00:00", 28);
    bestScoreGameOverText.emplace(font, "Best score: 0", 32);
    currentModeText.emplace(font, "Mode: Standard", 28);

    titleText->setFillColor(sf::Color::Black);
    standardText->setFillColor(sf::Color::Black);
    endlessText->setFillColor(sf::Color::Black);
    exitText->setFillColor(sf::Color::Black);
    restartText->setFillColor(sf::Color::Black);

    scoreText->setFillColor(sf::Color::Black);
    timerText->setFillColor(sf::Color::Black);
    missesText->setFillColor(sf::Color::Black);
    idleText->setFillColor(sf::Color::Black);
    comboText->setFillColor(sf::Color::Black);

    gameOverText->setFillColor(sf::Color::Black);
    finalScoreText->setFillColor(sf::Color::Black);
    hintText->setFillColor(sf::Color(70, 70, 70));
    bestScoreStandardMenuText->setFillColor(sf::Color::Black);
    bestScoreEndlessMenuText->setFillColor(sf::Color::Black);
    bestScoreGameOverText->setFillColor(sf::Color::Black);
    currentModeText->setFillColor(sf::Color::Black);
}

void Game::updateGameplayText()
{
    if (scoreText)
        scoreText->setString("Score: " + std::to_string(score));

    if (timerText)
    {
        if (currentMode == Mode::Standard)
            timerText->setString("Time: " + std::to_string(timeLeft));
        else
            timerText->setString("Time: " + formatTime(endlessTimeSeconds));
    }

    if (missesText)
        missesText->setString("Misses: " + std::to_string(misses) + "/" + std::to_string(maxMisses));

    if (idleText)
    {
        int idleLeft = static_cast<int>(std::ceil(maxInactivityTime - inactivityClock.getElapsedTime().asSeconds()));
        if (idleLeft < 0)
            idleLeft = 0;

        idleText->setString("Idle: " + std::to_string(idleLeft));
    }

    if (comboText)
    {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "Combo x%.2f", comboMultiplier);
        comboText->setString(buffer);
    }

    if (finalScoreText)
    {
        if (currentMode == Mode::Standard)
        {
            finalScoreText->setString("Final score: " + std::to_string(score));
        }
        else
        {
            finalScoreText->setString(
                "Final score: " + std::to_string(score) +
                " | " + formatTime(endlessTimeSeconds)
            );
        }
    }

    if (bestScoreStandardMenuText)
        bestScoreStandardMenuText->setString("Best Standard: " + std::to_string(bestScoreStandard));

    if (bestScoreEndlessMenuText)
        bestScoreEndlessMenuText->setString(
            "Best Endless: " + std::to_string(bestScoreEndless) +
            " pts | " + formatTime(bestTimeEndless)
        );

    if (bestScoreGameOverText)
    {
        if (currentMode == Mode::Standard)
        {
            bestScoreGameOverText->setString("Best Standard: " + std::to_string(bestScoreStandard));
        }
        else
        {
            bestScoreGameOverText->setString(
                "Best Endless: " + std::to_string(bestScoreEndless) +
                " pts | " + formatTime(bestTimeEndless)
            );
        }
    }

    if (currentModeText)
    {
        if (currentMode == Mode::Standard)
            currentModeText->setString("Mode: Standard");
        else
            currentModeText->setString("Mode: Endless");
    }
}

void Game::updateTextPositions()
{
    auto size = window.getSize();
    float w = static_cast<float>(size.x);
    float h = static_cast<float>(size.y);

    auto centerTextX = [](sf::Text& text, float x)
        {
            sf::FloatRect bounds = text.getLocalBounds();
            text.setOrigin({ bounds.position.x + bounds.size.x / 2.f, 0.f });
            text.setPosition({ x, text.getPosition().y });
        };

    if (titleText)
    {
        titleText->setPosition({ w / 2.f, h * 0.13f });
        centerTextX(*titleText, w / 2.f);
    }

    if (standardText)
    {
        standardText->setPosition({ w / 2.f, h * 0.33f });
        centerTextX(*standardText, w / 2.f);
    }

    if (endlessText)
    {
        endlessText->setPosition({ w / 2.f, h * 0.43f });
        centerTextX(*endlessText, w / 2.f);
    }

    if (restartText)
    {
        restartText->setPosition({ w / 2.f, h * 0.43f });
        centerTextX(*restartText, w / 2.f);
    }

    if (exitText)
    {
        exitText->setPosition({ w / 2.f, h * 0.55f });
        centerTextX(*exitText, w / 2.f);
    }

    if (gameOverText)
    {
        gameOverText->setPosition({ w / 2.f, h * 0.16f });
        centerTextX(*gameOverText, w / 2.f);
    }

    if (finalScoreText)
    {
        finalScoreText->setPosition({ w / 2.f, h * 0.26f });
        centerTextX(*finalScoreText, w / 2.f);
    }

    if (bestScoreStandardMenuText)
    {
        bestScoreStandardMenuText->setPosition({ w / 2.f, h * 0.66f });
        centerTextX(*bestScoreStandardMenuText, w / 2.f);
    }

    if (bestScoreEndlessMenuText)
    {
        bestScoreEndlessMenuText->setPosition({ w / 2.f, h * 0.72f });
        centerTextX(*bestScoreEndlessMenuText, w / 2.f);
    }

    if (bestScoreGameOverText)
    {
        bestScoreGameOverText->setPosition({ w * 0.82f, h * 0.22f });
        centerTextX(*bestScoreGameOverText, w * 0.82f);
    }

    if (scoreText)
        scoreText->setPosition({ 20.f, 20.f });

    if (currentModeText)
        currentModeText->setPosition({ 20.f, 60.f });

    if (comboText)
        comboText->setPosition({ 20.f, 100.f });

    if (timerText)
    {
        sf::FloatRect bounds = timerText->getLocalBounds();
        timerText->setPosition({ w - bounds.size.x - 30.f, 20.f });
    }

    if (missesText)
    {
        sf::FloatRect bounds = missesText->getLocalBounds();
        missesText->setPosition({ w - bounds.size.x - 30.f, 60.f });
    }

    if (idleText)
    {
        sf::FloatRect bounds = idleText->getLocalBounds();
        idleText->setPosition({ w - bounds.size.x - 30.f, 100.f });
    }

    if (hintText)
        hintText->setPosition({ 20.f, h - 45.f });
}

void Game::startGame(Mode mode)
{
    currentMode = mode;
    score = 0;
    timeLeft = 60;
    misses = 0;
    comboHits = 0;
    endlessTimeSeconds = 0;
    comboMultiplier = 1.0f;
    gameState = State::Playing;

    targets.clear();
    bulletMarks.clear();
    hitEffects.clear();
    shotFlashes.clear();

    gameClock.restart();
    spawnClock.restart();
    inactivityClock.restart();

    updateGameplayText();
    updateTextPositions();
}

void Game::resetGame()
{
    startGame(currentMode);
}

void Game::returnToMenu()
{
    gameState = State::Menu;
    targets.clear();
    bulletMarks.clear();
    hitEffects.clear();
    shotFlashes.clear();
    comboHits = 0;
    comboMultiplier = 1.0f;
    timeLeft = 60;
    misses = 0;
    endlessTimeSeconds = 0;

    updateGameplayText();
    updateTextPositions();
}

int Game::getBasePointsByRadius(float radius) const
{
    if (radius >= 50.f)
        return 1;
    if (radius >= 30.f)
        return 2;
    return 4;
}

int Game::getClickedTargetIndex(int mouseX, int mouseY) const
{
    for (int i = 0; i < static_cast<int>(targets.size()); i++)
    {
        float centerX = targets[i].position.x + targets[i].radius;
        float centerY = targets[i].position.y + targets[i].radius;

        float dx = static_cast<float>(mouseX) - centerX;
        float dy = static_cast<float>(mouseY) - centerY;

        if (std::sqrt(dx * dx + dy * dy) <= targets[i].radius)
            return i;
    }

    return -1;
}

bool Game::isTextClicked(const sf::Text& text, int mouseX, int mouseY) const
{
    return text.getGlobalBounds().contains({ static_cast<float>(mouseX), static_cast<float>(mouseY) });
}

void Game::addBulletMark(float x, float y)
{
    BulletMark mark;
    mark.shape.setRadius(7.f);
    mark.shape.setPointCount(64);
    mark.shape.setOrigin({ 7.f, 7.f });
    mark.shape.setPosition({ x, y });
    mark.shape.setFillColor(sf::Color(25, 25, 25));
    mark.shape.setOutlineThickness(2.f);
    mark.shape.setOutlineColor(sf::Color(120, 120, 120));

    bulletMarks.push_back(mark);
}

void Game::addHitEffect(float x, float y, float radius)
{
    HitEffect effect;
    effect.shape.setRadius(radius * 0.35f);
    effect.shape.setPointCount(80);
    effect.shape.setOrigin({ effect.shape.getRadius(), effect.shape.getRadius() });
    effect.shape.setPosition({ x, y });
    effect.shape.setFillColor(sf::Color(255, 220, 120, 120));
    effect.shape.setOutlineThickness(4.f);
    effect.shape.setOutlineColor(sf::Color(255, 180, 60, 220));
    effect.growthSpeed = radius * 0.08f;

    hitEffects.push_back(effect);
}

void Game::addShotFlash(float x, float y)
{
    ShotFlash flash;
    flash.shape.setRadius(10.f);
    flash.shape.setPointCount(80);
    flash.shape.setOrigin({ 10.f, 10.f });
    flash.shape.setPosition({ x, y });
    flash.shape.setFillColor(sf::Color(255, 245, 180, 120));
    flash.shape.setOutlineThickness(3.f);
    flash.shape.setOutlineColor(sf::Color(255, 220, 120, 220));
    flash.growthSpeed = 2.8f;

    shotFlashes.push_back(flash);
}

void Game::updateTargets()
{
    auto size = window.getSize();
    float windowWidth = static_cast<float>(size.x);
    float windowHeight = static_cast<float>(size.y);

    for (auto& target : targets)
    {
        if (!target.moving)
            continue;

        target.position.x += target.speedX;
        target.position.y += target.speedY;

        if (target.position.x <= 0.f || target.position.x + target.radius * 2.f >= windowWidth)
        {
            target.speedX = -target.speedX;
            target.position.x += target.speedX;
        }

        if (target.position.y <= 0.f || target.position.y + target.radius * 2.f >= windowHeight)
        {
            target.speedY = -target.speedY;
            target.position.y += target.speedY;
        }
    }
}

void Game::updateBulletMarks()
{
    for (int i = static_cast<int>(bulletMarks.size()) - 1; i >= 0; i--)
    {
        float life = bulletMarks[i].lifeClock.getElapsedTime().asSeconds();

        if (life >= 2.0f)
        {
            bulletMarks.erase(bulletMarks.begin() + i);
        }
        else
        {
            float alphaRatio = 1.0f - (life / 2.0f);

            sf::Color fill = bulletMarks[i].shape.getFillColor();
            sf::Color outline = bulletMarks[i].shape.getOutlineColor();

            fill.a = static_cast<std::uint8_t>(180.f * alphaRatio);
            outline.a = static_cast<std::uint8_t>(220.f * alphaRatio);

            bulletMarks[i].shape.setFillColor(fill);
            bulletMarks[i].shape.setOutlineColor(outline);
        }
    }
}

void Game::updateHitEffects()
{
    for (int i = static_cast<int>(hitEffects.size()) - 1; i >= 0; i--)
    {
        float life = hitEffects[i].lifeClock.getElapsedTime().asSeconds();

        if (life >= 0.35f)
        {
            hitEffects.erase(hitEffects.begin() + i);
        }
        else
        {
            float alphaRatio = 1.0f - (life / 0.35f);
            float newRadius = hitEffects[i].shape.getRadius() + hitEffects[i].growthSpeed;

            hitEffects[i].shape.setRadius(newRadius);
            hitEffects[i].shape.setOrigin({ newRadius, newRadius });

            sf::Color fill = hitEffects[i].shape.getFillColor();
            sf::Color outline = hitEffects[i].shape.getOutlineColor();

            fill.a = static_cast<std::uint8_t>(100.f * alphaRatio);
            outline.a = static_cast<std::uint8_t>(220.f * alphaRatio);

            hitEffects[i].shape.setFillColor(fill);
            hitEffects[i].shape.setOutlineColor(outline);
        }
    }
}

void Game::updateShotFlashes()
{
    for (int i = static_cast<int>(shotFlashes.size()) - 1; i >= 0; i--)
    {
        float life = shotFlashes[i].lifeClock.getElapsedTime().asSeconds();

        if (life >= 0.10f)
        {
            shotFlashes.erase(shotFlashes.begin() + i);
        }
        else
        {
            float alphaRatio = 1.0f - (life / 0.10f);
            float newRadius = shotFlashes[i].shape.getRadius() + shotFlashes[i].growthSpeed;

            shotFlashes[i].shape.setRadius(newRadius);
            shotFlashes[i].shape.setOrigin({ newRadius, newRadius });

            sf::Color fill = shotFlashes[i].shape.getFillColor();
            sf::Color outline = shotFlashes[i].shape.getOutlineColor();

            fill.a = static_cast<std::uint8_t>(120.f * alphaRatio);
            outline.a = static_cast<std::uint8_t>(220.f * alphaRatio);

            shotFlashes[i].shape.setFillColor(fill);
            shotFlashes[i].shape.setOutlineColor(outline);
        }
    }
}

void Game::drawTarget(const Target& target)
{
    float cx = target.position.x + target.radius;
    float cy = target.position.y + target.radius;

    auto makeCircle = [&](float radius, const sf::Color& color)
        {
            sf::CircleShape circle(radius);
            circle.setPointCount(100);
            circle.setOrigin({ radius, radius });
            circle.setPosition({ cx, cy });
            circle.setFillColor(color);
            return circle;
        };

    sf::CircleShape shadow = makeCircle(target.radius * 1.02f, sf::Color(0, 0, 0, 35));
    shadow.move({ 4.f, 5.f });

    sf::CircleShape outer = makeCircle(target.radius, sf::Color::White);
    outer.setOutlineThickness(4.f);
    outer.setOutlineColor(target.outlineColor);

    sf::CircleShape redRing = makeCircle(target.radius * 0.75f, sf::Color(210, 35, 35));
    sf::CircleShape whiteRing = makeCircle(target.radius * 0.50f, sf::Color::White);
    sf::CircleShape center = makeCircle(target.radius * 0.24f, sf::Color(35, 35, 35));

    window.draw(shadow);
    window.draw(outer);
    window.draw(redRing);
    window.draw(whiteRing);
    window.draw(center);
}

void Game::drawBulletMark(const BulletMark& mark)
{
    window.draw(mark.shape);

    sf::CircleShape ring(mark.shape.getRadius() + 3.f);
    ring.setPointCount(64);
    ring.setOrigin({ ring.getRadius(), ring.getRadius() });
    ring.setPosition(mark.shape.getPosition());
    ring.setFillColor(sf::Color(0, 0, 0, 0));

    sf::Color outline = mark.shape.getOutlineColor();
    ring.setOutlineThickness(1.5f);
    ring.setOutlineColor(outline);

    window.draw(ring);
}

void Game::drawHitEffect(const HitEffect& effect)
{
    window.draw(effect.shape);
}

void Game::drawShotFlash(const ShotFlash& flash)
{
    window.draw(flash.shape);
}

void Game::run()
{
    while (window.isOpen())
    {
        processEvents();
        update();
        render();
    }
}

void Game::processEvents()
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }

        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                if (gameState == State::Playing || gameState == State::GameOver)
                {
                    returnToMenu();
                }
            }
        }

        if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button != sf::Mouse::Button::Left)
                continue;

            int mouseX = mousePressed->position.x;
            int mouseY = mousePressed->position.y;

            if (gameState == State::Menu)
            {
                if (standardText && isTextClicked(*standardText, mouseX, mouseY))
                {
                    startGame(Mode::Standard);
                }
                else if (endlessText && isTextClicked(*endlessText, mouseX, mouseY))
                {
                    startGame(Mode::Endless);
                }
                else if (exitText && isTextClicked(*exitText, mouseX, mouseY))
                {
                    window.close();
                }
            }
            else if (gameState == State::Playing)
            {
                if (currentMode == Mode::Endless)
                {
                    inactivityClock.restart();
                }

                addShotFlash(static_cast<float>(mouseX), static_cast<float>(mouseY));

                int index = getClickedTargetIndex(mouseX, mouseY);

                if (index != -1)
                {
                    float radius = targets[index].radius;
                    float centerX = targets[index].position.x + radius;
                    float centerY = targets[index].position.y + radius;

                    int basePoints = getBasePointsByRadius(radius);
                    float difficulty = targets[index].difficultyMultiplier;
                    float total = basePoints * difficulty * comboMultiplier;

                    score += static_cast<int>(std::round(total));

                    comboHits++;
                    comboMultiplier = 1.0f + comboHits * 0.05f;

                    addHitEffect(centerX, centerY, radius);
                    targets.erase(targets.begin() + index);
                }
                else
                {
                    score -= 1;
                    misses += 1;
                    comboHits = 0;
                    comboMultiplier = 1.0f;
                    addBulletMark(static_cast<float>(mouseX), static_cast<float>(mouseY));

                    if (currentMode == Mode::Endless && misses >= maxMisses)
                    {
                        checkBestScore();
                        gameState = State::GameOver;
                    }
                }

                updateGameplayText();
                updateTextPositions();
            }
            else if (gameState == State::GameOver)
            {
                if (restartText && isTextClicked(*restartText, mouseX, mouseY))
                {
                    resetGame();
                }
                else if (exitText && isTextClicked(*exitText, mouseX, mouseY))
                {
                    window.close();
                }
            }
        }
    }
}

void Game::update()
{
    if (standardText && endlessText && exitText && restartText)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        int mouseX = mousePos.x;
        int mouseY = mousePos.y;

        standardText->setFillColor(sf::Color::Black);
        endlessText->setFillColor(sf::Color::Black);
        exitText->setFillColor(sf::Color::Black);
        restartText->setFillColor(sf::Color::Black);

        standardText->setScale({ 1.f, 1.f });
        endlessText->setScale({ 1.f, 1.f });
        exitText->setScale({ 1.f, 1.f });
        restartText->setScale({ 1.f, 1.f });

        if (gameState == State::Menu)
        {
            if (isTextClicked(*standardText, mouseX, mouseY))
            {
                standardText->setFillColor(sf::Color(200, 30, 30));
                standardText->setScale({ 1.12f, 1.12f });
            }

            if (isTextClicked(*endlessText, mouseX, mouseY))
            {
                endlessText->setFillColor(sf::Color(200, 30, 30));
                endlessText->setScale({ 1.12f, 1.12f });
            }

            if (isTextClicked(*exitText, mouseX, mouseY))
            {
                exitText->setFillColor(sf::Color(200, 30, 30));
                exitText->setScale({ 1.12f, 1.12f });
            }
        }
        else if (gameState == State::GameOver)
        {
            if (isTextClicked(*restartText, mouseX, mouseY))
            {
                restartText->setFillColor(sf::Color(200, 30, 30));
                restartText->setScale({ 1.12f, 1.12f });
            }

            if (isTextClicked(*exitText, mouseX, mouseY))
            {
                exitText->setFillColor(sf::Color(200, 30, 30));
                exitText->setScale({ 1.12f, 1.12f });
            }
        }
    }

    if (gameState == State::Playing)
    {
        if (spawnClock.getElapsedTime().asSeconds() >= spawnInterval)
        {
            spawnTarget();
            spawnClock.restart();
        }

        updateTargets();
        updateBulletMarks();
        updateHitEffects();
        updateShotFlashes();

        if (currentMode == Mode::Standard)
        {
            int elapsed = static_cast<int>(gameClock.getElapsedTime().asSeconds());
            timeLeft = 60 - elapsed;

            if (timeLeft <= 0)
            {
                timeLeft = 0;
                checkBestScore();
                gameState = State::GameOver;
            }
        }

        if (currentMode == Mode::Endless)
        {
            endlessTimeSeconds = static_cast<int>(gameClock.getElapsedTime().asSeconds());

            if (inactivityClock.getElapsedTime().asSeconds() >= maxInactivityTime)
            {
                checkBestScore();
                gameState = State::GameOver;
            }
        }

        updateGameplayText();
        updateTextPositions();
    }
    else
    {
        updateBulletMarks();
        updateHitEffects();
        updateShotFlashes();
        updateGameplayText();
        updateTextPositions();
    }
}

void Game::render()
{
    window.clear(sf::Color(245, 245, 245));

    if (gameState == State::Menu)
    {
        if (titleText)                 window.draw(*titleText);
        if (standardText)              window.draw(*standardText);
        if (endlessText)               window.draw(*endlessText);
        if (exitText)                  window.draw(*exitText);
        if (bestScoreStandardMenuText) window.draw(*bestScoreStandardMenuText);
        if (bestScoreEndlessMenuText)  window.draw(*bestScoreEndlessMenuText);
        if (hintText)                  window.draw(*hintText);
    }
    else if (gameState == State::Playing)
    {
        for (const auto& target : targets)
        {
            drawTarget(target);
        }

        for (const auto& mark : bulletMarks)
        {
            drawBulletMark(mark);
        }

        for (const auto& effect : hitEffects)
        {
            drawHitEffect(effect);
        }

        for (const auto& flash : shotFlashes)
        {
            drawShotFlash(flash);
        }

        if (scoreText)       window.draw(*scoreText);
        if (currentModeText) window.draw(*currentModeText);
        if (comboText)       window.draw(*comboText);
        if (timerText)       window.draw(*timerText);

        if (currentMode == Mode::Endless)
        {
            if (missesText) window.draw(*missesText);
            if (idleText)   window.draw(*idleText);
        }

        if (hintText) window.draw(*hintText);
    }
    else if (gameState == State::GameOver)
    {
        for (const auto& mark : bulletMarks)
        {
            drawBulletMark(mark);
        }

        for (const auto& effect : hitEffects)
        {
            drawHitEffect(effect);
        }

        for (const auto& flash : shotFlashes)
        {
            drawShotFlash(flash);
        }

        if (gameOverText)          window.draw(*gameOverText);
        if (finalScoreText)        window.draw(*finalScoreText);
        if (bestScoreGameOverText) window.draw(*bestScoreGameOverText);
        if (currentModeText)       window.draw(*currentModeText);
        if (restartText)           window.draw(*restartText);
        if (exitText)              window.draw(*exitText);
        if (hintText)              window.draw(*hintText);
    }

    window.display();
}