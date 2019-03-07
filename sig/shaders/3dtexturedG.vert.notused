# version 330

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;
layout (location = 2) in vec2 vTexc;

uniform mat4	 vProj;
uniform mat4	 vView;
uniform vec3     lPos;	   // light position
uniform vec3[3]  lInt;	   // light intensities: ambient, diffuse, and specular 
uniform vec3[4]  mColors;  // material colors  : ambient, diffuse, specular, and emission 
uniform float[2] mParams;  // material params  : shininess, transparency

out vec4 Color;
out vec2 Texc;

vec4 shade ( vec3 p, vec3 n, vec3 lp, vec3[3] li, vec3 ka, vec3 kd, vec3 ks, vec3 emi, float sh, float alpha );

void main ()
{
	vec4 p4 = vec4(vPos,1.0f)*vView; // vertex pos in eye coords
	vec3 p = p4.xyz / p4.w;

	vec3 n = normalize ( vNorm*transpose(inverse(mat3(vView))) ); // vertex normal 

	vec3 kd = mColors[1];

	Color = shade ( p, n, lPos, lInt, mColors[0], kd, mColors[2], mColors[3], mParams[0], mParams[1] );
	Texc = vTexc;

	gl_Position = vec4(p,1.0)*vProj;
}
