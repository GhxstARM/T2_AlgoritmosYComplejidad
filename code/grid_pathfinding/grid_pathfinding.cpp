/*
 * Programa principal y mediciones experimentales.
 * Fuentes de consulta:
 * - Enunciado Tarea 2.
 * - Se uso Google Gemini como apoyo para la generación, comprensión y revisión
 *   de la implementación.
 *
 * Decisiones de diseño:
 * - Se utiliza C++17 y la biblioteca filesystem para
 *   recorrer los directorios de instancias.
 * - Se utiliza chrono para medir tiempos de ejecución.
 * - Se implementa un mecanismo de medición de memoria
 *   dinámica mediante el reemplazo de operator new/delete.
 * - Se limita la ejecución de fuerza bruta a las instancias
 *   que cumplen N + M <= 32.
 * - Se comparan los resultados de fuerza bruta y
 *   programación dinámica para verificar su consistencia.
 * - Los resultados experimentales se almacenan en CSV.
 */
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cstdlib>
#include <cstddef>
#include <new>

namespace fs = std::filesystem;

#ifndef PATH_RESULT_H
#define PATH_RESULT_H
struct PathResult {
    bool found;
    long long score;
    std::string path;
    int remaining_lives;
};
#endif


static std::size_t g_mem_cur = 0;   // bytes vivos actualmente
static std::size_t g_mem_peak = 0;  // pico desde el ultimo mem_reset_peak()

void* operator new(std::size_t n) {
    std::size_t* p = static_cast<std::size_t*>(std::malloc(n + 16));
    if (!p) throw std::bad_alloc();
    p[0] = n;
    g_mem_cur += n;
    if (g_mem_cur > g_mem_peak) g_mem_peak = g_mem_cur;
    return p + 2;
}

void operator delete(void* q) noexcept {
    if (!q) return;
    std::size_t* p = static_cast<std::size_t*>(q) - 2;
    g_mem_cur -= p[0];
    std::free(p);
}

void operator delete(void* q, std::size_t) noexcept { operator delete(q); }

// Inicia una medicion: devuelve la memoria viva actual (la grilla de entrada y
// el resto del programa), que se descuenta al leer el pico.
static std::size_t mem_begin() {
    g_mem_peak = g_mem_cur;
    return g_mem_cur;
}

// Pico de heap (KB) usado por el algoritmo desde mem_begin(), sin contar la grilla.
static double mem_end_kb(std::size_t base) {
    return static_cast<double>(g_mem_peak - base) / 1024.0;
}


// Estimacion de memoria de pila para Fuerza Bruta:
// Se utiliza un tamano aproximado de 128 bytes por marco
// de llamada.
//
// Es importante tener en cuenta que este valor es una aproximacion y puede variar
// dependiendo del compilador, sus optimizaciones y la
// arquitectura del sistema. Por lo tanto, el consumo de
// memoria reportado para Fuerza Bruta debe interpretarse
// como una estimacion, no como una medicion exacta.
//
// La estimacion se suma al consumo dinamico medido en heap.
static const double BF_STACK_FRAME_BYTES = 128.0;

// Firmas de los algoritmos definidos en algorithms/
PathResult solveBruteForce(int n, int m, int v, int p,
                          const std::vector<std::vector<int>>& types,
                          const std::vector<std::vector<long long>>& values);

PathResult solveDP(int n, int m, int v, int p,
                   const std::vector<std::vector<int>>& types,
                   const std::vector<std::vector<long long>>& values);

PathResult solveGreedy(int n, int m, int v, int p,
                       const std::vector<std::vector<int>>& types,
                       const std::vector<std::vector<long long>>& values);

// Estructura auxiliar para metadatos del archivo de grilla
struct GridMetadata {
    int n = 0;
    int m = 0;
    std::string type;
    std::string domain;
    std::string sample;
};

// Parsea nombres con formato {n}_{m}_{tipo}_{dominio}_{muestra}
GridMetadata parseFilename(const std::string& stem) {
    GridMetadata meta;
    std::stringstream ss(stem);
    std::string token;
    std::vector<std::string> parts;
    while (std::getline(ss, token, '_')) {
        parts.push_back(token);
    }
    if (parts.size() >= 5) {
        try {
            meta.n = std::stoi(parts[0]);
            meta.m = std::stoi(parts[1]);
        } catch (...) {
            meta.n = 0;
            meta.m = 0;
        }
        meta.type = parts[2];
        meta.domain = parts[3];
        meta.sample = parts[4];
    }
    return meta;
}

// Guarda la salida en el formato especificado por el enunciado
void writeOutput(const std::string& filepath, const PathResult& res) {
    std::ofstream fout(filepath);
    if (!res.found) {
        fout << "SIN SOLUCION\n";
    } else {
        fout << res.score << "\n";
        fout << res.path << "\n";
        fout << res.remaining_lives << "\n";
    }
}

int main(int argc, char* argv[]) {
    // Determinar rutas relativas
    std::string input_dir = "data/grid_input";
    std::string output_dir = "data/grid_output";
    std::string measurements_dir = "data/measurements";

    if (!fs::exists(input_dir) && fs::exists("code/grid_pathfinding/data/grid_input")) {
        input_dir = "code/grid_pathfinding/data/grid_input";
        output_dir = "code/grid_pathfinding/data/grid_output";
        measurements_dir = "code/grid_pathfinding/data/measurements";
    }

    fs::create_directories(output_dir);
    fs::create_directories(measurements_dir);

    // Lista de archivos a procesar
    std::vector<fs::path> input_files;

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            fs::path p(argv[i]);
            if (fs::exists(p) && p.extension() == ".txt") {
                input_files.push_back(p);
            }
        }
    } else {
        if (!fs::exists(input_dir)) {
            std::cerr << "Error: No existe el directorio de entrada " << input_dir << std::endl;
            return 1;
        }
        for (const auto& entry : fs::directory_iterator(input_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                input_files.push_back(entry.path());
            }
        }
    }

    // Ordenar archivos para ejecucion determinista y ordenada
    std::sort(input_files.begin(), input_files.end(), [](const fs::path& a, const fs::path& b) {
        GridMetadata ma = parseFilename(a.stem().string());
        GridMetadata mb = parseFilename(b.stem().string());
        if (ma.n != mb.n) return ma.n < mb.n;
        if (ma.m != mb.m) return ma.m < mb.m;
        return a.filename().string() < b.filename().string();
    });

    std::cout << "Iniciando procesamiento de " << input_files.size() << " casos de prueba..." << std::endl;

    // Archivos CSV de mediciones
    std::string csv_path = measurements_dir + "/results.csv";
    std::ofstream csv(csv_path);
    csv << "filename,n,m,size,grid_type,domain,sample,algorithm,time_ms,memory_kb,score,remaining_lives,found,delta\n";

    std::string summary_csv_path = measurements_dir + "/summary_instances.csv";
    std::ofstream sum_csv(summary_csv_path);
    sum_csv << "filename,n,m,size,grid_type,domain,sample,"
            << "bf_time_ms,bf_mem_kb,bf_score,bf_found,"
            << "dp_time_ms,dp_mem_kb,dp_score,dp_found,"
            << "greedy_time_ms,greedy_mem_kb,greedy_score,greedy_found,delta\n";

    int processed = 0;
    for (const auto& filepath : input_files) {
        std::string filename = filepath.stem().string();
        GridMetadata meta = parseFilename(filename);

        std::ifstream fin(filepath);
        if (!fin.is_open()) {
            std::cerr << "No se pudo abrir el archivo: " << filepath << std::endl;
            continue;
        }

        int n = 0, m = 0, v = 0, p = 0;
        if (!(fin >> n >> m >> v >> p)) {
            std::cerr << "Error al leer cabecera en: " << filename << std::endl;
            continue;
        }

        if (meta.n == 0) meta.n = n;
        if (meta.m == 0) meta.m = m;

        std::vector<std::vector<int>> types(n, std::vector<int>(m));
        std::vector<std::vector<long long>> values(n, std::vector<long long>(m));

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                fin >> types[i][j];
            }
        }
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                fin >> values[i][j];
            }
        }
        fin.close();

        int size = n * m;

        // Variables para resumen de instancia
        bool bf_executed = false;
        double bf_ms = 0.0;
        double bf_mem = 0.0;
        PathResult bf_res = {false, 0, "", 0};

        // 1. Fuerza Bruta (solo si N + M <= 32)
        if (n + m <= 32) {
            bf_executed = true;
            std::size_t base = mem_begin();
            auto t0 = std::chrono::high_resolution_clock::now();
            bf_res = solveBruteForce(n, m, v, p, types, values);
            auto t1 = std::chrono::high_resolution_clock::now();
            bf_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

            // Heap medido + pila de recursion estimada (N+M-1 frames), en KB
            bf_mem = mem_end_kb(base) + ((double)(n + m - 1) * BF_STACK_FRAME_BYTES) / 1024.0;

            writeOutput(output_dir + "/" + filename + "_bruteforce_out.txt", bf_res);
        }

        // 2. Programacion Dinamica (DP)
        std::size_t base_dp = mem_begin();
        auto t0_dp = std::chrono::high_resolution_clock::now();
        PathResult dp_res = solveDP(n, m, v, p, types, values);
        auto t1_dp = std::chrono::high_resolution_clock::now();
        double dp_ms = std::chrono::duration<double, std::milli>(t1_dp - t0_dp).count();

        // Pico de heap medido (tablas dp y parent)
        double dp_mem = mem_end_kb(base_dp);
        writeOutput(output_dir + "/" + filename + "_dp_out.txt", dp_res);

        // 3. Greedy
        std::size_t base_gr = mem_begin();
        auto t0_gr = std::chrono::high_resolution_clock::now();
        PathResult gr_res = solveGreedy(n, m, v, p, types, values);
        auto t1_gr = std::chrono::high_resolution_clock::now();
        double gr_ms = std::chrono::duration<double, std::milli>(t1_gr - t0_gr).count();

        // Pico de heap medido (string del camino)
        double gr_mem = mem_end_kb(base_gr);

        // se fija un piso de 0.01 KB para que el punto exista en el grafico log.
        if (gr_mem < 0.01) gr_mem = 0.01;
        writeOutput(output_dir + "/" + filename + "_greedy_out.txt", gr_res);

        // Calcular brecha Delta = OPT - Greedy (cuando greedy encuentra solucion)
        std::string delta_str = "NA";
        if (dp_res.found && gr_res.found) {
            long long delta_val = dp_res.score - gr_res.score;
            delta_str = std::to_string(delta_val);
        }

        // Verificacion de consistencia BF == DP
        if (bf_executed && bf_res.found != dp_res.found) {
            std::cerr << "[ADVERTENCIA] Inconsistencia en " << filename
                      << ": BF found=" << bf_res.found << " != DP found=" << dp_res.found << std::endl;
        } else if (bf_executed && bf_res.found && dp_res.found) {
            if (bf_res.score != dp_res.score) {
                std::cerr << "[ADVERTENCIA] Inconsistencia en " << filename
                          << ": BF=" << bf_res.score << " != DP=" << dp_res.score << std::endl;
            }
        }

        // Escribir en CSV por algoritmo (delta: NA en BF y DP, solo aplica a greedy)
        if (bf_executed) {
            csv << filename << "," << n << "," << m << "," << size << ","
                << meta.type << "," << meta.domain << "," << meta.sample << ","
                << "bruteforce," << std::fixed << std::setprecision(4)
                << bf_ms << "," << bf_mem << "," << bf_res.score << ","
                << bf_res.remaining_lives << "," << (bf_res.found ? 1 : 0) << ",NA\n";
        }

        csv << filename << "," << n << "," << m << "," << size << ","
            << meta.type << "," << meta.domain << "," << meta.sample << ","
            << "dp," << std::fixed << std::setprecision(4)
            << dp_ms << "," << dp_mem << "," << dp_res.score << ","
            << dp_res.remaining_lives << "," << (dp_res.found ? 1 : 0) << ",NA\n";

        csv << filename << "," << n << "," << m << "," << size << ","
            << meta.type << "," << meta.domain << "," << meta.sample << ","
            << "greedy," << std::fixed << std::setprecision(4)
            << gr_ms << "," << gr_mem << "," << gr_res.score << ","
            << gr_res.remaining_lives << "," << (gr_res.found ? 1 : 0) << ","
            << delta_str << "\n";

        // Escribir en CSV de resumen
        sum_csv << filename << "," << n << "," << m << "," << size << ","
                << meta.type << "," << meta.domain << "," << meta.sample << ",";
        if (bf_executed) {
            sum_csv << bf_ms << "," << bf_mem << "," << bf_res.score << "," << (bf_res.found ? 1 : 0) << ",";
        } else {
            sum_csv << "NA,NA,NA,0,";
        }
        sum_csv << dp_ms << "," << dp_mem << "," << dp_res.score << "," << (dp_res.found ? 1 : 0) << ",";
        sum_csv << gr_ms << "," << gr_mem << "," << gr_res.score << "," << (gr_res.found ? 1 : 0) << ",";
        sum_csv << delta_str << "\n";

        processed++;
        if (processed % 10 == 0 || processed == (int)input_files.size()) {
            std::cout << "Progreso: " << processed << "/" << input_files.size() << " instancias procesadas." << std::endl;
        }
    }

    csv.close();
    sum_csv.close();

    std::cout << "Ejecucion finalizada exitosamente. Resultados guardados en "
              << output_dir << " y " << measurements_dir << "." << std::endl;
    return 0;
}