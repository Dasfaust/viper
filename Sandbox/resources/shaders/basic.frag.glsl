#version 450

layout(location = 0) out vec4 color;

in vec2 vPosition;

void main()
{
    color = vec4(vPosition, 0.0, 1.0);
}