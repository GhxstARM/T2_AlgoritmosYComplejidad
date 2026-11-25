# Documentación del Código - Tarea 2: Grid Pathfinding

**Semestre:** 2026-2  
**Alumno:** Adrian Alejandro Rangel Morales  
**Rol:** 202373631-7  
**Campus:** San Joaquín, Santiago  

---

## 1. Descripción General

El problema consiste en encontrar un camino con movimientos únicamente hacia la **Derecha** `R` y hacia **Abajo** `D` desde la celda superior izquierda $(1,1)$ hasta la celda inferior derecha $(N,M)$ en una grilla de dimensiones $N \times M$ que maximice el puntaje acumulado, respetando las restricciones de celdas con murallas y celdas con minas que restan vidas y penalizan el puntaje acumulado.

Se implementan, evalúan y comparan tres algorítmicos:
1. **Fuerza Bruta** (Backtracking recursivo).
2. **Programación Dinámica** (Formulación óptima con tabulación).
3. **Algoritmo Greedy** (selección miope por ganancia inmediata).

---

## 2. Estructura del Directorio

```text
code/
├── README.md                           # Documentación principal del código (este archivo)
└── grid_pathfinding/
    ├── algorithms/                     # Implementación modular de los tres enfoques
    │   ├── brute_force.cpp             # Búsqueda exhaustiva DFS
    │   ├── dynamic_programming.cpp     # Programación Dinámica bottom-up
    │   └── greedy.cpp                  # Algoritmo greedy
    ├── data/                           # Almacenamiento de entradas, salidas, mediciones y gráficos.
    │   ├── grid_input/                 # Instancias de prueba generadas (.txt)
    │   ├── grid_output/                # Respuestas por cada algoritmo (.txt)
    │   ├── measurements/               # Métricas tabuladas de tiempo, memoria y calidad (.csv)
    │   └── plots/                      # Gráficos comparativos (.png)
    ├── scripts/                        # Scripts en Python
    │   ├── grid_generator.py           # Generador de instancias aleatorias
    │   └── plot_generator.py           # Script para generar los graficos para el informe
    ├── grid_pathfinding.cpp            # Programa principal
    └── makefile                        # Script de compilación y ejecución
```

---

## 3. Requisitos del Sistema y Dependencias

- **Compilador C++:** `g++` con compatibilidad para C++17.
- **Herramienta de compilación:** `make`.
- **Python:** Versión **3.8+** para los scripts de generación y graficado.
  - Dependencias de Python requeridas:
    ```
    pip install numpy pandas matplotlib
    ```

---

## 4. Algoritmos Implementados (`algorithms/`)

- Todos los algoritmos comparten la misma estructura de retorno `PathResult` y la misma interfaz funcional:
```cpp
struct PathResult {
    bool found;             // true si existe un camino válido al destino; false en caso contrario
    long long score;        // puntaje total acumulado
    std::string path;       // secuencia de movimientos ('D' y 'R')
    int remaining_lives;    // cantidad de vidas al alcanzar (N,M)
};
```

### 4.1 Fuerza Bruta 
- **Estrategia:** usando backtracking explora todas las trayectorias posibles.

- **Complejidad Temporal:** $O\left(\binom{N+M-2}{N-1}\right)$.
- **Complejidad Espacial:** $O(N+M)$ en memoria de pila para almacenar la profundidad de la recursión.
- **Restricción:** solo se ejecuta en instancias donde $N + M \le 32$.

### 4.2 Programación Dinámica
- **Estrategia:** Enfoque *bottom-up*.
- **Reconstrucción del camino:** Se utiliza una matriz de punteros `parent[n][m][v+1]` para reconstruir en $O(N+M)$ el camino óptimo retrocediendo desde el destino hacia el origen.
- **Complejidad Temporal:** $O(N \cdot M \cdot V)$.
- **Complejidad Espacial:** $O(N \cdot M \cdot V)$ para almacenar las tablas de valores y transiciones.

### 4.3 Algoritmo Greedy
- **Estrategia:** Algoritmo miope que avanza tomando la mejor decisión local.
- **Complejidad Temporal:** $O(N + M)$ pasos.
- **Complejidad Espacial:** $O(N + M)$ para almacenar la cadena del camino generado.

---

## 5. Compilación y Ejecución del Programa Principal
- se compiló y se ejecutó en una terminal de Linux Ubuntu 22.04.2 LTS

- El programa principal `grid_pathfinding.cpp` orquesta la lectura de datos, medición de tiempo/memoria, validación cruzada y guarda de resultados.

- Ubicarse en el directorio `code/grid_pathfinding/`:

- Para ejecutar todo primero se debe generar los imputs con el script `grid_generator.py` en `code/grid_pathfinding/scripts\`

- luego de eso usaremos `make clean` en `code/grid_pathfinding/` para asegurar de que no hay ejecuciones previas.

- luego con `make run` para empezar con el test de los algoritmo con cada caso de prueba.

- para la creacion de graficos se debe esta ubicado en la carpeta `code/grid_pathfinding/scripts/` para luego ejecutar el comando python3 `plot_generator.py`.
- PRECAUCION: si intenta probar los algoritmos con los .csv generados, puede ocasionar sobreescritura y alterar los resultados.