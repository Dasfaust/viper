#version 450

layout(location = 0) out vec4 color;

in vec2 vPosition;

void main()
{
    color = vec4(vPosition.x + 0.2, vPosition.y + 0.2, 0.2, 1.0);
}