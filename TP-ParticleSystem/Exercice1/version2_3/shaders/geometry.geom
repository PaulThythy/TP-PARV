#version 450

layout(points) in;
layout(triangle_strip, max_vertices = 146) out;

uniform mat4 MVP;
uniform mat4 VIEW;
uniform mat4 PROJECTION;
uniform mat4 MODEL;

float PI = 3.14159265;

in vec3 vPosition[];
in vec3 vColor[];
in float vRadius[];

out vec2 gs_texCoord; 
out vec3 gs_normal;
out vec3 gs_color;

int latitudes = 5;
int longitudes = 5;

void main() {
    vec3 centerLocal = vPosition[0];
    float radius = vRadius[0];

    for (int i = 0; i < latitudes; i++) {
        float theta1 = float(i) / float(latitudes) * PI;
        float theta2 = float(i+1) / float(latitudes) * PI;
        for (int j = 0; j <= longitudes; j++) {
            float phi = float(j) / float(longitudes) * 2.0 * PI;
            vec3 v1;
            v1.x = radius * sin(theta1) * cos(phi);
            v1.y = radius * sin(theta1) * sin(phi);
            v1.z = radius * cos(theta1);
            vec3 v2;
            v2.x = radius * sin(theta2) * cos(phi);
            v2.y = radius * sin(theta2) * sin(phi);
            v2.z = radius * cos(theta2);
            vec3 n1 = normalize(v1);
            vec3 n2 = normalize(v2);
            vec2 t1 = vec2(phi / (2.0 * PI), theta1 / PI);
            vec2 t2 = vec2(phi / (2.0 * PI), theta2 / PI);
            
            gl_Position = MVP * vec4(centerLocal + v1, 1.0);
            gs_texCoord = t1;
            gs_normal = n1;
            gs_color = vColor[0];
            EmitVertex();
            
            gl_Position = MVP * vec4(centerLocal + v2, 1.0);
            gs_texCoord = t2;
            gs_normal = n2;
            gs_color = vColor[0];
            EmitVertex();
        }
        EndPrimitive();
    }
}