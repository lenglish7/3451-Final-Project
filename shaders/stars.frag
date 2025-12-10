#version 330 core

uniform vec2 iResolution;
uniform float iTime;
uniform int iFrame;

uniform sampler2D tex_buzz; 

in vec3 vtx_pos; // [-1, 1]
in vec2 vtx_uv; // [0, 1]

out vec4 frag_color;

#define NUM_STAR 100.
#define NUM_SHOOTING_STAR 1.
#define STAR_VEL 2.
#define DURATION 2.
#define Gravity 2.

// return random vec2 between 0 and 1
vec2 hash2d(float t)
{
    t += 1.;
    float x = fract(sin(t * 674.3) * 453.2);
    float y = fract(sin((t + x) * 714.3) * 263.2);

    return vec2(x, y);
}

vec3 hash3d(float t)
{
    t += 1.;
    float x = fract(sin(t * 674.3) * 453.2);
    float y = fract(sin((t + x) * 714.3) * 263.2);
    float z = fract(sin((t + y) * 134.3) * 534.2);

    return vec3(x, y, z);
}

vec3 renderParticle(vec2 uv, vec2 pos, float brightness, vec3 color)
{
    float d = length(uv - pos);
    return brightness / d * color;
}

vec3 renderStars(vec2 uv)
{
    vec3 fragColor = vec3(0.0);

    float t = iTime;
    for(float i = 0.; i < NUM_STAR; i++)
    {
        vec2 pos = hash2d(i) * 2. - 1.; // map to [-1, 1]
        float brightness = .001;
        brightness *= sin(1.5 * t + i) * .5 + .5; // flicker
        vec3 color = vec3(0.15, 0.71, 0.92);

        fragColor += renderParticle(uv, pos, brightness, color);
    }

    return fragColor;
}

vec3 simSingleShootingStar(vec2 fragPos, vec2 launchPos, vec2 launchVel, float t, vec3 color)
{
    vec3 fragColor = vec3(0.0);
    float emitTime = 0.5;

    if(t < emitTime){
        float brightness = .005 * (emitTime - t) / emitTime;
        vec2 initPos = launchPos;
        vec2 initVel = launchVel;
        vec2 pos = initPos + initVel * t - vec2(0.0, Gravity) * t * t / 2;
        fragColor += renderParticle(fragPos, pos, brightness, color);
    }

    return fragColor;
}

vec3 renderShootingStars(vec2 fragPos)
{
    vec3 fragColor = vec3(0.0);

    for(float i = 0.; i < NUM_SHOOTING_STAR; i++){
        float lauchTime = i;
        float relTime = iTime - lauchTime;
        float t = mod(relTime, DURATION);
        float idx = floor(relTime / DURATION);

        vec2 launchPos = vec2(0, 0.8) + (hash2d(floor(relTime / DURATION)) * 2 - 1) * vec2(1.5, 0.2);
        vec2 launchVel = vec2(-1., 0.0);
        vec3 color = sin(40. * hash3d(lauchTime) * idx) * 0.25 + 0.75;

        fragColor += simSingleShootingStar(fragPos, launchPos, launchVel, t, color);
    }

    return fragColor;
}

void main()
{
    vec3 outputColor = renderStars(vtx_pos.xy) + renderShootingStars(vtx_pos.xy);

    vec2 uv = vec2(vtx_uv.x, -vtx_uv.y);
    vec3 buzzColor = texture(tex_buzz, uv).xyz;

    frag_color = vec4(outputColor, 1.0);
}
