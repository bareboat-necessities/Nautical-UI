# Windows packaging notes

Phase 1 targets MSYS2 UCRT64 builds.

The future package step should collect:

- `helm-ui.exe`
- required GTK/gtkmm DLLs from the UCRT64 runtime
- assets under `share/helm-ui`

Later phases can add a WiX/MSI or NSIS installer.
