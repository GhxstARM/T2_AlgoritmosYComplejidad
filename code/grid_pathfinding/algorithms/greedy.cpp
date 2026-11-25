/*
 * Implementación: Greedy
 *
 * Fuentes de consulta:
 * - Enunciado Tarea 2.
 * - Se uso Google Gemini como apoyo para la generación, comprensión y revisión
 *   de la implementación.
 *
 * Decisiones de diseño:
 * - Se evalúan localmente los movimientos hacia la derecha
 *   y hacia abajo.
 * - Se calcula la ganancia neta de cada celda candidata.
 * - Se evita ingresar a minas cuando queda una sola vida.
 * - En caso de empate se prioriza el movimiento hacia la derecha.
 * - Si no existen movimientos seguros, se retorna
 *   SIN SOLUCION.
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

PathResult solveGreedy(int n, int m, int v, int p,
                       const std::vector<std::vector<int>>& types,
                       const std::vector<std::vector<long long>>& values) {
    int i = 0, j = 0, vidas = v;
    long long puntaje = values[0][0]; 
    std::string camino;
    camino.reserve(n + m - 2);

    while (i != n - 1 || j != m - 1) {
        bool r_ok = false, d_ok = false;
        long long r_gain = LLONG_MIN, d_gain = LLONG_MIN;

        // Evaluar celda derecha
        if (j + 1 < m) {
            int t = types[i][j + 1];
            if (t != 0 && !(t == -1 && vidas == 1)) {
                r_ok = true;
                r_gain = (t == -1) ? -(long long)p : values[i][j + 1];
            }
        }
        // Evaluar celda abajo
        if (i + 1 < n) {
            int t = types[i + 1][j];
            if (t != 0 && !(t == -1 && vidas == 1)) {
                d_ok = true;
                d_gain = (t == -1) ? -(long long)p : values[i + 1][j];
            }
        }

        if (!r_ok && !d_ok) {
            return {false, 0, "SIN SOLUCION", 0};
        }

        // Elegir: mayor ganancia neta; empate -> derecha (R)
        if (r_ok && (!d_ok || r_gain >= d_gain)) {
            j++;
            camino += 'R';
        } else {
            i++;
            camino += 'D';
        }

        // Actualizar estado segun celda actual
        if (types[i][j] == -1) {
            vidas--;
            puntaje -= p;
        } else {
            puntaje += values[i][j];
        }
    }

    return {true, puntaje, camino, vidas};
}
