#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <stdio.h>

static GLuint shader_program;
static GLuint vbo;
static GLuint vao;

static char *
read_file(const char *filename, long *length)
{
        char *buffer = 0;
        FILE *f = fopen(filename, "rb");

        if (f) {
                fseek(f, 0, SEEK_END);
                *length = ftell(f);
                fseek(f, 0, SEEK_SET);
                buffer = malloc(*length);
                if (buffer) {
                        fread(buffer, 1, *length, f);
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
        GLuint status;

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
load()
{
        shader_program = load_shader_program("vertex-shader.glsl",
                                             "fragment-shader.glsl");

        float vertices[] = {
                0.0f, 0.5f, 0.0f,
                0.0f, -0.5f, 0.0f,
                0.5f, -0.5f, 0.0f,
        };

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        GLint position_attr = glGetAttribLocation(shader_program, "position");
        glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(position_attr);

        /* glVertexAttribPointer already registered with the VAO, so
         * we can safely unbind */
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        /* Unbind VAO */
        glBindVertexArray(0);
}

static void
render()
{
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

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
                }

        case SDL_WINDOWEVENT:
                if (e->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        int winw, winh;
                        SDL_GetWindowSize(window, &winw, &winh);

                        // Update OpenGL viewport.
                        glViewport(0, 0, winw, winh);

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
