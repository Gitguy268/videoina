#version 440
// Liquid-glass panel: blurs what is behind it, bends it near the rounded edge
// (refraction) and adds a light-catching rim. `src` is the scene behind the panel.

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec2 size;      // panel size in px
    float radius;   // corner radius in px
};
layout(binding = 1) uniform sampler2D src;

float roundRect(vec2 p, vec2 halfSize, float r)
{
    vec2 q = abs(p) - halfSize + r;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

void main()
{
    vec2 hs = size * 0.5;
    vec2 p = (qt_TexCoord0 - 0.5) * size;
    float d = roundRect(p, hs, radius); // < 0 inside
    float mask = clamp(0.5 - d, 0.0, 1.0);

    vec2 e = vec2(1.0, 0.0);
    vec2 n = normalize(vec2(roundRect(p + e.xy, hs, radius) - roundRect(p - e.xy, hs, radius),
                            roundRect(p + e.yx, hs, radius) - roundRect(p - e.yx, hs, radius)) + 1e-5);

    // Refraction: inside a band along the edge, pull samples toward the centre,
    // so the edge magnifies what is behind it like a thick lens.
    float band = min(radius * 1.5, min(hs.x, hs.y));
    float edge = clamp(1.0 + d / band, 0.0, 1.0); // 0 deep inside, 1 at the rim
    vec2 uv = qt_TexCoord0 - n * edge * edge * band * 0.8 / size;

    // Frost: 7x7 Gaussian, taps spread 7 px apart (texture is half-res already).
    vec4 c = vec4(0.0);
    float w = 0.0;
    for (int i = -3; i <= 3; i++) {
        for (int j = -3; j <= 3; j++) {
            float k = exp(-float(i * i + j * j) / 8.0);
            c += texture(src, uv + vec2(i, j) * 7.0 / size) * k;
            w += k;
        }
    }
    vec3 col = c.rgb / w;

    // Darken slightly so white controls stay readable on bright video.
    col = col * 0.72 + 0.05;

    // Rim light: thin bright line inside the edge, strongest where the edge
    // faces the light (top-left) and the opposite corner (bottom-right).
    float rim = exp(-abs(d + 1.2) * 1.3);
    float facing = abs(dot(n, normalize(vec2(-0.6, -0.8))));
    col += rim * (0.18 + 0.5 * facing);
    // Soft inner glow along the edge band.
    col += edge * edge * 0.06;

    fragColor = vec4(col, 1.0) * mask * qt_Opacity;
}
