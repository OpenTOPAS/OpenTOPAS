name: CI

# Make workflow manually triggerable
on:
  workflow_dispatch:
    inputs:
      logLevel:
        description: 'Log level'     
        required: true
        default: 'warning'

jobs:
  build:
    runs-on: ubuntu-latest
    
    steps:
    # Check out the repository code
    - uses: actions/checkout@v3
    
    # Install necessary system dependencies
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt install -y cmake \
                            libexpat1-dev \
                            libgl1-mesa-dev \
                            libglu1-mesa-dev \
                            libxt-dev \
                            xorg-dev \
                            build-essential \
                            libharfbuzz-dev \
                            qtbase5-dev \
                            qtchooser \
                            qt5-qmake \
                            qtbase5-dev-tools

    # Set up Qt framework
    - name: Setup Qt
      uses: jurplel/install-qt-action@v3
      with:
        version: '5.15.2'

    - name: Cache Geant4 data
      uses: actions/cache@v3
      id: cache-data
      with:
        path: ~/geant4-data
        key: ${{ runner.os }}-geant4-data-v11.1.3-${{ hashFiles('.github/workflows/g4data_urls.txt') }}
        restore-keys: |
          ${{ runner.os }}-geant4-data-v11.1.3-

    - name: Download Geant4 data
      if: steps.cache-data.outputs.cache-hit != 'true'
      run: |
        mkdir -p ~/geant4-data
        cd ~/geant4-data
        while read url; do
          wget "$url"
        done < $GITHUB_WORKSPACE/.github/workflows/data_urls.txt
        for file in *.tar.gz; do
          tar -xzf "$file"
          rm "$file"
        done

    - name: Clone Geant4
      run: git clone https://github.com/Geant4/geant4 --branch v11.1.3

    - name: Install Geant4
      run: |
        mkdir geant4-build
        cd geant4-build
        cmake ../geant4 -DGEANT4_INSTALL_DATA=OFF \
                        -DGEANT4_BUILD_MULTITHREADED=ON \
                        -DCMAKE_PREFIX_PATH=/usr/lib/qt5 \
                        -DGEANT4_USE_QT=ON \
                        -DGEANT4_USE_OPENGL_X11=ON \
                        -DGEANT4_USE_RAYTRACER_X11=ON \
                        -DCMAKE_INSTALL_PREFIX=~/geant4-install \
        make -j$(nproc) install
