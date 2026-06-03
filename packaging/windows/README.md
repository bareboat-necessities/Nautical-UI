# Windows packaging notes

The GitHub Actions build now produces a portable Windows x64 zip from MSYS2 UCRT64:

- `nautical-ui-<tag-or-sha>-windows-x64.zip`
- `helm-ui.exe`
- recursively discovered UCRT64 DLL dependencies
- GLib/GDBus spawn helpers
- GTK/GLib schema, pixbuf loader, icon, and theme runtime data
- `share/helm-ui` assets
- `run.bat`, which keeps bundled DLLs first on `PATH` and sets the GTK runtime environment variables

Packaging runs from the `windows-msys2-ucrt64-release` CMake preset and executes the release test preset before creating the zip.

Manual local build from an MSYS2 UCRT64 shell:

```bash
cmake --preset windows-msys2-ucrt64-release
cmake --build --preset windows-msys2-ucrt64-release --parallel
ctest --preset windows-msys2-ucrt64-release
```
