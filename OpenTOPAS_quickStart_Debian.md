# QuickStart Guide for OpenTOPAS: TOol for PArticle Simulation
This file details the steps to be followed by <ins>Debian 10, 11 or 12</ins> in order to install OpenTOPAS and launch your first simulation. 

These instructions target **v4.2.2** built against Geant4 **v11.3.2**.

> [!TIP]
> The directory path setting proposed in this quickStart guide sets a local installation at the home directory (path defined by `$HOME` environment variable). 
> Then, it requires the existence or creation of a `$HOME/Applications` folder.
> For a global installation, a directory named `/Applications` should be created, which might need admin privileges. In this case, change the occurrences
> of `$HOME/Applications` in this quickStart guide to `/Applications`.

> [!NOTE]
> **Steps 1-4 are used to prepare your system for installation of OpenTOPAS**. Run these steps from a "terminal" window when logged in as a user with administrative privileges (a so-called super user or su). 

> [!TIP]
> Steps 1 to 4 are only needed if you have never installed the necessary libraries, `Cmake`, `git` or `qt5`. Otherwise you can skip these steps.

## Step 1
Install the following libraries (you can copy commands from here and paste them to your terminal):

        sudo apt install -y libexpat1-dev
        sudo apt install -y libgl1-mesa-dev
        sudo apt install -y libglu1-mesa-dev
        sudo apt install -y libxt-dev
        sudo apt install -y xorg-dev
        sudo apt install -y build-essential
        sudo apt install -y libharfbuzz-dev

## Step 2 
Install Cmake:

        sudo apt install -y cmake

> [!WARNING]
> Geant4 requires a minimum `CMake` version (<em>3.16</em> and <em>3.243</em> or higher). If already installed, check your version of `CMake` as follows:

        cmake --version

## Step 3
Install `git`:

        sudo apt install git-all

> [!TIP]
> This step is only necessary if you want to clone OpenTOPAS source code from the GitHub repository (recommended). See Step 8.1 for more details.

## Step 4
Install `qt5`:

        sudo apt install qt6-base-dev qt6-base-dev-tools qt6-tools-dev qt6-tools-dev-tools libqt6opengl6-dev 

> [!WARNING]
> The visualization of the current version of OpenTOPAS is only compatible with `qt`, not `qt` versions. 

> [!NOTE]
> Steps 5-7 are used to install Geant4, the Monte Carlo toolkit that provides the radiation transport.

## Step 5
If you have not done so already, download Geant4 version <em>11.3.2</em>.

5.1.a Download Geant4 from the terminal as follows:

        mkdir $HOME/Applications/GEANT4
        cd $HOME/Applications/GEANT4
        wget https://gitlab.cern.ch/geant4/geant4/-/archive/v11.3.2/geant4-v11.3.2.tar.gz
        tar -zxf geant4-v11.3.2.tar.gz

5.1.b Alternatively, you can download Geant4 manually. For a manual download, go to the Geant4 [website](https://geant4.web.cern.ch/download/all), choose your version, scroll down to "Source code", and download the compressed file. Create a directory called `GEANT4` in your `$HOME/Applications` directory, move the compressed folder into this directory, and decompress the file. Assuming you downloaded version <em>11.3.2</em>, you should have the directory `$HOME/Applications/GEANT4/geant4-v11.3.2`

## Step 6
If you have not done so already, download the Geant4 data files which correspond to your version of Geant4.

6.1. First create the G4DATA directory which will house the data files:

        mkdir $HOME/Applications/GEANT4/G4DATA

6.2.a Next, download the files using the `wget` command in your terminal; e.g., for Geant4-11.3.2:

        cd $HOME/Applications/GEANT4/G4DATA/
        wget https://cern.ch/geant4-data/datasets/G4NDL.4.7.1.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4EMLOW.8.6.1.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4PhotonEvaporation.6.1.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4RadioactiveDecay.6.1.2.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4PARTICLEXS.4.1.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4ABLA.3.3.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4INCL.1.2.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4ENSDFSTATE.3.0.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4CHANNELING.1.0.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4NUDEXLIB.1.0.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4URRPT.1.1.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4PII.1.3.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4RealSurface.2.2.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4SAIDDATA.2.0.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4TENDL.1.4.tar.gz

and decompress them using `tar -zxf`.

        tar -zxf G4NDL.4.7.1.tar.gz
        tar -zxf G4EMLOW.8.6.1.tar.gz
        tar -zxf G4PhotonEvaporation.6.1.tar.gz
        tar -zxf G4RadioactiveDecay.6.1.2.tar.gz
        tar -zxf G4PARTICLEXS.4.1.tar.gz
        tar -zxf G4ABLA.3.3.tar.gz
        tar -zxf G4INCL.1.2.tar.gz
        tar -zxf G4ENSDFSTATE.3.0.tar.gz
        tar -zxf G4CHANNELING.1.0.tar.gz
        tar -zxf G4NUDEXLIB.1.0.tar.gz
        tar -zxf G4URRPT.1.1.tar.gz
        tar -zxf G4PII.1.3.tar.gz
        tar -zxf G4RealSurface.2.2.tar.gz
        tar -zxf G4SAIDDATA.2.0.tar.gz
        tar -zxf G4TENDL.1.4.tar.gz

6.2.b Alternatively, download manually the data files which correspond to your version of Geant4. The datasets for the latest Geant4 release may be donwloaded from (https://geant4.web.cern.ch/download/all). The datesets for earlier versions of Geant4 can be found by going to the specific webpage for your Geant4 [version](https://geant4.web.cern.ch/download/11.3.2.html). On the webpage, scroll down to “Datasets”, and hover over each data set name. For a manual download, get the <em>.tar.gz</em> files from the website, move them into `$HOME/Applications/GEANT4/G4DATA`, and decompress them.

## Step 7
Build Geant4.

7.1. Check where qt5 is stored in your system. The commands below assume it is at `/usr/lib/qt5`. In case qt5 is not stored in that directory in your system, replace the directory path after the command `-DCMAKE_PREFIX_PATH=` by the appropriate directory. 

7.2. Run the following commands: 

        cd $HOME/Applications/GEANT4/
        rm -rf geant4-install geant4-build
        mkdir geant4-{build,install}
        cd geant4-build
        cmake ../geant4-v11.3.2 -DGEANT4_INSTALL_DATA=OFF -DGEANT4_BUILD_MULTITHREADED=ON -DGEANT4_BUILD_VERBOSE_CODE=OFF -DCMAKE_INSTALL_PREFIX=../geant4-install -DCMAKE_PREFIX_PATH=/usr/lib/qt6 -DGEANT4_USE_QT=ON -DGEANT4_USE_QT_QT6=ON 
        sudo make -j20 install

> [!NOTE]
> The remaining steps complete the download and installation of OpenTOPAS and start you on the road to successful simulations.

## Step 8
Downloading and installing OpenTOPAS and GDCM.

8.1.a Get OpenTOPAS from the GitHub [repository](https://github.com/OpenTOPAS/OpenTOPAS) running the following commands from the terminal:

        mkdir $HOME/Applications/TOPAS
        cd $HOME/Applications/TOPAS
        git clone https://github.com/OpenTOPAS/OpenTOPAS.git
        cd OpenTOPAS
        git checkout v4.2.2

8.1.b Alternatively, you can download OpenTOPAS manually. For a manual download, go to the OpenTOPAS GitHub [website](https://github.com/OpenTOPAS/OpenTOPAS/tree/master), click on the green tab named `<> Code` and `Download ZIP`. Create a directory called `TOPAS` in your `$HOME/Applications` directory, move the compressed folder into this directory, and decompress the file. To follow the following commands, rename the decompressed folder `OpenTOPAS-main` as `OpenTOPAS`. You should have the directory `$HOME/Applications/TOPAS/Open-TOPAS` 

8.2. Next, check if the $HOME/Applications/GDCM already exists (GDCM is already installed). If so, rename the directory to GDCM-OLD (or another name) using the following command. 

        mv $HOME/Applications/GDCM $HOME/Applications/GDCM-OLD

Then use the following commands to move GDCM(<em>gdcm-2.6.8.tar.gz</em>) from the OpenTOPAS source code folder to its own directory and decompress it.

        mkdir $HOME/Applications/GDCM
        cd $HOME/Applications/TOPAS/OpenTOPAS
        mv gdcm-2.6.8.tar.gz ../../GDCM
        cd ../../GDCM
        tar -zxf gdcm-2.6.8.tar.gz

8.3. Build GDCM with the following commands:

        rm -rf gdcm-install gdcm-build
        mkdir gdcm-{build,install}
        cd gdcm-build
        cmake ../gdcm-2.6.8 -DGDCM_BUILD_SHARED_LIBS=ON -DGDCM_BUILD_DOCBOOK_MANPAGES:BOOL=OFF  -DCMAKE_INSTALL_PREFIX=../gdcm-install 
        sudo make -j20 install

8.4. With GDCM built and installed, you can continue with the OpenTOPAS installation:

        cd $HOME/Applications/TOPAS
        rm -rf OpenTOPAS-install OpenTOPAS-build
        mkdir OpenTOPAS-{build,install}
        cd OpenTOPAS-build
        export Geant4_DIR=$HOME/Applications/GEANT4/geant4-install
        export GDCM_DIR=$HOME/Applications/GDCM/gdcm-install
        cmake ../OpenTOPAS -DCMAKE_INSTALL_PREFIX=../OpenTOPAS-install -DTOPAS_USE_QT=ON -DTOPAS_USE_QT6=ON
        sudo make -j20 install

## Step 9
Setup the environment. 

9.1.a This can be done manually as follows, however it needs to be repeated each time you open a new terminal window.

        export QT_QPA_PLATFORM_PLUGIN_PATH=$HOME/Applications/TOPAS/OpenTOPAS-install/Frameworks
        export TOPAS_G4_DATA_DIR=$HOME/Applications/GEANT4/G4DATA
        export LD_LIBRARY_PATH=$HOME/Applications/TOPAS/OpenTOPAS-install/lib:$LD_LIBRARY_PATH
        export LD_LIBRARY_PATH=$HOME/Applications/GEANT4/geant4-install/lib:$LD_LIBRARY_PATH

9.1.b.1 Alternatively, you can automate setting up the environment. We recommend adding all the exports into a dedicated shell script folder as follows:

        mkdir ~/shellScripts
        cd ~/shellScripts
        echo '#!/bin/bash' > topas
        echo '' >> topas
        echo 'export QT_QPA_PLATFORM_PLUGIN_PATH=$HOME/Applications/TOPAS/OpenTOPAS-install/Frameworks' >> topas
        echo 'export TOPAS_G4_DATA_DIR=$HOME/Applications/GEANT4/G4DATA' >> topas
        echo 'export LD_LIBRARY_PATH=$HOME/Applications/TOPAS/OpenTOPAS-install/lib:$LD_LIBRARY_PATH' >> topas
        echo 'export LD_LIBRARY_PATH=$HOME/Applications/GEANT4/geant4-install/lib:$LD_LIBRARY_PATH' >> topas
        echo '$HOME/Applications/TOPAS/OpenTOPAS-install/bin/topas $1' >> topas
        chmod +x topas

and export the appropriate path to the OpenTOPAS shell script folder has been created in your `~/.bashrc` file.

        cd ~
        echo 'export PATH=~/shellScripts:$PATH' >> .bashrc

## Step 10
Running your first OpenTOPAS simulation in a new terminal window. 

10.a For those that decided to set the environment up manually, as described in Step 9.1.a, OpenTOPAS can now be run by accessing the executable located in `$HOME/Applications/TOPAS/OpenTOPAS-install/bin/topas`. For example, to run the simple OpenTOPAS example to test Qt visualization:

        cd $HOME/Applications/TOPAS/OpenTOPAS-install/examples/Basic
        ../../bin/topas QtShapeTest.txt

10.b For those that decided to automate the process as described in Step 9.1.b onwards, OpenTOPAS can now be run with the `topas` command in your terminal without having to setup the environment variables:

        topas $HOME/Applications/TOPAS/OpenTOPAS/examples/Basic/QtShapeTest.txt

> [!TIP]
> If you require assistance with any of your OpenTOPAS simulations, don't hesitate to request help from the developers and other users on the [Discussions](https://github.com/OpenTOPAS/OpenTOPAS/discussions) tab of the OpenTOPAS [GitHub](https://github.com/OpenTOPAS/OpenTOPAS) page.

______________________________________________________________________________________________

## Step 11
As an additional step for those interested in running quality checks, the continuous integration test suite for OpenTOPAS can be used. Python and `pip3` will be needed. 

> [!WARNING]
> We recommend that installation be performed using [Homebrew](https://brew.sh/) to avoid messing with your system Python. The following command installs Python 3.x and `pip3` is installed automatically.

        brew install python

The OpenTOPAS tests are located [here](https://github.com/OpenTOPAS/qi-opentopas.git), listed in the OpenTOPAS organization repositories, and testing is performed using `nrtest` and the OpenTOPAS-specific plugins contained in [nrtest-topas](https://github.com/davidchall/nrtest-topas):

        cd $HOME/Applications/TOPAS
        git clone https://github.com/OpenTOPAS/qi-opentopas.git
        cd qi-opentopas
        pip3 install nrtest
        pip3 install git+https://github.com/davidchall/nrtest-topas.git

Modify the `apps/topas-v4.2.2.json` metadata file according to your directories and configuration (remember to set your environment variables) and execute the entire test suite as follows:

        nrtest execute apps/topas-v4.2.2.json tests/ -o benchmarks/todayDate

Comparisons can also be made with the following command:
        
        nrtest compare benchmarks/today benchmarks/yesterday
