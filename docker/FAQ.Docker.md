# FAQ - Using `topas-docker`

## How do I enable Qt/OpenGL visualization on macOS?
1. Install and launch XQuartz. In **Preferences ▸ Security** enable “Allow connections from network clients”.
2. Enable GLX forwarding: `defaults write org.xquartz.X11 enable_iglx -bool true`, quit XQuartz, then relaunch it.
3. Allow Docker Desktop to talk to XQuartz: `xhost +127.0.0.1` (revert later with `xhost -127.0.0.1`).
4. Run `./topas-docker --x11 --display=host.docker.internal:0 -g4data=$HOME/Applications/GEANT4/G4DATA MySimulation.txt`. The launcher sets `LIBGL_ALWAYS_INDIRECT=1`, so OpenGL renders through XQuartz.

## How do I reuse my Linux DISPLAY instead of the container’s virtual X server?
Use `./topas-docker --x11 …` and make sure your host display is accessible (e.g., run `xhost +local:docker`). The launcher detects the `DISPLAY` value and exports `TOPAS_USE_HOST_DISPLAY=1`, which tells the container entrypoint to skip `Xvfb` and forward Qt straight to your X server. Running the image manually? Add `-e TOPAS_USE_HOST_DISPLAY=1 -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix:rw` to your `docker run` command so the same behavior is enabled.

## Can I change the virtual display settings for headless runs?
Yes. The container’s entrypoint reads `TOPAS_XVFB_DISPLAY` (default `:99`) and `TOPAS_XVFB_SCREEN` (default `1280x800x24`). Export new values before invoking `topas-docker` if you need multiple concurrent headless sessions or a different resolution.

## TOPAS says “unable to open parameter file: -g4data=…”. Why?
This happens when CLI flags are passed straight to TOPAS. Use the updated `topas-docker` script in this repo (it strips handled options before invoking TOPAS) and ensure that the actual parameter file is the final argument:  
`./topas-docker -g4data=/path/to/G4DATA Demo.txt`

In MacOS systems, g4data files saved at /Applications might give errors due to lack of rights. Save the data files elsewhere, e.g., at $HOME/G4Data.

## How do I install and reuse TOPAS extensions?
Pass both `-extensions=/path/to/your/extensions` and `--build-extensions` once. The helper mounts your source tree at `/extensions/src`, builds in an isolated cache under `${TOPAS_DOCKER_CACHE:-$HOME/.cache/topas-docker}`, and installs the resulting TOPAS binaries into `/extensions/install`. Future runs can omit `--build-extensions`; the cached install is detected automatically as long as `/extensions/install/bin/topas` exists. When sources change, rerun with `--build-extensions` (or delete the matching cache directory) to force a rebuild. Set `TOPAS_DOCKER_CACHE` if you prefer storing the build artifacts somewhere else.

## The Qt shell appears but the scene view stays empty.
Verify that this parameter also exists `s:Gr/view/Type = "OpenGL"` 

## Where do output files land?
Anything TOPAS writes to `/simulations` (e.g., CSV scorers) ends up in the host directory you mounted with `-data` or the working directory by default. Example: running from the repo root with `./topas-docker …` stores outputs next to `OneBox.txt` on the host.
