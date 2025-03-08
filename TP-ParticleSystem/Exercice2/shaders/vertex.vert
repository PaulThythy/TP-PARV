#version 450

struct Particle {
  vec3 position;
  vec3 previousPosition;
  vec3 velocity;
  float weight;
  int isFixed;
};

layout(std430, binding = 0) buffer ParticlesBuffer {
    Particle particles[];
};

uniform mat4 MVP;
uniform mat4 MODEL;

void main(){
    Particle p = particles[gl_VertexID];
    gl_Position = MVP * vec4(p.position, 1.0);
}