# FAQ - Using `topas-docker` (and `topas-apptainer`)

## How do I enable Qt/OpenGL visualization on macOS?
1. Install and open XQuartz. In **Preferences -> Security** enable “Allow connections from network clients”.
2. Enable GLX forwarding: `defaults write org.xquartz.X11 enable_iglx -bool true`, quit XQuartz, then relaunch it.
3. Allow Docker Desktop to talk to XQuartz: `xhost +127.0.0.1` (**revert later with `xhost -127.0.0.1`**).
4. Run `./topas-docker --x11 --display=host.docker.internal:0 -g4data=$HOME/Applications/GEANT4/G4DATA MySimulation.txt`. Or use the path to your Geant4 data files. 

## Can I change the virtual display settings for headless runs (no live vis)?
Yes. The container’s entrypoint reads `TOPAS_XVFB_DISPLAY` (default `:99`) and `TOPAS_XVFB_SCREEN` (default `1280x800x24`). Export new values before running `topas-docker` if you need multiple concurrent sessions or a different resolution.

## Why TOPAS says “unable to open parameter file: -g4data=…”?
In some (MacOS) systems, g4data files saved at /Applications might give errors due to lack of rights. Save the data files elsewhere, e.g., at $HOME/G4Data.

## What if my HPC cluster does not allow Docker?
Use the Apptainer script `docker/topas-apptainer`. It supports `-data`, `-image`, `-extensions`, `-g4data`, and `--build-extensions`, and runs `apptainer run` (or `singularity run`) with `docker://opentopas/opentopas:v4.3.0-geant4-11.4.2` by default. For example:
```bash
./topas-apptainer -g4data=$HOME/G4Data MySimulation.txt
```
It binds `/simulations`, `/Applications/G4Data`, and optional `/extensions` caches exactly like the Docker script while keeping X11 disabled for HPC-based runnings.

## How do I install and reuse TOPAS extensions?
For public TOPAS-nBio, select `-image=opentopas/opentopas:v4.3.0-geant4-11.3.2` with Docker, or `-image=docker://opentopas/opentopas:v4.3.0-geant4-11.3.2` with Apptainer, and mount matching Geant4 datasets. Geant4 11.4.2 is currently incompatible with its chemistry.

Set both in-line options `-extensions=/path/to/your/extensions` and `--build-extensions` once. These options mounts your source directory at `/extensions/src`, builds in an isolated cache under `${TOPAS_DOCKER_CACHE:-$HOME/.cache/topas-docker}`, and installs the resulting TOPAS binaries into `/extensions/install`. Future runs can skip `--build-extensions`; the cached install is detected automatically as long as `/extensions/install/bin/topas` exists. **When sources files or directories change, rerun** with `--build-extensions` (or delete the matching cache directory) to force a rebuild. Set `TOPAS_DOCKER_CACHE` if you prefer storing the build artifacts somewhere else.

## The Qt shell appears but the scene view stays empty.
Verify that this parameter also exists e.g., `s:Gr/view/Type = "OpenGl"` 

## Where do output files are saved?
In the container anything that TOPAS produces is save it in `/simulations` (e.g., CSV scorers), and in the host its save it in directory you mounted with `-data` or **the working directory by default**. 
