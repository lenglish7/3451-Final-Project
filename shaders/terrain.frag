#version 330 core

layout (std140) uniform camera
{
	mat4 projection;
	mat4 view;
	mat4 pvm;
	mat4 ortho;
	vec4 position;
};

/* set light ubo. do not modify.*/
struct light
{
	ivec4 att; 
	vec4 pos; // position
	vec4 dir;
	vec4 amb; // ambient intensity
	vec4 dif; // diffuse intensity
	vec4 spec; // specular intensity
	vec4 atten;
	vec4 r;
};
layout(std140) uniform lights
{
	vec4 amb;
	ivec4 lt_att; // lt_att[0] = number of lights
	light lt[4];
};

uniform float iTime;
uniform mat4 model;		/*model matrix*/

in vec3 vtx_pos;

out vec4 frag_color;


uniform vec3 ka;             /* object material ambient */
uniform vec3 kd;             /* object material diffuse */
uniform vec3 ks;             /* object material specular */
uniform float shininess;     /* object material shininess */
uniform sampler2D water_tex; /* object texture */

vec2 hash2(vec2 v)
{
	vec2 rand = vec2(0,0);
	
	rand  = 50.0 * 1.05 * fract(v * 0.3183099 + vec2(0.71, 0.113));
    rand = -1.0 + 2 * 1.05 * fract(rand.x * rand.y * (rand.x + rand.y) * rand);
	return rand;
}

float perlin_noise(vec2 v) 
{
    float noise = 0;
	vec2 i = floor(v);
    vec2 f = fract(v);
    vec2 m = f*f*(3.0-2.0*f);
	
	noise = mix( mix( dot( hash2(i + vec2(0.0, 0.0)), f - vec2(0.0,0.0)),
					 dot( hash2(i + vec2(1.0, 0.0)), f - vec2(1.0,0.0)), m.x),
				mix( dot( hash2(i + vec2(0.0, 1.0)), f - vec2(0.0,1.0)),
					 dot( hash2(i + vec2(1.0, 1.0)), f - vec2(1.0,1.0)), m.x), m.y);
	return noise;
}

float noiseOctave(vec2 v, int num)
{
	float sum = 0;
	for(int i =0; i<num; i++){
		sum += pow(2,-1*i) * perlin_noise(pow(2,i) * v);
	}
	return sum;
}

float height(vec2 v){
    float h = 0;
	h = 0.75 * noiseOctave(v, 10);
	if(h<0) h *= .5;
	h *= 2.;
	
	return h;
}

vec3 compute_normal(vec2 v, float d)
{	
	vec3 normal_vector = vec3(0,0,0);
	vec3 v1 = vec3(v.x + d, v.y, height(vec2(v.x + d, v.y)));
	vec3 v2 = vec3(v.x - d, v.y, height(vec2(v.x - d, v.y)));
	vec3 v3 = vec3(v.x, v.y + d, height(vec2(v.x, v.y + d)));
	vec3 v4 = vec3(v.x, v.y - d, height(vec2(v.x, v.y - d)));
	
	normal_vector = normalize(cross(v1-v2, v3-v4));
	return normal_vector;
}

vec4 shading_phong(light li, vec3 e, vec3 p, vec3 s, vec3 n) 
{
    vec3 v = normalize(e - p);
    vec3 l = normalize(s - p);
    vec3 r = normalize(reflect(-l, n));

    vec3 ambColor = ka * li.amb.rgb;
    vec3 difColor = kd * li.dif.rgb * max(0., dot(n, l));
    vec3 specColor = ks * li.spec.rgb * pow(max(dot(v, r), 0.), shininess);

    return vec4(ambColor + difColor + specColor, 1);
}

vec3 shading_terrain(vec3 pos) {
    // Water area
    if (pos.z <= -0.15) {
        vec3 e       = position.xyz;
        vec3 p_local = pos;
        vec3 p_world = (model * vec4(p_local, 1.0)).xyz;


        // animated waves
        float wave1 = sin(2.5 * pos.x + 2.0 * iTime);
        float wave2 = cos(5.0 * pos.y + 1.5 * iTime);
        vec3 n_local = normalize(vec3(0.25 * wave1, 0.25 * wave2, 1.0));
        vec3 n_world = normalize((model * vec4(n_local, 0.0)).xyz);


        vec2 flowDir   = normalize(vec2(0.0, 1.0));
        float flowSpeed = 0.35;
        vec2 flowOffset = flowDir * (flowSpeed * iTime);

        vec2 uvBase = pos.xy * 10.0 + flowOffset;
        vec2 uv1    = uvBase + 0.07 * n_local.xy;      // warped by waves
        vec2 uv2    = vec2(-uvBase.y, uvBase.x) * 0.8; // rotated second layer

        vec3 tex1 = texture(water_tex, uv1).rgb;
        vec3 tex2 = texture(water_tex, uv2).rgb;
        vec3 texCol = 0.5 * tex1 + 0.5 * tex2;

        // color grading
        vec3 deep        = vec3(0.03, 0.12, 0.22);
        vec3 surfaceTint = vec3(0.04, 0.35, 0.45);
        float brightness = clamp(dot(texCol, vec3(0.3, 0.4, 0.3)), 0.0, 1.0);
        vec3 waterBase   = mix(deep, surfaceTint, brightness);

        float cosTheta = max(dot(normalize(e - p_world), n_world), 0.0);
        float fresnel  = pow(1.0 - cosTheta, 3.0);
        waterBase += fresnel * vec3(0.2, 0.25, 0.3);

        vec3 s   = lt[0].pos.xyz;
        vec3 lit = shading_phong(lt[0], e, p_world, s, n_world).rgb;

        return waterBase * lit;
    }

    // Mountain area
    vec3 n = compute_normal(pos.xy, 0.01);
    vec3 e = position.xyz;
    vec3 p = pos.xyz;
    vec3 s = lt[0].pos.xyz;

    n = normalize((model * vec4(n, 0)).xyz);
    p = (model * vec4(p, 1)).xyz;

    vec3 color = shading_phong(lt[0], e, p, s, n).xyz;

    float h = pos.z + .8;
    h = clamp(h, 0.0, 1.0);
    vec3 emissiveColor = mix(vec3(.3,.3,.3), vec3(1.0,1.0,1.0), h);

    return color * emissiveColor;
}




void main()
{
    frag_color = vec4(shading_terrain(vtx_pos), 1.0);
}
