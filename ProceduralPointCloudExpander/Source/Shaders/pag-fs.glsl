#version 410
layout(location = 0) out vec4 colorFragmento;
in salidaVS
{
    vec3 posicionV;
    vec3 normalV;
    vec3 posicionTg;
    mat3 matrizTBN;
    vec2 texturaV;
    vec4 coordenadasSombra;
} entrada;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform uint phong;

uniform vec3 Ia;
uniform vec3 Id;
uniform vec3 Is;
uniform float spotAngle;
uniform uint expBordes;

uniform vec3 posLuz;
uniform vec3 dirLuz;

uniform sampler2D muestreador;
uniform sampler2D muestreadorNormalMap;
uniform sampler2DShadow muestreadorSombra;

vec3 usableKa;
vec3 usableKd;
vec3 usablePosLuz;
vec3 usableDirLuz;
vec3 usablePos;
vec3 usableNormal;

subroutine void colorMoT();
subroutine uniform colorMoT colorElegido;

subroutine void usarNormalMap();
subroutine uniform usarNormalMap normalMap;

subroutine vec3 calcularLuz();
subroutine uniform calcularLuz luzElegida;

float atenuacionDistancia(){
    float distancia = distance(usablePosLuz, usablePos);
    float distanciaCuadrado = pow(distancia, 2);
    float factor = min(1/(0.01f + 0.1f * distancia + 0.2f * distanciaCuadrado), 1);
    return factor;
}

//----- Subrutinas color material o textura --------
subroutine (colorMoT)
void colorTextura(){
    usableKa = texture(muestreador, entrada.texturaV).rgb;
    usableKd = usableKa;
}

subroutine (colorMoT)
void colorMaterial(){
    usableKa = Ka;
    usableKd = Kd;
}

//----- Subrutinas normalMap --------
subroutine(usarNormalMap)
void noUsarNormalMap(){
    usableDirLuz = dirLuz;
    usablePosLuz = posLuz;
    usableNormal = entrada.normalV;
    usablePos = entrada.posicionV;
}

subroutine(usarNormalMap)
void siUsarNormalMap(){
    usableDirLuz = entrada.matrizTBN * dirLuz;
    usablePosLuz = entrada.matrizTBN * posLuz;
    usableNormal = (2 * (texture(muestreadorNormalMap, entrada.texturaV) - 0.5)).rgb;
    usablePos = entrada.posicionTg;
}


//----- Subrutinas luces --------
subroutine (calcularLuz)
vec3 colorDefecto ()
{
    return usableKa;
}

subroutine (calcularLuz)
vec3 luzAmbiente()
{
    return usableKa*Ia;
}

subroutine (calcularLuz)
vec3 luzPuntual()
{
    vec3 n = normalize(usableNormal);

    vec3 l = normalize(usablePosLuz-usablePos);
    vec3 v = normalize(-usablePos);
    vec3 r = reflect(-l, n);

    vec3 difusa = (Id * usableKd * max(dot(l, n), 0.0));
    vec3 especular = (Is * Ks * pow(max(dot(r, v), 0.0), phong));

    return atenuacionDistancia() * (difusa + especular);
}

subroutine (calcularLuz)
vec3 luzDireccional ()
{
    vec3 n = normalize(usableNormal);

    vec3 l = -usableDirLuz;
    vec3 v = normalize(-usablePos);
    vec3 r = reflect(-l, n);

    vec3 difusa = (Id * usableKd * max(dot(l, n), 0.0));
    vec3 especular = (Is * Ks * pow(max(dot(r, v), 0.0), phong));

    float sombra = textureProj(muestreadorSombra, entrada.coordenadasSombra);

    return sombra * (difusa + especular);
}

subroutine (calcularLuz)
vec3 luzFoco ()
{
    vec3 l = normalize(usablePosLuz-usablePos);
    vec3 d = usableDirLuz;
    float cosGamma = cos(spotAngle);
    float cosDelta = dot(-l, d);
    float spotFactor = pow(cosDelta, expBordes);
    if (cosDelta < cosGamma) { spotFactor = 0.0; }
    vec3 n = normalize(usableNormal);
    vec3 v = normalize(-usablePos);
    vec3 r = reflect(-l, n);

    vec3 difusa = (Id * usableKd * max(dot(l, n), 0.0));
    vec3 especular = (Is * Ks * pow(max(dot(r, v), 0.0), phong));

    float sombra = textureProj(muestreadorSombra, entrada.coordenadasSombra);

    return atenuacionDistancia() * sombra * spotFactor * (difusa + especular);
}

void main ()
{
    colorElegido();
    normalMap();
    colorFragmento = vec4(luzElegida(), 1);
}