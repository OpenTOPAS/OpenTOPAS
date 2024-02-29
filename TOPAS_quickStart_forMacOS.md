# QuickStart Guide for TOPAS: TOol for PArticle Simulation
This file details the steps that need to be followed in order to install TOPAS and launch your first simulation for <ins>Mac users</ins>.

### Step 1
Install [XCode](https://apps.apple.com/fr/app/xcode/id497799835?l=en-GB&mt=12), which can be found on the Mac App Store, following on from which install xcode-select:

        sudo xcode-select install

### Step 2
Install [Homebrew](https://brew.sh/). Copy and paste the installation command provided at the top of the aforementioned website into your terminal.

### Step 3
Once Homebrew is installed, you will have access to the command `brew install`. Use this command to install `qt@5`, `git`, `wget`, and `cmake`.

        brew install qt@5
        brew install git
        brew install wget
        brew install cmake

### Step 4
Download Geant4 version <em>11.1.3</em> (or <em>11.1.2</em>).

1. For a manual download, go to the Geant4 [website]([https://brew.sh/](https://geant4.web.cern.ch/download/all)), choose your version, scroll down to "Source code", and download the compressed file. Create a directory called `GEANT4` in your `/Applications` directory, move the compressed folder into this directory, and decompress the file. Assuming you downloaded version <em>11.1.3</em>, you should have the directory `/Applications/GEANT4/geant4-v11.1.3`
   
2. Alternatively, download Geant4 from the terminal as follows:

        mkdir /Applications/GEANT4
        cd /Applications/GEANT4
        wget https://gitlab.cern.ch/geant4/geant4/-/archive/v11.1.3/geant4-v11.1.3.tar.gz
        tar zxf geant4-v11.1.3.tar.gz
 
### Step 5
Download the appropriate Geant4 data files.

1. First create the G4DATA directory which will house the data files:

        cd /Applications/GEANT4/
        mkdir G4DATA

2. Next, download the data files which correspond to your version of Geant4. The exact versions of these files can be found by going to the specific webpage for your Geant4 [version](https://geant4.web.cern.ch/download/11.1.3.html), scrolling down to “Datasets”, and hovering over each data set name. For a manual download, get the <em>.tar.gz</em> files from the website, move them into `/Applications/GEANT4/G4DATA`, and decompress them.

3. Alternatively, download the files using the `wget` command in your terminal.

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

4. Decompress them using `tar zxf`.

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

### Step 6
Build Geant4. Take note of the following warnings before continuing:

1. Check and copy the `qt@5` version you have installed (see below). Also, verify that `qt@6` is not linked or installed in your system. The second command shown below should yield no output if `qt@6` is indeed NOT installed on your system.

        brew list --versions qt@5
        brew list --versions qt@6

2. Geant4 requires a minimum `CMake` version (between <em>3.16</em> and <em>3.243</em>). Check your version of `CMake` as follows:

        cmake --version

3. Those with M1 or M2 chips (check by going to the apple logo and clicking on “About this Mac”) have arm64 architecture and should include this architecture in the `DCMAKE_OSX_ARCHITECTURES` command shown in step 4 below. Those with Intel chips should not include this command and can delete this line.

4. Run the following commands:

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

### Step 7
Downloading and installing TOPAS and GDCM.

1. First get TOPAS from the GitHub [repository](https://github.com/OpenTOPAS/topas).

        mkdir -p /Applications/TOPAS
        cd /Applications/TOPAS
        git clone https://github.com/OpenTOPAS/topas.git

2. Next, move GDCM(<em>gdcm-2.6.8.tar.gz</em>) from the topas source code folder to its own directory and decompress it.

        mkdir /Applications/GDCM
        cd /Applications/TOPAS/topas
        mv gdcm-2.6.8.tar.gz ../../GDCM
        cd ../../GDCM
        tar zxf gdcm-2.6.8.tar.gz

3. Build GDCM with the following commands:

        mkdir gdcm-{build,install}
        cd gdcm-build
        cmake ../gdcm-2.6.8 -DGDCM_BUILD_SHARED_LIBS=ON \
                            -DGDCM_BUILD_DOCBOOK_MANPAGES:BOOL=OFF \
                            -DCMAKE_INSTALL_PREFIX=../gdcm-install \
                            -DCMAKE_INSTALL_RPATH=@loader_path OFF \
                            -DCMAKE_MACOSX_RPATH=ON
        make -j20 install

4. With GDCM built and installed, you can continue with the TOPAS installation:

        cd /Applications/TOPAS
        mkdir topas-{build,install}
        cd topas-build
        export Geant4_DIR=/Applications/GEANT/Geant4-11.1.3/geant4-install \
               GDCM_DIR=/Applications/GDCM/gdcm-install/
        cmake ../topas -DCMAKE_INSTALL_PREFIX=../topas-install
        make -j20 install

### Step 8
Setup the environment. 

1. This can be done manually as follows, however it needs to be repeated each time you open a new terminal window.

        export QT_QPA_PLATFORM_PLUGIN_PATH=/Applications/TOPAS/topas-install/Frameworks
        export TOPAS_G4_DATA_DIR=/Applications/GEANT4/G4DATA
        export DYLD_LIBRARY_PATH=/Applications/TOPAS/topas-install/lib:$DYLD_LIBRARY_PATH
        export DYLD_LIBRARY_PATH=/Applications/GEANT4/geant4-install/lib:$DYLD_LIBRARY_PATH

3. Alternatively, you can automate setting up the environment. We recommend adding all the exports into a dedicated shell script folder as follows:

        mkdir ~/shellScripts
        cd ~/shellScripts
        touch topas
        echo export QT_QPA_PLATFORM_PLUGIN_PATH=/Applications/TOPAS/topas-install/Frameworks >> topas
        echo export TOPAS_G4_DATA_DIR=/Applications/GEANT4/G4DATA >> topas
        echo export DYLD_LIBRARY_PATH=/Applications/TOPAS/topas-install/lib:$DYLD_LIBRARY_PATH >> topas
        echo export DYLD_LIBRARY_PATH=/Applications/GEANT4/geant4-install/lib:$DYLD_LIBRARY_PATH >> topas
        echo /Applications/TOPAS/topas-install/bin/topas $1 >> topas
        chmod +x topas

4. After the TOPAS shell script folder has been created as outlined above, export the appropriate path in either your `~/.zshrc` or `~/.bash_profile` file. You can find out which shell you are using by typing `echo $SHELL` in your terminal. For `zsh`:

        cd ~
        echo 'export PATH=~/shellScripts:$PATH' >> .zshrc

And for `bash`:

        cd ~
        echo 'export PATH=~/shellScripts:$PATH' >> .bash_profile

### Step 9
Running your first TOPAS simulation. 

1. For those that decided to set the environment up manually, as described in Step 8.1, TOPAS can now be run by accessing the executable located in `/Applications/TOPAS/topas-install/bin/topas`. For example, to run the simple TOPAS example of dose being scored inside a water phantom:

        cd /Applications/TOPAS/topas-install/examples/Scoring
        ../../bin/topas Dose.txt

2. For those that decided automate the process as described in Step 8.2 onwards, TOPAS can now be run with the `topas` command in your terminal without having to setup the environment variables:

        topas Dose.txt

______________________________________________________________________________________________

### Step 10
As an aside, for those interested in being able to run quality checks, this is done through the continuous integration test suite for TOPAS. Python and `pip3` will be needed to run these tests, and so we recommend that installation be performed using [Homebrew](https://brew.sh/) to avoid messing with your system Python. The following command installs Python 3.x and `pip3` is installed automatically.

        brew install python

The TOPAS tests are located in the “tests” folder of the TOPAS source code directory, and testing is performed using `nrtest` and the TOPAS-specific plugins contained in [nrtest-topas](https://github.com/davidchall/nrtest-topas):

        cd /Applications/TOPAS/topas
        pip3 install nrtest
        pip3 install git+https://github.com/davidchall/nrtest-topas.git

Modify the `apps/topas-HEAD_4.11.1.2.json` metadata file according to your directories and configuration (remember to set your environment variables) and execute the entire test suite as follows:

        nrtest execute apps/topas-HEAD_4.11.1.2.json tests/ -o benchmark/todayDate

Comparisons can also be made with the following command:
        
        nrtest compare benchmarks/today benchmarks/yesterday


