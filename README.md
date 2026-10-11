# tinyctl

Control center for the [tiny-suite](https://github.com/tiny-suite). It discovers every
component in the suite, shows what each one is, and builds, runs, tests, installs or
uninstalls them. No dependencies beyond libc.

## build

    make
    sudo make install

## use

Run it with no arguments to open the interactive tui:

    tinyctl

Or drive it from the command line:

    tinyctl list                 list components
    tinyctl info tinytop         show a component in detail
    tinyctl doctor               check that build dependencies are present
    tinyctl build tinytop        build a component
    tinyctl run tinyfetch        run a component
    tinyctl test tinymake        smoke test a component
    tinyctl install tinytop      install a component
    tinyctl uninstall tinytop    remove a component
    tinyctl where tinyctl        print a component's path
    tinyctl tui                  open the tui explicitly
    tinyctl help                 show this help
    tinyctl version              show the version

### tui keys

    j / down     move down
    k / up       move up
    b            build the selected component
    r            run the selected component
    t            test the selected component
    i            install the selected component
    u            uninstall the selected component
    d            run the dependency doctor
    q            quit

## finding the suite

tinyctl looks at every immediate subdirectory of the suite root and reads the
`component` file inside each one. The suite root defaults to the directory that
contains the tiny projects, and can be overridden:

    TINYSUITE_ROOT=/path/to/tiny-suite tinyctl list

## the component file

Every project in the suite ships a `component` file describing how to work with it.
It is a list of `key = value` pairs:

    name = tinytop
    summary = tiny process viewer, replaces top/htop
    build = make
    run = ./tinytop
    test = ./tinytop --once
    install = make install
    uninstall = make uninstall
    deps = cc

`name` and `summary` are shown in the ui. `build`, `run`, `test`, `install` and
`uninstall` are shell commands run from the component's directory. `deps` lists the
programs that must be on `PATH`, and is what `doctor` checks.
