#ifndef __BUILD_H__
#include "build.h"
#endif

target_config lib;
lib.target_name = "shared_lib";
lib.type = shared_lib;
lib.defines = {"RH_EXPORT"};
lib.link_dir;
lib.link_libs = {};
lib.include_dirs = relative_dirs("src");
lib.src_files = find_all_files("src", ".cpp");
lib.warnings_to_ignore = {};
lib.warning_level = 4;
lib.warnings_are_errors = true;
lib.subsystem = "console";

conf.targets.push_back(lib);