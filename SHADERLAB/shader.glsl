#version 330 core
out vec4 FragColor;
uniform float uTime;
uniform vec2 uResolution;

float cubeSDF(vec3 p, vec3 size) {
    vec3 q = abs(p)-size;
    float sDist = length(p)-1.3;
    float cDist = length(vec3(max(q, 0.0))+min(max(q.x, max(q.y, q.z)), 0.0));
    return min(sDist, cDist);
}
vec3 rotated(vec3 p, float angle) {
    float s = sin(angle);
    float c = cos(angle);
    mat3 rx = mat3(
        1.0, 0.0, 0.0,
        0.0, c, -s,
        0.0, s, c
    ); 
    mat3 ry = mat3(
        c, -s, 0.0,
        s, c, 0.0,
        0.0, 0.0, 1.0
    );
    return p*rx*ry;
}
void mainImage(out vec4 O, in vec2 I) {
    vec2 uv = (2.0*I-uResolution)/uResolution.y;
    vec3 col = vec3(0.0);
    
    vec3 ro = vec3(0.0, 0.0, 3.0);
    vec3 rd = normalize(vec3(uv, -1.0));

    float t = 0.0;
    float maxDist = 10.0;
    bool hit = false;

    for (int i=0; i<80; i++) {
        vec3 p = ro+rd*t;
        float angle = uTime/1.0;
        float d = cubeSDF(rotated(p, angle), vec3(1.0, 1.0, 1.0));
        if (d<0.001) { hit = true; break; }
        if (t>maxDist) { break; }
        t+=d;
    } 
    
    if (hit) { 
        float depth = t/4.0;
        col = vec3(1.0-depth);
    } O.rgba = vec4(col, 1.0);
}