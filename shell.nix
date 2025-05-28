{ pkgs ? import <nixpkgs> { config.allowUnfree = true; } }:
pkgs.clangStdenv.mkDerivation {
    name = "dev-env";
    src = null;
    nativeBuildInputs = with pkgs; [
        clang-tools
    ];

    # any libraries used go here
    # so their headers can be recognized by the lsp
    buildInputs = with pkgs; [
        direnv
        libGL
        glfw-wayland
        cglm
        assimp
        gdb
    ];
}
