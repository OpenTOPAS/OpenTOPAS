# QuickStart Guide for WSL
This page details the steps to be followed by <ins>Windows users</ins> in order to install OpenTOPAS in <ins>WSL</ins> and launch your first simulation. 

> [!WARNING]
> We recommend Windows 11 or higher since it includes all the visualization libraries built in. We take no responsibility if users encounter an issue with their PCs while following this guide.

> [!NOTE]
> **Steps 1-4 are used to install and prepare your WSL for installation of OpenTOPAS.** You can skip steps 1 to 4 if your system has already installed a Debian based Linux distribution in WSL.

## Step 1
Open the Windows command prompt by typing

        cmd

in the Windows search bar. The next image shows the program you should open:

<img src="https://github.com/OpenTOPAS/OpenTOPAS_Documentation/blob/8060e85cd1aa68d054f9f3a6df4ae6020f7b3a2c/getting-started/images/WSL_CMD_Image.png" alt="WSL CMD" width="600px">

## Step 2 

Install WSL by typing:

        wsl --install

in the Windows command prompt. Accept the pop-up windows asking for permissions (all software should come from Microsoft itself). The process may take a few minutes. After installation, verify that all components were sucessfully installed by typing the next command:

        wsl --version

The output should look like this after installation and verification:

![image2](https://github.com/OpenTOPAS/OpenTOPAS_Documentation/blob/8060e85cd1aa68d054f9f3a6df4ae6020f7b3a2c/getting-started/images/WSL_InstallingWSL.png)

## Step 3
Install a Linux distribution (Ubuntu, Debian, Suse, Oracle, etc)

> [!NOTE]
> **The rest of this guide will assume Ubuntu 24.04 as Linux distribution. Users may use a different distribution but they are responsible for checking out the different commands of each Linux distribution by themselves.**

To install Ubuntu 24.04, type in the terminal:

        wsl --install Ubuntu-24.04

Aftwe downloading all the necesary files, it will ask you to create a new user for this Linux distribution and a password. Both wil be needed when installing programs to access super user (sudo) privileges. Once the account has been created, it will automatically enter into Ubuntu terminal mode. This will be evident by the change in coloring of the text.

![image3](https://github.com/OpenTOPAS/OpenTOPAS_Documentation/blob/8060e85cd1aa68d054f9f3a6df4ae6020f7b3a2c/getting-started/images/WSL_InstallingUbuntu24.png)

> [!TIP]
> To exit the Ubuntu terminal, users can use the "exit" command. Similarly, to access it again, users can type "wsl" or "bash" in the windows command prompt. Users can have multiple Linux distributions installed at the same time in windows. In which case, WSL will ask for the user to specify which distribution to use by providing its name.

## Step 4
Update the repositories of the Ubuntu distribution by typing the following commands in the Ubuntu terminal:

        sudo apt-get update
        sudo apt-get upgrade

Accept the changes when prompted by typing Y and hitting enter.

## Step 5
Create the folders that will contain Geant4, GDCM and TOPAS.
By default, the Ubuntu terminal session will always be located at some "virtual" location whose path is in the following format:

        /mnt/c/Users/[WindowsUser]

To avoid incorrect directory linking, create an "Applications" folder in the Ubuntu home directory. To do so we will use the following commands:

        mkdir ~/Applications

To verify that this procedure was successful, move to the home directory and print its contents by using the next commands:

        cd $HOME
        ls -l

The contents of this directory will be printed in the terminal, similar to:

![image4](https://github.com/OpenTOPAS/OpenTOPAS_Documentation/blob/8060e85cd1aa68d054f9f3a6df4ae6020f7b3a2c/getting-started/images/WSL_MovingHome.png)

## Step 6
Install all the required libraries for Geant4 and TOPAS by using the following commands:

        sudo apt-get install -y libexpat1-dev
        sudo apt-get install -y libgl1-mesa-dev
        sudo apt-get install -y libglu1-mesa-dev
        sudo apt-get install -y libxt-dev
        sudo apt-get install -y xorg-dev
        sudo apt-get install -y build-essential
        sudo apt-get install -y libharfbuzz-dev
        sudo apt-get install -y git-all
        sudo apt-get install -y qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools

## Step 7
Install Gean4 11.1.3.
Make sure you are located in the "Applications" folder by doing the following command:

        cd ~/Applications

Then follow the next commands to create a Geant4 folder and download Geant4 from GitHub:

        mkdir ~/Applications/GEANT4
        cd ~/Applications/GEANT4
        wget https://gitlab.cern.ch/geant4/geant4/-/archive/v11.1.3/geant4-v11.1.3.tar.gz
        tar -zxf geant4-v11.1.3.tar.gz 

Your GEANT4 folder should look like this

![image5](https://github.com/OpenTOPAS/OpenTOPAS_Documentation/blob/8060e85cd1aa68d054f9f3a6df4ae6020f7b3a2c/getting-started/images/WSL_DownloadingG4.png)

Now we will prepare the folders for the Geant4 installation and Geant4 data files. For that, run the following commands:

        mkdir ~/Applications/GEANT4/geant4-build
        mkdir ~/Applications/GEANT4/geant4-install
        mkdir ~/Applications/GEANT4/G4DATA

Again, the GEANT4 folder should look as follows:

![image6](https://github.com/OpenTOPAS/OpenTOPAS_Documentation/blob/8060e85cd1aa68d054f9f3a6df4ae6020f7b3a2c/getting-started/images/WSL_G4Folders.png)

Now we can finish up the installation of Geant4 by using the following commands:

        cd ~/Applications/GEANT4/geant4-build
        cmake ../geant4-v11.1.3 -DGEANT4_INSTALL_DATA=OFF -DGEANT4_BUILD_MULTITHREADED=ON -DCMAKE_INSTALL_PREFIX=~/Applications/GEANT4/geant4-install -DCMAKE_PREFIX_PATH=/usr/lib/qt5 -DGEANT4_USE_QT=ON -DGEANT4_USE_OPENGL_X11=ON -DGEANT4_USE_RAYTRACER_X11=ON
        make -j10
        make install

Next, download the Geant4 data files by typing the following commands in the terminal:

        cd ~/Applications/GEANT4/G4DATA
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

Then, decompress the files with tar -zxf and clean the folder:

        tar -zxf G4NDL.4.7.tar.gz
        tar -zxf G4EMLOW.8.2.tar.gz
        tar -zxf G4PhotonEvaporation.5.7.tar.gz
        tar -zxf G4RadioactiveDecay.5.6.tar.gz
        tar -zxf G4PARTICLEXS.4.0.tar.gz
        tar -zxf G4PII.1.3.tar.gz
        tar -zxf G4RealSurface.2.2.tar.gz
        tar -zxf G4SAIDDATA.2.0.tar.gz
        tar -zxf G4ABLA.3.1.tar.gz
        tar -zxf G4INCL.1.0.tar.gz
        tar -zxf G4ENSDFSTATE.2.3.tar.gz
        tar -zxf G4TENDL.1.4.tar.gz
        tar -zxf LEND_GND1.3_ENDF.BVII.1.tar.gz
        rm *tar.gz

Your folder should look as follows:

![image7](https://github.com/OpenTOPAS/OpenTOPAS_Documentation/blob/8060e85cd1aa68d054f9f3a6df4ae6020f7b3a2c/getting-started/images/WSL_G4DataFiles.png)

## Step 8
Install GDCM and TOPAS.
All versions of TOPAS downloaded from the OpenTOPAS GitHub contain the source code of GDCM which needs to be compiled before installing OpenTOPAS. Start by creating the necessary folders to contain the TOPAS installation files:

        mkdir ~/Applications/TOPAS

Then, download OpenTOPAS from the GitHub repository using:

        cd ~/Applications/TOPAS
        git clone https://github.com/OpenTOPAS/OpenTOPAS.git

Compile GDCM as follows:

        cd OpenTOPAS
        mkdir gdcm-install
        mkdir gdcm-build
        tar -zxf gdcm-2.6.8.tar,gz
        cd gdcm-build
        cmake ../gdcm-2.6.8 -DGDCM_BUILD_SHARED_LIBS=ON -DGCM_BUILD_DOCBOOK_MANPAGES:BOOL=OFF -DCMAKE_INSTALL_PREFIX=../gdcm-install
        sudo make -j10 install

Compile OpenTOPAS by using the following commands:

        cd ~/Applications/TOPAS
        mkdir OpenTOPAS-build
        mkdir OpenTOPAS-install
        cd OpenTOPAS-build
        export Geant4_DIR=~/Applications/GEANT4/geant4-install
        export GDCM_DIR=~/Applications/TOPAS/OpenTOPAS/gdcm-install
        cmake ../OpenTOPAS -DCMAKE_INSTALL_PREFIX=../OpenTOPAS-install
        make -j10 install

Next, copy the Geant4 libraries into the "lib" folder of OpenTOPAS. To do so, we will use the following commands:

        cd ~/Applications/TOPAS/OpenTOPAS-install/lib
        cp ~/Applications/GEANT4/geant4-install/lib/libG4* ./

Whithout closing this same terminal, check if the installation was succesful using the following command:

        ~/Applications/TOPAS/OpenTOPAS-install/bin/topas

The output should look as follows if OpenTPAS was succesfully installed:

![image8](https://github.com/OpenTOPAS/OpenTOPAS_Documentation/blob/8060e85cd1aa68d054f9f3a6df4ae6020f7b3a2c/getting-started/images/WSL_Running_TOPAS.png)

## Step 9
Setup the environment variables so that the Geant4, GDCM and TOPAS paths don't need to be loaded every time a new terminal is open. We recommend creating a folder dedicated to your shell scripts with the following commands:

        mkdir ~/shellScripts
        echo "export PATH=~/shellScripts:$PATH" >> ~/.bashrc
        cd ~/shellScripts
        echo '#!/bin/bash' > topas
        echo '' >> topas
        echo ’export QT_QPA_PLATFORM_PLUGIN_PATH=~/Applications/TOPAS/OpenTOPAS-install/Frameworks’ >> topas
        echo ’export TOPAS_G4_DATA_DIR=~/Applications/GEANT4/G4DATA’ >> topas
        echo ’export LD_LIBRARY_PATH=~/Applications/TOPAS/OpenTOPAS-install/lib:$LD_LIBRARY_PATH’ >> topas
        echo ’~/Applications/TOPAS/OpenTOPAS-install/bin/topas $1’ >> topas
        chmod +x topas


## Step 10
Restart the Ubuntu terminal by typing "exit" and then "wsl". Then, use the following commands to test TOPAS:

       cd ~/Applications/TOPAS/OpenTOPAS-install/examples/Graphics
       topas QtTest.txt

The output should look like this:

![image9](https://github.com/OpenTOPAS/OpenTOPAS_Documentation/blob/8060e85cd1aa68d054f9f3a6df4ae6020f7b3a2c/getting-started/images/WSL_QTExample.png)

> [!NOTE]
> **Because every keyboard input is interpreted differently, there is a chance that the last commands for setting up the environment variables might have gone wrong.** To check that it is okay, ensure that the output of the following command is the same as the following image:

       cat ~/shellScripts/topas

![image8](https://github.com/OpenTOPAS/OpenTOPAS_Documentation/blob/8060e85cd1aa68d054f9f3a6df4ae6020f7b3a2c/getting-started/images/WSL_CheckingEnvironment.png)

If there is an additional " or ' anywhere in the output, then make sure to correct it by using vi/nano or any other text editor. Also, make sure that your user is shown in the sections where [naokikondo] appears.

______________________________________________________________________________________________

## Step 11
As an additional step for those interested in running quality checks, the continuous integration test suite for OpenTOPAS can be used. Python and `pip3` will be needed. 

> [!WARNING]
> We recommend that installation be performed using [Homebrew](https://brew.sh/) to avoid messing with your system Python. The following command installs Python 3.x and `pip3` is installed automatically.

        brew install python

The OpenTOPAS tests are located [here](https://github.com/OpenTOPAS/qi-opentopas.git), listed in the OpenTOPAS organization repositories, and testing is performed using `nrtest` and the OpenTOPAS-specific plugins contained in [nrtest-topas](https://github.com/davidchall/nrtest-topas):

        cd ~/Applications/TOPAS
        git clone https://github.com/OpenTOPAS/qi-opentopas.git
        cd qi-opentopas
        pip3 install nrtest
        pip3 install git+https://github.com/davidchall/nrtest-topas.git

Modify the `apps/topas-v4.0.json` metadata file according to your directories and configuration (remember to set your environment variables) and execute the entire test suite as follows:

        nrtest execute apps/topas-v4.0.json tests/ -o benchmarks/todayDate

Comparisons can also be made with the following command:
        
        nrtest compare benchmarks/today benchmarks/yesterday