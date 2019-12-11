#version 450

layout(location = 0) out vec4 color;

in vec2 sTexCoord;

uniform sampler2D uTexture0;
uniform sampler2D uTexture1;

void main()
{
    color = texture(uTexture1, sTexCoord) * texture(uTexture0, sTexCoord);
}