#version 450

struct Particule {
    vec3 position;
    vec3 velocity;
    vec3 color;
    float weight;
    float radius;
    float restitution;
};

layout(std430, binding = 0) buffer ParticlesBuffer {
    Particule particles[];
};

uniform mat4 MVP;
uniform mat4 MODEL;

out vec3 vPosition;
out vec3 vVelocity;
out vec3 vColor;
out float vRadius;

void main(){
    Particule p = particles[gl_VertexID];
    vPosition = p.position;
    vVelocity = p.velocity;
    vColor = p.color;
    vRadius = p.radius;
    gl_Position = MVP * vec4(p.position, 1.0);
}