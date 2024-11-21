import sys

def parse_obj(file_path):
    vertices = []
    texcoords = []
    normals = []
    faces = []

    with open(file_path, 'r') as file:
        for line in file:
            parts = line.strip().split()
            if not parts:
                continue
            if parts[0] == 'v':  # Vertex
                vertices.append([float(p) for p in parts[1:]])
            elif parts[0] == 'vt':  # Texture coordinate
                texcoords.append([float(p) for p in parts[1:]])
            elif parts[0] == 'vn':  # Normal
                normals.append([float(p) for p in parts[1:]])
            elif parts[0] == 'f':  # Face
                face = [part.split('/') for part in parts[1:]]
                faces.append(face)

    return vertices, texcoords, normals, faces

# Analyze an OBJ file

# Ensure that a file path is passed as an argument
if len(sys.argv) != 2:
    print("Usage: python analyze_obj.py <path_to_model>")
    sys.exit(1)

# The file path is passed as the second argument
file_path = sys.argv[1]
vertices, texcoords, normals, faces = parse_obj(file_path)

print(f"Vertex Count: {len(vertices)}")
print(f"Texture Coordinate Count: {len(texcoords)}")
print(f"Normal Count: {len(normals)}")
print(f"Face Count: {len(faces)}")

# Optionally print the first few entries for inspection
print("First 5 Vertices:", vertices[:5])
print("First 5 TexCoords:", texcoords[:5])
print("First 5 Normals:", normals[:5])