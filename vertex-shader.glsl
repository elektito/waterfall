#version 330 core

// vertex attributes
in int index;

// instance attributes
in vec2 obj_position;
in vec2 obj_size;
in vec4 obj_color;

// output
out vec4 coords;
out vec4 color;

// uniforms
uniform vec2 camera_pos;
uniform vec2 camera_size;

void main()
{
        vec2 pos;

        switch (index) {
        case 0: // bottom-left
                pos = obj_position;
                break;
        case 1: // top-left
                pos = obj_position + vec2(0, obj_size.y);
                break;
        case 2: // top-right
                pos = obj_position + obj_size;
                break;
        case 3: // bottom-right
                pos = obj_position + vec2(obj_size.x, 0);
                break;
        }

        // camera transform
        pos = (pos - camera_pos) / camera_size * 2.0 - 1.0;

        coords = vec4(pos, -pos.y, 1.0);
        gl_Position = coords;
        color = obj_color;
}
