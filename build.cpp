#include "build.h"

int main(int argc, char** argv) {
    for (int n = 1; n < argc; n++) {
        printf("argv[%d] = '%s'\n", n, argv[n]);
    }
    printf("--------------------------------\n");

    auto_rebuild_self(argc, argv);

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

    #include "shared_lib/build.cpp"
    #include "executable/build.cpp"

    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);

    QueryPerformanceCounter(&start);
    int err_code;
    //err_code = build_project(conf);
    err_code = build_project_incremental(conf);
    QueryPerformanceCounter(&end);

    double elapsed = (double)(end.QuadPart - start.QuadPart) / (double)(freq.QuadPart) * 1000.0;
    if (elapsed > 2000.0)
        printf("%.2f sec elapsed.\n", elapsed/1000.0);
    else
        printf("%.3f ms elapsed.\n", elapsed);

    return err_code;
}
