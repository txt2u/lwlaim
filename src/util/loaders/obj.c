#include <loaders/obj.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_MTL_NAME_LENGTH 128

// Helper function to skip whitespace in a file
static void skip_whitespace(FILE *file) {
    char ch;
    while ((ch = fgetc(file)) != EOF && isspace(ch));
    if (ch != EOF) ungetc(ch, file);
}

// Function to extract the directory from a file path
void extract_directory(const char *path, char *dir) {
    const char *last_slash = strrchr(path, '/');
    if (last_slash) {
        size_t len = last_slash - path;
        strncpy(dir, path, len);
        dir[len] = '\0';
    } else {
        dir[0] = '\0';  // No directory part, return empty string
    }
}

// Function to load the MTL file
void load_mtl(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open MTL file: %s\n", filename);
        return;
    }

    char line[256];
    char current_material[MAX_MTL_NAME_LENGTH] = "";

    while (fgets(line, sizeof(line), file)) {
        skip_whitespace(file);

        // New material definition (newmtl)
        if (strncmp(line, "newmtl ", 7) == 0) {
            sscanf(line + 7, "%s", current_material);
            printf("Found material: %s\n", current_material);
        }
        // Material properties (ambient, diffuse, etc.)
        else if (strncmp(line, "Ka ", 3) == 0) {
            // Ambient color
            float r, g, b;
            sscanf(line + 3, "%f %f %f", &r, &g, &b);
            printf("Ambient color for %s: (%f, %f, %f)\n", current_material, r, g, b);
        }
        else if (strncmp(line, "Kd ", 3) == 0) {
            // Diffuse color
            float r, g, b;
            sscanf(line + 3, "%f %f %f", &r, &g, &b);
            printf("Diffuse color for %s: (%f, %f, %f)\n", current_material, r, g, b);
        }
        else if (strncmp(line, "Ks ", 3) == 0) {
            // Specular color
            float r, g, b;
            sscanf(line + 3, "%f %f %f", &r, &g, &b);
            printf("Specular color for %s: (%f, %f, %f)\n", current_material, r, g, b);
        }
        // Texture files (for now, we print them)
        else if (strncmp(line, "map_Kd ", 7) == 0) {
            char texture_filename[256];
            sscanf(line + 7, "%s", texture_filename);
            printf("Diffuse texture for %s: %s\n", current_material, texture_filename);
        }
    }

    fclose(file);
}

// Function to load the OBJ file
int load_obj(const char *filename, StaticModel *model) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return 0;
    }

    // Initialize model data
    model->vertices = NULL;
    model->normals = NULL;
    model->texcoords = NULL;
    model->indices = NULL;
    model->vertex_count = 0;
    model->normal_count = 0;
    model->texcoord_count = 0;
    model->index_count = 0;

    char line[256];
    char mtl_filename[256] = "";
    char dir[256] = "";

    while (fgets(line, sizeof(line), file)) {
        skip_whitespace(file);

        // Check for material file (mtllib)
        if (strncmp(line, "mtllib ", 7) == 0) {
            sscanf(line + 7, "%s", mtl_filename);
            extract_directory(filename, dir);
            strcat(dir, "/");
            strcat(dir, mtl_filename);
            printf("Found MTL file: %s\n", dir);
            load_mtl(dir);  // Load the material file
        }

        // Vertex position
        else if (strncmp(line, "v ", 2) == 0) {
            model->vertex_count++;
            model->vertices = realloc(model->vertices, model->vertex_count * 3 * sizeof(float));
            if (!model->vertices) {
                fclose(file);
                return 0;  // Memory allocation failure
            }
            sscanf(line + 2, "%f %f %f", 
                    &model->vertices[(model->vertex_count - 1) * 3], 
                    &model->vertices[(model->vertex_count - 1) * 3 + 1], 
                    &model->vertices[(model->vertex_count - 1) * 3 + 2]);
        }
        // Normal vector
        else if (strncmp(line, "vn ", 3) == 0) {
            model->normal_count++;
            model->normals = realloc(model->normals, model->normal_count * 3 * sizeof(float));
            if (!model->normals) {
                fclose(file);
                return 0;  // Memory allocation failure
            }
            sscanf(line + 3, "%f %f %f", 
                    &model->normals[(model->normal_count - 1) * 3],
                    &model->normals[(model->normal_count - 1) * 3 + 1], 
                    &model->normals[(model->normal_count - 1) * 3 + 2]);
        }
        // Texture coordinates
        else if (strncmp(line, "vt ", 3) == 0) {
            model->texcoord_count++;
            model->texcoords = realloc(model->texcoords, model->texcoord_count * 2 * sizeof(float));
            if (!model->texcoords) {
                fclose(file);
                return 0;  // Memory allocation failure
            }
            sscanf(line + 3, "%f %f", 
                    &model->texcoords[(model->texcoord_count - 1) * 2],
                    &model->texcoords[(model->texcoord_count - 1) * 2 + 1]);
        }
        // Face data (indices)
        else if (strncmp(line, "f ", 2) == 0) {
            model->index_count += 3; // Assuming triangle faces for simplicity
            model->indices = realloc(model->indices, model->index_count * sizeof(unsigned int));
            if (!model->indices) {
                fclose(file);
                return 0;  // Memory allocation failure
            }

            unsigned int v[3], t[3], n[3];
            // Extract indices from face line (up to 3 vertices per face)
            if (sscanf(line + 2, "%u/%u/%u %u/%u/%u %u/%u/%u", 
                &v[0], &t[0], &n[0], &v[1], &t[1], &n[1], &v[2], &t[2], &n[2]) == 9) {
                for (int i = 0; i < 3; i++) {
                    model->indices[(model->index_count - 3) + i] = v[i] - 1; // OBJ indices are 1-based
                }
            }
        }
    }

    fclose(file);
    return 1;
}

// Free model data
void free_static_model(StaticModel *model) {
    free(model->vertices);
    free(model->normals);
    free(model->texcoords);
    free(model->indices);
    model->vertices = NULL;
    model->normals = NULL;
    model->texcoords = NULL;
    model->indices = NULL;
    model->vertex_count = 0;
    model->normal_count = 0;
    model->texcoord_count = 0;
    model->index_count = 0;
}