/**
 * ESTRUCTURAS DE DATOS Y ALGORITMOS
 * Práctica I – Ordenamiento de Dataset
 * Universidad EAFIT
 *
 * Implementa: QuickSort, HeapSort, Árbol AVL
 * con análisis de rendimiento (tiempo + memoria).
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>   // solo para std::swap, no para ordenamiento
#include <random>
#include <iomanip>
#include <cstring>

// ─────────────────────────────────────────────
// UTILIDAD: cargar dataset desde archivo
// ─────────────────────────────────────────────
std::vector<std::string> loadDataset(const std::string& filename) {
    std::vector<std::string> words;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: no se puede abrir " << filename << "\n";
        return words;
    }
    std::string word;
    while (file >> word) {
        words.push_back(word);
    }
    return words;
}

// ─────────────────────────────────────────────
// 1. QUICKSORT (std::vector<string>)
// ─────────────────────────────────────────────
int partition(std::vector<std::string>& arr, int low, int high) {
    // Selección de pivote por mediana de tres para reducir el riesgo del peor caso
    int mid = low + (high - low) / 2;
    if (arr[mid] < arr[low])  std::swap(arr[low], arr[mid]);
    if (arr[high] < arr[low]) std::swap(arr[low], arr[high]);
    if (arr[mid] < arr[high]) std::swap(arr[mid], arr[high]);
    // arr[high] es ahora el pivote (valor mediano)
    const std::string pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        // Si el elemento actual es menor o igual al pivote, lo movemos a la izquierda
        if (arr[j] <= pivot) {
            ++i;
            std::swap(arr[i], arr[j]);
        }
    }
    // Colocamos el pivote en su posición final correcta
    std::swap(arr[i + 1], arr[high]);
    return i + 1;
}

void quickSort(std::vector<std::string>& arr, int low, int high) {
    // QuickSort iterativo con pila explícita (evita stack overflow con 100k elementos)
    std::vector<std::pair<int,int>> stack;
    stack.push_back({low, high});
    while (!stack.empty()) {
        auto [l, h] = stack.back();
        stack.pop_back();
        if (l < h) {
            int pi = partition(arr, l, h);
            // Apilamos los subarreglos izquierdo y derecho para procesarlos después
            stack.push_back({l, pi - 1});
            stack.push_back({pi + 1, h});
        }
    }
}

// ─────────────────────────────────────────────
// 2. HEAPSORT (Binary Max-Heap)
// ─────────────────────────────────────────────
void heapify(std::vector<std::string>& arr, int n, int i) {
    int largest = i;       // Asumimos que la raíz es el mayor
    int left    = 2 * i + 1; // Hijo izquierdo
    int right   = 2 * i + 2; // Hijo derecho

    // Verificamos si el hijo izquierdo es mayor que la raíz
    if (left  < n && arr[left]  > arr[largest]) largest = left;
    // Verificamos si el hijo derecho es mayor que el mayor actual
    if (right < n && arr[right] > arr[largest]) largest = right;

    if (largest != i) {
        // Si la raíz no es el mayor, intercambiamos y continuamos hundiéndola
        std::swap(arr[i], arr[largest]);
        heapify(arr, n, largest);
    }
}

void heapSort(std::vector<std::string>& arr) {
    int n = (int)arr.size();

    // Fase 1: construir el Max-Heap in-place
    // Empezamos desde el último nodo no-hoja (n/2 - 1)
    for (int i = n / 2 - 1; i >= 0; --i)
        heapify(arr, n, i);

    // Fase 2: extraer elementos del heap uno por uno
    for (int i = n - 1; i > 0; --i) {
        // Movemos la raíz (máximo actual) al final del arreglo
        std::swap(arr[0], arr[i]);
        // Restauramos la propiedad del heap sobre el arreglo reducido
        heapify(arr, i, 0);
    }
}

// ─────────────────────────────────────────────
// 3. ÁRBOL AVL
// ─────────────────────────────────────────────
struct AVLNode {
    std::string key;            // Palabra almacenada
    AVLNode*    left  = nullptr; // Hijo izquierdo
    AVLNode*    right = nullptr; // Hijo derecho
    int         height = 1;     // Altura del nodo (hojas = 1)
    explicit AVLNode(const std::string& k) : key(k) {}
};

// Retorna la altura de un nodo (0 si es nulo)
int avlHeight(AVLNode* n) { return n ? n->height : 0; }

// Calcula el factor de balance: altura(izq) - altura(der)
// Un AVL válido mantiene este valor en {-1, 0, 1}
int avlBalance(AVLNode* n) {
    return n ? avlHeight(n->left) - avlHeight(n->right) : 0;
}

// Actualiza la altura de un nodo según sus hijos
void avlUpdateHeight(AVLNode* n) {
    if (n) n->height = 1 + std::max(avlHeight(n->left), avlHeight(n->right));
}

// Rotación simple a la derecha (caso LL)
AVLNode* avlRotateRight(AVLNode* y) {
    AVLNode* x  = y->left;
    AVLNode* T2 = x->right;
    x->right = y;  // y se convierte en hijo derecho de x
    y->left  = T2; // El subárbol derecho de x pasa a ser hijo izquierdo de y
    avlUpdateHeight(y);
    avlUpdateHeight(x);
    return x; // x es la nueva raíz
}

// Rotación simple a la izquierda (caso RR)
AVLNode* avlRotateLeft(AVLNode* x) {
    AVLNode* y  = x->right;
    AVLNode* T2 = y->left;
    y->left  = x;  // x se convierte en hijo izquierdo de y
    x->right = T2; // El subárbol izquierdo de y pasa a ser hijo derecho de x
    avlUpdateHeight(x);
    avlUpdateHeight(y);
    return y; // y es la nueva raíz
}

// Inserta una clave en el árbol AVL y rebalancea si es necesario
AVLNode* avlInsert(AVLNode* node, const std::string& key) {
    // Caso base: posición encontrada, crear nuevo nodo
    if (!node) return new AVLNode(key);

    // Inserción estándar de BST
    if (key < node->key)      node->left  = avlInsert(node->left,  key);
    else if (key > node->key) node->right = avlInsert(node->right, key);
    else                      return node; // Duplicados ignorados

    // Actualizamos la altura del nodo actual
    avlUpdateHeight(node);
    int bf = avlBalance(node);

    // Caso LL: desbalance izquierdo, inserción en hijo izquierdo del izquierdo
    if (bf > 1 && key < node->left->key)
        return avlRotateRight(node);

    // Caso RR: desbalance derecho, inserción en hijo derecho del derecho
    if (bf < -1 && key > node->right->key)
        return avlRotateLeft(node);

    // Caso LR: desbalance izquierdo, inserción en hijo derecho del izquierdo
    if (bf > 1 && key > node->left->key) {
        node->left = avlRotateLeft(node->left);
        return avlRotateRight(node);
    }

    // Caso RL: desbalance derecho, inserción en hijo izquierdo del derecho
    if (bf < -1 && key < node->right->key) {
        node->right = avlRotateRight(node->right);
        return avlRotateLeft(node);
    }

    return node; // Nodo balanceado, no requiere rotación
}

// Recorrido inorden iterativo (evita recursión profunda con 100k nodos)
// Produce los elementos en orden alfabético ascendente
void avlInorder(AVLNode* root, std::vector<std::string>& result) {
    std::vector<AVLNode*> stack;
    AVLNode* curr = root;
    while (curr || !stack.empty()) {
        // Bajamos hasta el nodo más a la izquierda
        while (curr) { stack.push_back(curr); curr = curr->left; }
        // Procesamos el nodo y continuamos por la derecha
        curr = stack.back(); stack.pop_back();
        result.push_back(curr->key);
        curr = curr->right;
    }
}

// Libera toda la memoria del árbol en postorden
void avlFree(AVLNode* node) {
    if (!node) return;
    avlFree(node->left);
    avlFree(node->right);
    delete node;
}

// ─────────────────────────────────────────────
// ESTIMACIÓN DE MEMORIA
// ─────────────────────────────────────────────
size_t estimateVectorMemory(const std::vector<std::string>& v) {
    size_t total = sizeof(v); // Tamaño del objeto vector
    for (const auto& s : v) total += sizeof(std::string) + s.capacity(); // Más cada string
    return total;
}

size_t estimateAVLMemory(const std::vector<std::string>& words) {
    size_t nodeSize = sizeof(AVLNode); // Tamaño de cada nodo (punteros + height + string)
    size_t total = 0;
    for (const auto& s : words) total += nodeSize + s.size() + 1; // +1 por el terminador null
    return total;
}

// ─────────────────────────────────────────────
// VERIFICACIÓN: comprueba que el resultado esté ordenado
// ─────────────────────────────────────────────
bool isSorted(const std::vector<std::string>& v) {
    for (size_t i = 1; i < v.size(); ++i)
        if (v[i] < v[i-1]) return false;
    return true;
}

// ─────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────
int main(int argc, char* argv[]) {
    std::string datasetFile = "dataset.txt";
    if (argc > 1) datasetFile = argv[1]; // Permite pasar el archivo como argumento

    std::cout << "╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║          – Práctica I: Ordenamiento de Dataset (C++) ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n\n";

    // ── Carga del dataset ─────────────────────
    std::cout << "Cargando dataset desde: " << datasetFile << " ...\n";
    std::vector<std::string> base = loadDataset(datasetFile);
    if (base.empty()) { std::cerr << "Dataset vacío o no encontrado.\n"; return 1; }
    std::cout << "Palabras cargadas: " << base.size() << "\n\n";

    // ── Encabezado de la tabla de resultados ──
    std::cout << std::left
              << std::setw(20) << "Algoritmo"
              << std::setw(16) << "Tiempo (ms)"
              << std::setw(20) << "Memoria (KB)"
              << std::setw(10) << "Ordenado?"
              << "\n";
    std::cout << std::string(66, '-') << "\n";

    // ══ 1. QUICKSORT ══════════════════════════
    {
        std::vector<std::string> arr = base; // Copia del dataset original
        size_t mem = estimateVectorMemory(arr);

        auto t1 = std::chrono::high_resolution_clock::now();
        quickSort(arr, 0, (int)arr.size() - 1);
        auto t2 = std::chrono::high_resolution_clock::now();

        double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        bool ok   = isSorted(arr);

        std::cout << std::left
                  << std::setw(20) << "QuickSort"
                  << std::setw(16) << std::fixed << std::setprecision(2) << ms
                  << std::setw(20) << (mem / 1024)
                  << std::setw(10) << (ok ? "SI" : "NO")
                  << "\n";
    }

    // ══ 2. HEAPSORT ═══════════════════════════
    {
        std::vector<std::string> arr = base; // Copia del dataset original
        size_t mem = estimateVectorMemory(arr);

        auto t1 = std::chrono::high_resolution_clock::now();
        heapSort(arr);
        auto t2 = std::chrono::high_resolution_clock::now();

        double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        bool ok   = isSorted(arr);

        std::cout << std::left
                  << std::setw(20) << "HeapSort"
                  << std::setw(16) << std::fixed << std::setprecision(2) << ms
                  << std::setw(20) << (mem / 1024)
                  << std::setw(10) << (ok ? "SI" : "NO")
                  << "\n";
    }

    // ══ 3. ÁRBOL AVL ══════════════════════════
    {
        size_t mem = estimateAVLMemory(base);

        auto t1 = std::chrono::high_resolution_clock::now();
        AVLNode* root = nullptr;
        // Insertamos las 100,000 palabras en el árbol con auto-balanceo
        for (const auto& w : base) root = avlInsert(root, w);
        std::vector<std::string> sorted;
        sorted.reserve(base.size());
        // Recorrido inorden para obtener las palabras ordenadas
        avlInorder(root, sorted);
        auto t2 = std::chrono::high_resolution_clock::now();

        double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        bool ok   = isSorted(sorted);
        avlFree(root); // Liberamos la memoria del árbol

        std::cout << std::left
                  << std::setw(20) << "Arbol AVL"
                  << std::setw(16) << std::fixed << std::setprecision(2) << ms
                  << std::setw(20) << (mem / 1024)
                  << std::setw(10) << (ok ? "SI" : "NO")
                  << "\n";
    }

    // ── Análisis comparativo ──────────────────
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║              ANÁLISIS COMPARATIVO                   ║\n";
    std::cout << "╠══════════════════════════════════════════════════════╣\n";
    std::cout << "║ Algoritmo │ Mejor   │ Promedio │ Peor     │ Espacio  ║\n";
    std::cout << "╠═══════════╪═════════╪══════════╪══════════╪══════════╣\n";
    std::cout << "║ QuickSort │ O(nlogn)│ O(nlogn) │ O(n²)    │ O(logn)  ║\n";
    std::cout << "║ HeapSort  │ O(nlogn)│ O(nlogn) │ O(nlogn) │ O(1)     ║\n";
    std::cout << "║ Arbol AVL │ O(nlogn)│ O(nlogn) │ O(nlogn) │ O(n)     ║\n";
    std::cout << "╚═══════════╧═════════╧══════════╧══════════╧══════════╝\n";

    std::cout << "\nNotas:\n";
    std::cout << "  - QuickSort usa pivote por mediana de tres (pila iterativa) para evitar\n";
    std::cout << "    el peor caso O(n²) en datos completamente ordenados.\n";
    std::cout << "  - HeapSort garantiza O(n log n) pero tiene mala localidad de caché\n";
    std::cout << "    debido a sus patrones de acceso no secuenciales a memoria.\n";
    std::cout << "  - El tiempo del Árbol AVL incluye inserción (con rotaciones) y recorrido\n";
    std::cout << "    inorden. Mayor constante por overhead de punteros y asignaciones dinámicas.\n";
    std::cout << "  - QuickSort y HeapSort reutilizan el mismo vector in-place;\n";
    std::cout << "    el AVL asigna un nodo (~56 bytes) por cada palabra única.\n";

    return 0;
}
