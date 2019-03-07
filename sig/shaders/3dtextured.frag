# version 330

in vec3 Norm;
in vec3 Pos;
in vec2 Texc;

out vec4 fColor;

uniform vec3     lPos;	   // light position
uniform vec3[3]  lInt;	   // light intensities: ambient, diffuse, and specular 
uniform vec3[4]  mColors;  // material colors  : ambient, diffuse, specular, and emission 
uniform float[2] mParams;  // material params  : shininess, transparency

uniform int Mode;        // 0:textured, 1:not textured
uniform sampler2D TexId; // diffuse color texture

vec4 shade ( vec3 p, vec3 n, vec3 lp, vec3[3] li, vec3 ka, vec3 kd, vec3 ks, vec3 emi, float sh, float alpha );

void main()
{
	vec3 kd;
	float alpha;

	// When condition is uniform (cte for the entire pass) branching is free
	// because specific versions of the shader should be compiled.
	if ( Mode==0 )
	{	vec4 tex = texture2D ( TexId, Texc );
		kd = tex.rgb;
		alpha = tex.a;
	}
	else
	{	kd = mColors[1];
		alpha = mParams[1];
	}

	fColor = shade ( Pos, Norm, lPos, lInt, mColors[0], kd, mColors[2], mColors[3], mParams[0], alpha );
}
