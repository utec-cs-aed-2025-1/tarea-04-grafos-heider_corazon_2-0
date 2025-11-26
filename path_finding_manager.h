//
// Created by juan-diego on 3/29/24.
//

#ifndef HOMEWORK_GRAPH_PATH_FINDING_MANAGER_H
#define HOMEWORK_GRAPH_PATH_FINDING_MANAGER_H


#include "window_manager.h"
#include "graph.h"
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <iostream>


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
        std::unordered_map<Node *, Node *> parent;//aca guardamos el camino resultante
        std::unordered_map<Node *, double> dist;//aca guardamos la distancia resultante
        std::set<Entry> pq;//aca guardamos la cola de prioridad
        
        // Inicializar distancias           
        for (auto &[id, node] : graph.nodes){
            dist[node] = std::numeric_limits<double>::max();
        }
        
        dist[src] = 0.0;//distancia del origen al origen es 0
        pq.insert({src, 0.0});//insertamos el origen en la cola de prioridad
        
        while (!pq.empty()) {
            Entry current = *pq.begin();//obtenemos el elemento con menor distancia
            pq.erase(pq.begin());
            
            Node* u = current.node;//obtenemos el nodo con menor distancia
            
            if (u == dest) break;//Si el nodo actual es el destino, salimos del bucle
            
            // Explorar vecinos
            for (Edge* edge : u->edges) {
                Node* v = (edge->src == u) ? edge->dest : edge->src;//obtenemos el vecino
                double weight = edge->length;//obtenemos el peso de la arista
                double newDist = dist[u] + weight;//obtenemos la distancia acumulada
                
                if (newDist < dist[v]) {
                    // Registrar arista explorada (en CYAN y más gruesa para visibilidad)
                    visited_edges.push_back(sfLine(u->coord, v->coord, sf::Color::Cyan, 2.5f));
                    
                    // Remover la entrada antigua si existe
                    auto it = pq.find({v, dist[v]});
                    if (it != pq.end()) pq.erase(it);
                    
                    dist[v] = newDist;//actualizamos la distancia
                    parent[v] = u;//actualizamos el camino
                    pq.insert({v, newDist});//insertamos el vecino en la cola de prioridad
                }
            }
        }

        set_final_path(parent);
        std::cout << ">> [DIJKSTRA] Aristas exploradas: " << visited_edges.size() 
                  << " | Camino final: " << path.size() << " aristas" << std::endl;
    }

    void best_first_search(Graph &graph) {
        std::unordered_map<Node *, Node *> parent;
        std::set<Entry> openSet;
        std::unordered_set<Node*> visited;
        
        // Función heurística: distancia euclidiana
        auto heuristic = [](Node* a, Node* b) -> double {
            float dx = a->coord.x - b->coord.x;
            float dy = a->coord.y - b->coord.y;
            return std::sqrt(dx*dx + dy*dy) * 140.0;
        };
        
        // Best First Search usa SOLO la heurística (greedy)
        openSet.insert({src, heuristic(src, dest)});
        
        while (!openSet.empty()) {
            Entry current = *openSet.begin();
            openSet.erase(openSet.begin());
            Node* u = current.node;
            
            if (visited.find(u) != visited.end()) continue;
            visited.insert(u);
            
            if (u == dest) break;
            
            for (Edge* edge : u->edges) {
                Node* v = (edge->src == u) ? edge->dest : edge->src;
                
                if (visited.find(v) == visited.end()) {
                    // Registrar arista explorada (en CYAN y más gruesa para visibilidad)
                    visited_edges.push_back(sfLine(u->coord, v->coord, sf::Color::Cyan, 2.5f));
                    
                    if (parent.find(v) == parent.end()) {
                        parent[v] = u;
                        // Solo usa heurística, no costo acumulado
                        openSet.insert({v, heuristic(v, dest)});
                    }
                }
            }
        }

        set_final_path(parent);
        std::cout << ">> [BEST FIRST SEARCH] Aristas exploradas: " << visited_edges.size() 
                  << " | Camino final: " << path.size() << " aristas" << std::endl;
    }

    void a_star(Graph &graph) {
        std::unordered_map<Node *, Node *> parent;
        std::unordered_map<Node *, double> gScore;  // Costo real desde src
        std::set<Entry> openSet;
        std::unordered_set<Node*> closedSet;  // Nodos ya procesados
        
        // Función heurística: distancia euclidiana
        auto heuristic = [](Node* a, Node* b) -> double {
            float dx = a->coord.x - b->coord.x;
            float dy = a->coord.y - b->coord.y;
            return std::sqrt(dx*dx + dy*dy) * 140.0;
        };
        
        // Solo inicializar el nodo origen
        gScore[src] = 0.0;
        double fScore_src = heuristic(src, dest);
        openSet.insert({src, fScore_src});
        
        while (!openSet.empty()) {
            Entry current = *openSet.begin();
            openSet.erase(openSet.begin());
            Node* u = current.node;
            
            // Si ya procesamos este nodo, skip
            if (closedSet.find(u) != closedSet.end()) continue;
            closedSet.insert(u);
            
            if (u == dest) break;
            
            for (Edge* edge : u->edges) {
                Node* v = (edge->src == u) ? edge->dest : edge->src;
                
                // Si ya procesamos v, skip
                if (closedSet.find(v) != closedSet.end()) continue;
                
                double tentative_gScore = gScore[u] + edge->length;
                
                // Si no hemos visitado v, o encontramos un camino mejor
                if (gScore.find(v) == gScore.end() || tentative_gScore < gScore[v]) {
                    // Registrar arista explorada (en CYAN y más gruesa para visibilidad)
                    visited_edges.push_back(sfLine(u->coord, v->coord, sf::Color::Cyan, 2.5f));
                    
                    // Remover entrada antigua si existe
                    if (gScore.find(v) != gScore.end()) {
                        double oldF = gScore[v] + heuristic(v, dest);
                        auto it = openSet.find({v, oldF});
                        if (it != openSet.end()) openSet.erase(it);
                    }
                    
                    parent[v] = u;
                    gScore[v] = tentative_gScore;
                    double fScore_v = gScore[v] + heuristic(v, dest);
                    openSet.insert({v, fScore_v});
                }
            }
        }

        set_final_path(parent);
        std::cout << ">> [A*] Aristas exploradas: " << visited_edges.size() 
                  << " | Camino final: " << path.size() << " aristas" << std::endl;
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

        // Solo limpiar los caminos previos, NO resetear src/dest
        path.clear();
        visited_edges.clear();
        
        // Asegurar que src y dest tengan los colores correctos
        src->color = sf::Color::Green;
        src->radius = 3.0f;
        dest->color = sf::Color::Cyan;
        dest->radius = 3.0f;
        
        switch (algorithm) {
            case Dijkstra:
                dijkstra(graph);
                break;
            case BestFirstSearch:
                best_first_search(graph);
                break;
            case AStar:
                a_star(graph);
                break;
            default:
                break;
        }
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
