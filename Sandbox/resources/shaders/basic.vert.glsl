#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in mat4 model;

uniform mat4 view;
uniform mat4 proj;

out vec2 vPosition;

void main()
{
    vPosition = position;
    gl_Position = view * proj * model * vec4(position.xy, 0.0, 1.0);
}