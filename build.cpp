#include "build.h"

int main(int argc, char** argv) {
    for (int n = 1; n < argc; n++) {
        printf("argv[%d] = '%s'\n", n, argv[n]);
    }
    printf("--------------------------------\n");

    project_config conf;
    conf.project_name = "test";
    conf.cpp_standard = 14;
    conf.bin_dir = ".\\bin";
    conf.obj_dir = ".\\bin\\int";
    conf.debug_build = true;
    conf.static_link_std = true;
    conf.opt_level = 0;
    conf.opt_intrinsics = true;
    conf.generate_debug_info = true;
    conf.incremental_link = false;
    conf.remove_unref_funcs = true;

    #include "Engine/build_engine_dll.cpp"
    #include "RohinApp/build_rohinapp_exe.cpp"
    #include "Game/build_game_dll.cpp"

    return build_project(conf);
}