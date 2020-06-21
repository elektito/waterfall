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

        coords = vec4(pos, -pos.y, 1.0);
        gl_Position = coords;
        color = obj_color;
}
