#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#define NOB_EXPERIMENTAL_DELETE_OLD
#define NOB_WARN_DEPRECATED
#include "./nob.h"

#define CC "gcc"
#define EMCC "clang"
#define COMMON_CFLAGS "-Wall", "-Wextra", "-ggdb", "-I.", "-I./thirdparty/"
#define SDL2_PATH "/c/Users/lirong/Desktop/MouseWithoutBorders/SDL2-2.30.8/x86_64-w64-mingw32/"

bool build_tools(Cmd *cmd, Procs *procs)
{
    if (!mkdir_if_not_exists("bin")) return false;

    cmd_append(cmd, CC, COMMON_CFLAGS, "-o", "./bin/png2c", "./tools/png2c.c", "-lm");
    if (!cmd_run(cmd, .async = procs)) return false;

    cmd_append(cmd, CC, COMMON_CFLAGS, "-o", "./bin/obj2c", "./tools/obj2c.c", "-lm");
    if (!cmd_run(cmd, .async = procs)) return false;

    return true;
}

bool build_assets(Cmd *cmd, Procs *procs)
{
    if (!mkdir_if_not_exists("assets")) return false;

    cmd_append(cmd, "./bin/png2c", "-o", "./bin/assets/tsodinPog.c", "./assets/tsodinPog.png");
    if (!cmd_run(cmd, .async = procs)) return false;

    cmd_append(cmd, "./bin/png2c", "-o", "./assets/kun.c", "./assets/kun.png", "-n", "kun");
    if (!cmd_run(cmd, .async = procs)) return false;

    cmd_append(cmd, "./bin/obj2c", "-o", "./assets/cup.c", "./assets/cup.obj");
    if (!cmd_run(cmd, .async = procs)) return false;

    cmd_append(cmd, "./bin/obj2c", "-o", "./assets/teapot.c", "./assets/teapot.obj", "-s", "0.40");
    if (!cmd_run(cmd, .async = procs)) return false;

    return true;
}

bool build_tests(Cmd *cmd, Procs *procs)
{
    cmd_append(cmd, CC, COMMON_CFLAGS, "-o", "./bin/test", "test.c", "-lm");
    if (!cmd_run(cmd, .async = procs)) return false;
    return true;
}

bool build_wasm_demo(Cmd *cmd, Procs *procs, const char *name)
{
    cmd_append(cmd, EMCC, COMMON_CFLAGS, "-O2", "-fno-builtin", "--target=wasm32", "--no-standard-libraries", "-Wl,--no-entry", "-Wl,--export=render", "-Wl,--export=__heap_base", "-Wl,--allow-undefined", "-o", temp_sprintf("./bin/%s.wasm", name), "-DPLATFORM=WASM_PLATFORM", temp_sprintf("./examples/%s.c", name));
    if (!cmd_run(cmd, .async = procs)) return false;
}

bool build_term_demo(Cmd *cmd, Procs *procs, const char *name)
{
    cmd_append(cmd, CC, COMMON_CFLAGS, "-O2", "-o", temp_sprintf("./bin/%s.term", name), "-DPLATFORM=TERM_PLATFORM", temp_sprintf("./examples/%s.c", name), "-lm");
    if (!cmd_run(cmd, .async = procs)) return false;
}

bool build_sdl_demo(Cmd *cmd, Procs *procs, const char *name)
{
    cmd_append(cmd, CC, COMMON_CFLAGS, temp_sprintf("-I%s/include/", SDL2_PATH), "-O2", "-o", temp_sprintf("./bin/%s.sdl", name), "-DPLATFORM=SDL_PLATFORM", "-DSDL_MAIN_HANDLED", temp_sprintf("./examples/%s.c", name), "-lm", temp_sprintf("-L%s/lib/", SDL2_PATH), "-lSDL2", NULL);
    if (!cmd_run(cmd, .async = procs)) return false;
}

bool build_vc_demo(Cmd *cmd, Procs *procs, const char *name)
{
    if (!build_wasm_demo(cmd, procs, name)) return false;
    if (!build_term_demo(cmd, procs, name)) return false;
    if (!build_sdl_demo(cmd, procs, name))  return false;
    return true;
}

const char *names[] = {
    "triangle",
    "rotating_3d",
    "squish",
    "triangle_3d",
    "triangle_texture",
    "cup3d",
    "teapot3d",
};

bool copy_all_vc_demos_to_build(void)
{
    for (size_t i = 0; i < ARRAY_LEN(names); ++i) {
        const char *src_path = temp_sprintf("./bin/%s.wasm", names[i]);
        const char *dst_path = temp_sprintf("./wasm/%s.wasm", names[i]);
        if (!copy_file(src_path, dst_path)) return false;
    }
    return true;
}

bool build_all_vc_demos(Cmd *cmd, Procs *procs)
{
    if (!mkdir_if_not_exists("bin")) return false;

    for (size_t i = 0; i < ARRAY_LEN(names); ++i) {
        if (!build_vc_demo(cmd, procs, names[i])) return false;
    }
    return true;
}

void usage(const char *program)
{
    nob_log(INFO, "Usage: %s [<subcommand>]", program);
    nob_log(INFO, "Subcommands:");
    nob_log(INFO, "    tools");
    nob_log(INFO, "        Build all the tools. Things like png2c, obj2c, etc.");
    nob_log(INFO, "    assets");
    nob_log(INFO, "        Build the assets in the assets/ folder.");
    nob_log(INFO, "        Basically convert their data to C code so we can bake them in demos.");
    nob_log(INFO, "    test[s] [<args>]");
    nob_log(INFO, "        Build and run test.c");
    nob_log(INFO, "        If <args> are provided the test utility is run with them.");
    nob_log(INFO, "    demos [<platform>] [run]");
    nob_log(INFO, "        Build demos.");
    nob_log(INFO, "        Available platforms are: sdl, term, or wasm.");
    nob_log(INFO, "        Optional [run] runs the demo after the build.");
    nob_log(INFO, "        [run] is not available for wasm platform.");
    nob_log(INFO, "    help");
    nob_log(INFO, "         Print this message");
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    Cmd cmd = {0};
    Procs procs = {0};

    const char *program = shift_args(&argc, &argv);

    if (argc > 0) {
        const char *subcmd = shift_args(&argc, &argv);
        if (strcmp(subcmd, "tools") == 0) {
            if (!build_tools(&cmd, &procs)) return 1;
            if (!procs_flush(&procs)) return 1;
        } else if (strcmp(subcmd, "assets") == 0) {
            if (!build_assets(&cmd, &procs)) return 1;
            if (!procs_flush(&procs)) return 1;
        } else if (strcmp(subcmd, "tests") == 0 || strcmp(subcmd, "test") == 0) {
            if (!build_tests(&cmd, &procs)) return 1;
            if (!procs_flush(&procs)) return 1;
            if (argc > 0) {
                cmd_append(&cmd, "./bin/test");
                da_append_many(&cmd, argv, argc);
                if (!cmd_run(&cmd)) return 1;
            }
        } else if (strcmp(subcmd, "demos") == 0) {
            if (argc <= 0) {
                if (!build_all_vc_demos(&cmd, &procs)) return 1;
                if (!procs_flush(&procs));
                return 0;
            }

            const char *name = shift(argv, argc);
            if (argc <= 0) {
                if (build_vc_demo(&cmd, &procs, name)) return 1;
                if (!procs_flush(&procs)) return 1;
                const char *src_path = temp_sprintf("./bin/%s.wasm", name);
                const char *dst_path = temp_sprintf("./wasm/%s.wasm", name);
                if (!copy_file(src_path, dst_path)) return 1;
                return 0;
            }

            const char *platform = shift(argv, argc);
            if (strcmp(platform, "sdl") == 0) {
                if (!build_sdl_demo(&cmd, &procs, name)) return 1;
                if (!procs_flush(&procs)) return 1;
                if (argc <= 0) return 0;
                const char *run = shift(argv, argc);
                if (strcmp(run, "run") != 0) {
                    usage(program);
                    nob_log(ERROR, "unknown action `%s` for SDL demo: %s", run, name);
                    return 1;
                }
                cmd_append(&cmd, temp_sprintf("./bin/%s.sdl", name));
                if (!cmd_run(&cmd)) return 1;
                return 0;
            } else if (strcmp(platform, "term") == 0) {
                if (!build_term_demo(&cmd, &procs, name)) return 1;
                if (!procs_flush(&procs)) return 1;
                if (argc <= 0) return 0;
                const char *run = shift(argv, argc);
                if (strcmp(run, "run") != 0) {
                    usage(program);
                    nob_log(ERROR, "unknown action `%s` for Terminal demo: %s", run, name);
                    return 1;
                }
                cmd_append(&cmd, temp_sprintf("./bin/%s.term", name));
                if (!cmd_run(&cmd)) return 1;
                return 0;
            } else if (strcmp(platform, "wasm") == 0) {
                if (!build_wasm_demo(&cmd, &procs, name)) return 1;
                if (!procs_flush(&procs)) return 1;
                const char *src_path = temp_sprintf("./bin/%s.wasm", name);
                const char *dst_path = temp_sprintf("./wasm/%s.wasm", name);
                if (!copy_file(src_path, dst_path)) return 1;
            } else {
                usage(program);
                nob_log(ERROR, "unknown demo platform %s", platform);
                return 1;
            }
        } else if(strcmp(subcmd, "help") == 0) {
            usage(program);
        } else {
            usage(program);
            nob_log(ERROR, "Unknown command `%s`", subcmd);
            return 1;
        }
    } else {
        if (!build_tools(&cmd, &procs)) return 1;
        if (!procs_flush(&procs)) return 1;

        if (!build_assets(&cmd, &procs)) return 1;
        if (!procs_flush(&procs)) return 1;

        if (!build_tests(&cmd, &procs)) return 1;
        if (!procs_flush(&procs)) return 1;

        if (!build_all_vc_demos(&cmd, &procs)) return 1;
        if (!procs_flush(&procs)) return 1;

        if (!copy_all_vc_demos_to_build()) return 1;
    }

    return 0;
}