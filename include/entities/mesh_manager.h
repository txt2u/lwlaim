#include <entities/mesh.h>

typedef struct {
    char* name;       // Unique name for the mesh
    Mesh* mesh;        // Pointer to the mesh data
} MeshEntry;

typedef struct {
    size_t count;
    MeshEntry* entries;
} MeshManager;