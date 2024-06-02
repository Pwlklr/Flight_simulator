#version 330

//zmienne teksturowania
uniform sampler2D textureMap0;
uniform sampler2D textureMap1;
uniform float lightIntensity;



in vec4 iC;
in vec4 n;
in vec4 v;
in vec4 l;
in vec2 iTexCoord0;
in vec2 iTexCoord1;

out vec4 pixelColor; //Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela


float toonify(float nl, float ilosc_przedzialow) {
        return round(nl * ilosc_przedzialow) / ilosc_przedzialow;
}

void main(void) {

        vec4 kd = texture(textureMap0, iTexCoord0);
        vec4 ks = vec4(1,1,1,1);    
        // vec4 ks = kd / 3;
        // vec4 ks = texture(textureMap1,iTexCoord0);

        // kd = mix(kd, texture(textureMap1,iTexCoord1), 0.3); //do dodawania refleksi z innych tekstur (np. z nieba)

        vec4 ml = normalize(l);
        vec4 mn = normalize(n);
        vec4 mv = normalize(v);

        vec4 r = reflect(-ml,mn);

        float nl = clamp(dot(mn,ml), 0, 1);
        float rv = pow(clamp(dot(r,mv), 0, 1), 25);
        // nl = toonify(nl, 4);

        pixelColor=vec4(kd.rgb * nl * lightIntensity, kd.a) + vec4(ks.rgb * rv , 0);
}