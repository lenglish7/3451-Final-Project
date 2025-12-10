#version 330 core

/*default camera matrices. do not modify.*/
layout(std140) uniform camera
{
    mat4 projection;	/*camera's projection matrix*/
    mat4 view;			/*camera's view matrix*/
    mat4 pvm;			/*camera's projection*view*model matrix*/
    mat4 ortho;			/*camera's ortho projection matrix*/
    vec4 position;		/*camera's position in world space*/
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

/*input variables*/
in vec3 vtx_normal; // vtx normal in world space
in vec3 vtx_position; // vtx position in world space
in vec3 vtx_model_position; // vtx position in model space
in vec4 vtx_color;
in vec2 vtx_uv;
in vec3 vtx_tangent;

uniform vec3 ka;            /* object material ambient */
uniform vec3 kd;            /* object material diffuse */
uniform vec3 ks;            /* object material specular */
uniform float shininess;    /* object material shininess */

uniform sampler2D tex_color;   /* texture sampler for color */
uniform sampler2D tex_normal;   /* texture sampler for normal vector */
uniform float u_time;

/*output variables*/
out vec4 frag_color;

vec3 shading_texture_with_phong(light li, vec3 e, vec3 p, vec3 surfaceColor, vec3 n)
{
    vec3 N = normalize(n);
    vec3 L = normalize(li.pos.xyz - p);   // light direction
    vec3 V = normalize(e - p);            // view direction
    vec3 R = reflect(-L, N);              // reflection direction

    float diff = max(dot(N, L), 0.0);
    float specTerm = pow(max(dot(R, V), 0.0), shininess);

    // Use this framework’s light fields: amb / dif / spec
    vec3 ambient  = ka * surfaceColor * li.amb.xyz;
    vec3 diffuse  = kd * surfaceColor * li.dif.xyz * diff;
    vec3 specular = ks * li.spec.xyz * specTerm;

    return ambient + diffuse + specular;
}

vec3 read_normal_texture()
{
    vec3 normal = texture(tex_normal, vtx_uv).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    return normal;
}

void main()
{
    vec3 e = position.xyz;
    vec3 p = vtx_position;

    // Geometry normal & tangent
    vec3 N_geom = normalize(vtx_normal);
    vec3 T = normalize(vtx_tangent);
    vec3 B = normalize(cross(N_geom, T));
    mat3 TBN = mat3(T, B, N_geom);

    // Normal from normal map (if you bind tex_normal).
    // If you don't want a normal map, you can just use N_geom instead.
    vec3 N_map = read_normal_texture();  // tangent-space
    N_map = normalize(TBN * N_map);      // world-space

    float wave1 = sin(4.0 * p.x + 2.0 * u_time);
    float wave2 = cos(3.0 * p.z + 1.5 * u_time);
    float bump  = 0.08 * (wave1 + wave2);

    vec3 N = normalize(N_map + vec3(bump, 0.0, bump));

    vec3 surfaceColor = texture(tex_color, vtx_uv).rgb;

    vec3 color = vec3(0.0);

    color += amb.xyz * ka * surfaceColor;

    int numLights = lt_att[0];
    for (int i = 0; i < numLights; ++i) {
        color += shading_texture_with_phong(lt[i], e, p, surfaceColor, N);
    }

    frag_color = vec4(color, 1.0);
}