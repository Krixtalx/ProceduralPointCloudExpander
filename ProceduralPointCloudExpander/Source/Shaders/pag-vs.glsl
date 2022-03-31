#version 410
layout (location = 0) in vec3 posicion;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangente;
layout (location = 3) in vec3 bitangente;
layout (location = 4) in vec2 textura;

uniform mat4 matrizMVP;
uniform mat4 matrizMV;
uniform mat4 matrizMS;

out salidaVS
{
    vec3 posicionV;
    vec3 normalV;
    vec3 posicionTg;
    mat3 matrizTBN;
    vec2 texturaV;
    vec4 coordenadasSombra;
} salida;


void main ()
{
    mat4 matrizMVit = transpose(inverse(matrizMV));
    gl_Position = matrizMVP * vec4 (posicion, 1);
    salida.posicionV = vec3(matrizMV * vec4(posicion, 1));

    salida.normalV = normalize (vec3(matrizMVit * vec4(normal, 0)));
    vec3 tangenteMV = normalize (vec3 (matrizMVit * vec4 (tangente, 0)));
    vec3 bitangenteMV = normalize (vec3 (matrizMVit * vec4 (bitangente, 0)));
    salida.matrizTBN = transpose (mat3 (tangenteMV, bitangenteMV, salida.normalV));
    salida.posicionTg = salida.matrizTBN * vec3 (matrizMV * vec4 (posicion, 1));

    salida.texturaV = textura;

    salida.coordenadasSombra = matrizMS * vec4 (posicion, 1);
}