g++ -std=c++17 -O2 -Iinclude \
    src/main.cpp \
    src/graph_utils.cpp \
    src/kruskal.cpp \
    src/prim.cpp \
    src/boruvka.cpp \
    -o mst_benchmark
