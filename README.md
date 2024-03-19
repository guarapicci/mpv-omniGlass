# mpv-omniGlass: touchpad gestures for mpv using libomniglass

## now available on the Arch User Repository!
grab a PKGBUILD at [the repository](https://aur.archlinux.org/packages/mpv-omniglass-git).
## status: v0.2, supports two gestures.
- **slide on bottom edge:** seek.
- **slide on right edge:** volume control.

https://github.com/guarapicci/mpv-omniGlass/assets/143821200/d76cfa4c-902c-4cea-83ea-c51126f85f82

## how to use:
- 1: install [libomniglass](https://github.com/guarapicci/omniGlass) (currently linux only) and specify your touchpad at the config file.
- 2: run the install.sh script. Requires GCC and the headers for libmpv and omniglass.
- 3: the plugin is now installed as "omniglass.so" at the default scripts folder. drag your finger on the bottom edge of the touchpad to seek.
