argc=$#

usage_exit() {
    echo "Usage: utility.sh [help | build | test]\nnote: utility.sh build <preset> <generate-clangd-config>[1 | 0] [refresh]?";
    exit $1;
}

if [[ $argc -lt 1 ]]; then
    usage_exit 1;
fi

action="$1"
build_status=0

if [[ $action = "help" ]]; then
    usage_exit 0;
elif [[ $action = "build" && $argc -ge 2 && $argc -le 4 ]]; then
    if [[ $argc -eq 4 && "$4" = "refresh" ]]; then
        cmake --fresh -S . -B build --preset "$2" && cmake --build build;
        build_status=$?
    else
        cmake -S . -B build --preset "$2" && cmake --build build;
        build_status=$?
    fi

    if [[ $build_status -eq 0 && $3 -eq 1 ]]; then
        mv ./build/compile_commands.json .;
    fi
elif [[ $action = "test" && $argc -eq 1 ]]; then
    touch ./logs/all.txt;
    ctest --test-dir build --timeout 2 -V 1> ./logs/all.txt;

    if [[ $? -eq 0 ]]; then
        echo "All tests OK";
    else
        echo "Some tests failed: see logs";
    fi
else
    usage_exit 1;
fi
