#include <loaders/obj.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cglm/cglm.h>

int load_obj(const char *filename, StaticMesh *mesh) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Could not open file: %s\n", filename);
        return -1;
    }

    size_t positions_size = 0;
    size_t normals_size = 0;
    size_t texcoords_size = 0;
    size_t indices_size = 0;

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            positions_size++;
        } else if (strncmp(line, "vn ", 3) == 0) {
            normals_size++;
        } else if (strncmp(line, "vt ", 3) == 0) {
            texcoords_size++;
        } else if (strncmp(line, "f ", 2) == 0) {
            // We don't know the number of vertices per face, so count them dynamically
            int vertex_count = 0;
            char *token = strtok(line + 2, " ");
            while (token != NULL) {
                vertex_count++;
                token = strtok(NULL, " ");
            }
            indices_size += vertex_count-1; // Triangulate the face
        }
    }

    // Allocate memory for data
    mesh->positions = malloc(sizeof(float) * 3 * positions_size); // 3 floats per vertex
    mesh->normals = malloc(sizeof(float) * 3 * normals_size);     // 3 floats per normal
    mesh->texcoords = malloc(sizeof(float) * 2 * texcoords_size); // 2 floats per texture coordinate
    mesh->indices = malloc(sizeof(unsigned int) * indices_size);  // Indices for faces

    mesh->vertex_count = positions_size;
    mesh->index_count = indices_size;
    mesh->texcoords_count = texcoords_size;
    mesh->normals_count = normals_size;

    // Reset file pointer and parse the data
    rewind(file);

    size_t position_index = 0;
    size_t normal_index = 0;
    size_t texcoord_index = 0;
    size_t index_index = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            // Parse position
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            mesh->positions[position_index * 3] = x;
            mesh->positions[position_index * 3 + 1] = y;
            mesh->positions[position_index * 3 + 2] = z;
            position_index++;
        } else if (strncmp(line, "vn ", 3) == 0) {
            // Parse normal
            float nx, ny, nz;
            sscanf(line, "vn %f %f %f", &nx, &ny, &nz);
            mesh->normals[normal_index * 3] = nx;
            mesh->normals[normal_index * 3 + 1] = ny;
            mesh->normals[normal_index * 3 + 2] = nz;
            normal_index++;
        } else if (strncmp(line, "vt ", 3) == 0) {
            // Parse texture coordinate
            float u, v;
            sscanf(line, "vt %f %f", &u, &v);
            mesh->texcoords[texcoord_index * 2] = u;
            mesh->texcoords[texcoord_index * 2 + 1] = v;
            texcoord_index++;
        } else if (strncmp(line, "f ", 2) == 0) {
            // Parse face (indices)
            unsigned int p_idx[10], t_idx[10], n_idx[10];  // Max 10 vertices per face
            int vertex_count = 0;
            char *token = strtok(line + 2, " ");
            while (token != NULL) {
                int matched = sscanf(token, "%u/%u/%u", &p_idx[vertex_count], &t_idx[vertex_count], &n_idx[vertex_count]);
                if (matched == 3) {
                    vertex_count++;
                }
                token = strtok(NULL, " ");
            }

            // Triangulate the face (if necessary)
            for (int i = 1; i < vertex_count - 1; i++) {
                // Indices
                // mesh->indices[index_index++] = p_idx[0] - 1; // 1-based to 0-based index
                // mesh->indices[index_index++] = p_idx[i] - 1;
                // mesh->indices[index_index++] = p_idx[i + 1] - 1;

                // // Texture coordinates
                // mesh->texcoords[index_index - 3] = mesh->texcoords[(t_idx[0] - 1) * 2];
                // mesh->texcoords[index_index - 2] = mesh->texcoords[(t_idx[i] - 1) * 2];
                // mesh->texcoords[index_index - 1] = mesh->texcoords[(t_idx[i + 1] - 1) * 2];

                // // Normals
                // mesh->normals[index_index - 3] = mesh->normals[(n_idx[0] - 1) * 3];
                // mesh->normals[index_index - 2] = mesh->normals[(n_idx[i] - 1) * 3];
                // mesh->normals[index_index - 1] = mesh->normals[(n_idx[i + 1] - 1) * 3];
            }
        }
    }

    fclose(file);
    return 1;
}