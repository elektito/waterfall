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
        float x, y;

        switch (index) {
        case 0: // bottom-left
                x = obj_position.x;
                y = obj_position.y;
                break;
        case 1: // top-left
                x = obj_position.x;
                y = obj_position.y + obj_size.y;
                break;
        case 2: // top-right
                x = obj_position.x + obj_size.x;
                y = obj_position.y + obj_size.y;
                break;
        case 3: // bottom-right
                x = obj_position.x + obj_size.x;
                y = obj_position.y;
                break;
        }

        coords = vec4(x, y, -y, 1.0);
        gl_Position = coords;
        color = obj_color;
}
