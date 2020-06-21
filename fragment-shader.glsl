#version 330 core

// input
in vec4 coords;
in vec4 color;

out vec4 frag_color;

void main()
{
        frag_color = vec4(color);
}
