#version 330 core

// input
in vec4 coords;
in vec2 texture_coords;

// output
out vec4 frag_color;

// uniforms
uniform sampler2D texture0;

void main()
{
        frag_color = texture(texture0, texture_coords);
}
