# =========================================================
# OpenTOPAS + Geant4 + GDCM (Debian 12)
# =========================================================
FROM debian:12

LABEL maintainer="Jose Ramos-Mendez <Jose.RamosMendez@ucsf.edu>" \
      description="Docker image for TOPAS v4.2.0 (Geant4 11.3.2, GDCM 2.6.8)"

# -----------------------------
# Base dependencies
# -----------------------------
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential cmake git wget tar \
    libexpat1-dev libgl1-mesa-dev libglu1-mesa-dev \
    libxt-dev xorg-dev libharfbuzz-dev \
    qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools \
    xvfb && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# -----------------------------
# Environment paths
# -----------------------------
ARG TOPAS_VERSION=v4.2.0
ENV TOPAS_VERSION=${TOPAS_VERSION} \
    APP_HOME=/Applications
RUN mkdir -p $APP_HOME
WORKDIR $APP_HOME

# =========================================================
# 1. Build Geant4
# =========================================================
ARG BUILD_JOBS=20
ENV BUILD_JOBS=${BUILD_JOBS} \
    G4_VERSION=11.3.2
RUN mkdir GEANT4 && cd GEANT4 && \
    wget https://gitlab.cern.ch/geant4/geant4/-/archive/v${G4_VERSION}/geant4-v${G4_VERSION}.tar.gz && \
    tar -zxf geant4-v${G4_VERSION}.tar.gz && \
    mkdir geant4-build geant4-install && cd geant4-build && \
    cmake ../geant4-v${G4_VERSION} \
        -DGEANT4_INSTALL_DATA=OFF \
        -DGEANT4_BUILD_MULTITHREADED=ON \
        -DCMAKE_INSTALL_PREFIX=../geant4-install \
        -DCMAKE_PREFIX_PATH=/usr/lib/qt5 \
        -DGEANT4_USE_QT=ON \
        -DGEANT4_USE_OPENGL_X11=ON \
        -DGEANT4_USE_RAYTRACER_X11=ON \
        -DGEANT4_BUILD_VERBOSE_CODE=OFF && \
    make -j${BUILD_JOBS} install

WORKDIR $APP_HOME
# Pin to the released OpenTOPAS tag so image builds stay reproducible
RUN git clone --branch ${TOPAS_VERSION} --depth 1 https://github.com/OpenTOPAS/OpenTOPAS.git

# =========================================================
# 2. Build GDCM 2.6.8
# =========================================================
RUN mkdir GDCM && cd GDCM && \
    tar -zxf $APP_HOME/OpenTOPAS/gdcm-2.6.8.tar.gz && \
    mkdir gdcm-build gdcm-install && cd gdcm-build && \
    cmake ../gdcm-2.6.8 \
        -DGDCM_BUILD_SHARED_LIBS=ON \
        -DGDCM_BUILD_DOCBOOK_MANPAGES:BOOL=OFF \
        -DCMAKE_INSTALL_PREFIX=../gdcm-install && \
    make -j${BUILD_JOBS} install

# =========================================================
# 3. Build OpenTOPAS
# =========================================================
RUN mkdir TOPAS && cd TOPAS && \
    mv $APP_HOME/OpenTOPAS $APP_HOME/TOPAS && \
    mkdir OpenTOPAS-build OpenTOPAS-install && cd OpenTOPAS-build && \
    export Geant4_DIR=$APP_HOME/GEANT4/geant4-install && \
    export GDCM_DIR=$APP_HOME/GDCM/gdcm-install && \
    cmake ../OpenTOPAS -DCMAKE_INSTALL_PREFIX=../OpenTOPAS-install -DTOPAS_USE_QT=ON -DTOPAS_USE_QT6=OFF && \
    make -j${BUILD_JOBS} install

# =========================================================
# 4. Environment setup
# =========================================================
ENV Geant4_DIR=$APP_HOME/GEANT4/geant4-install \
    TOPAS_G4_DATA_DIR=$APP_HOME/G4Data \
    GDCM_DIR=$APP_HOME/GDCM/gdcm-install \
    QT_QPA_PLATFORM_PLUGIN_PATH=$APP_HOME/TOPAS/OpenTOPAS-install/Frameworks \
    LD_LIBRARY_PATH=$APP_HOME/TOPAS/OpenTOPAS-install/lib:$APP_HOME/GEANT4/geant4-install/lib:$APP_HOME/GDCM/gdcm-install/lib \
    PATH=$APP_HOME/TOPAS/OpenTOPAS-install/bin:$PATH

# Provide a configurable launcher so Linux users can reuse host DISPLAY
RUN cat <<'EOF' > /usr/local/bin/topas-entrypoint.sh && chmod +x /usr/local/bin/topas-entrypoint.sh
#!/bin/bash
set -euo pipefail

XVFB_DISPLAY="${TOPAS_XVFB_DISPLAY:-:99}"
XVFB_SCREEN="${TOPAS_XVFB_SCREEN:-1280x800x24}"
USE_HOST_DISPLAY="${TOPAS_USE_HOST_DISPLAY:-}"

cleanup() {
  if [[ -n "${XVFB_PID:-}" ]]; then
    kill "${XVFB_PID}" >/dev/null 2>&1 || true
  fi
}

if [[ -n "$USE_HOST_DISPLAY" && -n "${DISPLAY:-}" ]]; then
  echo "Using host DISPLAY ${DISPLAY}"
else
  echo "Starting Xvfb on ${XVFB_DISPLAY} (${XVFB_SCREEN})"
  Xvfb "${XVFB_DISPLAY}" -screen 0 "${XVFB_SCREEN}" &
  XVFB_PID=$!
  trap cleanup EXIT
  export DISPLAY="${XVFB_DISPLAY}"
fi

exec topas "$@"
EOF

# Optional: copy G4DATA if available (recommended mount)
RUN mkdir -p /simulations $APP_HOME/G4Data /extensions
VOLUME ["/Applications/G4Data", "/simulations","/extensions"]
WORKDIR /simulations

ENTRYPOINT ["/usr/local/bin/topas-entrypoint.sh"]

