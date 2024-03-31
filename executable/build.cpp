#ifndef __BUILD_H__
#include "build.h"
#endif

target_config exe;
exe.target_name = "executable";
exe.type = executable;
exe.defines = {};
exe.link_dir = "bin";
exe.link_libs = { "shared_lib.lib" };
exe.include_dirs = relative_dirs("src", "../shared_lib/src");
exe.src_files = find_all_files("src", ".cpp");
exe.warnings_to_ignore = {};
exe.warning_level = 4;
exe.warnings_are_errors = true;
exe.subsystem = "console";

conf.targets.push_back(exe);