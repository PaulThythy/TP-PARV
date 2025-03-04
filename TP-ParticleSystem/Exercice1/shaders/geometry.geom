#version 450

layout(points) in;
layout(triangle_strip, max_vertices = 146) out;

uniform mat4 MVP;
uniform float pointSize; // définit la moitié de la taille du quad en espace NDC ou en world (selon votre configuration)

in vec4 worldPos[];
in vec3 vCouleur[];
in float vRadius[];

out vec2 gs_texCoord; 
out vec3 gs_normal;
out vec3 gs_couleur;

int latitudes = 5;
int longitudes = 5;

void main() {
    // Utilise la position monde pour le centre
    vec4 center = worldPos[0];
    float radius = vRadius[0];

    for (int i = 0; i < latitudes; i++) {
        float theta1 = float(i) / float(latitudes) * 3.1415926;
        float theta2 = float(i+1) / float(latitudes) * 3.1415926;
        for (int j = 0; j <= longitudes; j++) {
            float phi = float(j) / float(longitudes) * 2.0 * 3.1415926;
            
            // Coordonnées du premier point sur la bande
            vec3 v1;
            v1.x = radius * sin(theta1) * cos(phi);
            v1.y = radius * sin(theta1) * sin(phi);
            v1.z = radius * cos(theta1);
            
            // Coordonnées du deuxième point sur la bande
            vec3 v2;
            v2.x = radius * sin(theta2) * cos(phi);
            v2.y = radius * sin(theta2) * sin(phi);
            v2.z = radius * cos(theta2);
            
            // Normales pour un éclairage simple
            vec3 n1 = normalize(v1);
            vec3 n2 = normalize(v2);
            
            // Coordonnées de texture (optionnelles)
            vec2 t1 = vec2(phi / (2.0 * 3.1415926), theta1 / 3.1415926);
            vec2 t2 = vec2(phi / (2.0 * 3.1415926), theta2 / 3.1415926);
            
            // Émission des deux sommets de la bande
            gl_Position = MVP * (center + vec4(v1, 0.0));
            gs_texCoord = t1;
            gs_normal = n1;
            gs_couleur = vCouleur[0];
            EmitVertex();
            
            gl_Position = MVP * (center + vec4(v2, 0.0));
            gs_texCoord = t2;
            gs_normal = n2;
            gs_couleur = vCouleur[0];
            EmitVertex();
        }
        EndPrimitive();
    }
}