#version 450

in vec2 gs_texCoord;
in vec3 gs_normal;
in vec3 vCouleur;

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
    finalColor = vec4(vCouleur, 1.0);
}