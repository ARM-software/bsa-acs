if [ $(uname -m) != "aarch64" ] && [ -v $GCC49_AARCH64_PREFIX ]
then
    echo "GCC49_AARCH64_PREFIX is not set"
    echo "set using export GCC49_AARCH64_PREFIX=<lib_path>/bin/aarch64-linux-gnu-"
    return 0
fi

if [ "$1" == "ENABLE_OOB" ]; then
    build -a AARCH64 -t GCC49 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/bsa-acs/baremetal_app/BsaAcs.inf -D ENABLE_OOB
    return 0;
fi

    build -a AARCH64 -t GCC49 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/bsa-acs/uefi_app/BsaAcs.inf
