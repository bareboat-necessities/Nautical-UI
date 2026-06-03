# Linux packaging notes

Phase 1 does not include a distro package. The project installs with CMake:

```bash
cmake --preset linux-release
cmake --build --preset linux-release
cmake --install build/linux-release --prefix /opt/helm-ui
```

Later phases can add a `.deb`, AppImage, or Flatpak manifest.
