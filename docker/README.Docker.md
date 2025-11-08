# TOPAS Docker

This repository provides Docker recipes and helper scripts for (re)building and running TOPAS (OpenTOPAS v4.1.1, Geant4 11.1.3, GDCM 2.6.8) on macOS or Linux. Use it to keep Geant4 data outside the container while iterating on parameter files, extensions, or visualization workflows.

## Run Simulations
Mount your Geant4 data and simulation files from the host:
```bash
./topas-docker -g4data=$HOME/Applications/G4Data OneBox.txt
```
- `/simulations` maps to the directory where you run the command (or `-data=PATH`).
- `/Applications/G4Data` is mounted read-only from `-g4data=/path/to/G4DATA`.
Headless operation (default on macOS and whenever you skip `--x11`) is handled by the container entrypoint, which launches `Xvfb` internally so you can still capture scorers written under `/simulations/output`. If you need a different virtual display, set `TOPAS_XVFB_DISPLAY` or `TOPAS_XVFB_SCREEN` before running `topas-docker`.

### Building & Reusing Extensions
**Initial build**  

1. **Add** `-extensions=/path/to/extensions --build-extensions` to compile custom extension; the helper caches them under `~/.cache/topas-docker/<ext>/install` (mounted inside the container as `/extensions/install`) so later runs automatically use the cached TOPAS build.

   ```bash
   ./topas-docker \
     -extensions=$HOME/Applications/TOPAS/Extensions/TOPAS-nBio \
     --build-extensions \
     -g4data=$HOME/Applications/G4Data \
     MySimulation.txt
   ```  
   The helper mounts your source tree at `/extensions/src`, builds inside an isolated cache, and installs the resulting TOPAS binaries under `~/.cache/topas-docker/<ext-name>-<hash>/install`.
2. **Re-use cache** — Subsequent runs can omit `--build-extensions`; the launcher detects `/extensions/install/bin/topas`, prepends it to `PATH`, and reuses the cached build automatically.
3. **Force rebuild** — Pass `--build-extensions` again (or delete the cache directory) when you change extension sources. The cache lives under `${TOPAS_DOCKER_CACHE:-~/.cache/topas-docker}`, so you can purge a specific build with `rm -rf ~/.cache/topas-docker/TOPAS-nBio-*`.
4. **Custom cache location** — Set `TOPAS_DOCKER_CACHE=/path/to/cache` before running if you want the build artifacts on another disk.
5. **Troubleshooting** — If TOPAS reports outdated symbols, make sure the cache is rebuilt (step 3) and that the final argument to `topas-docker` is the parameter file rather than another command.

## macOS X11 / Qt Visualization
1. Install [XQuartz](https://www.xquartz.org/), enable **Preferences ▸ Security ▸ Allow connections from network clients**, then run:
   ```bash
   defaults write org.xquartz.X11 enable_iglx -bool true
   ```
   Quit and reopen XQuartz so GLX forwarding is active.
2. Permit Docker Desktop’s Virtual Machine to reach XQuartz:
   ```bash
   xhost +127.0.0.1
   ```
3. Launch TOPAS with GUI:
   ```bash
   ./topas-docker --x11 --display=host.docker.internal:0 \
       -g4data=$HOME/Applications/G4Data MySimulation.txt
   ```
   The launcher exports `LIBGL_ALWAYS_INDIRECT=1` so OpenGL renders through XQuartz.
4. **When finished, revoke access**: `xhost -127.0.0.1`.

Use `-display=<host:screen>` if you run a non-default XQuartz display or forward from another host.

## Linux Visualization
Native X11 is enabled automatically. Before running, allow the Docker user with:
```bash
xhost +local:docker
./topas-docker -g4data=/path/to/G4DATA MyCase.txt
xhost -local:docker
```
When `topas-docker` detects a usable host display it sets to the image `TOPAS_USE_HOST_DISPLAY=1`, so the container reuses your X server instead of starting its own `Xvfb`. For ad-hoc runs without the helper script, pass the same flag yourself:
```bash
docker run --rm -it \
  -e TOPAS_USE_HOST_DISPLAY=1 \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
  -v $HOME/Applications/G4Data:/Applications/G4Data:ro \
  -v $PWD:/simulations \
  opentopas/opentopas:v4.1.1 GuiCase.txt
```

## Directory Expectations Inside the Image
```
/
 ├─ /Applications/G4Data         # mount point for host Geant4 datasets
 ├─ /Applications/TOPAS
 │    └─ OpenTOPAS-install       # TOPAS installation
 ├─ /Applications/GEANT4
 │    └─ geant4-install          # Geant4 installation
 └─ /simulations                 # mount point for simulation output files
 └─ /extensions                  # mount point for extension files 
 
```

Outputs (e.g., CSV scorers) land under your host directory mapped to `/simulations`. For frequently asked questions see `FAQ.md`.

