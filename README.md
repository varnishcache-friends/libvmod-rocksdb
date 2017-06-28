libvmod-leveldb
===============

[![Gitter](https://badges.gitter.im/Join Chat.svg)](https://gitter.im/fgsch/libvmod-leveldb?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

## Building

Install dependencies as listed under `Build-Depends` in `debian/control`:

Then you can build a Debian package from this repository directly. Brief
instructions are:

    $ git submodule update --init
    $ debian/rules vcs-mk-origtargz # if it doesn't already exist
    $ debuild -uc -us

For more details, including more fine-grained tools you can use to take you
through individual parts of the build process, see the Debian New Maintainer's
Guide, Debian Developers' Reference, and various online documentation on
debhelper and javahelper. See `man fhs` and the Debian Java FAQ for a guide to
the layout of the installed files.

