{ pkgs ? import <nixpkgs> {} }:
  let 
    my-python = pkgs.python3;
    # jinja2 is required to build Coyote. 
    python-with-my-packages = my-python.withPackages (p: with p; 
    [
      jinja2
    ]);
  in
  pkgs.mkShell {
    # nativeBuildInputs is usually what you want -- tools you need to run
    nativeBuildInputs = with pkgs.buildPackages; [ 
      linuxPackages_latest.kernel.dev
      cmake
      boost
      coreutils
      git
      nasm
      binutils
      usbutils
      pciutils

      python3
      python-with-my-packages
    ];

    shellHook = ''
      export NIXKERNEL=$(nix-build -E '(import <nixpkgs> {}).linuxPackages_latest.kernel.dev' --no-out-link);
      export PYTHONPATH=${python-with-my-packages}/${python-with-my-packages.sitePackages}
    '';
}

