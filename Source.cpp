#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <vector>

const float PI = 3.14159265;

class Maze {
public:
  Maze(sf::Vector2u windowSize) {
    shape.setSize(sf::Vector2f(windowSize.x * 0.9f, windowSize.y * 0.9f));
    shape.setPosition(windowSize.x * 0.05f, windowSize.y * 0.05f);
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineThickness(5);
    shape.setOutlineColor(sf::Color::White);
    updateBounds();
  }

  void updateBounds() { mazeBounds = shape.getGlobalBounds(); }

  void draw(sf::RenderWindow &window) { window.draw(shape); }

  sf::RectangleShape shape;
  sf::FloatRect mazeBounds;
};

class Bullet {
public:
  Bullet(sf::Vector2f position, float angle, float speed, Maze *maze)
      : maze(maze), lifetime(10),
        velocity(cos(angle) * speed, sin(angle) * speed) {
    shape.setRadius(5.0f);
    shape.setFillColor(sf::Color::White);
    shape.setPosition(position);
  }

  void move(float deltaTime) {
    sf::Vector2f nextPosition = shape.getPosition() + velocity * deltaTime;

    if (!maze->mazeBounds.contains(nextPosition)) {
      if (nextPosition.x < maze->mazeBounds.left ||
          nextPosition.x > maze->mazeBounds.left + maze->mazeBounds.width) {
        velocity.x = -velocity.x;
      }
      if (nextPosition.y < maze->mazeBounds.top ||
          nextPosition.y > maze->mazeBounds.top + maze->mazeBounds.height) {
        velocity.y = -velocity.y;
      }
    }

    shape.move(velocity * deltaTime);
    lifetime -= deltaTime;
  }

  bool isAlive() { return lifetime >= 0; }

  void draw(sf::RenderWindow &window) { window.draw(shape); }

  sf::CircleShape shape;
  sf::Vector2f velocity;
  Maze *maze;
  float lifetime;
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

    if (maze->mazeBounds.contains(nextPos.left, nextPos.top) &&
        maze->mazeBounds.contains(nextPos.left + nextPos.width, nextPos.top) &&
        maze->mazeBounds.contains(nextPos.left, nextPos.top + nextPos.height) &&
        maze->mazeBounds.contains(nextPos.left + nextPos.width,
                                  nextPos.top + nextPos.height)) {
      sprite.move(movement);
    }
  }

  void rotate(float deltaTime) {
    if (sf::Keyboard::isKeyPressed(leftKey)) {
      sprite.rotate(-rotationSpeed * deltaTime);
    }
    if (sf::Keyboard::isKeyPressed(rightKey)) {
      sprite.rotate(rotationSpeed * deltaTime);
    }
  }

  void shoot(std::vector<Bullet> &bullets, float deltaTime) {
    if (sf::Keyboard::isKeyPressed(shootKey) && timeSinceLastShot >= fireRate) {
      sf::Vector2f bulletPos =
          sprite.getPosition() +
          sf::Vector2f(cos(sprite.getRotation() * PI / 180) *
                           (sprite.getLocalBounds().width / 2 + 50),
                       sin(sprite.getRotation() * PI / 180) *
                           (sprite.getLocalBounds().height / 2 + 50));
      bullets.emplace_back(bulletPos, sprite.getRotation() * PI / 180, 300.0f,
                           maze);
      timeSinceLastShot = 0.0f;
    } else {
      timeSinceLastShot += deltaTime;
    }
  }

  void detectBulletCollision(std::vector<Bullet> &bullets) {
    sf::FloatRect tankBounds = sprite.getGlobalBounds();
    float reductionFactor = 0.1f;
    tankBounds.width *= (1.0f - reductionFactor);
    tankBounds.height *= (1.0f - reductionFactor);
    tankBounds.left += sprite.getGlobalBounds().width * reductionFactor / 2.0f;
    tankBounds.top += sprite.getGlobalBounds().height * reductionFactor / 2.0f;
    auto it = bullets.begin();
    while (it != bullets.end()) {
      if (it->shape.getGlobalBounds().intersects(tankBounds)) {
        alive = false;
        it = bullets.erase(it);
      } else {
        ++it;
      }
    }
  }

  void update(float deltaTime, std::vector<Bullet> &bullets) {
    detectBulletCollision(bullets);
    move(deltaTime);
    rotate(deltaTime);
    shoot(bullets, deltaTime);
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

  Tank player1(texture, sf::Vector2f(300.f, 500.f), &maze);
  Tank player2(texture, sf::Vector2f(600.f, 500.f), &maze);

  player1.setControls(sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A,
                      sf::Keyboard::D, sf::Keyboard::Space);
  player2.setControls(sf::Keyboard::Up, sf::Keyboard::Down, sf::Keyboard::Left,
                      sf::Keyboard::Right, sf::Keyboard::Enter);

  std::vector<Bullet> bullets;

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
      it->move(deltaTime);
      if (!it->isAlive()) {
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
      window.draw(bullet.shape);
    }
    window.display();
  }

  return 0;
}
