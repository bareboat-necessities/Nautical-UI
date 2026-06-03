# Linux packaging notes

The GitHub Actions build now produces release artifacts for `amd64` and `arm64` Linux runners:

- `nautical-ui-<tag-or-sha>-linux-<arch>.tar.gz` with `bin/helm-ui`, assets, README, and license files.
- `nautical-ui_<deb-version>_<deb-arch>.deb` with the executable in `/usr/bin` and assets in `/usr/share/helm-ui`.

Packaging runs from the `linux-release` CMake preset, executes the release test preset, and computes Debian shared-library dependencies with `dpkg-shlibdeps` before building the package with `fakeroot dpkg-deb`.

Tags that match `v*` are published to a GitHub Release. The release job also uploads `Packages`, `Packages.gz`, and `Release` metadata, then mirrors those APT repository files to a stable release tag named `apt`.

Manual local build:

```bash
cmake --preset linux-release
cmake --build --preset linux-release --parallel
ctest --preset linux-release
cmake --install build/linux-release --prefix /opt/helm-ui
```
