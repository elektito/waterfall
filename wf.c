#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static GLuint texture;
static GLuint object_program;
static GLuint vbo;
static GLuint instance_vbo;
static GLuint vao;

static float cam_x = 0.0;
static float cam_y = 0.0;
static float cam_w = 1.0;
static float cam_h = 1.0;
static float zoom = 1.0;

const int UNIT_SIZE = 16;

static char *
read_file(const char *filename, long *length)
{
        int bytes_read;
        char *buffer = 0;
        FILE *f = fopen(filename, "rb");

        if (f) {
                fseek(f, 0, SEEK_END);
                *length = ftell(f);
                fseek(f, 0, SEEK_SET);
                buffer = malloc(*length);
                if (buffer) {
                        bytes_read = fread(buffer, 1, *length, f);
                        if (bytes_read != *length) {
                                printf("Could not read file: %s\n",
                                       filename);
                                exit(1);
                        }
                }
                fclose(f);
        }

        return buffer;
}

static GLuint
load_shader(GLenum shader_type,
            const char *shader_source,
            int source_length)
{
        GLint lengths[] = {source_length};
        GLuint shader = glCreateShader(shader_type);
        glShaderSource(shader, 1, &shader_source, lengths);
        glCompileShader(shader);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) {
                const char *type_name = shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment";
                printf("Failed to compile %s shader.\n", type_name);

                char info_log[512];
                glGetShaderInfoLog(shader, 512, NULL, info_log);
                printf("Compile log:\n");
                printf("%s", info_log);

                exit(1);
        }

        return shader;
}

static GLuint
load_shader_program(const char *vertex_shader_filename,
                    const char *fragment_shader_filename)
{
        char *vertex_shader_source;
        char *fragment_shader_source;
        long vertex_shader_source_length;
        long fragment_shader_source_length;
        GLuint vertex_shader;
        GLuint fragment_shader;
        GLuint program;
        GLint status;

        vertex_shader_source = read_file(vertex_shader_filename,
                                         &vertex_shader_source_length);
        fragment_shader_source = read_file(fragment_shader_filename,
                                           &fragment_shader_source_length);

        vertex_shader = load_shader(GL_VERTEX_SHADER,
                                    vertex_shader_source,
                                    vertex_shader_source_length);
        fragment_shader = load_shader(GL_FRAGMENT_SHADER,
                                      fragment_shader_source,
                                      fragment_shader_source_length);

        free(vertex_shader_source);
        free(fragment_shader_source);

        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
                printf("failed linking shaders.\n");
                exit(1);
        }

        return program;
}

static void
update_camera(void)
{
        GLint camera_pos, camera_size;

        camera_pos = glGetUniformLocation(object_program, "camera_pos");
        camera_size = glGetUniformLocation(object_program, "camera_size");

        glUseProgram(object_program);
        glUniform2f(camera_pos, cam_x, cam_y);
        glUniform2f(camera_size, cam_w * zoom, cam_h * zoom);
        glUseProgram(0);
}

static GLuint
load_texture(const char *filename)
{
        GLuint tex;
        int w, h, channels;
        uint8_t *img;
        GLenum err;

        /* Since the direction of the y-axis in the image files is the
           reverse of its direction in OpenGL textures, instruct
           stb_image to flip the y-axis when loading textures. */
        stbi_set_flip_vertically_on_load(1);

        img = stbi_load(filename, &w, &h, &channels, 0);
        if (img == NULL) {
                printf("Unable to load image. stb_image error: %s\n",
                       stbi_failure_reason());
                exit(1);
        }

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, img);
        stbi_image_free(img);
        err = glGetError();
        if (err != GL_NO_ERROR) {
                printf("OpenGL error %d while loading image file: %s.\n",
                       (int) err, filename);
                exit(1);
        }
        printf("Loaded texture: filename=%s size=%dx%d channels=%d\n",
               filename, w, h, channels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        /* Generate mipmaps. */
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        return tex;
}

static void
init_objects(void)
{
        object_program = load_shader_program("vertex-shader.glsl",
                                             "fragment-shader.glsl");

        // Vertex data are actually only the vertex indices. 0, 1, 2,
        // and 3 being the bottom-left, top-left, top-right and
        // bottom-right corners of the quad.
        int vertex_data[] = {
                0, 1, 3, // triangle 1
                1, 2, 3, // triangle 2
        };

        float instance_data[] = {
                // pos         size         texture coords
                0.0f, 0.0f,    5.0f, 5.0f,  0.0f, 0.5f, 0.5f, 1.0f,
                -5.0f, -5.0f,  2.0f, 2.0f,  0.5f, 0.0f, 1.0f, 0.5f,
                10.0f, 5.0f,   2.0f, 2.0f,  0.5f, 0.5f, 1.0f, 1.0f,
        };

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &instance_vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

        GLint index_attr = glGetAttribLocation(object_program, "index");
        glVertexAttribIPointer(index_attr, 1, GL_INT, 1 * sizeof(int), (void *) 0);
        glEnableVertexAttribArray(index_attr);

        /* glVertexAttribPointer already registered with the VAO, so
         * we can safely unbind */
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(instance_data), instance_data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);

        GLint obj_position_attr = glGetAttribLocation(object_program, "obj_position");
        glEnableVertexAttribArray(obj_position_attr);
        glVertexAttribPointer(obj_position_attr, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
        glVertexAttribDivisor(obj_position_attr, 1);

        GLint obj_size_attr = glGetAttribLocation(object_program, "obj_size");
        glEnableVertexAttribArray(obj_size_attr);
        glVertexAttribPointer(obj_size_attr, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (2 * sizeof(GLfloat)));
        glVertexAttribDivisor(obj_size_attr, 1);

        GLint obj_tex_coords_attr = glGetAttribLocation(object_program, "obj_texture_coords");
        glEnableVertexAttribArray(obj_tex_coords_attr);
        glVertexAttribPointer(obj_tex_coords_attr, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (4 * sizeof(GLfloat)));
        glVertexAttribDivisor(obj_tex_coords_attr, 1);

        /* glVertexAttribPointer already registered with the VAO, so
         * we can safely unbind */
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        /* Unbind VAO */
        glBindVertexArray(0);
}

static void
load(void)
{
        texture = load_texture("sheet.png");

        init_objects();

        update_camera();

        /* Enable blending */
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void
render(void)
{
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, texture);
        glUseProgram(object_program);
        glBindVertexArray(vao);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 3);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindVertexArray(0);
        glUseProgram(0);
}

static void
handle_events(SDL_Event *e, SDL_Window *window, int *quit)
{
        SDL_Event quitEvent;

        switch (e->type) {
        case SDL_QUIT:
                *quit = 1;
                break;

        case SDL_KEYDOWN:
                switch (e->key.keysym.sym) {
                case SDLK_q:
                        quitEvent.type = SDL_QUIT;
                        SDL_PushEvent(&quitEvent);
                        break;

                case SDLK_LEFT:
                        cam_x += cam_w / 100.0;
                        update_camera();
                        break;

                case SDLK_RIGHT:
                        cam_x -= cam_w / 100.0;
                        update_camera();
                        break;

                case SDLK_UP:
                        cam_y -= cam_h / 100.0;
                        update_camera();
                        break;

                case SDLK_DOWN:
                        cam_y += cam_h / 100;
                        update_camera();
                        break;

                case SDLK_MINUS:
                case SDLK_UNDERSCORE:
                        zoom += 0.1;
                        update_camera();
                        break;

                case SDLK_PLUS:
                case SDLK_EQUALS:
                        zoom -= 0.1;
                        if (zoom <= 0.1)
                                zoom = 0.1;
                        update_camera();
                        break;
                }

        case SDL_WINDOWEVENT:
                if (e->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        int winw, winh;
                        SDL_GetWindowSize(window, &winw, &winh);

                        // Update OpenGL viewport.
                        glViewport(0, 0, winw, winh);

                        cam_w = winw / UNIT_SIZE;
                        cam_h = winh / UNIT_SIZE;
                        update_camera();

                        printf("Window resized: %dx%d\n", winw, winh);
                }
                break;
        }
}

int
main(int argc, char *argv[])
{
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
                printf("SDL could not be initialized. SDL_Error: %s\n",
                       SDL_GetError());
                return 1;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE);

        SDL_Window *window = SDL_CreateWindow(
                "waterfall",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                -1,
                -1,
                SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        if (window == NULL) {
                printf("Window could not be created. SDL_Error: %s",
                       SDL_GetError());
                return 1;
        }

        /* Create an OpenGL context and make it current.o */
        SDL_GL_CreateContext(window);

        if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
                printf("Failed to initialize GLAD\n");
                return 1;
        }

        load();

        SDL_ShowWindow(window);

        SDL_Event e;
        int quit = 0;
        while (!quit) {
                if (SDL_PollEvent(&e))
                        handle_events(&e, window, &quit);

                render();

                SDL_GL_SwapWindow(window);
        }

        return 0;
}
