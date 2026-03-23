/**
 * ST0245 - DATA STRUCTURES AND ALGORITHMS
 * Practice I – Sorting Large Dataset
 * EAFIT University
 *
 * Implements: QuickSort, HeapSort, AVL Tree
 * with performance analysis (time + memory).
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>   // only for std::shuffle (dataset randomization, not sorting)
#include <random>
#include <iomanip>
#include <cstring>

// ─────────────────────────────────────────────
// UTILITY: load dataset
// ─────────────────────────────────────────────
std::vector<std::string> loadDataset(const std::string& filename) {
    std::vector<std::string> words;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open " << filename << "\n";
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
    // Median-of-three pivot to reduce worst-case risk
    int mid = low + (high - low) / 2;
    if (arr[mid] < arr[low])  std::swap(arr[low], arr[mid]);
    if (arr[high] < arr[low]) std::swap(arr[low], arr[high]);
    if (arr[mid] < arr[high]) std::swap(arr[mid], arr[high]);
    // arr[high] is now the median pivot
    const std::string pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (arr[j] <= pivot) {
            ++i;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return i + 1;
}

void quickSort(std::vector<std::string>& arr, int low, int high) {
    // Iterative QuickSort using explicit stack (avoids stack overflow on 100k items)
    std::vector<std::pair<int,int>> stack;
    stack.push_back({low, high});
    while (!stack.empty()) {
        auto [l, h] = stack.back();
        stack.pop_back();
        if (l < h) {
            int pi = partition(arr, l, h);
            stack.push_back({l, pi - 1});
            stack.push_back({pi + 1, h});
        }
    }
}

// ─────────────────────────────────────────────
// 2. HEAPSORT (Binary Max-Heap)
// ─────────────────────────────────────────────
void heapify(std::vector<std::string>& arr, int n, int i) {
    int largest = i;
    int left    = 2 * i + 1;
    int right   = 2 * i + 2;
    if (left  < n && arr[left]  > arr[largest]) largest = left;
    if (right < n && arr[right] > arr[largest]) largest = right;
    if (largest != i) {
        std::swap(arr[i], arr[largest]);
        heapify(arr, n, largest);
    }
}

void heapSort(std::vector<std::string>& arr) {
    int n = (int)arr.size();
    // Build max-heap
    for (int i = n / 2 - 1; i >= 0; --i)
        heapify(arr, n, i);
    // Extract elements one by one
    for (int i = n - 1; i > 0; --i) {
        std::swap(arr[0], arr[i]);
        heapify(arr, i, 0);
    }
}

// ─────────────────────────────────────────────
// 3. AVL TREE
// ─────────────────────────────────────────────
struct AVLNode {
    std::string key;
    AVLNode*    left  = nullptr;
    AVLNode*    right = nullptr;
    int         height = 1;
    explicit AVLNode(const std::string& k) : key(k) {}
};

int avlHeight(AVLNode* n) { return n ? n->height : 0; }

int avlBalance(AVLNode* n) {
    return n ? avlHeight(n->left) - avlHeight(n->right) : 0;
}

void avlUpdateHeight(AVLNode* n) {
    if (n) n->height = 1 + std::max(avlHeight(n->left), avlHeight(n->right));
}

AVLNode* avlRotateRight(AVLNode* y) {
    AVLNode* x  = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left  = T2;
    avlUpdateHeight(y);
    avlUpdateHeight(x);
    return x;
}

AVLNode* avlRotateLeft(AVLNode* x) {
    AVLNode* y  = x->right;
    AVLNode* T2 = y->left;
    y->left  = x;
    x->right = T2;
    avlUpdateHeight(x);
    avlUpdateHeight(y);
    return y;
}

AVLNode* avlInsert(AVLNode* node, const std::string& key) {
    if (!node) return new AVLNode(key);
    if (key < node->key)      node->left  = avlInsert(node->left,  key);
    else if (key > node->key) node->right = avlInsert(node->right, key);
    else                      return node; // duplicates ignored

    avlUpdateHeight(node);
    int bf = avlBalance(node);

    // Left-Left
    if (bf > 1 && key < node->left->key)
        return avlRotateRight(node);
    // Right-Right
    if (bf < -1 && key > node->right->key)
        return avlRotateLeft(node);
    // Left-Right
    if (bf > 1 && key > node->left->key) {
        node->left = avlRotateLeft(node->left);
        return avlRotateRight(node);
    }
    // Right-Left
    if (bf < -1 && key < node->right->key) {
        node->right = avlRotateRight(node->right);
        return avlRotateLeft(node);
    }
    return node;
}

// Iterative inorder traversal (avoids deep recursion on 100k nodes)
void avlInorder(AVLNode* root, std::vector<std::string>& result) {
    std::vector<AVLNode*> stack;
    AVLNode* curr = root;
    while (curr || !stack.empty()) {
        while (curr) { stack.push_back(curr); curr = curr->left; }
        curr = stack.back(); stack.pop_back();
        result.push_back(curr->key);
        curr = curr->right;
    }
}

void avlFree(AVLNode* node) {
    if (!node) return;
    avlFree(node->left);
    avlFree(node->right);
    delete node;
}

// ─────────────────────────────────────────────
// MEMORY ESTIMATION
// ─────────────────────────────────────────────
size_t estimateVectorMemory(const std::vector<std::string>& v) {
    size_t total = sizeof(v);
    for (const auto& s : v) total += sizeof(std::string) + s.capacity();
    return total;
}

size_t estimateAVLMemory(const std::vector<std::string>& words) {
    size_t nodeSize = sizeof(AVLNode);
    size_t total = 0;
    for (const auto& s : words) total += nodeSize + s.size() + 1;
    return total;
}

// ─────────────────────────────────────────────
// VERIFY sort correctness (sample check)
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
    if (argc > 1) datasetFile = argv[1];

    std::cout << "╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║   ST0245 – Practice I: Sorting Large Dataset (C++)   ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n\n";

    // ── Load ──────────────────────────────────
    std::cout << "Loading dataset from: " << datasetFile << " ...\n";
    std::vector<std::string> base = loadDataset(datasetFile);
    if (base.empty()) { std::cerr << "Dataset empty or not found.\n"; return 1; }
    std::cout << "Words loaded: " << base.size() << "\n\n";

    // ── Results table header ──────────────────
    std::cout << std::left
              << std::setw(20) << "Algorithm"
              << std::setw(16) << "Time (ms)"
              << std::setw(20) << "Memory (KB)"
              << std::setw(10) << "Sorted?"
              << "\n";
    std::cout << std::string(66, '-') << "\n";

    // ══ 1. QUICKSORT ══════════════════════════
    {
        std::vector<std::string> arr = base;
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
                  << std::setw(10) << (ok ? "YES" : "NO")
                  << "\n";
    }

    // ══ 2. HEAPSORT ═══════════════════════════
    {
        std::vector<std::string> arr = base;
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
                  << std::setw(10) << (ok ? "YES" : "NO")
                  << "\n";
    }

    // ══ 3. AVL TREE ═══════════════════════════
    {
        size_t mem = estimateAVLMemory(base);

        auto t1 = std::chrono::high_resolution_clock::now();
        AVLNode* root = nullptr;
        for (const auto& w : base) root = avlInsert(root, w);
        std::vector<std::string> sorted;
        sorted.reserve(base.size());
        avlInorder(root, sorted);
        auto t2 = std::chrono::high_resolution_clock::now();

        double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        bool ok   = isSorted(sorted);
        avlFree(root);

        std::cout << std::left
                  << std::setw(20) << "AVL Tree"
                  << std::setw(16) << std::fixed << std::setprecision(2) << ms
                  << std::setw(20) << (mem / 1024)
                  << std::setw(10) << (ok ? "YES" : "NO")
                  << "\n";
    }

    // ── Comparative Analysis ──────────────────
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║               COMPARATIVE ANALYSIS                  ║\n";
    std::cout << "╠══════════════════════════════════════════════════════╣\n";
    std::cout << "║ Algorithm │ Best    │ Avg      │ Worst    │ Space    ║\n";
    std::cout << "╠═══════════╪═════════╪══════════╪══════════╪══════════╣\n";
    std::cout << "║ QuickSort │ O(nlogn)│ O(nlogn) │ O(n²)    │ O(logn)  ║\n";
    std::cout << "║ HeapSort  │ O(nlogn)│ O(nlogn) │ O(nlogn) │ O(1)     ║\n";
    std::cout << "║ AVL Tree  │ O(nlogn)│ O(nlogn) │ O(nlogn) │ O(n)     ║\n";
    std::cout << "╚═══════════╧═════════╧══════════╧══════════╧══════════╝\n";

    std::cout << "\nNotes:\n";
    std::cout << "  - QuickSort uses median-of-three pivot (iterative stack) to avoid\n";
    std::cout << "    worst-case O(n²) on already-sorted or nearly-sorted data.\n";
    std::cout << "  - HeapSort guarantees O(n log n) but has poor cache locality\n";
    std::cout << "    due to non-sequential memory access patterns.\n";
    std::cout << "  - AVL Tree time includes both insertion (w/ rotations) and inorder\n";
    std::cout << "    traversal. Higher constant factor due to pointer overhead and\n";
    std::cout << "    heap allocations per node.\n";
    std::cout << "  - Memory: vector-based sorts reuse the same array in-place;\n";
    std::cout << "    AVL allocates one node (~56 bytes) per unique word.\n";

    return 0;
}
