{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  } @ inputs:
    flake-utils.lib.eachDefaultSystem (
      system: let
        pkgs = nixpkgs.legacyPackages.${system};
        pkgsLLVM = pkgs.pkgsLLVM;

        # ucrt64 = pkgs.pkgsCross.ucrt64;
        # libcxx = llvm.llvmPackages_21.libcxx;
        # libcxx-dev = libcxx.dev;
        # clang = pkgs.llvmPackages_21.clang-unwrapped;
        # bintools = pkgs.llvmPackages_21.bintools-unwrapped;

        llvmCross = cross: let
          llvmPackages = cross.llvmPackages_21;
        in
          llvmPackages
          // {
            mkDerivation = llvmPackages.stdenv.mkDerivation;
            mkShell = cross.mkShell.override {stdenv = llvmPackages.stdenv;};
          };

        nativeBuildInputs = cross: let
          llvm = llvmCross cross;
        in [
          llvm.libllvm
          llvm.clang-tools
          llvm.lldb
          pkgs.cmake
          pkgs.cmake-language-server
          pkgs.ninja
          pkgs.vcpkg
        ];

        buildInputs = cross: let
          llvm = llvmCross cross;
        in [
          llvm.libclang
          llvm.libllvm
        ];

        vcpkgRoot = "${pkgs.vcpkg}/share/vcpkg";
      in {
        devShells = rec {
          default = gcc;

          gcc = (llvmCross pkgs).mkShell {
            packages = (nativeBuildInputs pkgs) ++ [];
            buildInputs = (buildInputs pkgs) ++ [];
            env = {
              VCPKG_ROOT = vcpkgRoot;
            };
            hardeningDisable = ["all"];
          };

          llvm = (llvmCross pkgsLLVM).mkShell {
            packages = (nativeBuildInputs pkgsLLVM) ++ [];
            buildInputs = (buildInputs pkgsLLVM) ++ [];
            env = {
              VCPKG_ROOT = vcpkgRoot;
            };
          };

          # mingw = ucrt64.pkgsLLVM.mkShell {
          #   # buildInputs = [
          #   #   ucrt64.pkgsLLVM.windows.mingw_w64
          #   # ];
          #   packages = [
          #     ninja
          #     cmake
          #     cmake-language-server
          #     # pkgs.llvmPackages_21.clang-tools
          #     (pkgs.llvmPackages_21.clang-tools.override {
          #       stdenv = ucrt64.pkgsLLVM.stdenv;
          #       enableLibcxx = true;
          #     })
          #     vcpkg
          #   ];
          #   env = {
          #     VCPKG_ROOT = "${vcpkg}/share/vcpkg";
          #   };
          # };
          #
          # msvc = mkShellNoCC {
          #   packages = [
          #     clang
          #     bintools
          #     # libcxx
          #     xwin
          #
          #     ninja
          #     cmake
          #     cmake-language-server
          #     # clang-tools
          #     vcpkg
          #   ];
          #   env = {
          #     # LIBCXX_INCLUDE = "${libcxx-dev}/include/c++/v1";
          #     # LIBCXX_LIB = "${libcxx}/lib";
          #     VCPKG_ROOT = "${vcpkg}/share/vcpkg";
          #   };
          # };
        };
      }
    );
}
