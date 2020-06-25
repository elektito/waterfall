#version 330 core

// vertex attributes
in int index;

// instance attributes
in vec2 obj_position;
in vec2 obj_size;
in vec4 obj_texture_coords;

// output
out vec4 coords;
out vec2 texture_coords;

// uniforms
uniform vec2 camera_pos;
uniform vec2 camera_size;

void main()
{
        vec2 pos;

        switch (index) {
        case 0: // bottom-left
                pos = obj_position;
                texture_coords = obj_texture_coords.xy + vec2(0.001, 0.001);
                break;
        case 1: // top-left
                pos = obj_position + vec2(0, obj_size.y);
                texture_coords = obj_texture_coords.xw + vec2(0.001, -0.001);
                break;
        case 2: // top-right
                pos = obj_position + obj_size;
                texture_coords = obj_texture_coords.zw + vec2(-0.001, -0.001);
                break;
        case 3: // bottom-right
                pos = obj_position + vec2(obj_size.x, 0);
                texture_coords = obj_texture_coords.zy + vec2(-0.001, 0.001);
                break;
        }

        // camera transform
        pos = 2 * (pos - camera_pos) / camera_size;

        // positions are now from zero upwards. translate to (-1, -1)
        // so that the origin is at the bottom-left corner of the
        // viewport, not at the center.
        pos -= vec2(1, 1);

        coords = vec4(pos, 0.0, 1.0);
        gl_Position = coords;
}
