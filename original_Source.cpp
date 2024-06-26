#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <iostream>
#include <vector>

struct Bullet {
  sf::CircleShape shape;
  sf::Vector2f velocity;
};

int main() {
  // Crear una ventana
  sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "SFML Tank Game",
                          sf::Style::Fullscreen);

  // Cargar una textura desde un archivo
  sf::Texture texture;
  if (!texture.loadFromFile("gTank.png")) {
    std::cerr << "Error al cargar la imagen" << std::endl;
    return -1;
  }

  // Crear un sprite con la textura cargada
  sf::Sprite sprite;
  sprite.setTexture(texture);

  // Establecer el origen del sprite al centro del tanque para facilitar la
  // rotación
  sprite.setOrigin(134.0f / 2, 92.0f / 2);
  sprite.setPosition(window.getSize().x / 2.0f,
                     window.getSize().y /
                         2.0f); // Centrando el tanque en la ventana

  // Crear un rectángulo que represente las paredes del laberinto
  sf::Vector2u windowSize = window.getSize();
  sf::RectangleShape maze(
      sf::Vector2f(windowSize.x * 0.9f, windowSize.y * 0.9f));
  maze.setPosition(windowSize.x * 0.05f, windowSize.y * 0.05f);
  maze.setFillColor(sf::Color::Transparent);
  maze.setOutlineThickness(5);
  maze.setOutlineColor(sf::Color::White);

  // Variables para el control del movimiento
  float moveSpeed = 200.0f;       // Velocidad de movimiento de los tanques
  float rotationSpeed = 200.0f;   // Velocidad de rotación de los tanques
  float bulletSpeed = 300.0f;     // Velocidad de las balas
  float fireRate = 0.2f;          // Tiempo entre disparos en segundos
  float timeSinceLastShot = 0.0f; // Tiempo desde el último disparo

  std::vector<Bullet> bullets; // Vector para almacenar las balas

  // Reloj para medir el tiempo
  sf::Clock clock;

  // Bucle principal
  while (window.isOpen()) {
    // Procesar eventos
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    // Calcular el tiempo transcurrido desde el último frame
    float deltaTime = clock.restart().asSeconds();
    timeSinceLastShot +=
        deltaTime; // Actualizar el tiempo desde el último disparo

    // Movimiento del tanque
    sf::Vector2f movement(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
      // Mover hacia adelante
      movement.x +=
          cos(sprite.getRotation() * 3.14159265 / 180) * moveSpeed * deltaTime;
      movement.y +=
          sin(sprite.getRotation() * 3.14159265 / 180) * moveSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
      // Mover hacia atrás
      movement.x -=
          cos(sprite.getRotation() * 3.14159265 / 180) * moveSpeed * deltaTime;
      movement.y -=
          sin(sprite.getRotation() * 3.14159265 / 180) * moveSpeed * deltaTime;
    }

    // Aplicar movimiento y comprobar colisiones con las paredes del laberinto
    sf::FloatRect nextPos = sprite.getGlobalBounds();
    nextPos.left += movement.x;
    nextPos.top += movement.y;

    sf::FloatRect mazeBounds = maze.getGlobalBounds();
    if (mazeBounds.contains(nextPos.left, nextPos.top) &&
        mazeBounds.contains(nextPos.left + nextPos.width, nextPos.top) &&
        mazeBounds.contains(nextPos.left, nextPos.top + nextPos.height) &&
        mazeBounds.contains(nextPos.left + nextPos.width,
                            nextPos.top + nextPos.height)) {
      sprite.move(movement);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
      // Rotar hacia la izquierda
      sprite.rotate(-rotationSpeed * deltaTime);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
      // Rotar hacia la derecha
      sprite.rotate(rotationSpeed * deltaTime);
    }

    // Disparar una bala con cooldown
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) &&
        timeSinceLastShot >= fireRate) {
      Bullet bullet;
      bullet.shape.setRadius(5.0f);
      bullet.shape.setFillColor(sf::Color::White); // Cambiar el color a blanco

      sf::Vector2f bulletPos;
      bulletPos.x = sprite.getPosition().x +
                    cos(sprite.getRotation() * 3.14159265 / 180) *
                        sprite.getLocalBounds().width / 2;
      bulletPos.y = sprite.getPosition().y +
                    sin(sprite.getRotation() * 3.14159265 / 180) *
                        sprite.getLocalBounds().height / 2;

      bullet.shape.setPosition(bulletPos);
      bullet.velocity = sf::Vector2f(
          cos(sprite.getRotation() * 3.14159265 / 180) * bulletSpeed,
          sin(sprite.getRotation() * 3.14159265 / 180) * bulletSpeed);
      bullets.push_back(bullet);
      timeSinceLastShot = 0.0f; // Reiniciar el tiempo desde el último disparo
    }

    // Actualizar las balas
    for (auto &bullet : bullets) {
      bullet.shape.move(bullet.velocity * deltaTime);

      // Comprobar colisiones con las paredes del laberinto y rebotar
      sf::FloatRect bulletBounds = bullet.shape.getGlobalBounds();
      if (bulletBounds.left <= mazeBounds.left ||
          bulletBounds.left + bulletBounds.width >=
              mazeBounds.left + mazeBounds.width) {
        bullet.velocity.x = -bullet.velocity.x;
        bullet.shape.move(bullet.velocity.x * deltaTime,
                          0); // Evitar que se quede pegada
      }
      if (bulletBounds.top <= mazeBounds.top ||
          bulletBounds.top + bulletBounds.height >=
              mazeBounds.top + mazeBounds.height) {
        bullet.velocity.y = -bullet.velocity.y;
        bullet.shape.move(0, bullet.velocity.y *
                                 deltaTime); // Evitar que se quede pegada
      }
    }

    // Limpiar la ventana
    window.clear();

    // Dibujar el laberinto
    window.draw(maze);

    // Dibujar el tanque
    window.draw(sprite);

    // Dibujar las balas
    for (const auto &bullet : bullets) {
      window.draw(bullet.shape);
    }

    // Mostrar el contenido de la ventana
    window.display();
  }

  return 0;
}
