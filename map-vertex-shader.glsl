#version 330 core

// vertex attributes
in int index;

// instance attributes
in vec4 tile_texture_coords;

// output
out vec4 coords;
out vec2 texture_coords;

// uniforms
uniform int map_width;
uniform vec2 camera_pos;
uniform vec2 camera_size;

void main()
{
        vec2 tile;
        vec2 pos;

        // One instance is run for each tile, so we get the tile
        // position based on the instance ID.
        tile = vec2(gl_InstanceID % map_width,
                    gl_InstanceID / map_width);

        // "index" determines which tile vertex we have.
        switch (index) {
        case 0: // bottom-left
                pos = tile;
                texture_coords = tile_texture_coords.xy;
                break;
        case 1: // top-left
                pos = tile + vec2(0, 1);
                texture_coords = tile_texture_coords.xw;
                break;
        case 2: // top-right
                pos = tile + vec2(1, 1);
                texture_coords = tile_texture_coords.zw;
                break;
        case 3: // bottom-right
                pos = tile + vec2(1, 0);
                texture_coords = tile_texture_coords.zy;
                break;
        }

        // camera transform
        pos = 2 * (pos - camera_pos) / camera_size;

        // position is now from zero upwards. translate to (-1, -1) so
        // that the origin is at the bottom-left corner of the
        // viewport, not at the center.
        pos -= vec2(1, 1);

        coords = vec4(pos, 0.0, 1.0);
        gl_Position = coords;
}
