#version 330


out vec4 pixelColor; //Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela
in vec4 iC;
in vec4 n;
in vec4 v;
in vec4 l;


float toonify(float nl, float ilosc_przedzialow) {
        return round(nl * ilosc_przedzialow) / ilosc_przedzialow;
}

void main(void) {
        vec4 ml = normalize(l);
        vec4 mn = normalize(n);
        vec4 mv = normalize(v);

        vec4 r = reflect(-ml,mn);

        float nl = clamp(dot(mn,ml), 0, 1);
    float rv = pow(clamp(dot(r,mv), 0, 1), 25);
        // nl = toonify(nl, 4);


        pixelColor=vec4(iC.rgb * nl, iC.a) + vec4(rv,rv,rv,0);
}