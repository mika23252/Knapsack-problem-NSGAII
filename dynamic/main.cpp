#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    int N, M;
    std::cin >> N >> M;

    std::vector<int> m(N + 1), c(N + 1);
    m[0] = c[0] = 0;

    for (int i = 1; i <= N; ++i) {
        std::cin >> m[i];
    }

    for (int i = 1; i <= N; ++i) {
        std::cin >> c[i];
    }

    std::vector<std::vector<int>> dp(N + 1, std::vector<int>(M + 1, 0));

    for (int i = 1; i <= N; ++i) {
        for (int j = 0; j <= M; ++j) {
            dp[i][j] = dp[i - 1][j];
            if (m[i] <= j) {
                dp[i][j] = std::max(dp[i][j], dp[i - 1][j - m[i]] + c[i]);
            }
        }
    }
    std::cout << *std::max_element(dp[N].begin(), dp[N].end()) << std::endl;
    int maxVal = *std::max_element(dp[N].begin(), dp[N].end());

    // Visualize the result using SFML
    sf::RenderWindow window(sf::VideoMode(1000, 800), "Knapsack Problem Visualization");
    sf::RectangleShape rectangle(sf::Vector2f(90, 90));
    sf::Font font;
    font.loadFromFile("arialmt.ttf"); // Load a font for text

    for (int i = 1; i <= N; ++i) {
        for (int j = 1; j <= M; ++j) {
            if (dp[i][j] == maxVal) {
                rectangle.setFillColor(sf::Color::Green);
            }
            else {
                rectangle.setFillColor(sf::Color::White);
            }

            rectangle.setPosition((j-1) * 100 + 5, (i-1) * 100 + 5);
            window.draw(rectangle);
            sf::Text text;
            text.setFont(font);
            text.setCharacterSize(20);
            text.setFillColor(sf::Color::Black);
            text.setPosition((j - 1) * 100 + 5, (i - 1) * 100 + 5);
            text.setString(std::to_string(dp[i][j]) + "\n(" + std::to_string(i) + "," + std::to_string(j) + ")");
            window.draw(text);
        }
    }

    window.display();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
    }

    return 0;
}