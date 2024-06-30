#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <vector>
#include <stack>
#include <cstdlib>

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

        for (int i = 0; i < neighbors.size(); i++) {
            std::cout << "Neighbor["<< i <<"]: " << neighbors[i]->i << ", " << neighbors[i]->j << std::endl;
        }

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
        if (visited) {
            sf::RectangleShape rect;
            rect.setPosition(sf::Vector2f(180 + i * w, 100 + j * w));
            rect.setSize(sf::Vector2f(w, w));
            rect.setFillColor(sf::Color(0, 255, 0, 160));
            window.draw(rect);
        }
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
        std::cout << "Current cell: " << current->i << ", " << current->j << std::endl;

        while (true) {
            Cell* next = current->checkNeighbors(grid);
            if (next != nullptr) {
                std::cout << "Next cell: " << next->i << ", " << next->j << std::endl;
                next->visited = true;

                stack.push(current);

                removeWalls(current, next);

                current = next;
            } else if (stack.size() > 0) {
                current = stack.top();
                stack.pop();
            } else {
                break;
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        for (auto& cell : grid) {
            cell.draw(window);
        }
    }

    std::vector<Cell> grid;
    Cell* current;
    std::stack<Cell*> stack;
};

int main() {
    srand(time(0));

    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "SFML Maze Generator",
        sf::Style::Fullscreen);

    Maze maze(window.getSize());

    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        float deltaTime = clock.restart().asSeconds();

        // Renderizado
        window.clear();
        maze.draw(window);
        window.display();
    }

    return 0;
}