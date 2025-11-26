//
// Created by juan-diego on 3/29/24.
//

#ifndef HOMEWORK_GRAPH_WINDOW_MANAGER_H
#define HOMEWORK_GRAPH_WINDOW_MANAGER_H

#include <SFML/Graphics.hpp>

//*
// ---- Window Manager ----
// Esta clase sirve como wrapper de nuestra instancia de sf::RenderWindow
// para realizar las manipulaciones de la instancia de manera segura.
//*
class WindowManager {
    sf::RenderWindow window;

public:
    explicit WindowManager(int window_width = 600, int window_height = 800)
        : window(
            // Estilo SFML 3.0: VideoMode({width, height})
            sf::VideoMode({static_cast<unsigned int>(window_width),
                           static_cast<unsigned int>(window_height)}),
            "Lima City Graph"
          ) {}

    bool is_open() {
        return window.isOpen();
    }

    void close() {
        window.close();
    }

    // Wrapper de pollEvent
    // pero implementado con la API nueva de SFML 3.0 (std::optional<sf::Event>)
    bool poll_event(sf::Event &event) {
        if (const std::optional polled = window.pollEvent()) {
            event = *polled;  // copiamos el evento al parámetro de salida
            return true;
        }
        return false;
    }

    void clear(sf::Color color = sf::Color::Black) {
        window.clear(color);
    }

    void display() {
        window.display();
    }

    sf::RenderWindow &get_window() {
        return window;
    }
};

#endif //HOMEWORK_GRAPH_WINDOW_MANAGER_H
