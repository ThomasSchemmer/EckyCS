#version 330 core
out vec4 FragColor;

void main(){
    float Depth = gl_FragCoord.z;
    float dx = dFdx(Depth);
    float dy = dFdy(Depth);
    float Moment2 = Depth * Depth + 0.25 * (dx * dx + dy * dy);
    FragColor = vec4(Depth, Moment2, 0, 0);
}