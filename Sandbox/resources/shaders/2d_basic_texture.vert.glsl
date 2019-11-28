#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in mat4 model;

uniform mat4 view;
uniform mat4 proj;

out vec2 sTexCoord;

void main()
{
    sTexCoord = texCoord;
    gl_Position = view * proj * model * vec4(position.xy, 0.0, 1.0);
}