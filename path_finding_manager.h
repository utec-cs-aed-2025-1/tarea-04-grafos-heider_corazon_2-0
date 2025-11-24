//
// Created by juan-diego on 3/29/24.
//

#ifndef HOMEWORK_GRAPH_PATH_FINDING_MANAGER_H
#define HOMEWORK_GRAPH_PATH_FINDING_MANAGER_H


#include "window_manager.h"
#include "graph.h"
#include <unordered_map>
#include <set>


// Este enum sirve para identificar el algoritmo que el usuario desea simular
enum Algorithm {
    None,
    Dijkstra,
    BestFirstSearch,
    AStar
};


//* --- PathFindingManager ---
//
// Esta clase sirve para realizar las simulaciones de nuestro grafo.
//
// Variables miembro
//     - path           : Contiene el camino resultante del algoritmo que se desea simular
//     - visited_edges  : Contiene todas las aristas que se visitaron en el algoritmo, notar que 'path'
//                        es un subconjunto de 'visited_edges'.
//     - window_manager : Instancia del manejador de ventana, es utilizado para dibujar cada paso del algoritmo
//     - src            : Nodo incial del que se parte en el algoritmo seleccionado
//     - dest           : Nodo al que se quiere llegar desde 'src'
//*
class PathFindingManager {
    WindowManager *window_manager;
    std::vector<sfLine> path;
    std::vector<sfLine> visited_edges;

    struct Entry {
        Node* node;
        double dist;

        bool operator < (const Entry& other) const {
            return dist < other.dist;
        }
    };


    void dijkstra(Graph &graph) {
        std::unordered_map<Node *, Node *> parent;
        std::unordered_map<Node *, double> dist;
        std::set<Entry> pq;

        // Inicializar distancias
        for (auto &[id, node] : graph.nodes) {
            dist[node] = std::numeric_limits<double>::max();
        }

        dist[src] = 0.0;
        pq.insert({src, 0.0});

        while (!pq.empty()) {
            Entry current = *pq.begin();
            pq.erase(pq.begin());

            Node* u = current.node;

            if (u == dest) break;  // Encontró el destino

            // Explorar vecinos
            for (Edge* edge : u->edges) {
                Node* v = (edge->src == u) ? edge->dest : edge->src;
                double weight = edge->length;
                double newDist = dist[u] + weight;

                if (newDist < dist[v]) {
                    // Registrar arista explorada (en CYAN y más gruesa para visibilidad)
                    visited_edges.push_back(sfLine(u->coord, v->coord, sf::Color::Cyan, 2.5f));

                    // Remover la entrada antigua si existe
                    auto it = pq.find({v, dist[v]});
                    if (it != pq.end()) pq.erase(it);

                    dist[v] = newDist;
                    parent[v] = u;
                    pq.insert({v, newDist});
                }
            }
        }

        set_final_path(parent);
        std::cout << ">> [DIJKSTRA] Aristas exploradas: " << visited_edges.size()
                  << " | Camino final: " << path.size() << " aristas" << std::endl;
    }


    void a_star(Graph &graph) {
        std::unordered_map<Node *, Node *> parent;
        // TODO: Add your code here

        set_final_path(parent);
    }

    //* --- render ---
    // En cada iteración de los algoritmos esta función es llamada para dibujar los cambios en el 'window_manager'
    void render() {
        sf::sleep(sf::milliseconds(10));
        // TODO: Add your code here
    }

    //* --- set_final_path ---
    // Esta función se usa para asignarle un valor a 'this->path' al final de la simulación del algoritmo.
    // 'parent' es un std::unordered_map que recibe un puntero a un vértice y devuelve el vértice anterior a el,
    // formando así el 'path'.
    //
    // ej.
    //     parent(a): b
    //     parent(b): c
    //     parent(c): d
    //     parent(d): NULL
    //
    // Luego, this->path = [Line(a.coord, b.coord), Line(b.coord, c.coord), Line(c.coord, d.coord)]
    //
    // Este path será utilizado para hacer el 'draw()' del 'path' entre 'src' y 'dest'.
    //*
    void set_final_path(std::unordered_map<Node *, Node *> &parent) {
        path.clear();

        if (parent.find(dest) == parent.end()) {
            return;  // No hay camino
        }

        Node* current = dest;

        while (current != src && parent.find(current) != parent.end()) {
            Node* prev = parent[current];
            // Camino final en ROJO para mejor visibilidad
            path.push_back(sfLine(prev->coord, current->coord, sf::Color::Red, 3.0f));
            current = prev;
        }
    }

public:
    Node *src = nullptr;
    Node *dest = nullptr;

    explicit PathFindingManager(WindowManager *window_manager) : window_manager(window_manager) {}

    void exec(Graph &graph, Algorithm algorithm) {
        if (src == nullptr || dest == nullptr) {
            return;
        }

        // TODO: Add your code here
    }

    void reset() {
        path.clear();
        visited_edges.clear();

        if (src) {
            src->reset();
            src = nullptr;
            // ^^^ Pierde la referencia luego de restaurarlo a sus valores por defecto
        }
        if (dest) {
            dest->reset();
            dest = nullptr;
            // ^^^ Pierde la referencia luego de restaurarlo a sus valores por defecto
        }
    }

    void draw(bool draw_extra_lines) {
        // Dibujar todas las aristas visitadas
        if (draw_extra_lines) {
            for (sfLine &line: visited_edges) {
                line.draw(window_manager->get_window(), sf::RenderStates::Default);
            }
        }

        // Dibujar el camino resultante entre 'str' y 'dest'
        for (sfLine &line: path) {
            line.draw(window_manager->get_window(), sf::RenderStates::Default);
        }

        // Dibujar el nodo inicial
        if (src != nullptr) {
            src->draw(window_manager->get_window());
        }

        // Dibujar el nodo final
        if (dest != nullptr) {
            dest->draw(window_manager->get_window());
        }
    }
};


#endif //HOMEWORK_GRAPH_PATH_FINDING_MANAGER_H
