#ifndef __BUILD_H__
#define __BUILD_H__

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdarg>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlobj_core.h>

#pragma comment( lib, "Shell32" )

std::string ReadFromPipe(HANDLE read_from);
int run_command(const std::string& cmd, std::string& std_out, std::string& std_err);

struct target_config;

/* a `project` is an overall collection of targets with common settings 
 * lots of settings will be project-global for now.
 */
struct project_config {
    std::string project_name;
    unsigned int cpp_standard;
    std::string bin_dir;
    std::string obj_dir;

    std::vector<std::string> common_defines = {"_CRT_SECURE_NO_WARNINGS"};

    bool debug_build = false;
    bool static_link_std = false;
    unsigned int opt_level = 0;
    bool opt_intrinsics = false;
    bool generate_debug_info = true;
    bool incremental_link = false;
    bool remove_unref_funcs = true;

    std::vector<target_config> targets;
};

enum target_type {
    executable = 0,
    shared_lib,
    static_lib
};

struct target_config {
    std::string target_name = "[target]";
    target_type type;

    unsigned int warning_level = 0;
    bool warnings_are_errors = false;
    std::vector<int> warnings_to_ignore;
    std::vector<std::string> defines;
    std::vector<std::string> include_dirs;
    std::vector<std::string> src_files;
    std::vector<std::string> link_libs;
    std::string link_dir = "";
    std::string subsystem = "console";
};

std::string generate_target_build_cmd(const project_config& conf, const target_config& targ) {
    // start building options into flag strings 
    std::string default_flags = "/nologo /Gm- /GR- /EHa- /FC ";

    std::string std_cmd = "/std:c++" + std::to_string(conf.cpp_standard) + " ";

    std::string opt_cmd = "/O";
    if (conf.opt_level == 0) opt_cmd += "d";
    else                     opt_cmd += std::to_string(conf.opt_level);

    if (conf.opt_intrinsics) opt_cmd += "i";

    opt_cmd += " ";

    std::string msvc_link = "/M";
    if (conf.static_link_std) msvc_link += "T";
    else                      msvc_link += "D";
    if (conf.debug_build)     msvc_link += "d";
    msvc_link += " ";

    std::string compile_flags = default_flags + msvc_link + opt_cmd + std_cmd;
    if (conf.generate_debug_info) compile_flags += "/Z7 ";
    if (targ.type == shared_lib) compile_flags += "/LD ";



    std::string link_flags = "/link ";
    
    if (conf.incremental_link) link_flags += "/INCREMENTAL ";
    else                       link_flags += "/INCREMENTAL:NO ";

    if (conf.remove_unref_funcs) link_flags += "/OPT:REF ";
    else                         link_flags += "/OPT:NOREF ";

    std::string subsystem = targ.subsystem;
    for (auto & c: subsystem) c = toupper(c);
    link_flags += "/SUBSYSTEM:" + subsystem + " ";

    if (targ.link_dir.size()) link_flags += "/LIBPATH:\"" + targ.link_dir + "\" ";

    for (auto l : targ.link_libs) {
        link_flags += l + " ";
    }

    // assemble full command
    // cl %IncludeDirs% %CompilerFlags% %SrcFiles% /Fe: %OutputName% /Fo: %obj_dir% /link %LinkerFlags%
    std::string compile_cmd = "cl.exe ";
    for (auto s : targ.include_dirs) {
        compile_cmd += "/I" + s + " ";
    }

    compile_cmd += compile_flags;

    switch (targ.warning_level) {
        case 0: compile_cmd += "/W0 "; break;
        case 1: compile_cmd += "/W1 "; break;
        case 2: compile_cmd += "/W2 "; break;
        case 3: compile_cmd += "/W3 "; break;
        case 4: compile_cmd += "/W4 "; break;
    }

    if (targ.warnings_are_errors) {
        compile_cmd += "/WX ";
    }

    for (auto w : targ.warnings_to_ignore) {
        compile_cmd += "/wd" + std::to_string(w) + " ";
    }

    for (auto d : conf.common_defines) {
        compile_cmd += "/D" + d + " ";
    }
    for (auto d : targ.defines) {
        compile_cmd += "/D" + d + " ";
    }

    for (auto s : targ.src_files) {
        compile_cmd += s + " ";
    }


    compile_cmd += "/Fe: " + conf.bin_dir + "\\" + targ.target_name + " ";
    compile_cmd += "/Fo: " + conf.obj_dir + "\\ ";

    compile_cmd += link_flags;

    return compile_cmd;
}

void ensure_output_dirs(const project_config& conf) {
    char full_path[MAX_PATH];
    WIN32_FIND_DATA data;
    HANDLE hFind;

    // bin dir
    GetFullPathNameA(conf.bin_dir.c_str(), MAX_PATH, full_path, NULL);
    hFind = FindFirstFile(full_path, &data);
    if (hFind == INVALID_HANDLE_VALUE) {
        //CreateDirectory(full_path, NULL);
        SHCreateDirectoryExA(NULL, full_path, NULL);
    }

    // obj dir
    GetFullPathNameA(conf.obj_dir.c_str(), MAX_PATH, full_path, NULL);
    hFind = FindFirstFile(full_path, &data);
    if (hFind == INVALID_HANDLE_VALUE) {
        //CreateDirectory(full_path, NULL);
        SHCreateDirectoryExA(NULL, full_path, NULL);
    }

    bool done = true;
}

std::string ReadFromPipe(HANDLE read_from)  { 
    const size_t BUF_SIZE = 2048;

    // Read output from the child process's pipe for STDOUT
    // and write to the parent process's pipe for STDOUT. 
    // Stop when there is no more data. 
    DWORD dwRead; 
    CHAR chBuf[BUF_SIZE]; 
    BOOL bSuccess = FALSE;

    std::string output;
    for (;;) { 
        bSuccess = ReadFile( read_from, chBuf, BUF_SIZE, &dwRead, NULL);
        if( ! bSuccess || dwRead == 0 ) break; 

        output += std::string(chBuf, dwRead);
    } 

    return output;
} 

int run_command(const std::string& cmd, std::string& std_out, std::string& std_err) {
    HANDLE STDOUT_Read  = NULL;
    HANDLE STDOUT_Write = NULL;
    HANDLE STDERR_Read  = NULL;
    HANDLE STDERR_Write = NULL;

    // Set the bInheritHandle flag so pipe handles are inherited. 
    SECURITY_ATTRIBUTES saAttr; 
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
    saAttr.bInheritHandle = TRUE; 
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT. 
    if (!CreatePipe(&STDOUT_Read, &STDOUT_Write, &saAttr, 0)) {
        return -1;
    }
    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if (!SetHandleInformation(STDOUT_Read, HANDLE_FLAG_INHERIT, 0)) {
        return -1;
    }

    // Create a pipe for the child process's STDERR. 
    if (!CreatePipe(&STDERR_Read, &STDERR_Write, &saAttr, 0)) {
        return -1;
    }
    // Ensure the read handle to the pipe for STDERR is not inherited.
    if (!SetHandleInformation(STDERR_Read, HANDLE_FLAG_INHERIT, 0)) {
        return -1;
    }

    // Configure the child process
    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE; 
 
    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
 
    // Set up members of the STARTUPINFO structure. 
    // This structure specifies the STDIN and STDOUT handles for redirection.
    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
    siStartInfo.cb = sizeof(STARTUPINFO); 
    siStartInfo.hStdError = STDERR_Write;
    siStartInfo.hStdOutput = STDOUT_Write;
    siStartInfo.hStdInput = NULL;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process. 
    char* cmd_line = (char*)malloc(cmd.size() + 1);
    strcpy(cmd_line, cmd.c_str());
    cmd_line[cmd.size()] = 0;
    bSuccess = CreateProcess(NULL, 
                             cmd_line,      // command line 
                             NULL,          // process security attributes 
                             NULL,          // primary thread security attributes 
                             TRUE,          // handles are inherited 
                             0,             // creation flags 
                             NULL,          // use parent's environment 
                             NULL,          // use parent's current directory 
                             &siStartInfo,  // STARTUPINFO pointer 
                             &piProcInfo);  // receives PROCESS_INFORMATION 

    // If an error occurs, exit the application. 
    if (!bSuccess) {
        return -1;
    } else {
        // Close handles to the child process and its primary thread.
        // Some applications might keep these handles to monitor the status
        // of the child process, for example. 
        //CloseHandle(piProcInfo.hProcess);
        //CloseHandle(piProcInfo.hThread);
      
        // Close handles to the stdin and stdout pipes no longer needed by the child process.
        // If they are not explicitly closed, there is no way to recognize that the child process has ended.
        CloseHandle(STDOUT_Write);
        CloseHandle(STDERR_Write);
    }

    std_out = ReadFromPipe(STDOUT_Read);
    std_err = ReadFromPipe(STDERR_Read);

    // get the error code once its done
    DWORD exit_code;
    GetExitCodeProcess(piProcInfo.hProcess, &exit_code);
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    return exit_code;
}

int build_project(const project_config& conf) {
    int num_targets = conf.targets.size();
    printf("Project [%s]: %d targets.\n", conf.project_name.c_str(), num_targets);

    ensure_output_dirs(conf);

    for (int n = 0; n < num_targets; n++) {
        const target_config& targ = conf.targets[n];

        printf("  Generating build_cmd...");
        std::string cmd = generate_target_build_cmd(conf, targ);
        printf("Done.\n");

        printf("    Building [%s]...", targ.target_name.c_str());
        //int res = system(cmd.c_str());

        std::string std_out, std_err;
        int res = run_command(cmd, std_out, std_err);

        if (res) {
            printf("Failed! ErrorCode: %d\n", res);
            printf("%s\n", std_out.c_str());
            return res;
        }

        printf("Done.\n");
    }

    return 0;
}

char space_buf[] = "                                                                      ";
bool get_all_files_in_dir(std::vector<std::string>& files, const std::string& dir, const std::vector<std::string>& ext_types, int level) {
    std::string search = dir + "\\*";
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile(search.c_str(), &data);  // finds .
    FindNextFile(hFind, &data); // finds ..

    while (FindNextFile(hFind, &data)) {
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            //printf("%.*sDir:  [%s]\n", 2*level, space_buf, data.cFileName);

            std::string new_dir = dir + "\\" + std::string(data.cFileName);
            get_all_files_in_dir(files, new_dir, ext_types, level + 1);
        } else {
            std::string filename(data.cFileName);

            bool valid_ending = false;
            for (const auto& ending : ext_types) {
                if (std::equal(ending.rbegin(), ending.rend(), filename.rbegin())) {
                    valid_ending = true;
                    break;
                }
            }
            if (valid_ending) {
                std::string full_filename = dir + "\\" + filename;
                //printf("%.*sFile: [%s] {%u}\n", 2 * level, space_buf, full_filename.c_str(), data.dwFileAttributes);

                files.push_back(full_filename);
            }
        }
    }

    return true;
}

#define find_all_files(folder, ext_types) __find_all_files(__FILE__, folder, ext_types)
std::vector<std::string> __find_all_files(const char* calling_file, const std::string& folder, std::string extension_list) {
    // parse the extension list into individual strings
    // assume its a comma-separated list ".ext1,.ext2"
    std::vector<std::string> ext_types;
    size_t n2 = extension_list.find_first_of(',');
    while (n2 != std::string::npos) {
        std::string ext = extension_list.substr(0, n2);
        ext_types.push_back(ext);
        //printf("extension: [%s]\n", ext.c_str());

        extension_list = extension_list.substr(n2+1, extension_list.length()-n2-1);
        n2 = extension_list.find_first_of(',');
    }
    std::string ext = extension_list;
    ext_types.push_back(ext);

    // get the base directory from calling_file (which includes the filename)
    // we do this by finding the last '\'
    std::string base_dir(calling_file);
    std::replace(base_dir.begin(), base_dir.end(), '/', '\\');
    auto last = base_dir.find_last_of('\\');
    base_dir = base_dir.substr(0, last+1);

    //printf("base_dir: %s\n", base_dir.c_str());
    std::vector<std::string> files;

    std::string search = base_dir + folder;
    //printf("search:   %s\n", search.c_str());

    get_all_files_in_dir(files, search, ext_types, 0);
    //printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    //for (const auto& a : files) {
    //    printf("File: [%s]\n", a.c_str());
    //}
    //printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");

    return files;
}

#define TO_STR_VECTOR(...) std::vector < std::string > {__VA_ARGS__}
#define relative_dirs(...) __get_relative_dirs(__FILE__, TO_STR_VECTOR(__VA_ARGS__))
std::vector<std::string> __get_relative_dirs(const char* calling_file, std::vector<std::string> dirs) {
    // get the base directory from calling_file (which includes the filename)
    // we do this by finding the last '\'
    std::string base_dir(calling_file);
    std::replace(base_dir.begin(), base_dir.end(), '/', '\\');
    auto last = base_dir.find_last_of('\\');
    base_dir = base_dir.substr(0, last+1);

    std::vector<std::string> full_dirs;
    for (const auto& d : dirs) {
        std::string full = base_dir + d; // base_dir has a trailing slash!
        full_dirs.push_back(full);
    }

    return full_dirs;
}

#endif