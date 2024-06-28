#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

const float PI = 3.14159265;

class Wall {
public:
  Wall(sf::Vector2f position, sf::Vector2f size) {
    shape.setSize(size);
    shape.setPosition(position);
    shape.setFillColor(sf::Color::White);
  }
  void draw(sf::RenderWindow &window) { window.draw(shape); }
  sf::RectangleShape shape;
};

class Maze {
public:
  Maze(sf::Vector2u windowSize) { generateRandomMaze(windowSize); }

  void generateRandomMaze(sf::Vector2u windowSize) {
    float wallThickness = 6.0; // Grosor de las paredes

    float startX = windowSize.x * 0.1f;
    float endX = windowSize.x * 0.9f;
    float startY = windowSize.y * 0.2f;
    float endY = windowSize.y * 0.8f;
    float widthMaze = endX - startX;
    float heightMaze = endY - startY;
    float separationX = widthMaze / 12;
    float separationY = heightMaze / 5;

    // Paredes horizontales
    walls.emplace_back(sf::Vector2f(startX, startY),
                       sf::Vector2f(widthMaze, wallThickness));
    walls.emplace_back(sf::Vector2f(startX, startY + separationY),
                       sf::Vector2f(2 * separationY, wallThickness));
    walls.emplace_back(
        sf::Vector2f(startX + (4 * separationX), startY + separationY),
        sf::Vector2f(separationY, wallThickness));
    walls.emplace_back(
        sf::Vector2f(startX + (2 * separationX), startY + (2 * separationY)),
        sf::Vector2f(2 * separationY, wallThickness));
    walls.emplace_back(
        sf::Vector2f(startX + (8 * separationX), startY + (2 * separationY)),
        sf::Vector2f(separationY, wallThickness));
    walls.emplace_back(
        sf::Vector2f(startX + (11 * separationX), startY + (2 * separationY)),
        sf::Vector2f(separationY, wallThickness));
    walls.emplace_back(sf::Vector2f(startX, startY + (3 * separationY)),
                       sf::Vector2f(separationY, wallThickness));
    walls.emplace_back(
        sf::Vector2f(startX + (2 * separationX), startY + (3 * separationY)),
        sf::Vector2f(separationY, wallThickness));
    walls.emplace_back(
        sf::Vector2f(startX + (4 * separationX), startY + (3 * separationY)),
        sf::Vector2f(separationY, wallThickness));
    walls.emplace_back(
        sf::Vector2f(startX + (6 * separationX), startY + (3 * separationY)),
        sf::Vector2f(separationY, wallThickness));
    walls.emplace_back(
        sf::Vector2f(startX + (9 * separationX), startY + (3 * separationY)),
        sf::Vector2f(2 * separationY, wallThickness));
    walls.emplace_back(sf::Vector2f(startX, startY + (4 * separationY)),
                       sf::Vector2f(separationY, wallThickness));
    walls.emplace_back(
        sf::Vector2f(startX + (4 * separationX), startY + (4 * separationY)),
        sf::Vector2f(separationY, wallThickness));
    walls.emplace_back(
        sf::Vector2f(startX + (7 * separationX), startY + (4 * separationY)),
        sf::Vector2f(separationY, wallThickness));
    walls.emplace_back(
        sf::Vector2f(startX + (10 * separationX), startY + (4 * separationY)),
        sf::Vector2f(separationY, wallThickness));
    walls.emplace_back(sf::Vector2f(startX, startY + (5 * separationY)),
                       sf::Vector2f(widthMaze, wallThickness));

    // Paredes verticales
    walls.emplace_back(sf::Vector2f(startX, startY),
                       sf::Vector2f(wallThickness, heightMaze));
    walls.emplace_back(
        sf::Vector2f(startX + separationX, startY + (2 * separationY)),
        sf::Vector2f(wallThickness, 3 * separationY));
    walls.emplace_back(
        sf::Vector2f(startX + (2 * separationX), startY + (3 * separationY)),
        sf::Vector2f(wallThickness, separationY));
    walls.emplace_back(
        sf::Vector2f(startX + (3 * separationX), startY + separationY),
        sf::Vector2f(wallThickness, separationY));
    walls.emplace_back(
        sf::Vector2f(startX + (3 * separationX), startY + (3 * separationY)),
        sf::Vector2f(wallThickness, separationY));
    walls.emplace_back(sf::Vector2f(startX + (4 * separationX), startY),
                       sf::Vector2f(wallThickness, separationY));
    walls.emplace_back(
        sf::Vector2f(startX + (4 * separationX), startY + (4 * separationY)),
        sf::Vector2f(wallThickness, separationY));
    walls.emplace_back(
        sf::Vector2f(startX + (5 * separationX), startY + (2 * separationY)),
        sf::Vector2f(wallThickness, 3 * separationY));
    walls.emplace_back(
        sf::Vector2f(startX + (6 * separationX), startY + separationY),
        sf::Vector2f(wallThickness, 3 * separationY));
    walls.emplace_back(sf::Vector2f(startX + (7 * separationX), startY),
                       sf::Vector2f(wallThickness, 4 * separationY));
    walls.emplace_back(sf::Vector2f(startX + (8 * separationX), startY),
                       sf::Vector2f(wallThickness, 2 * separationY));
    walls.emplace_back(
        sf::Vector2f(startX + (8 * separationX), startY + (3 * separationY)),
        sf::Vector2f(wallThickness, separationY));
    walls.emplace_back(
        sf::Vector2f(startX + (9 * separationX), startY + separationY),
        sf::Vector2f(wallThickness, separationY));
    walls.emplace_back(
        sf::Vector2f(startX + (9 * separationX), startY + (4 * separationY)),
        sf::Vector2f(wallThickness, separationY));
    walls.emplace_back(
        sf::Vector2f(startX + (10 * separationX), startY + separationY),
        sf::Vector2f(wallThickness, separationY));
    walls.emplace_back(
        sf::Vector2f(startX + (10 * separationX), startY + (3 * separationY)),
        sf::Vector2f(wallThickness, separationY));
    walls.emplace_back(
        sf::Vector2f(startX + (11 * separationX), startY + separationY),
        sf::Vector2f(wallThickness, 2 * separationY));
    walls.emplace_back(sf::Vector2f(startX + (12 * separationX), startY),
                       sf::Vector2f(wallThickness, heightMaze));
  }

  void draw(sf::RenderWindow &window) {
    for (auto &wall : walls) {
      wall.draw(window);
    }
  }

  std::vector<Wall> walls;
};

class Bullet {
public:
  sf::CircleShape shape;
  sf::Vector2f velocity;
  float speed;
  Maze *maze;
  float lifetime;

  Bullet(sf::Vector2f position, float angle, Maze *maze)
      : maze(maze), lifetime(10), speed(300.0f) {
    velocity = sf::Vector2f(cos(angle) * speed, sin(angle) * speed);
    shape.setRadius(5.0f);
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
    for (auto &wall : maze->walls) {
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
          } else {
            shape.move(overlapRight, 0);
            velocity.x = -velocity.x; // Bala viene desde la derecha
            shape.move(adjustment, 0);
          }
        } else {
          // Manejar colision vertical
          if (absOverlapTop < absOverlapBottom) {
            shape.move(0, overlapTop);
            velocity.y = -velocity.y; // Bala viene desde arriba
            shape.move(0, -adjustment);
          } else {
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

  void draw(sf::RenderWindow &window) { window.draw(shape); }
};

class Shooter {
public:
  virtual void shoot(std::vector<std::unique_ptr<Bullet>> &bullets,
                     float deltaTime, const sf::Sprite &sprite, Maze *maze) = 0;
};

class ShootDefault : public Shooter {
public:
  void shoot(std::vector<std::unique_ptr<Bullet>> &bullets, float deltaTime,
             const sf::Sprite &sprite, Maze *maze) {
    sf::Vector2f bulletPos =
        sprite.getPosition() +
        sf::Vector2f(cos(sprite.getRotation() * PI / 180) *
                         (sprite.getLocalBounds().width / 2 + 32),
                     sin(sprite.getRotation() * PI / 180) *
                         (sprite.getLocalBounds().height / 2 + 32));
    auto bullet = std::make_unique<Bullet>(
        bulletPos, sprite.getRotation() * PI / 180, maze);
    bullets.push_back(std::move(bullet));
  }
};

class Tank {
public:
  Tank(sf::Texture &texture, sf::Vector2f position, Maze *maze)
      : maze(maze), alive(true), moveSpeed(200.0f), rotationSpeed(200.0f),
        fireRate(0.4f), timeSinceLastShot(0.0f) {
    sprite.setTexture(texture);
    sprite.setOrigin(texture.getSize().x / 2.0f, texture.getSize().y / 2.0f);
    sprite.setPosition(position);
  }
  bool isAlive() const { return alive; }
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

    sf::FloatRect nextPos = sprite.getGlobalBounds();
    nextPos.left += movement.x;
    nextPos.top += movement.y;

    bool canMove = true;
    for (auto &wall : maze->walls) {
      if (wall.shape.getGlobalBounds().intersects(nextPos)) {
        canMove = false;
        break;
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
      sf::FloatRect nextPos = tempSprite.getGlobalBounds();
      bool canRotate = true;
      for (auto &wall : maze->walls) {
        if (wall.shape.getGlobalBounds().intersects(nextPos)) {
          canRotate = false;
          break;
        }
      }
      if (canRotate) {
        sprite.rotate(rotationAmount);
      }
    }
  }

  void shoot(float deltaTime, std::vector<std::unique_ptr<Bullet>> &bullets) {
    if (sf::Keyboard::isKeyPressed(shootKey) && timeSinceLastShot >= fireRate) {
      shooter->shoot(bullets, deltaTime, sprite, maze);
      timeSinceLastShot = 0.0f;
    } else {
      timeSinceLastShot += deltaTime;
    }
  }

  void detectBulletCollision(std::vector<std::unique_ptr<Bullet>> &bullets) {
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
      } else {
        ++it;
      }
    }
  }

  void update(float deltaTime, std::vector<std::unique_ptr<Bullet>> &bullets) {
    shoot(deltaTime,bullets);
    move(deltaTime);
    rotate(deltaTime);
    detectBulletCollision(bullets);
  }

  void draw(sf::RenderWindow &window) { window.draw(sprite); }

  void setControls(sf::Keyboard::Key up, sf::Keyboard::Key down,
                   sf::Keyboard::Key left, sf::Keyboard::Key right,
                   sf::Keyboard::Key shoot) {
    upKey = up;
    downKey = down;
    leftKey = left;
    rightKey = right;
    shootKey = shoot;
  }

  void setShotter(Shooter * shooterInstance){
    shooter = shooterInstance;
  }

private:
  sf::Sprite sprite;
  Maze *maze;

  float moveSpeed;
  float rotationSpeed;
  float fireRate;
  float timeSinceLastShot;

  bool alive;

  sf::Keyboard::Key upKey;
  sf::Keyboard::Key downKey;
  sf::Keyboard::Key leftKey;
  sf::Keyboard::Key rightKey;
  sf::Keyboard::Key shootKey;

  Shooter *shooter;
};

int main() {
  sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "SFML Tank Game",
                          sf::Style::Fullscreen);

  sf::Texture texture;
  if (!texture.loadFromFile("gTank.png")) {
    std::cerr << "Error al cargar la imagen" << std::endl;
    return -1;
  }

  Maze maze(window.getSize());

  ShootDefault defaultShooter;

  Tank player1(texture, sf::Vector2f(280.f, 520.f), &maze);
  Tank player2(texture, sf::Vector2f(1500.f, 520.f), &maze);

  player1.setControls(sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A,
                      sf::Keyboard::D, sf::Keyboard::Space);
  player2.setControls(sf::Keyboard::Up, sf::Keyboard::Down, sf::Keyboard::Left,
                      sf::Keyboard::Right, sf::Keyboard::Enter);

  player1.setShotter(&defaultShooter);
  player2.setShotter(&defaultShooter);

  std::vector<std::unique_ptr<Bullet>> bullets;

  sf::Clock clock;

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    float deltaTime = clock.restart().asSeconds();

    if (player1.isAlive()) {
      player1.update(deltaTime, bullets);
    }
    if (player2.isAlive()) {
      player2.update(deltaTime, bullets);
    }

    auto it = bullets.begin();
    while (it != bullets.end()) {
      (*it)->move(deltaTime);
      if (!(*it)->isAlive()) {
        it = bullets.erase(it);
      } else {
        ++it;
      }
    }

    // Renderizado
    window.clear();
    maze.draw(window);
    if (player1.isAlive()) {
      player1.draw(window);
    }
    if (player2.isAlive()) {
      player2.draw(window);
    }
    for (const auto &bullet : bullets) {
      window.draw(bullet->shape);
    }
    window.display();
  }

  return 0;
}
