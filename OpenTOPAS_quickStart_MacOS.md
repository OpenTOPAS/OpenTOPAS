# QuickStart Guide for OpenTOPAS: TOol for PArticle Simulation
This file details the steps to be followed by <ins>Mac users</ins> in order to install OpenTOPAS and launch your first simulation. 

> [!WARNING]
> We recommend macOS version 14.0 (Sonoma) or higher. Furthermore these instructions are only compatible with `qt@5` (see the warnings in Step 6 to ensure that `qt@6` is not installed on your system). We take no responsibility for users who wish to proceed with `qt@6` already installed on their system.

> [!NOTE]
> **Steps 1-3 are used to prepare your system for installation of OpenTOPAS**. Run these steps from a "terminal" window (found in the Utilities subfolder of the Applications folder on your system) when logged in as a user with administrative privileges (a so-called super user or su). 

> [!TIP]
> You can skip steps 1 and 2 if your system has XCode and Homebrew. You can check for this by entering the command `which xcode-select` for XCode and `which brew` for Homebrew. If the command is available the system will respond, showing you where it is installed on your system. 

## Step 1
Download [XCode](https://apps.apple.com/fr/app/xcode/id497799835?l=en-GB&mt=12), which can be found on the Mac App Store, then enter the following command to install xcode-select (you can copy commands from here and paste them to your terminal):

        sudo xcode-select --install

## Step 2 
Install [Homebrew](https://brew.sh/). Access the Homebrew website given in Step 2 from your web browser then copy and paste the installation command provided near the top of the website page into your terminal; e.g.:

        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

Follow the instructions posted to your terminal at the end of the Homebrew installation; e.g., enter the following commands:

        (echo; echo 'eval "$(/opt/homebrew/bin/brew shellenv)"') >> $HOME/.zprofile
        eval "$(/opt/homebrew/bin/brew shellenv)"

## Step 3
Once Homebrew is installed, you will have access to the command `brew install`. Use this command to install `qt@5`, `git`, `wget`, and `cmake` by entering the following commands into your terminal:

        brew install qt@5
        brew install git
        brew install wget
        brew install cmake

> [!NOTE]
> Steps 4-6 are used to install Geant4, the Monte Carlo toolkit that provides the radiation transport.

## Step 4
If you have not done so already, download Geant4 version <em>11.1.3</em> (or <em>11.1.2</em>).

4.1. For a manual download, go to the Geant4 [website](https://geant4.web.cern.ch/download/all), choose your version, scroll down to "Source code", and download the compressed file. Create a directory called `GEANT4` in your `/Applications` directory, move the compressed folder into this directory, and decompress the file. Assuming you downloaded version <em>11.1.3</em>, you should have the directory `/Applications/GEANT4/geant4-v11.1.3`
   
4.2. Alternatively, download Geant4 from the terminal as follows:

        mkdir /Applications/GEANT4
        cd /Applications/GEANT4
        wget https://gitlab.cern.ch/geant4/geant4/-/archive/v11.1.3/geant4-v11.1.3.tar.gz
        tar zxf geant4-v11.1.3.tar.gz
 
## Step 5
If you have not done so already, download the Geant4 data files which correspond to your version of Geant4.

5.1. First create the G4DATA directory which will house the data files:

        cd /Applications/GEANT4/
        mkdir G4DATA

5.2. Next, download the data files which correspond to your version of Geant4. The datasets for the latest Geant4 release may be donwloaded from (https://geant4.web.cern.ch/download/all). The datesets for earlier versions of Geant4 can be found by going to the specific webpage for your Geant4 [version](https://geant4.web.cern.ch/download/11.1.3.html). On the webpage, scroll down to “Datasets”, and hover over each data set name. For a manual download, get the <em>.tar.gz</em> files from the website, move them into `/Applications/GEANT4/G4DATA`, and decompress them.

5.3. Alternatively, download the files using the `wget` command in your terminal; e.g., for Geant4-11.1.3:

        cd /Applications/GEANT4/G4DATA/
        wget https://cern.ch/geant4-data/datasets/G4NDL.4.7.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4EMLOW.8.2.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4PhotonEvaporation.5.7.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4RadioactiveDecay.5.6.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4PARTICLEXS.4.0.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4PII.1.3.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4RealSurface.2.2.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4SAIDDATA.2.0.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4ABLA.3.1.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4INCL.1.0.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4ENSDFSTATE.2.3.tar.gz
        wget https://cern.ch/geant4-data/datasets/G4TENDL.1.4.tar.gz
        wget ftp://gdo-nuclear.ucllnl.org/LEND_GND1.3/LEND_GND1.3_ENDF.BVII.1.tar.gz

5.4. Decompress them using `tar zxf`.

        tar zxf G4NDL.4.7.tar.gz
        tar zxf G4EMLOW.8.2.tar.gz
        tar zxf G4PhotonEvaporation.5.7.tar.gz
        tar zxf G4RadioactiveDecay.5.6.tar.gz
        tar zxf G4PARTICLEXS.4.0.tar.gz
        tar zxf G4PII.1.3.tar.gz
        tar zxf G4RealSurface.2.2.tar.gz
        tar zxf G4SAIDDATA.2.0.tar.gz
        tar zxf G4ABLA.3.1.tar.gz
        tar zxf G4INCL.1.0.tar.gz
        tar zxf G4ENSDFSTATE.2.3.tar.gz
        tar zxf G4TENDL.1.4.tar.gz
        tar zxf LEND_GND1.3_ENDF.BVII.1.tar.gz

## Step 6
Build Geant4. Take note of the following warnings before running the commands shown in step 6.2:

> [!WARNING]
> Geant4 requires a minimum `CMake` version (between <em>3.16</em> and <em>3.243</em>). Check your version of `CMake` as follows:

        cmake --version

> [!WARNING]
> Verify that `qt@6` is not linked or installed in your system. The following command should yield no output if `qt@6` is indeed **NOT** installed on your system. See the warning at the top of the document about `qt@6` compatibility.

        brew list --versions qt@6

> [!WARNING]
> Depending on your MacOS version you may or may not have XQuartz installed on your system. This can be tested with the following command which should yield no output if it is **NOT** installed. If this is the case please head to the official [Xquartz](https://www.xquartz.org) website to download the application.

        which xquartz

> [!WARNING]
> Those with M1, M2 or M3 chips (check by going to the apple logo on the upper left of your screen and clicking on “About this Mac”) have `arm64` architecture and should include this architecture in the `DCMAKE_OSX_ARCHITECTURES` option of the cmake command in step 6.2 below. Those with Intel chips should not include this command and can delete the last line of the cmake command.

6.1. Check which version of `qt@5` you have installed on your system as well as the associated installation path with the following command. 

        readlink -f $(brew --prefix qt@5)

Replace the path supplied to `DCMAKE_PREFIX_PATH` in step 6.2 with the output of the above command.

6.2. Run the following commands: 

        cd /Applications/GEANT4/
        mkdir geant4-{build,install}
        cd geant4-build
        cmake ../geant4-v11.1.3 -DGEANT4_INSTALL_DATA=OFF \
                                -DGEANT4_BUILD_MULTITHREADED=ON \
                                -DCMAKE_INSTALL_PREFIX=../geant4-install \
                                -DCMAKE_PREFIX_PATH=/opt/homebrew/Cellar/qt@5/5.15.11 \
                                -DGEANT4_USE_QT=ON -DGEANT4_USE_OPENGL_X11=ON \
                                -DGEANT4_USE_RAYTRACER_X11=ON \
                                -DCMAKE_OSX_ARCHITECTURES=arm64
        make -j20 install

> [!NOTE]
> The remaining steps complete the download and installation of OpenTOPAS and start you on the road to successful simulations.

## Step 7
Downloading and installing OpenTOPAS and GDCM.

7.1. First get OpenTOPAS from the GitHub [repository](https://github.com/OpenTOPAS/OpenTOPAS).

        mkdir -p /Applications/TOPAS
        cd /Applications/TOPAS
        git clone https://github.com/OpenTOPAS/OpenTOPAS.git

7.2. Next, check if the /Applications/GDCM already exists (GDCM is already installed). If so, rename the directory to GDCM-OLD (or another name) using the following command. 

        mv /Applications/GDCM /Applications/GDCM-OLD
        
Then use the following commands to move GDCM(<em>gdcm-2.6.8.tar.gz</em>) from the OpenTOPAS source code folder to its own directory and decompress it.

        mkdir /Applications/GDCM
        cd /Applications/TOPAS/OpenTOPAS
        mv gdcm-2.6.8.tar.gz ../../GDCM
        cd ../../GDCM
        tar zxf gdcm-2.6.8.tar.gz

7.3. Build GDCM with the following commands:

        mkdir gdcm-{build,install}
        cd gdcm-build
        cmake ../gdcm-2.6.8 -DGDCM_BUILD_SHARED_LIBS=ON \
                            -DGDCM_BUILD_DOCBOOK_MANPAGES:BOOL=OFF \
                            -DCMAKE_INSTALL_PREFIX=../gdcm-install \
                            -DCMAKE_INSTALL_RPATH=@loader_path \
                            -DCMAKE_MACOSX_RPATH=ON
        make -j20 install

7.4. With GDCM built and installed, you can continue with the OpenTOPAS installation:

        cd /Applications/TOPAS
        mkdir OpenTOPAS-{build,install}
        cd OpenTOPAS-build
        export Geant4_DIR=/Applications/GEANT4/geant4-install \
               GDCM_DIR=/Applications/GDCM/gdcm-install/
        cmake ../OpenTOPAS -DCMAKE_INSTALL_PREFIX=../OpenTOPAS-install
        make -j20 install

## Step 8
Setup the environment. 

8.1. This can be done manually as follows, however it needs to be repeated each time you open a new terminal window.

        export QT_QPA_PLATFORM_PLUGIN_PATH=/Applications/TOPAS/OpenTOPAS-install/Frameworks
        export TOPAS_G4_DATA_DIR=/Applications/GEANT4/G4DATA
        export DYLD_LIBRARY_PATH=/Applications/TOPAS/OpenTOPAS-install/lib:$DYLD_LIBRARY_PATH
        export DYLD_LIBRARY_PATH=/Applications/GEANT4/geant4-install/lib:$DYLD_LIBRARY_PATH

8.2.1. Alternatively, you can automate setting up the environment. We recommend adding all the exports into a dedicated shell script folder as follows:

        mkdir ~/shellScripts
        cd ~/shellScripts
        echo '#!/bin/bash' > topas
        echo '' >> topas
        echo 'export QT_QPA_PLATFORM_PLUGIN_PATH=/Applications/TOPAS/OpenTOPAS-install/Frameworks' >> topas
        echo 'export TOPAS_G4_DATA_DIR=/Applications/GEANT4/G4DATA' >> topas
        echo 'export DYLD_LIBRARY_PATH=/Applications/TOPAS/OpenTOPAS-install/lib:$DYLD_LIBRARY_PATH' >> topas
        echo 'export DYLD_LIBRARY_PATH=/Applications/GEANT4/geant4-install/lib:$DYLD_LIBRARY_PATH' >> topas
        echo '/Applications/TOPAS/OpenTOPAS-install/bin/topas $1' >> topas
        chmod +x topas

8.2.2 After the OpenTOPAS shell script folder has been created as outlined above, export the appropriate path in either your `~/.zshrc` or `~/.bash_profile` file. You can find out which shell you are using by typing `echo $SHELL` in your terminal. For `zsh`:

        cd ~
        echo 'export PATH=~/shellScripts:$PATH' >> .zshrc

And for `bash`:

        cd ~
        echo 'export PATH=~/shellScripts:$PATH' >> .bash_profile

## Step 9
Running your first OpenTOPAS simulation. 

9.1. For those that decided to set the environment up manually, as described in Step 8.1, OpenTOPAS can now be run by accessing the executable located in `/Applications/TOPAS/OpenTOPAS-install/bin/topas`. For example, to run the simple OpenTOPAS example of dose being scored inside a water phantom:

        cd /Applications/TOPAS/OpenTOPAS-install/examples/Scoring
        ../../bin/topas Dose.txt

9.2. For those that decided to automate the process as described in Step 8.2 onwards, OpenTOPAS can now be run with the `topas` command in your terminal without having to setup the environment variables:

        topas Dose.txt

> [!TIP]
> If you require assistance with any of your OpenTOPAS simulations, don't hesitate to request help from the developers and other users on the [Discussions](https://github.com/OpenTOPAS/OpenTOPAS/discussions) tab of the OpenTOPAS [GitHub](https://github.com/OpenTOPAS/OpenTOPAS) page.

______________________________________________________________________________________________

## Step 10
As an additional step for those interested in running quality checks, the continuous integration test suite for OpenTOPAS can be used. Python and `pip3` will be needed. 

> [!WARNING]
> We recommend that installation be performed using [Homebrew](https://brew.sh/) to avoid messing with your system Python. The following command installs Python 3.x and `pip3` is installed automatically.

        brew install python

The OpenTOPAS tests are located [here](https://github.com/OpenTOPAS/qi-opentopas.git), listed in the OpenTOPAS organization repositories, and testing is performed using `nrtest` and the OpenTOPAS-specific plugins contained in [nrtest-topas](https://github.com/davidchall/nrtest-topas):

        cd /Applications/TOPAS
        git clone https://github.com/OpenTOPAS/qi-opentopas.git
        cd qi-opentopas
        pip3 install nrtest
        pip3 install git+https://github.com/davidchall/nrtest-topas.git

Modify the `apps/topas-v4.0.json` metadata file according to your directories and configuration (remember to set your environment variables) and execute the entire test suite as follows:

        nrtest execute apps/topas-v4.0.json tests/ -o benchmarks/todayDate

Comparisons can also be made with the following command:
        
        nrtest compare benchmarks/today benchmarks/yesterday


