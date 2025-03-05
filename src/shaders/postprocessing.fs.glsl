#version 420 core

out vec4 frag_color;

in vec2 tex_coords;

uniform sampler2D screen_texture;
uniform sampler2D bloom_texture;
uniform float exposure;
uniform int bloom_enabled;
uniform int tonemapper_choice;

// NOTE: tonemapping code taken from https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
mat3x3 ACESInputMat = mat3x3
(
	0.59719, 0.35458, 0.04823,
	0.07600, 0.90834, 0.01566,
	0.02840, 0.13383, 0.83777
);
mat3x3 ACESOutputMat = mat3x3
(
	 1.60475, -0.53108, -0.07367,
	-0.10208,  1.10813, -0.00605,
	-0.00327, -0.07276,  1.07602
);
vec3 RRTAndODTFit(vec3 v)
{
	vec3 a = v * (v + 0.0245786f) - 0.000090537f;
	vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
	return(a / b);
}
vec3 ACESFitted(vec3 color)
{
	color = transpose(ACESInputMat) * color;
	color = RRTAndODTFit(color);
	color = transpose(ACESOutputMat) * color;
	color = clamp(color, 0, 1);
	return(color);
}

void main()
{
    vec3 color = texture(screen_texture, tex_coords).rgb;

    if(bloom_enabled > 0)
    {
        vec3 bloom_sample = texture(bloom_texture, tex_coords).rgb;
        color = mix(color, bloom_sample, 0.04f);
    }

    // NOTE: tonemapping then gamma correct
    color = color * exposure;
    if(tonemapper_choice == 1)
    {
        color = vec3(1.0f) - exp(-color);
    }
    else if(tonemapper_choice == 2)
    {
        color = ACESFitted(color);
    }
    color = pow(color, vec3(1.0f / 2.2f));
    frag_color = vec4(color, 1.0f);
}