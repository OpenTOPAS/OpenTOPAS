# TOPAS Docker

This repository provides Docker recipes and helper scripts for (re)building and running TOPAS (OpenTOPAS v4.1.1, Geant4 11.1.3, GDCM 2.6.8) on macOS or Linux. Use it to keep Geant4 data outside the container while iterating on parameter files, extensions, or visualization workflows.

## Run Simulations
Mount your Geant4 data and simulation files from the host:
```bash
./topas-docker -g4data=$HOME/Applications/G4Data OneBox.txt
```
- `/simulations` maps to the directory where you run the command (or `-data=PATH`).
- `/Applications/G4Data` is mounted read-only from `-g4data=/path/to/G4DATA`.
Headless operation (default on macOS and whenever you skip `--x11`) is handled by the container entrypoint, which launches `Xvfb` internally so you can still save scorers output in `/simulations/output`. If you need a different virtual display, set `TOPAS_XVFB_DISPLAY` or `TOPAS_XVFB_SCREEN` before running `topas-docker`.

### Apptainer / HPC Environments
For clusters that do not allow Docker, use the following headless script:
```bash
./topas-apptainer -g4data=$HOME/G4Data OneBox.txt
```
- It wraps `apptainer run` (or `singularity run`) against the same container image via `docker://opentopas/opentopas:v4.1.1`.
- It uses the identical `-data`, `-extensions`, `-g4data`, and `--build-extensions` flags, including cached builds under `~/.cache/topas-docker`.
- It runs without X11 forwarding (relies on the image’s internal Xvfb), making it good for HPC runs. 

**Initial build with extensions (Apptainer)**
```bash
./topas-apptainer \
  -extensions=$HOME/Applications/TOPAS/Extensions/TOPAS-nBio \
  --build-extensions \
  -g4data=$HOME/G4Data \
  MySimulation.txt
```
The helper binds `/extensions/src`, `/extensions/build`, and `/extensions/install` exactly like the Docker one, so cached installs are stored on the host so that later runs can skip `--build-extensions`.

### Building & Reusing Extensions
**Initial build**  

1. **Add** `-extensions=/path/to/extensions --build-extensions` to compile user-defined extensions; the helper caches them under `~/.cache/topas-docker/<ext>/install` (mounted inside the container as `/extensions/install`) so later runs automatically use the cached TOPAS build.

   ```bash
   ./topas-docker \
     -extensions=$HOME/Applications/TOPAS/Extensions/TOPAS-nBio \
     --build-extensions \
     -g4data=$HOME/Applications/G4Data \
     MySimulation.txt
   ```  
   The helper mounts your source code at `/extensions/src`, builds inside an isolated cache, and installs the resulting TOPAS binaries under `~/.cache/topas-docker/<ext-name>-<hash>/install`.
2. **Re-use cache** — Subsequent runs can skip `--build-extensions`; the script detects `/extensions/install/bin/topas`, and add it to `PATH`, and reuses the cached build automatically.
3. **Force rebuild** — Pass `--build-extensions` again (or delete the cache directory) when you want to use a different extension directory. The cache is stored in `${TOPAS_DOCKER_CACHE:-~/.cache/topas-docker}`, so you can purge a specific build e.g., `rm -rf ~/.cache/topas-docker/TOPAS-nBio-*`.
4. **Custom cache location** — Set `TOPAS_DOCKER_CACHE=/path/to/cache` before running if you want the build artifacts on another disk.

## macOS X11 / Qt Visualization
1. Install [XQuartz](https://www.xquartz.org/), set **Preferences -> Security -> Allow connections from network clients**, then run:
   ```bash
   defaults write org.xquartz.X11 enable_iglx -bool true
   ```
   Quit and reopen XQuartz.
2. Open a host:
   ```bash
   xhost +127.0.0.1
   ```
3. Run TOPAS with GUI:
   ```bash
   ./topas-docker --x11 --display=host.docker.internal:0 \
       -g4data=$HOME/Applications/G4Data MySimulation.txt
   ```
   The script exports `LIBGL_ALWAYS_INDIRECT=1` so OpenGL goes through XQuartz.
4. **When finished, revoke access**: `xhost -127.0.0.1`.

Use `-display=<host:screen>` if you run another display than XQuartz.

## Linux Visualization
In linux X11 should be enabled automatically. Before running, open a host 
```bash
xhost +local:docker
./topas-docker -g4data=/path/to/G4DATA MySimulation.txt
xhost -local:docker
```
When `topas-docker` needs `TOPAS_USE_HOST_DISPLAY=1` to detect the host driver. Thus, for manual runs without the script, this command must be used:
```bash
docker run --rm -it \
  -e TOPAS_USE_HOST_DISPLAY=1 \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
  -v $HOME/Applications/G4Data:/Applications/G4Data:ro \
  -v $PWD:/simulations \
  opentopas/opentopas:v4.1.1 MySimulation.txt
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

**Outputs (e.g., CSV scorers) are stored in your host directory mapped to `/simulations`. For frequently asked questions see `FAQ.md`**.

