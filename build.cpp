#include <iostream>
#include <string>
#include <vector>

/* a `project` is an overall collection of targets with common settings 
 * lots of settings will be project-global for now.
 */
struct target_config;
struct project_config {
    std::string project_name = "[project]";
    unsigned int cpp_standard = 14; // c++14
    std::string bin_dir = "./bin/";
    std::string obj_dir = "./bin/int/";

    unsigned int warning_level = 0;
    bool warnings_are_errors = false;

    std::vector<int> common_warnings_to_ignore = {4189, 4201, 4100, 4101};
    std::vector<std::string> common_defines = {"_CRT_SECURE_NO_WARNINGS"};

    bool debug_build = false;
    bool static_link_std = false;
    unsigned int opt_level = 0;
    bool opt_intrinsics = false;
    bool generate_debug_info = true;
    bool incremental_link = false;
    bool remove_unref_funcs = true;
    std::string subsytem;

    std::vector<target_config> targets;
};

struct target_config {
    std::string target_name = "[target]";

    std::vector<int> warnings_to_ignore;
    std::vector<std::string> defines;
    std::vector<std::string> include_dirs;
    std::vector<std::string> src_files;
};

std::string generate_target_build_cmd(const project_config& conf, const target_config& targ) {
    std::string cmd = "";


    return cmd;
}

void build_project(const project_config& conf) {
    int num_targets = conf.targets.size();
    printf("Project [%s]: %d targets.\n", conf.project_name.c_str(), num_targets);

    for (int n = 0; n < num_targets; n++) {
        const target_config& targ = conf.targets[n];

        printf("  Generating build_cmd...");
        std::string cmd = generate_target_build_cmd(conf, targ);
        printf("Done.\n");

        printf("    Building [%s]...", targ.target_name.c_str());
        //system(cmd);
        printf("Done.\n");
    }
}

int main(int argc, char** argv) {
    for (int n = 1; n < argc; n++) {
        printf("argv[%d] = '%s'\n", n, argv[n]);
    }
    printf("--------------------------------\n");

    project_config conf;
    conf.project_name = "test";
    conf.cpp_standard = 14;
    conf.bin_dir = "./";
    conf.obj_dir = "./";
    conf.warning_level = 4;
    conf.warnings_are_errors = true;
    conf.common_warnings_to_ignore = {4189, 4201, 4100, 4101};
    conf.debug_build = true;
    conf.static_link_std = true;
    conf.opt_level = 0;
    conf.opt_intrinsics = true;
    conf.generate_debug_info = true;
    conf.incremental_link = false;
    conf.remove_unref_funcs = true;
    conf.subsytem = "console";

    target_config target;
    target.target_name = "targ";
    target.defines = {"_DEBUG"};
    target.include_dirs = { "./" };
    target.src_files = { "targ.cpp" };
    conf.targets.push_back(target);

    build_project(conf);

    return 0;

    // user-facing settings
    std::string output_name = "hehe";
    int warning_level = 4;
    bool warnings_are_errors = true;
    std::vector<int> ignore_warnings = {4189, 4201, 4100, 4101};
    std::vector<std::string> defines = { "_DEBUG", "_CRT_SECURE_NO_WARNINGS" };
    bool debug_build = true;
    bool static_link_std = true;
    int cpp_standard = 14;
    int opt_level = 0;
    bool opt_intrinsics = true;
    bool generate_debug_info = true;
    std::vector<std::string> include_dirs = {"src", "deps/math_lib/include"};
    bool incremental_link = false;
    bool remove_unref_funcs = true;
    std::string subsystem = "console";
    std::string obj_dir = "./";
    std::vector<std::string> src_files = {"proj.cpp"};

    // start building options into flag strings 
    std::string default_flags = "/nologo /Gm- /GR- /EHa- /FC ";

    std::string std_cmd = "/std:c++" + std::to_string(cpp_standard) + " ";

    std::string opt_cmd = "/O";
    if (opt_level == 0) opt_cmd += "d";
    else                opt_cmd += std::to_string(opt_level);

    if (opt_intrinsics) opt_cmd += "i";

    opt_cmd += " ";

    std::string msvc_link = "/M";
    if (static_link_std) msvc_link += "T";
    else                 msvc_link += "D";
    if (debug_build)     msvc_link += "d";
    msvc_link += " ";

    std::string compile_flags = default_flags + msvc_link + opt_cmd + std_cmd;
    if (generate_debug_info) compile_flags += "/Z7 ";



    std::string link_flags = "/link ";
    
    if (incremental_link) link_flags += "/INCREMENTAL ";
    else                  link_flags += "/INCREMENTAL:NO ";

    if (remove_unref_funcs) link_flags += "/OPT:REF ";
    else                    link_flags += "/OPT:NOREF ";

    for (auto & c: subsystem) c = toupper(c);
    link_flags += "/SUBSYSTEM:" + subsystem + " ";


    // assemble full command
    // cl %IncludeDirs% %CompilerFlags% %SrcFiles% /Fe: %OutputName% /Fo: %obj_dir% /link %LinkerFlags%
    std::string compile_cmd = "cl.exe ";
    for (auto s : include_dirs) {
        compile_cmd += "/I" + s + " ";
    }

    compile_cmd += compile_flags;

    switch (warning_level) {
        case 0: compile_cmd += "/W0 ";
        case 1: compile_cmd += "/W1 ";
        case 2: compile_cmd += "/W2 ";
        case 3: compile_cmd += "/W3 ";
        case 4: compile_cmd += "/W4 ";
    }

    if (warnings_are_errors) {
        compile_cmd += "/WX ";
    }

    for (auto w : ignore_warnings) {
        compile_cmd += "/wd" + std::to_string(w) + " ";
    }

    for (auto d : defines) {
        compile_cmd += "/D" + d + " ";
    }

    for (auto s : src_files) {
        compile_cmd += s + " ";
    }

    compile_cmd += "/Fe: ./" + output_name + ".exe ";
    compile_cmd += "/Fo: " + obj_dir + " ";

    compile_cmd += link_flags;

    printf("cmd: '%s'\n", compile_cmd.c_str());

    //system(compile_cmd.c_str());

    return 0;
}