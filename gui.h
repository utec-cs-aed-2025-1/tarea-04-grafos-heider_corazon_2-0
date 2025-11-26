//
// Created by juan-diego on 3/11/24.
//

#ifndef HOMEWORK_GRAPH_GUI_H
#define HOMEWORK_GRAPH_GUI_H


#include "window_manager.h"
#include "path_finding_manager.h"

#include <cmath>
#include <functional>
#include <limits>


class GUI {
    WindowManager window_manager;
    PathFindingManager path_finding_manager;

    Graph graph;

    // 1NN es un algoritmo muy popular que retorna el 1 Nearest Neighbour (de ahí el nombre 1NN), o vecino más cercano
    // de una coleccion de elementos a una query dada.
    // En este caso, nos interesa conocer cuál es el nodo mas cercano al punto 'query' pasado como parámetro.
    // Spatial Grid Optimization
    std::map<std::pair<int, int>, std::vector<Node*>> spatial_grid;
    float cell_size = 50.0f;

    void build_spatial_grid() {
        spatial_grid.clear();
        for (auto &[_, node] : graph.nodes) {
            int grid_x = static_cast<int>(node->coord.x / cell_size);
            int grid_y = static_cast<int>(node->coord.y / cell_size);
            spatial_grid[{grid_x, grid_y}].push_back(node);
        }
    }

    Node *_1NN(sf::Vector2i query) {
        Node *nearest = nullptr;
        double min_dist = std::numeric_limits<double>::max();

        int grid_x = static_cast<int>(query.x / cell_size);
        int grid_y = static_cast<int>(query.y / cell_size);

        auto euclidean = [&](sf::Vector2f point) -> double {
            double dx = static_cast<double>(point.x) - static_cast<double>(query.x);
            double dy = static_cast<double>(point.y) - static_cast<double>(query.y);
            return std::sqrt(dx * dx + dy * dy);
        };

        // Buscar en la celda actual y las 8 vecinas
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                auto it = spatial_grid.find({grid_x + dx, grid_y + dy});
                if (it != spatial_grid.end()) {
                    for (Node* node : it->second) {
                        double dist = euclidean(node->coord);
                        if (dist < min_dist) {
                            min_dist = dist;
                            nearest = node;
                        }
                    }
                }
            }
        }

        // Fallback: si no encontramos nada cerca (muy raro), buscar en todo (seguridad)
        if (nearest == nullptr) {
            for (auto &[_, node] : graph.nodes) {
                double dist = euclidean(node->coord);
                if (dist < min_dist) {
                    min_dist = dist;
                    nearest = node;
                }
            }
        }

        return nearest;
    }

public:

    explicit GUI(const std::string &nodes_path, const std::string &edges_path)
        : path_finding_manager(&window_manager),
          graph(&window_manager) {

        // Parsea los nodos y aristas leyendolos a partir del csv
        graph.parse_csv(nodes_path, edges_path);
        
        // Construir el grid espacial para optimizar búsquedas
        build_spatial_grid();

        // Para fines de la animación, puede variar dependiendo del computador
        window_manager.get_window().setFramerateLimit(200);
    }

    void main_loop() {
        bool draw_extra_lines = false;

        // Corre la GUI siempre y cuando la ventana esté abierta
        while (window_manager.is_open()) {

            // Verifica los eventos de la ventana que pueden ser 'triggereados' (lanzados) por el usuario en la
            // iteración actual - versión SFML 3.0 con std::optional<sf::Event>
            while (const std::optional event = window_manager.get_window().pollEvent()) {

                // Caso 1: El usuario cerro la ventana
                if (const auto *closed = event->getIf<sf::Event::Closed>()) {
                    (void)closed; // evitar warning de variable sin usar
                    // Cerrar la ventana y terminar la animación
                    window_manager.close();
                }
                // Caso 2: El usuario presionó una tecla
                else if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                    switch (keyPressed->code) {
                        // D = Ejecutar Dijkstra
                        case sf::Keyboard::Key::D: {
                            path_finding_manager.exec(graph, Dijkstra);
                            break;
                        }
                        // B = Ejecutar Best First Search
                        case sf::Keyboard::Key::B: {
                            path_finding_manager.exec(graph, BestFirstSearch);
                            break;
                        }
                        // A = Ejecutar AStar
                        case sf::Keyboard::Key::A: {
                            path_finding_manager.exec(graph, AStar);
                            break;
                        }
                        // R = Limpia la ultima simulación realizada.
                        //     También restaura los valores de 'src' y 'dest' a nullptr.
                        case sf::Keyboard::Key::R: {
                            path_finding_manager.reset();
                            break;
                        }
                        // E = Extra flag. Si es verdadero, hace un display de todos los 'edges'
                        //     visitados en la ejecución del último algoritmo.
                        case sf::Keyboard::Key::E: {
                            draw_extra_lines = !draw_extra_lines;
                            std::cout << "[INFO] Aristas exploradas: "
                                      << (draw_extra_lines ? "ACTIVADO" : "DESACTIVADO") << std::endl;
                            break;
                        }
                        // Q = Quit, misma funcionalidad que cerrar la ventana
                        case sf::Keyboard::Key::Q: {
                            window_manager.close();
                            break;
                        }
                        // Si no es alguna de las teclas anteriores, no hace nada
                        default:
                            break;
                    }
                }
                // Caso 3: El usuario presionó el mouse
                else if (const auto *mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                    (void)mousePressed; // no usamos el detalle del botón, solo la posición

                    // Obtiene las posiciones del mouse respecto a la ventana
                    sf::Vector2i mouse_position = sf::Mouse::getPosition(window_manager.get_window());

                    // Si no existe un nodo fuente ('src') asignado
                    if (path_finding_manager.src == nullptr) {
                        // Encuentra el vértice más cercano a la posición del mouse y asigna el vértice a 'src'
                        path_finding_manager.src = _1NN(mouse_position);
                        path_finding_manager.src->color = sf::Color::Green;
                        path_finding_manager.src->radius = 3.0f;
                        std::cout << ">> Nodo Origen (SRC) seleccionado: " << path_finding_manager.src->id << std::endl;
                    }
                    // Si no existe un nodo destino ('dest') asignado
                    else if (path_finding_manager.dest == nullptr) {
                        // Encuentra el vértice más cercano a la posición del mouse y asigna el vértice a 'dest'
                        path_finding_manager.dest = _1NN(mouse_position);
                        path_finding_manager.dest->color = sf::Color::Cyan;
                        path_finding_manager.dest->radius = 3.0f;
                        std::cout << ">> Nodo Destino (DEST) seleccionado: " << path_finding_manager.dest->id << std::endl;
                    }
                }
                // Cualquier otro evento es ignorado
                else {
                    // no-op
                }
            }

            // Limpia la ventana anterior
            window_manager.clear();

            // Dibuja el grafo en el frame actual
            graph.draw();
            // Dibuja el 'path' resultante de la simulacion,
            // si 'draw_extra_lines' es true, también dibujará el resto de aristas visitadas
            path_finding_manager.draw(draw_extra_lines);

            // Hace un display del frame actual
            window_manager.display();
        }
    }
};


#endif //HOMEWORK_GRAPH_GUI_H
