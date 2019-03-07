# version 330

uniform vec3     lPos;     // light position
uniform vec3[3]  lInt;     // light intensities: ambient, diffuse, and specular 
uniform vec3[4]  mColors;  // material colors  : ambient, diffuse, specular, and emission 
uniform float[2] mParams;  // material params  : shininess, transparency

in vec3 Pos;
in vec4 Color;
in vec3 Norm;
out vec4 fColor;

vec4 shade ( vec3 p, vec3 n, vec3 lp, vec3[3] li, vec3 ka, vec3 kd, vec3 ks, vec3 emi, float sh, float alpha );

void main() 
{
	fColor = shade ( Pos, Norm, lPos, lInt, mColors[0], Color.rgb, mColors[2], mColors[3], mParams[0], Color.a );
}
