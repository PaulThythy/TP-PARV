#version 450

uniform mat4 MODEL;

uniform struct Light {
  vec3 position;
  vec3 intensities;
  float ambientCoefficient;
  float attenuation;
} light;

out vec4 finalColor;

void main() {
    // Simple affichage en utilisant la couleur transmise
    finalColor = vec4(0.8, 0.2, 0.2, 1.0);
}