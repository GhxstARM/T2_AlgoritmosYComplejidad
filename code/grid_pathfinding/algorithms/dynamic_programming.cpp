/*
 * Implementación: Programación Dinámica
 *
 * Fuentes de consulta:
 * - Enunciado Tarea 2.
 * - Se uso Google Gemini como apoyo para la generación, comprensión y revisión
 *   de la implementación.
 *
 * Decisiones de diseño:
 * - Se utiliza un enfoque bottom-up mediante tabulación.
 * - El estado considera la posición y las vidas restantes.
 * - Se almacena el puntaje máximo para cada estado.
 * - Se utiliza una estructura parent para reconstruir
 *   el camino óptimo.
 * - Se utiliza un valor NEG_INF para representar estados
 *   inalcanzables.
 */
#include <vector>
#include <string>
#include <climits>
#include <algorithm>

#ifndef PATH_RESULT_H
#define PATH_RESULT_H
struct PathResult {
    bool found;
    long long score;
    std::string path;
    int remaining_lives;
};
#endif

PathResult solveDP(int n, int m, int v, int p,
                   const std::vector<std::vector<int>>& types,
                   const std::vector<std::vector<long long>>& values) {

    const long long NEG_INF = LLONG_MIN / 2;  // evitar overflow al sumar

    // dp[i][j][vl] = maximo puntaje para llegar a (i,j) con vl vidas
    std::vector<std::vector<std::vector<long long>>> dp(
        n, std::vector<std::vector<long long>>(m, std::vector<long long>(v + 1, NEG_INF)));

    // parent[i][j][vl] = direccion desde donde se llego: 'D' (arriba), 'R' (izquierda), 'S' (inicio)
    std::vector<std::vector<std::vector<char>>> parent(
        n, std::vector<std::vector<char>>(m, std::vector<char>(v + 1, ' ')));


    dp[0][0][v] = values[0][0];
    parent[0][0][v] = 'S';

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (i == 0 && j == 0) continue;
            if (types[i][j] == 0) continue;  // muralla

            for (int vl = 1; vl <= v; vl++) {
                long long best_val = NEG_INF;
                char best_dir = ' ';

                if (types[i][j] == -1) {
                    // Mina: necesitaba vl+1 vidas antes de entrar
                    if (vl + 1 <= v) {
                        // Desde arriba (movimiento D)
                        if (i > 0 && dp[i - 1][j][vl + 1] > NEG_INF) {
                            long long val = dp[i - 1][j][vl + 1] - p;
                            if (val > best_val) {
                                best_val = val;
                                best_dir = 'D';
                            }
                        }
                        // Desde la izquierda (movimiento R)
                        if (j > 0 && dp[i][j - 1][vl + 1] > NEG_INF) {
                            long long val = dp[i][j - 1][vl + 1] - p;
                            if (val > best_val) {
                                best_val = val;
                                best_dir = 'R';
                            }
                        }
                    }
                } else {
                    // Celda normal: mismas vidas
                    // Desde arriba (movimiento D)
                    if (i > 0 && dp[i - 1][j][vl] > NEG_INF) {
                        long long val = dp[i - 1][j][vl] + values[i][j];
                        if (val > best_val) {
                            best_val = val;
                            best_dir = 'D';
                        }
                    }
                    // Desde la izquierda (movimiento R)
                    if (j > 0 && dp[i][j - 1][vl] > NEG_INF) {
                        long long val = dp[i][j - 1][vl] + values[i][j];
                        if (val > best_val) {
                            best_val = val;
                            best_dir = 'R';
                        }
                    }
                }

                if (best_val > NEG_INF) {
                    dp[i][j][vl] = best_val;
                    parent[i][j][vl] = best_dir;
                }
            }
        }
    }

    // Encontrar el mejor puntaje en (n-1, m-1) sobre todas las vidas >= 1
    long long best_score = NEG_INF;
    int best_v = -1;
    for (int vl = 1; vl <= v; vl++) {
        if (dp[n - 1][m - 1][vl] > best_score) {
            best_score = dp[n - 1][m - 1][vl];
            best_v = vl;
        }
    }

    if (best_v == -1) {
        return {false, 0, "SIN SOLUCION", 0};
    }

    // Reconstruir camino (backtracking desde destino)
    std::string path;
    path.reserve(n + m - 2);
    int ci = n - 1, cj = m - 1, cv = best_v;

    while (ci != 0 || cj != 0) {
        char dir = parent[ci][cj][cv];
        path += dir;
        if (dir == 'D') {
            if (types[ci][cj] == -1) cv++;  // restaurar vida al retroceder
            ci--;
        } else {  // 'R'
            if (types[ci][cj] == -1) cv++;
            cj--;
        }
    }
    std::reverse(path.begin(), path.end());

    return {true, best_score, path, best_v};
}
