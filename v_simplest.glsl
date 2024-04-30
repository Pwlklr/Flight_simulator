#version 330

//Zmienne jednorodne
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec4 lp;    //wspó³rzêdne œwiat³a w przestrzeni œwiata

//Atrybuty
in vec4 vertex; //wspolrzedne wierzcholka w przestrzeni modelu
in vec4 color;
in vec4 normal;
out vec4 iC;
out vec4 n;
out vec4 v;
out vec4 l;



void main(void) {
    l = normalize(V*lp - V*M*vertex);
    n = normalize(V*M*normal);
    v = normalize(vec4(0,0,0,1)-V*M*vertex);

    iC = color;
    gl_Position=P*V*M*vertex;
}




//do kolorowania dystansowego
    /*
    d = distance(V*M*vertex, vec4(0,0,0,1));
    d = 1 - (d-3.3) / 3.4;
    iC = vec4(color.rgb * d, color.a);
    gl_Position=P*V*M*vertex;
    */