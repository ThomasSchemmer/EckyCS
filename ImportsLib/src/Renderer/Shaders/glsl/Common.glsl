

float map(float v, float min, float max, float nMin, float nMax){
    return (v - min) / (max - min) * (nMax - nMin) + nMin;
}