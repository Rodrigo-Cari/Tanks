#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Window.hpp>
#include <array>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <stack>
#include <vector>
#include <thread>

sf::Texture speed;
sf::Texture texture2;
sf::Texture texture1;
sf::Texture attack;

struct OrientedBoundingBox {
    std::array<sf::Vector2f, 4> points;
    OrientedBoundingBox(const sf::Sprite& sprite) {
        auto transform = sprite.getTransform();
        auto local = sprite.getTextureRect();
        points[0] = transform.transformPoint(0.f, 0.f);
        points[1] = transform.transformPoint(local.width, 0.f);
        points[2] = transform.transformPoint(local.width, local.height);
        points[3] = transform.transformPoint(0.f, local.height);
    }
    void projectOntoAxis(const sf::Vector2f& axis, float& min, float& max) {
        min = (points[0].x * axis.x + points[0].y * axis.y);
        max = min;
        for (int j = 1; j < points.size(); ++j) {
            auto projection = points[j].x * axis.x + points[j].y * axis.y;

            if (projection < min)
                min = projection;
            if (projection > max)
                max = projection;
        }
    }
};

bool boundingBoxTest(const sf::Sprite& sprite1, const sf::Sprite& sprite2) {
    auto OBB1 = OrientedBoundingBox(sprite1);
    auto OBB2 = OrientedBoundingBox(sprite2);
    std::array<sf::Vector2f, 4> axes = { {{OBB1.points[1].x - OBB1.points[0].x,
                                          OBB1.points[1].y - OBB1.points[0].y},
                                         {OBB1.points[1].x - OBB1.points[2].x,
                                          OBB1.points[1].y - OBB1.points[2].y},
                                         {OBB2.points[0].x - OBB2.points[3].x,
                                          OBB2.points[0].y - OBB2.points[3].y},
                                         {OBB2.points[0].x - OBB2.points[1].x,
                                          OBB2.points[0].y - OBB2.points[1].y}} };
    for (auto& axis : axes) {
        float minOBB1, maxOBB1, minOBB2, maxOBB2;
        OBB1.projectOntoAxis(axis, minOBB1, maxOBB1);
        OBB2.projectOntoAxis(axis, minOBB2, maxOBB2);
        if (!((minOBB2 <= maxOBB1) && (maxOBB2 >= minOBB1)))
            return false;
    }
    return true;
}

const float PI = 3.14159265;

const int cols = 12;
const int rows = 6;

int index(int i, int j) {
    if (i < 0 || j < 0 || i >= cols || j >= rows) {
        return -1;
    }
    return i + j * cols;
}

class Cell {
public:
    int i;       // column index
    int j;       // row index
    int w = 130; // width of the cell
    sf::RectangleShape lineTop;
    sf::RectangleShape lineRight;
    sf::RectangleShape lineBottom;
    sf::RectangleShape lineLeft;
    std::vector<bool> walls = { true, true, true, true };
    bool visited = false;

    Cell(int columnIndex, int rowIndex) : i(columnIndex), j(rowIndex) {
        int x = 180 + i * w;
        int y = 100 + j * w;
        lineTop.setPosition(sf::Vector2f(x, y));
        lineTop.setSize(sf::Vector2f(w, 6));
        lineRight.setPosition(sf::Vector2f(x + w, y));
        lineRight.setSize(sf::Vector2f(6, w));
        lineBottom.setPosition(sf::Vector2f(x, y + w));
        lineBottom.setSize(sf::Vector2f(w, 6));
        lineLeft.setPosition(sf::Vector2f(x, y));
        lineLeft.setSize(sf::Vector2f(6, w));
    }

    Cell* checkNeighbors(std::vector<Cell>& grid) {
        std::vector<Cell*> neighbors;

        int topIndex = index(i, j - 1);
        int rightIndex = index(i + 1, j);
        int bottomIndex = index(i, j + 1);
        int leftIndex = index(i - 1, j);

        if (topIndex != -1 && !grid[topIndex].visited)
            neighbors.push_back(&grid[topIndex]);
        if (rightIndex != -1 && !grid[rightIndex].visited)
            neighbors.push_back(&grid[rightIndex]);
        if (bottomIndex != -1 && !grid[bottomIndex].visited)
            neighbors.push_back(&grid[bottomIndex]);
        if (leftIndex != -1 && !grid[leftIndex].visited)
            neighbors.push_back(&grid[leftIndex]);

        if (!neighbors.empty()) {
            int r = rand() % neighbors.size();
            return neighbors[r];
        }
        return nullptr;
    }

    void draw(sf::RenderWindow& window) {
        if (walls[0])
            window.draw(lineTop);
        if (walls[1])
            window.draw(lineRight);
        if (walls[2])
            window.draw(lineBottom);
        if (walls[3])
            window.draw(lineLeft);
    }

    friend void removeWalls(Cell* a, Cell* b);
};

void removeWalls(Cell* a, Cell* b) {
    int x = a->i - b->i;
    if (x == 1) {
        a->walls[3] = false;
        b->walls[1] = false;
    }
    else if (x == -1) {
        a->walls[1] = false;
        b->walls[3] = false;
    }
    int y = a->j - b->j;
    if (y == 1) {
        a->walls[0] = false;
        b->walls[2] = false;
    }
    else if (y == -1) {
        a->walls[2] = false;
        b->walls[0] = false;
    }
}

class Wall {
public:
    sf::RectangleShape shape;
    Wall(sf::Vector2f position, sf::Vector2f size) {
        shape.setSize(size);
        shape.setPosition(position);
        shape.setFillColor(sf::Color::White);
    }
    void draw(sf::RenderWindow& window) { window.draw(shape); }
};

class Maze {
public:
    Maze(sf::Vector2u windowSize) { generateRandomMaze(); }

    void generateRandomMaze() {
        for (int j = 0; j < rows; j++) {
            for (int i = 0; i < cols; i++) {
                grid.push_back(Cell(i, j));
            }
        }

        current = &grid[0];
        current->visited = true;

        while (true) {
            Cell* next = current->checkNeighbors(grid);
            if (next != nullptr) {
                next->visited = true;
                stack.push(current);
                removeWalls(current, next);
                current = next;
            }
            else if (!stack.empty()) {
                current = stack.top();
                stack.pop();
            }
            else {
                break;
            }
        }

        for (const auto& cell : grid) {
            if (cell.walls[0]) {
                walls.emplace_back(cell.lineTop.getPosition(), cell.lineTop.getSize());
            }
            if (cell.walls[1]) {
                walls.emplace_back(cell.lineRight.getPosition(),
                    cell.lineRight.getSize());
            }
            if (cell.walls[2]) {
                walls.emplace_back(cell.lineBottom.getPosition(),
                    cell.lineBottom.getSize());
            }
            if (cell.walls[3]) {
                walls.emplace_back(cell.lineLeft.getPosition(),
                    cell.lineLeft.getSize());
            }
        }

        for (const auto wall : walls) {
            sf::Sprite wallSprite;
            wallSprite.setTextureRect(
                sf::IntRect(0, 0, wall.shape.getSize().x, wall.shape.getSize().y));
            wallSprite.setPosition(wall.shape.getPosition());
            wallSprite.setRotation(wall.shape.getRotation());
            wallSprite.setScale(wall.shape.getScale());
            wallSprite.setOrigin(wall.shape.getOrigin());
            wallSprites.push_back(wallSprite);
        }
    }

    void draw(sf::RenderWindow& window) {
        for (auto& wall : walls) {
            wall.draw(window);
        }
    }

    std::vector<sf::Vector2f> getCellCenters() {
        std::vector<sf::Vector2f> centers;
        for (const auto& cell : grid) {
            int x = 180 + cell.i * cell.w + cell.w / 2;
            int y = 100 + cell.j * cell.w + cell.w / 2;
            centers.push_back(sf::Vector2f(x, y));
        }
        return centers;
    }

    std::vector<Cell> grid;
    Cell* current;
    std::stack<Cell*> stack;
    std::vector<Wall> walls;
    std::vector<sf::Sprite> wallSprites;
};

class Bullet {
public:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float speed;
    Maze* maze;
    float lifetime;

    Bullet(sf::Vector2f position, float angle, Maze* maze, float radius)
        : maze(maze), lifetime(10), speed(300.0f) {
        velocity = sf::Vector2f(cos(angle) * speed, sin(angle) * speed);
        shape.setRadius(radius);
        shape.setFillColor(sf::Color::White);
        shape.setPosition(position);
    }

    void move(float deltaTime) {
        sf::Vector2f nextPosition = shape.getPosition() + velocity * deltaTime;
        sf::FloatRect bulletBounds = shape.getGlobalBounds();

        sf::FloatRect nextBounds(nextPosition.x, nextPosition.y, bulletBounds.width,
            bulletBounds.height);

        const float adjustment = 2.0f;

        bool collision = false;
        for (auto& wall : maze->walls) {
            sf::FloatRect wallBounds = wall.shape.getGlobalBounds();

            if (nextBounds.intersects(wallBounds)) {
                collision = true;
                // Profundida de inteseccion
                float overlapLeft =
                    wallBounds.left - (nextBounds.left + nextBounds.width);
                float overlapRight =
                    (nextBounds.left - wallBounds.left - wallBounds.width);
                float overlapTop =
                    wallBounds.top - (nextBounds.top + nextBounds.height);
                float overlapBottom =
                    (nextBounds.top - wallBounds.top - wallBounds.height);

                // Encontrar overlap mas peque�o
                float absOverlapLeft = std::abs(overlapLeft);
                float absOverlapRight = std::abs(overlapRight);
                float absOverlapTop = std::abs(overlapTop);
                float absOverlapBottom = std::abs(overlapBottom);

                float minOverlapX = std::min(absOverlapLeft, absOverlapRight);
                float minOverlapY = std::min(absOverlapTop, absOverlapBottom);

                if (minOverlapX < minOverlapY) {
                    // Manejar colision horizontal
                    if (absOverlapLeft < absOverlapRight) {
                        shape.move(overlapLeft, 0);
                        velocity.x = -velocity.x; // Bala viene desde la izquierda
                        shape.move(-adjustment, 0);
                    }
                    else {
                        shape.move(overlapRight, 0);
                        velocity.x = -velocity.x; // Bala viene desde la derecha
                        shape.move(adjustment, 0);
                    }
                }
                else {
                    // Manejar colision vertical
                    if (absOverlapTop < absOverlapBottom) {
                        shape.move(0, overlapTop);
                        velocity.y = -velocity.y; // Bala viene desde arriba
                        shape.move(0, -adjustment);
                    }
                    else {
                        shape.move(0, overlapBottom);
                        velocity.y = -velocity.y; // Bala viene desde abajo
                        shape.move(0, adjustment);
                    }
                }

                break;
            }
        }

        if (!collision) {
            shape.setPosition(nextPosition);
        }

        lifetime -= deltaTime;
    }

    bool isAlive() { return lifetime >= 0; }

    void draw(sf::RenderWindow& window) { window.draw(shape); }
};

class Shooter {
public:
    virtual void shoot(std::vector<std::unique_ptr<Bullet>>& bullets,
        float deltaTime, const sf::Sprite& sprite, Maze* maze) = 0;
};

class ShootDefault : public Shooter {
public:
    void shoot(std::vector<std::unique_ptr<Bullet>>& bullets, float deltaTime,
        const sf::Sprite& sprite, Maze* maze) {
        sf::Vector2f bulletPos =
            sprite.getPosition() +
            sf::Vector2f(cos(sprite.getRotation() * PI / 180) *
                (sprite.getLocalBounds().width / 2 + 32),
                sin(sprite.getRotation() * PI / 180) *
                (sprite.getLocalBounds().height / 2 + 32));
        auto bullet = std::make_unique<Bullet>(
            bulletPos, sprite.getRotation() * PI / 180, maze, 5.0f);
        bullets.push_back(std::move(bullet));
    }
};

class ShootSmall : public Shooter {
public:
    void shoot(std::vector<std::unique_ptr<Bullet>>& bullets, float deltaTime,
        const sf::Sprite& sprite, Maze* maze) {
        sf::Vector2f bulletPos =
            sprite.getPosition() +
            sf::Vector2f(cos(sprite.getRotation() * PI / 180) *
                (sprite.getLocalBounds().width / 2 + 32),
                sin(sprite.getRotation() * PI / 180) *
                (sprite.getLocalBounds().height / 2 + 32));
        auto bullet = std::make_unique<Bullet>(
            bulletPos, sprite.getRotation() * PI / 180, maze, 3.0f);
        bullets.push_back(std::move(bullet));
    }
};

class TankInterface;

class PowerUp {
protected:
    sf::Sprite sprite;
    float lifetime;

public:
    PowerUp(sf::Vector2f position) : lifetime(20) {}

    void update(float deltaTime) { lifetime -= deltaTime; }

    sf::Sprite getSprite() { return sprite; }

    bool isAlive() { return lifetime >= 0; }

    void draw(sf::RenderWindow& window) { window.draw(sprite); }

    virtual void apply(TankInterface*&) = 0;
};

class TankInterface {
public:
    virtual bool isAlive() const = 0;
    virtual void update(float deltaTime,
        std::vector<std::unique_ptr<Bullet>>& bullets) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual void setShooter(std::unique_ptr<Shooter> shooter) = 0;
    virtual void setMoveSpeed(float move_speed) = 0;
    virtual float getMoveSpeed() const = 0;
    virtual void setRotationSpeed(float rotation_speed) = 0;
    virtual float getRotationSpeed() const = 0;
    virtual void setFireRate(float fire_rate) = 0;
    virtual float getFireRate() const = 0;
    virtual sf::Sprite getSprite() = 0;
    virtual void setMaxBullets(float max) = 0;
    virtual float getMaxBullets() const = 0;
    virtual void setCurrentBullets(float current_bullets) = 0;
    virtual void setAlive() = 0;
    virtual void stop() = 0;
};

class Tank : public TankInterface {
public:
    Tank(sf::Texture& texture, sf::Vector2f position, Maze* maze)
        : maze(maze), alive(true), moveSpeed(200.0f), rotationSpeed(200.0f),
        fireRate(0.4f), timeSinceLastShot(0.0f), maxBullets(5),
        currentBullets(5), reloadTime(4.0f), timeSinceLastReload(0.0f) {
        sprite.setTexture(texture);
        sprite.setOrigin(texture.getSize().x / 2.0f, texture.getSize().y / 2.0f);
        sprite.setPosition(position);
    }

    ~Tank() {
        stop();
        if (tankThread.joinable()) {
            tankThread.join();
        }
    }

    void startThread(std::vector<std::unique_ptr<Bullet>>& bullets) {
        tankThread = std::thread(&Tank::run, this, std::ref(bullets));
    }

    void joinThread() {
        if (tankThread.joinable()) {
            tankThread.join();
        }
    }

    void run(std::vector<std::unique_ptr<Bullet>>& bullets) {
        sf::Clock clock;
        while (!shouldStop) {
            float deltaTime = clock.restart().asSeconds();
            update(deltaTime, bullets);
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // Definimos en 60 FPS
        }
    }

    void stop() {
        shouldStop = true;
    }

    void setPosition(sf::Vector2f positon) { sprite.setPosition(positon); }

    bool isAlive() const { return alive; }
    void setAlive() { alive = true; }

    void move(float deltaTime) {
        sf::Vector2f movement(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(upKey)) {
            movement.x +=
                cos(sprite.getRotation() * PI / 180) * moveSpeed * deltaTime;
            movement.y +=
                sin(sprite.getRotation() * PI / 180) * moveSpeed * deltaTime;
        }
        if (sf::Keyboard::isKeyPressed(downKey)) {
            movement.x -=
                cos(sprite.getRotation() * PI / 180) * moveSpeed * deltaTime;
            movement.y -=
                sin(sprite.getRotation() * PI / 180) * moveSpeed * deltaTime;
        }

        bool canMove = true;
        if (movement != sf::Vector2<float>(0, 0)) {
            sf::Sprite tempSprite = sprite;
            tempSprite.move(movement);

            for (auto& wall : maze->wallSprites) {
                if (boundingBoxTest(tempSprite, wall)) {
                    sprite.move(-movement);
                    canMove = false;
                    break;
                }
            }
        }

        if (canMove) {
            sprite.move(movement);
        }
    }

    void rotate(float deltaTime) {
        float rotationAmount = 0.f;
        if (sf::Keyboard::isKeyPressed(leftKey)) {
            rotationAmount = -rotationSpeed * deltaTime;
        }
        if (sf::Keyboard::isKeyPressed(rightKey)) {
            rotationAmount = rotationSpeed * deltaTime;
        }
        if (rotationAmount != 0.f) {
            sf::Sprite tempSprite = sprite;
            tempSprite.rotate(rotationAmount);

            bool canRotate = true;
            for (auto& wall : maze->wallSprites) {
                if (boundingBoxTest(tempSprite, wall)) {
                    canRotate = false;
                    sprite.rotate(-rotationAmount);
                    break;
                }
            }
            if (canRotate) {
                sprite.rotate(rotationAmount);
            }
        }
    }

    void shoot(float deltaTime, std::vector<std::unique_ptr<Bullet>>& bullets) {
        if (sf::Keyboard::isKeyPressed(shootKey) && timeSinceLastShot >= fireRate &&
            currentBullets > 0) {
            shooter->shoot(bullets, deltaTime, sprite, maze);
            timeSinceLastShot = 0.0f;
            --currentBullets;
        }
        else {
            timeSinceLastShot += deltaTime;
        }
    }

    void reload(float deltaTime) {
        timeSinceLastReload += deltaTime;
        if (timeSinceLastReload >= reloadTime) {
            if (currentBullets < maxBullets) {
                ++currentBullets;
            }
            timeSinceLastReload = 0.0f;
        }
    }

    void detectBulletCollision(std::vector<std::unique_ptr<Bullet>>& bullets) {
        sf::FloatRect tankBounds = sprite.getGlobalBounds();
        // Reducir hitbox en un 20%
        tankBounds.left += tankBounds.width * 0.1f;
        tankBounds.top += tankBounds.height * 0.1f;
        tankBounds.width *= 0.8f;
        tankBounds.height *= 0.8f;

        auto it = bullets.begin();
        while (it != bullets.end()) {
            if ((*it)->shape.getGlobalBounds().intersects(tankBounds)) {
                alive = false;
                it = bullets.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    void update(float deltaTime, std::vector<std::unique_ptr<Bullet>>& bullets) {
        shoot(deltaTime, bullets);
        reload(deltaTime);
        move(deltaTime);
        rotate(deltaTime);
        detectBulletCollision(bullets);
    }

    void draw(sf::RenderWindow& window) { window.draw(sprite); }

    void setControls(sf::Keyboard::Key up, sf::Keyboard::Key down,
        sf::Keyboard::Key left, sf::Keyboard::Key right,
        sf::Keyboard::Key shoot) {
        upKey = up;
        downKey = down;
        leftKey = left;
        rightKey = right;
        shootKey = shoot;
    }

    void setShooter(std::unique_ptr<Shooter> shooterInstance) {
        shooter = std::move(shooterInstance);
    }

    void setMoveSpeed(float move_speed) { moveSpeed = move_speed; }
    float getMoveSpeed() const { return moveSpeed; }
    void setRotationSpeed(float rotation_speed) {
        rotationSpeed = rotation_speed;
    }
    float getRotationSpeed() const { return rotationSpeed; }
    sf::Sprite getSprite() { return sprite; }
    void setFireRate(float fire_rate) { fireRate = fire_rate; }
    float getFireRate() const { return fireRate; }
    void setMaxBullets(float max) { maxBullets = max; }
    float getMaxBullets() const { return maxBullets; }
    void setCurrentBullets(float current_bullets) {
        currentBullets = current_bullets;
    }

private:
    sf::Sprite sprite;
    Maze* maze;

    float moveSpeed;
    float rotationSpeed;
    float fireRate;
    float timeSinceLastShot;

    int maxBullets;
    int currentBullets;
    float reloadTime;
    float timeSinceLastReload;

    bool alive;
    bool shouldStop;
    std::thread tankThread;

    sf::Keyboard::Key upKey;
    sf::Keyboard::Key downKey;
    sf::Keyboard::Key leftKey;
    sf::Keyboard::Key rightKey;
    sf::Keyboard::Key shootKey;

    std::unique_ptr<Shooter> shooter;
};

class TankDecorator : public TankInterface {
public:
    TankDecorator(TankInterface* tank) : tank(tank) {}

    void update(float deltaTime, std::vector<std::unique_ptr<Bullet>>& bullets) {
        tank->update(deltaTime, bullets);
    }

    void draw(sf::RenderWindow& window) { tank->draw(window); }

    void setShooter(std::unique_ptr<Shooter> shooterInstance) {
        tank->setShooter(std::move(shooterInstance));
    }
    bool isAlive() const { return tank->isAlive(); }
    void setMoveSpeed(float move_speed) { tank->setMoveSpeed(move_speed); };
    float getMoveSpeed() const { return tank->getMoveSpeed(); };
    sf::Sprite getSprite() { return tank->getSprite(); }
    void setRotationSpeed(float rotation_speed) {
        tank->setRotationSpeed(rotation_speed);
    }
    float getRotationSpeed() const { return tank->getRotationSpeed(); }
    void setFireRate(float fire_rate) { tank->setFireRate(fire_rate); }
    float getFireRate() const { return tank->getFireRate(); }
    void setMaxBullets(float max) { tank->setMaxBullets(max); }
    float getMaxBullets() const { return tank->getMaxBullets(); }
    void setCurrentBullets(float current_bullets) {
        tank->setCurrentBullets(current_bullets);
    }
    void setAlive() { tank->setAlive(); }

protected:
    TankInterface* tank;
};

class SpeedBoostDecorator : public TankDecorator {
public:
    SpeedBoostDecorator(TankInterface* tank)
        : TankDecorator(tank), boostAmount(100), duration(15), elapsed(0.f),
        boostApplied(false) {}

    void update(float deltaTime,
        std::vector<std::unique_ptr<Bullet>>& bullets) override {
        if (elapsed < duration && !boostApplied) {
            setMoveSpeed(getMoveSpeed() + boostAmount);
            setRotationSpeed(getRotationSpeed() + boostAmount);
            boostApplied = true;
        }

        elapsed += deltaTime;
        if (elapsed >= duration && boostApplied) {
            setMoveSpeed(getMoveSpeed() - boostAmount);
            setRotationSpeed(getRotationSpeed() - boostAmount);
            boostApplied = false;
        }

        TankDecorator::update(deltaTime, bullets);
    }

    void stop() override {
        tank->stop(); // Override Stop
    }

private:
    float boostAmount;
    float duration;
    float elapsed;
    bool boostApplied;
};

class SpeedPowerUp : public PowerUp {
public:
    SpeedPowerUp(sf::Vector2f position) : PowerUp(position) {
        sprite.setTexture(speed);
        sprite.setOrigin(speed.getSize().x / 2.0f, speed.getSize().y / 2.0f);
        sprite.setPosition(position);
    }
    void apply(TankInterface*& tank) { tank = new SpeedBoostDecorator(tank); }
};

class AttackBoostDecorator : public TankDecorator {
public:
    AttackBoostDecorator(TankInterface* tank)
        : TankDecorator(tank), duration(10), elapsed(0.f), boostApplied(false) {}

    void update(float deltaTime,
        std::vector<std::unique_ptr<Bullet>>& bullets) override {
        if (elapsed < duration && !boostApplied) {
            setFireRate(getFireRate() / 2);
            setMaxBullets(getMaxBullets() * 5);
            setCurrentBullets(25);
            setShooter((std::move(small_)));
            boostApplied = true;
        }

        elapsed += deltaTime;
        if (elapsed >= duration && boostApplied) {
            setFireRate(getFireRate() * 2);
            setMaxBullets(getMaxBullets() / 5);
            setShooter((std::move(default_)));
            boostApplied = false;
        }
        TankDecorator::update(deltaTime, bullets);
    }

    void stop() override {
        tank->stop(); // Override Stop
    }

private:
    std::unique_ptr<ShootSmall> small_ = std::make_unique<ShootSmall>();
    std::unique_ptr<ShootDefault> default_ = std::make_unique<ShootDefault>();
    float timeSinceLastShot_;
    float fireRate_;
    float duration;
    float elapsed;
    bool boostApplied;
};

class AttackPowerUp : public PowerUp {
public:
    AttackPowerUp(sf::Vector2f position) : PowerUp(position) {
        sprite.setTexture(attack);
        sprite.setOrigin(attack.getSize().x / 2.0f, attack.getSize().y / 2.0f);
        sprite.setPosition(position);
    }
    void apply(TankInterface*& tank) { tank = new AttackBoostDecorator(tank); }
};

class PowerUpFactory {
public:
    static std::unique_ptr<PowerUp>
        createRandomPowerUp(std::vector<sf::Vector2f> cellCenters) {
        int randomPosition = rand() % cellCenters.size();
        int randomType = rand() % 2;

        switch (randomType) {
        case 0:
            return std::make_unique<SpeedPowerUp>(cellCenters[randomPosition]);
        case 1:
            return std::make_unique<AttackPowerUp>(cellCenters[randomPosition]);
        default:
            return nullptr;
        }
    }
};

void resetGame(Tank& player1, Tank& player2, Maze& maze,
    std::vector<std::unique_ptr<Bullet>>& bullets,
    std::vector<std::unique_ptr<PowerUp>>& powers,
    std::vector<sf::Vector2f>& cellCenters) {
    bullets.clear();
    powers.clear();
    player1.setAlive();
    player2.setAlive();

    int r = rand() % cellCenters.size();
    player1.setPosition(cellCenters[r]);

    int r2;
    do {
        r2 = rand() % cellCenters.size();
    } while (r2 == r);
    player2.setPosition(cellCenters[r2]);

    PowerUpFactory factory;
    powers.push_back(factory.createRandomPowerUp(cellCenters));
    powers.push_back(factory.createRandomPowerUp(cellCenters));
    powers.push_back(factory.createRandomPowerUp(cellCenters));

    auto defaultShooter = std::make_unique<ShootDefault>();
    player1.setShooter(std::move(defaultShooter));
    auto defaultShooter1 = std::make_unique<ShootDefault>();
    player2.setShooter(std::move(defaultShooter1));
}

void setFonts(sf::Font& font, sf::Text& player1ScoreText,
    sf::Text& player2ScoreText, sf::RenderWindow& window) {
    if (!font.loadFromFile("pixelated.ttf")) {
        std::cerr << "Error al cargar la fuente" << std::endl;
    }

    player1ScoreText.setFont(font);
    player1ScoreText.setCharacterSize(40);
    player1ScoreText.setFillColor(sf::Color::White);
    player1ScoreText.setStyle(sf::Text::Bold);
    player1ScoreText.setPosition(80, window.getSize().y - 60);

    player2ScoreText.setFont(font);
    player2ScoreText.setCharacterSize(40);
    player2ScoreText.setFillColor(sf::Color::White);
    player2ScoreText.setStyle(sf::Text::Bold);
    player2ScoreText.setPosition(window.getSize().x - 150,
        window.getSize().y - 60);
}
int main() {
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Tank Game", sf::Style::Fullscreen);
    std::srand(std::time(nullptr));

    sf::Font font;
    sf::Text player1ScoreText;
    sf::Text player2ScoreText;

    setFonts(font, player1ScoreText, player2ScoreText, window);

    if (!texture1.loadFromFile("gTank.png") ||
        !texture2.loadFromFile("pTank.png") || !speed.loadFromFile("speed.png") ||
        !attack.loadFromFile("attack.png")) {
        std::cerr << "Error al cargar la imagen" << std::endl;
        return -1;
    }

    sf::Sprite player1Sprite(texture1);
    sf::Sprite player2Sprite(texture2);

    player1Sprite.setPosition(90, window.getSize().y - 100);
    player2Sprite.setPosition(window.getSize().x - 140, window.getSize().y - 100);
    int p1_wins = 0;
    int p2_wins = 0;

    Maze maze(window.getSize());
    std::vector<sf::Vector2f> cellCenters = maze.getCellCenters();

    Tank player1(texture1, cellCenters[rand() % cellCenters.size()], &maze);
    Tank player2(texture2, cellCenters[rand() % cellCenters.size()], &maze);

    player1.setControls(sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D, sf::Keyboard::Space);
    player2.setControls(sf::Keyboard::Up, sf::Keyboard::Down, sf::Keyboard::Left, sf::Keyboard::Right, sf::Keyboard::Enter);

    std::vector<std::unique_ptr<Bullet>> bullets;
    std::vector<std::unique_ptr<PowerUp>> powers;

    TankInterface* players[2] = { &player1, &player2 };

    sf::Clock clock;

    resetGame(player1, player2, maze, bullets, powers, cellCenters);

    player1.startThread(bullets);
    player2.startThread(bullets);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Manejo de balas
        auto it = bullets.begin();
        while (it != bullets.end()) {
            (*it)->move(clock.restart().asSeconds());
            if (!(*it)->isAlive()) {
                it = bullets.erase(it);
            }
            else {
                ++it;
            }
        }

        // Manejo de PowerUps
        auto it2 = powers.begin();
        while (it2 != powers.end()) {
            (*it2)->update(clock.restart().asSeconds());

            bool collected = false;
            for (auto& player : players) {
                if ((*it2)->getSprite().getGlobalBounds().intersects(player->getSprite().getGlobalBounds())) {
                    (*it2)->apply(player);
                    collected = true;
                    break;
                }
            }
            if (collected) {
                it2 = powers.erase(it2);
            }
            else {
                ++it2;
            }
        }

        player1ScoreText.setString("P1: " + std::to_string(p1_wins));
        player2ScoreText.setString("P2: " + std::to_string(p2_wins));

        // Comprobar si hay un ganador
        if (!players[0]->isAlive()) {
            ++p2_wins;
            resetGame(player1, player2, maze, bullets, powers, cellCenters);
        }
        if (!players[1]->isAlive()) {
            ++p1_wins;
            resetGame(player1, player2, maze, bullets, powers, cellCenters);
        }

        // Renderizado
        window.clear();
        maze.draw(window);
        for (const auto& player : players) {
            if (player->isAlive()) {
                player->draw(window);
            }
        }
        for (const auto& bullet : bullets) {
            window.draw(bullet->shape);
        }
        for (const auto& power : powers) {
            power->draw(window);
        }
        window.draw(player1ScoreText);
        window.draw(player2ScoreText);
        window.draw(player1Sprite);
        window.draw(player2Sprite);
        window.display();
    }

    player1.stop();
    player2.stop();
    player1.joinThread();
    player2.joinThread();

    return 0;
}
