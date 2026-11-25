/*
 * Fuerza Bruta
 * Fuentes de consulta:
 * - Enunciado Tarea 2.
 * - Se uso Google Gemini como apoyo para la generación, comprensión y revisión
 *   de la implementación.
 *
 * Decisiones de diseño:
 * - Se utiliza DFS recursivo para enumerar exhaustivamente
 *   los caminos posibles.
 * - Se utiliza backtracking para construir y deshacer
 *   los movimientos del camino.
 * - Se descartan únicamente los caminos inválidos por
 *   murallas o agotamiento de vidas.
 */
#include <vector>
#include <string>
#include <climits>

#ifndef PATH_RESULT_H
#define PATH_RESULT_H

struct PathResult {
    bool found;
    long long score;
    std::string path;
    int remaining_lives;
};
#endif

static int BF_n, BF_m, BF_p;
static const std::vector<std::vector<int>>* BF_types;
static const std::vector<std::vector<long long>>* BF_values;
static PathResult BF_best;

static void dfs(int i, int j, int vidas, long long puntaje, std::string& camino) {
    // Procesar celda actual
    if ((*BF_types)[i][j] == 0) return;  // muralla -> invalido
    if ((*BF_types)[i][j] == -1) {       // mina
        vidas--;
        puntaje -= BF_p;
        if (vidas == 0) return;          // destruido -> invalido
    } else {                             // celda normal
        puntaje += (*BF_values)[i][j];
    }

    // Si llegamos al destino
    if (i == BF_n - 1 && j == BF_m - 1) {
        if (!BF_best.found || puntaje > BF_best.score) {
            BF_best = {true, puntaje, camino, vidas};
        }
        return;
    }

    // Explorar derecha
    if (j + 1 < BF_m) {
        camino += 'R';
        dfs(i, j + 1, vidas, puntaje, camino);
        camino.pop_back();
    }
    // Explorar abajo
    if (i + 1 < BF_n) {
        camino += 'D';
        dfs(i + 1, j, vidas, puntaje, camino);
        camino.pop_back();
    }
}

PathResult solveBruteForce(int n, int m, int v, int p,
                           const std::vector<std::vector<int>>& types,
                           const std::vector<std::vector<long long>>& values) {
    BF_n = n; BF_m = m; BF_p = p;
    BF_types = &types;
    BF_values = &values;
    BF_best = {false, LLONG_MIN, "", 0};

    std::string camino;
    camino.reserve(n + m - 2);
    dfs(0, 0, v, 0, camino);
    return BF_best;
}
