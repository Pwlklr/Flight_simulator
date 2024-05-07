#version 330

//Zmienne jednorodne
uniform mat4 P;     // Projekcja
uniform mat4 V;     // Widok
uniform mat4 M;     // Model
uniform vec4 lp;    // Pozycja œwiat³a w przestrzeni œwiata


//Atrybuty
in vec4 vertex; //wspolrzedne wierzcholka w przestrzeni modelu
in vec4 color; //kolor zwi¹zany z wierzcho³kiem
in vec4 normal; //wektor normalny w przestrzeni modelu
in vec2 texCoord0;

//Zmienne interpolowane
out vec4 ic;
out vec4 l;
out vec4 n;
out vec4 v;
out vec2 iTexCoord0; 
out vec2 iTexCoord1;

void main(void) {
    l = normalize(V * lp - V*M*vertex); //wektor do œwiat³a w przestrzeni oka
    v = normalize(vec4(0, 0, 0, 1) - V * M * vertex); //wektor do obserwatora w przestrzeni oka
    n = normalize(V * M * normal); //wektor normalny w przestrzeni oka
    
    iTexCoord0 = texCoord0;
    iTexCoord1 = (n.xy + 1) / 2;

    ic = color;
    
    gl_Position=P*V*M*vertex;
}