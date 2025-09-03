{ pkgs ? import <nixpkgs> { config.allowUnfree = true; } }:
pkgs.clangStdenv.mkDerivation {
    name = "dev-env";
    src = null;
    nativeBuildInputs = with pkgs; [
        clang-tools
        gdb
    ];

    buildInputs = with pkgs; [
        glibc
        libGL
        glfw-wayland
        # glfw
        cglm
        assimp
        imgui
    ];
}
