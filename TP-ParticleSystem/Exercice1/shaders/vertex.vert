#version 450


uniform mat4 MVP;
uniform mat4 MODEL;
out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;
out vec3 vVitesse;
out vec3 vCouleur;
out float vRadius;
out vec4 worldPos;

layout(location = 0) in vec3 position; // le location permet de dire de quel flux/canal on récupère les données (doit être en accord avec le location du code opengl)
layout(location = 3) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 4) in vec3 vitesse;
layout(location = 5) in vec3 couleur;
layout(location = 6) in float radius;

void main(){
    gl_Position = MVP * vec4(position,1.);	
    worldPos = MODEL * vec4(position, 1.0);
    gl_PointSize = 20.0; // définie la taille des points mais il faut activer la fonctionnalité dans opengl par glEnable( GL_PROGRAM_POINT_SIZE );
    vPosition = position;
    vNormal = normal;
    vTexCoord = texCoord;
    vVitesse = vitesse;
    vCouleur = couleur;
    vRadius = radius;
}


