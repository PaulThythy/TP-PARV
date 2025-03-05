#version 450

struct Particule {
    vec3 position;
    vec3 vitesse;
    float masse;
    float radius;
    vec3 couleur;
};

layout(std430, binding = 0) buffer ParticlesBuffer {
    Particule particles[];
};

uniform mat4 MVP;
uniform mat4 MODEL;

out vec3 vPosition;
out vec3 vVitesse;
out vec3 vCouleur;
out float vRadius;

void main(){
    Particule p = particles[gl_VertexID];
    vPosition = p.position;
    vVitesse = p.vitesse;
    vCouleur = p.couleur;
    vRadius = p.radius;
    gl_Position = MVP * vec4(p.position, 1.0);
}