/** Taken from https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83 */
float rand(float2 c){
	return frac(sin(dot(c.xy ,float2(12.9898,78.233))) * 43758.5453);
}

float noise(float2 p, float freq ){
	float unit = 512/freq;
	float2 ij = floor(p/unit);
	float2 xy = (p % unit)/unit;
	//xy = 3.*xy*xy-2.*xy*xy*xy;
	xy = .5*(1.-cos(PI*xy));
	float a = rand((ij+float2(0.,0.)));
	float b = rand((ij+float2(1.,0.)));
	float c = rand((ij+float2(0.,1.)));
	float d = rand((ij+float2(1.,1.)));
	float x1 = lerp(a, b, xy.x);
	float x2 = lerp(c, d, xy.x);
	return lerp(x1, x2, xy.y);
}

float pNoise(float2 p, int res){
	float persistance = .5;
	float n = 0.;
	float normK = 0.;
	float f = 4.;
	float amp = 1.;
	int iCount = 0;
	for (int i = 0; i<50; i++){
		n+=amp*noise(p, f);
		f*=2.;
		normK+=amp;
		amp*=persistance;
		if (iCount == res) break;
		iCount++;
	}
	float nf = n/normK;
	return nf*nf*nf*nf;
}

// Simplex 2D noise
//
float3 permute(float3 x) { return (((x*34.0)+1.0)*x) % 289.0; }

float snoise(float2 v){
  const float4 C = float4(0.211324865405187, 0.366025403784439,
           -0.577350269189626, 0.024390243902439);
  float2 i  = floor(v + dot(v, C.yy) );
  float2 x0 = v -   i + dot(i, C.xx);
  float2 i1;
  i1 = (x0.x > x0.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);
  float4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = i % 289.0;
  float3 p = permute( permute( i.y + float3(0.0, i1.y, 1.0 ))
  + i.x + float3(0.0, i1.x, 1.0 ));
  float3 m = max(0.5 - float3(dot(x0,x0), dot(x12.xy,x12.xy),
    dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  float3 x = 2.0 * frac(p * C.www) - 1.0;
  float3 h = abs(x) - 0.5;
  float3 ox = floor(x + 0.5);
  float3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  float3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}