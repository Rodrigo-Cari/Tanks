#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>
#include <stack>
#include <cstdlib>
#include <functional>

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
    int i; // column index
    int j; // row index
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

        if (topIndex != -1 && !grid[topIndex].visited) neighbors.push_back(&grid[topIndex]);
        if (rightIndex != -1 && !grid[rightIndex].visited) neighbors.push_back(&grid[rightIndex]);
        if (bottomIndex != -1 && !grid[bottomIndex].visited) neighbors.push_back(&grid[bottomIndex]);
        if (leftIndex != -1 && !grid[leftIndex].visited) neighbors.push_back(&grid[leftIndex]);

        if (!neighbors.empty()) {
            int r = rand() % neighbors.size();
            return neighbors[r];
        }
        return nullptr;
    }

    void draw(sf::RenderWindow& window) {
        if (walls[0]) window.draw(lineTop);
        if (walls[1]) window.draw(lineRight);
        if (walls[2]) window.draw(lineBottom);
        if (walls[3]) window.draw(lineLeft);
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
    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }
};

class Maze {
public:
    Maze(sf::Vector2u windowSize) {
        generateRandomMaze();
    }

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
                walls.emplace_back(cell.lineRight.getPosition(), cell.lineRight.getSize());
            }
            if (cell.walls[2]) {
                walls.emplace_back(cell.lineBottom.getPosition(), cell.lineBottom.getSize());
            }
            if (cell.walls[3]) {
                walls.emplace_back(cell.lineLeft.getPosition(), cell.lineLeft.getSize());
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        for (auto& wall : walls) {
            wall.draw(window);
        }
    }

    std::vector<Cell> grid;
    Cell* current;
    std::stack<Cell*> stack;
    std::vector<Wall> walls;
};

class Bullet {
public:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float speed;
    Maze* maze;
    float lifetime;

    Bullet(sf::Vector2f position, float angle, Maze* maze)
        : maze(maze), lifetime(10), speed(300.0f) {
        velocity = sf::Vector2f(cos(angle) * speed, sin(angle) * speed);
        shape.setRadius(5.0f);
        shape.setFillColor(sf::Color::White);
        shape.setPosition(position);
    }

    void move(float deltaTime) {
        sf::Vector2f nextPosition = shape.getPosition() + velocity * deltaTime;
        sf::FloatRect bulletBounds = shape.getGlobalBounds();

        sf::FloatRect nextBounds(nextPosition.x, nextPosition.y, bulletBounds.width, bulletBounds.height);

        const float adjustment = 2.0f;

        bool collision = false;
        for (auto& wall : maze->walls) {
            sf::FloatRect wallBounds = wall.shape.getGlobalBounds();

            if (nextBounds.intersects(wallBounds)) {
                collision = true;
                // Profundida de inteseccion
                float overlapLeft = wallBounds.left - (nextBounds.left + nextBounds.width);
                float overlapRight = (nextBounds.left - wallBounds.left - wallBounds.width);
                float overlapTop = wallBounds.top - (nextBounds.top + nextBounds.height);
                float overlapBottom = (nextBounds.top - wallBounds.top - wallBounds.height);

                // Encontrar overlap mas pequeño
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
    virtual void shoot(std::vector<std::unique_ptr<Bullet>>& bullets, float deltaTime, const sf::Sprite& sprite, Maze* maze) = 0;
};

class ShootDefault : public Shooter {
public:
    void shoot(std::vector<std::unique_ptr<Bullet>>& bullets, float deltaTime, const sf::Sprite& sprite, Maze* maze) {
        sf::Vector2f bulletPos = sprite.getPosition() +
            sf::Vector2f(cos(sprite.getRotation() * PI / 180.0f) * 50.0f,
                sin(sprite.getRotation() * PI / 180.0f) * 50.0f);
        bullets.emplace_back(std::make_unique<Bullet>(bulletPos, sprite.getRotation() * PI / 180.0f, maze));
    }
};

class Tank {
public:
    sf::Sprite sprite;
    sf::Texture texture;
    float speed;
    float rotationSpeed;
    std::map<sf::Keyboard::Key, std::function<void(float)>> controls;
    float shootCooldown;
    float shootTimer;
    std::unique_ptr<Shooter> shooter;
    Maze* maze;
    bool alive;

    Tank(const sf::Texture& texture, sf::Vector2f position, Maze* maze)
        : speed(200.0f), rotationSpeed(200.0f), shootCooldown(1.0f), shootTimer(0.0f), maze(maze), alive(true) {
        sprite.setTexture(texture);
        sprite.setPosition(position);
        sprite.setOrigin(30.0f, 30.0f);
    }

    void move(float deltaTime) {
        if (!alive) return;

        sf::Vector2f movement(0.0f, 0.0f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            movement += sf::Vector2f(cos(sprite.getRotation() * PI / 180.0f) * speed * deltaTime,
                sin(sprite.getRotation() * PI / 180.0f) * speed * deltaTime);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            movement -= sf::Vector2f(cos(sprite.getRotation() * PI / 180.0f) * speed * deltaTime,
                sin(sprite.getRotation() * PI / 180.0f) * speed * deltaTime);
        }
        sprite.move(movement);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            sprite.rotate(-rotationSpeed * deltaTime);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            sprite.rotate(rotationSpeed * deltaTime);
        }

        // Manejo de colisiones
        sf::FloatRect nextBounds = sprite.getGlobalBounds();
        for (auto& wall : maze->walls) {
            if (nextBounds.intersects(wall.shape.getGlobalBounds())) {
                sprite.move(-movement);
                break;
            }
        }
    }

    void shoot(std::vector<std::unique_ptr<Bullet>>& bullets, float deltaTime) {
        if (!alive) return;

        shootTimer += deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && shootTimer >= shootCooldown) {
            shooter->shoot(bullets, deltaTime, sprite, maze);
            shootTimer = 0.0f;
        }
    }

    void update(std::vector<std::unique_ptr<Bullet>>& bullets, float deltaTime) {
        move(deltaTime);
        shoot(bullets, deltaTime);
    }

    void draw(sf::RenderWindow& window) {
        if (alive) {
            window.draw(sprite);
        }
    }

    void setShooter(Shooter* newShooter) {
        shooter.reset(newShooter);
    }
};

int main() {
    srand(static_cast<unsigned>(time(0)));

    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Tank Maze Game", sf::Style::Fullscreen);

    sf::Texture tankTexture;
    if (!tankTexture.loadFromFile("gTank.png")) {
        return -1;
    }

    Maze maze(window.getSize());
    Tank tank1(tankTexture, sf::Vector2f(240, 170), &maze);
    tank1.setShooter(new ShootDefault());

    std::vector<std::unique_ptr<Bullet>> bullets;

    sf::Clock clock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        float deltaTime = clock.restart().asSeconds();

        tank1.update(bullets, deltaTime);

        for (auto& bullet : bullets) {
            bullet->move(deltaTime);
        }

        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(), [](const std::unique_ptr<Bullet>& bullet) {
                return !bullet->isAlive();
                }),
            bullets.end()
        );

        window.clear();
        maze.draw(window);
        tank1.draw(window);
        for (auto& bullet : bullets) {
            bullet->draw(window);
        }
        window.display();
    }

    return 0;
}