#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include  <locale.h>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>

using namespace std;

// Параметры задачи
const vector<pair<int, int>> items = {
    {1, 5}, {2, 5}, {3, 10}, {4, 9}, {5, 6},
    {6, 8}, {7, 6}, {8, 5}, {9, 8}, {10, 6},
};
const int max_weight = 30;
const double mutation_coef = 0.3141592;
int population_size = 20;
int num_of_generations = 100;
int num_items = items.size();


void CocktailSort(vector<int>& a, vector<int>& b)
{
    bool swapped = true;
    int n = a.size();
    int start = 0;
    int end = n - 1;

    while (swapped) {
        swapped = false;

        for (int i = start; i < end; ++i) {
            if (b[i] > b[i + 1]) {
                swap(b[i], b[i + 1]);
                swap(a[i], a[i + 1]);
                swapped = true;
            }
        }

        if (!swapped)
            break;

        swapped = false;
        --end;

        for (int i = end - 1; i >= start; --i) {
            if (b[i] > b[i + 1]) {
                swap(b[i], b[i + 1]);
                swap(a[i], a[i + 1]);
                swapped = true;
            }
        }

        ++start;
    }
}


vector<int> generate_individual(int num_items) {
    vector<int> individual(num_items);
    for (int i = 0; i < num_items; ++i) {
        individual[i] = rand() % 2; // случайное значение 0 или 1
    }
    return individual;
}


pair<vector<int>, vector<int>> crossover(const vector<int>& parent1, const vector<int>& parent2) {
    int size = parent1.size();
    vector<int> child1(size);
    vector<int> child2(size);
    int crossover_point = rand() % size; // точка разрыва

    for (int i = 0; i < size; ++i) {
        if (i <= crossover_point) {
            child1[i] = parent1[i];
            child2[i] = parent2[i];
        }
        else {
            child1[i] = parent2[i];
            child2[i] = parent1[i];
        }
    }
    return { child1, child2 };
}


void mutate(vector<int>& individual) {
    for (int i = 0; i < individual.size(); ++i) {
        if ((rand() / double(RAND_MAX)) < mutation_coef) {
            individual[i] = 1 - individual[i];
        }
    }
}


pair<int, int> fitness(const vector<int>& individual) {
    int total_value = 0, total_weight = 0;
    for (size_t i = 0; i < individual.size(); ++i) {
        if (individual[i] == 1) {
            total_weight += items[i].first;
            total_value += items[i].second;
        }
    }

    if (total_weight > max_weight) {
        total_value = 0;
    }
    return { total_value, total_weight };
}


vector<vector<int>> non_dominated_sorting(vector<vector<int>>& population) {
    vector<int> el(0);
    vector<vector<int>> fronts = { {} };

    int size = population.size();
    vector<int> domination_count(size, 0);
    vector<vector<int>> dominated_solutions(size, el);
    vector<int> rank(size, 0);

    for (int p = 0; p < size; p++) {
        for (int q = 0; q < size; q++) {
            if (p != q) {
                auto fit_p = fitness(population[p]);
                auto fit_q = fitness(population[q]);

                if ((fit_p.first >= fit_q.first && fit_p.second <= fit_q.second) && (fit_p.first > fit_q.first || fit_p.second < fit_q.second)) {
                    dominated_solutions[p].push_back(q);
                }
                else if ((fit_q.first >= fit_p.first && fit_q.second <= fit_p.second) && (fit_q.first > fit_p.first || fit_q.second < fit_p.second)) {
                    domination_count[p] += 1;
                }
            }
        }

        if (domination_count[p] == 0) {
            rank[p] = 0;
            if (fronts.size() == 0) {
                fronts.push_back(el);
            }
            fronts[0].push_back(p);
        }
    }

    int i = 0;
    while (fronts[i].size() != 0) {
        vector<int> next_front = {};
        for (int p : fronts[i]) {
            for (int q : dominated_solutions[p]) {
                domination_count[q] -= 1;
                if (domination_count[q] == 0) {
                    rank[q] = i + 1;
                    if (find(next_front.begin(), next_front.end(), q) == next_front.end()) {
                        next_front.push_back(q);
                    }
                }
            }
        }
        i += 1;
        fronts.push_back(next_front);
    }
    fronts.pop_back();

    return fronts;
}

vector<int> calculate_crowding_distance(vector<int>& front, vector<vector<int>>& population) {
    int size = front.size();
    vector<int> distances(size, 0);

    for (int m = 0; m < 2; m++) {
        vector<int> sorted_indices = {};
        vector<int> sorting_values = {};
        if (m == 0) {
            for (int i = 0; i < size; i++) {
                sorting_values.push_back(fitness(population[front[i]]).first);
                sorted_indices.push_back(i);
            }
            CocktailSort(sorted_indices, sorting_values);
        }
        else {
            for (int i = 0; i < size; i++) {
                sorting_values.push_back(fitness(population[front[i]]).second);
                sorted_indices.push_back(i);
            }
            CocktailSort(sorted_indices, sorting_values);
        }

        distances[sorted_indices[0]] = 999999;
        distances[sorted_indices[size - 1]] = 999999;
        for (int i = 1; i < size - 1; i++) {
            if (m == 0) {
                distances[sorted_indices[i]] += (fitness(population[front[sorted_indices[i + 1]]]).first - fitness(population[front[sorted_indices[i - 1]]]).first);
            }
            else {
                distances[sorted_indices[i]] += (fitness(population[front[sorted_indices[i + 1]]]).second - fitness(population[front[sorted_indices[i - 1]]]).second);
            }
        }
    }
    return distances;
}


vector<vector<int>> select_population(vector<vector<int>>& population, vector<vector<int>>& fronts) {
    vector<vector<int>> new_population;

    for (vector<int> front : fronts) {
        if (new_population.size() + front.size() > population_size) {
            vector<int> distances = calculate_crowding_distance(front, population);

            int size = front.size();
            vector<int> sorted_indices = {};
            vector<int> sorting_values = {};

            for (int i = 0; i < size; i++) {
                sorted_indices.push_back(i);
                sorting_values.push_back(distances[i]);
            }

            CocktailSort(sorted_indices, sorting_values);
            reverse(sorted_indices.begin(), sorted_indices.end());

            int new_population_size = new_population.size();
            for (int i = 0; i < population_size - new_population_size && i < sorted_indices.size(); i++) {
                new_population.push_back(population[front[sorted_indices[i]]]);
            }
            break;
        }
        else {
            for (int i : front) {
                new_population.push_back(population[i]);
            }
        }
    }

    return new_population;
}


vector<vector<int>> nsga2(int num_generations, int population_size, int num_items) {
    vector<vector<int>> population;
    for (int i = 0; i < population_size; ++i) {
        population.push_back(generate_individual(num_items));
    }

    for (int generation = 0; generation < num_generations; generation++) {
        vector<vector<int>> new_population(0);

        while (new_population.size() < population_size) {
            int ind1 = rand() % population_size;
            int ind2 = rand() % population_size;
            while (ind1 == ind2) {
                ind2 = rand() % population_size;
            }
            vector<int> parent1 = population[ind1];
            vector<int> parent2 = population[ind2];

            vector<int> child1;
            vector<int> child2;
            pair<vector<int>, vector<int>> children = crossover(parent1, parent2);
            child1 = children.first;
            child2 = children.second;

            mutate(child1);
            mutate(child2);

            new_population.push_back(child1);
            new_population.push_back(child2);
        }

        for (vector<int> el : new_population) {
            population.push_back(el);
        }

        vector<vector<int>> fronts = non_dominated_sorting(population);
        population = select_population(population, fronts);
    }

    return population;
}


// Визуализация результатов с SFML
void visualize_results(const vector<int>& best_values) {

}

int main() {
    srand((unsigned)time(0));
    setlocale(LC_ALL, "rus");

    vector<int> pareto_front;
    vector<vector<int>> final_population;

    final_population = nsga2(num_of_generations, population_size, num_items);
    pareto_front = non_dominated_sorting(final_population)[0];
    pair<int, int> solution;
    const int size = pareto_front.size();
    vector<int> x;
    vector<int> y;
    for (int i : pareto_front) {
        solution = fitness(final_population[i]);
        cout << solution.first << " " << solution.second << "\n";
        x.push_back(solution.second);
        y.push_back(solution.first);
    }

    float window_x_size = 800;
    float window_y_size = 600;
    sf::RenderWindow window(sf::VideoMode(window_x_size, window_y_size), "SFML works!");
    float radius = window_y_size * 0.008;
    float x_line_start_x = 40.f / 800.f * window_x_size;
    float x_line_start_y = 570.f / 600.f * window_y_size;
    float y_line_start_y = 30.f / 600.f * window_y_size;
    sf::Vertex line_x[] =
    {
        sf::Vertex(sf::Vector2f(x_line_start_x, x_line_start_y), sf::Color::Black),
        sf::Vertex(sf::Vector2f(window_x_size - x_line_start_x, x_line_start_y), sf::Color::Black)
    };
    sf::Vertex line_y[] =
    {
        sf::Vertex(sf::Vector2f(x_line_start_x, y_line_start_y), sf::Color::Black),
        sf::Vertex(sf::Vector2f(x_line_start_x, window_y_size - y_line_start_y), sf::Color::Black)
    };

    float length_x_line = window_x_size - 2 * x_line_start_x;
    float length_y_line = window_y_size - 2 * y_line_start_y;
    float scale_x = length_x_line / 3;
    float max_x = *max_element(x.begin(), x.end());
    float scale_value_x = max_x / 3.f;
    
    sf::Vertex scales_x[4][2];
    for (int i = 0; i < 4; i++) {
        scales_x[i][0] = sf::Vertex(sf::Vector2f(x_line_start_x + scale_x*i, x_line_start_y - 10), sf::Color::Black);
        scales_x[i][1] = sf::Vertex(sf::Vector2f(x_line_start_x + scale_x * i, x_line_start_y + 10), sf::Color::Black);
    }

    float scale_y = length_y_line / 3;
    float max_y = *max_element(y.begin(), y.end());
    float scale_value_y = max_y / 3.f;

    sf::Vertex scales_y[4][2];
    for (int i = 0; i < 4; i++) {
        scales_y[i][0] = sf::Vertex(sf::Vector2f(x_line_start_x-10, y_line_start_y + scale_y * i), sf::Color::Black);
        scales_y[i][1] = sf::Vertex(sf::Vector2f(x_line_start_x+10, y_line_start_y + scale_y * i), sf::Color::Black);
    }


    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White);



        window.draw(line_y, 2, sf::Lines);
        window.draw(line_x, 2, sf::Lines);
        for (int j = 0; j < 4; j++) {
            sf::Vertex line[2];
            line[0] = scales_x[j][0];
            line[1] = scales_x[j][1];
            window.draw(line, 2, sf::Lines);

            string a = to_string((int)(scale_value_x * j));
            sf::Font font;
            font.loadFromFile("arial.ttf");

            sf::Text text(a, font);
            text.setCharacterSize(18);
            float text_x = x_line_start_x + scale_x * j;
            text.setPosition(text_x-10.f, x_line_start_y + 5.f);
            text.setFillColor(sf::Color::Black);
            window.draw(text);
        }

        for (int j = 0; j < 3; j++) {
            sf::Vertex line[2];
            line[0] = scales_y[j][0];
            line[1] = scales_y[j][1];
            window.draw(line, 2, sf::Lines);

            string a = to_string((int)(scale_value_y * (3-j)));
            sf::Font font;
            font.loadFromFile("arial.ttf");

            sf::Text text(a, font);
            text.setCharacterSize(18);
            float text_y = y_line_start_y + scale_y * j;
            text.setPosition(x_line_start_x - 30.f, text_y-10.f);
            text.setFillColor(sf::Color::Black);
            window.draw(text);
        }
        for (int i = 0; i < size; i++) {
            sf::CircleShape shape(radius);
            shape.setFillColor(sf::Color::Blue);
            float new_x = x_line_start_x - radius + (float)x[i] / max_x * (float)(window_x_size - 2 * x_line_start_x);
            float new_y = y_line_start_y + (float)(window_y_size - 2 * y_line_start_y) - radius - (float)y[i] / max_y * (float)(window_y_size - 2 * y_line_start_y);
            shape.setPosition(new_x, new_y);
            window.draw(shape);
        }
        window.display();
    }

    return 0;
}
