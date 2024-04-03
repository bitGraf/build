# build
C++ build system (written and configured in C++)

I got tired of trying to configure CMake and thought "If I'm writing a project in C++, why can't I configure my build system in C++?", so I started this little build tool. I know I'm probably not the first to have this idea, and I don't intend anyone to actually use this, this is just made as a fun little experiment, and a replacement for all the `build.bat` files I write.

# Usage
The tool works by creating a 'build script' that includes ```build.h```, defines a project and some targets (à la CMake) and then executing the build command.
A basic build-script looks like:

```c++
#include "build.h"

int main(int argc, char* argv[]) {
    // checks if this file (or build.h) is out of date, and
    // spawns a process to rebuild the build tool
    auto_rebuild_self(argc, argv);

    project_config proj;
    // set 'project' level settings here
    // ...

    target_config targ;
    // set 'target' level settings here
    // ...

    // add any number of targets to a project config
    // note: they get executed in the order you add them!
    proj.targets.push_back(targ);

    // build the actual project and return any error-codes
    return build_project(proj);
}

```

When you first create the build-script, you need to manually compile it to bootstrap the build system essentially.

```
cl.exe build.cpp
```

(or, optionally if you wanna suppress some warnings...)

```
cl.exe build.cpp -EHsc
```

After this first manual compile, you should have an executable called `build.exe` that if you run, will compile the actual project you configure. if you include `auto_rebuild_self(argc, argv);` at the start of your main function, you don't even need to rebuild build.cpp if you redefine your project configuration, as it will rebuild itself.

# Todo
* clean up the difference between 'project-level' and 'target-level' options
* improve the functionality of incremental_build
* maybe allow build.exe to take flags when run (e.g. build.exe debug x64)?