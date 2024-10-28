const char* positionTextureColor_frag = R"(
#version 320 es
#ifdef GL_ES
precision lowp float;
#endif

layout(location = 0) in vec4 v_fragmentColor;
layout(location = 1) in vec2 v_texCoord;

layout(location = 2) uniform sampler2D u_texture;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = v_fragmentColor * texture(u_texture, v_texCoord);
}
)";
