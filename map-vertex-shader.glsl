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
        //
        // Note about texture coordinates: texture coordinates are
        // offset by a small amount towards the inside of the tile,
        // this is to get rid of a situation where the tiles do not
        // completely mesh with each other, causing one pixel of
        // background to occasionally bleed through.
        switch (index) {
        case 0: // bottom-left
                pos = tile;
                texture_coords = tile_texture_coords.xy + vec2(0.001, 0.001);
                break;
        case 1: // top-left
                pos = tile + vec2(0, 1);
                texture_coords = tile_texture_coords.xw + vec2(0.001, -0.001);
                break;
        case 2: // top-right
                pos = tile + vec2(1, 1);
                texture_coords = tile_texture_coords.zw + vec2(-0.001, -0.001);
                break;
        case 3: // bottom-right
                pos = tile + vec2(1, -0);
                texture_coords = tile_texture_coords.zy + vec2(-0.001, 0.001);
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
