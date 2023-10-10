Installation instructions for TOPAS Version @TOPAS_VERSION_MAJOR@.@TOPAS_VERSION_MINOR@

All use of TOPAS is governed by the TOPAS License Agreement provided in this directory as LICENSE.txt.

This README shows how to install and run TOPAS with and without user extensions.

TOPAS has been developed for macOS and Linux operating systems.

Windows users can run TOPAS by installing it within Windows Subsystem for Linux (WSL).
See "Additional Installation Notes for Windows Subsystem for Linux" at:
http://www.topasmc.org/user-guides

Registered members of the TOPAS User and Development Community can access help
through the TOPAS Monte Carlo User Forum.
To join this Community, go to:
topasmc.org/registration

These instructions are written for single user installations.
Systems administrators performing multi-user installations may need to adapt these recipes.

These instructions assume the user has basic familiarity with the use of paths, shells and environment variables on their chosen operating system.
For users who have that familiarity, it should be obvious how to adapt these recipes to their own installations.
For others who do not have that familiarity, we suggest you first read some general tutorials for paths, shells and environment variables.

This product includes software developed by Members of the Geant4 Collaboration ( http://cern.ch/geant4 ), GDCM ( http://gdcm.sourceforge.net ) and The Qt Company ( http://qt.io ).


0) Pre-Requisites:
You will only need to do this part one time for each new computer.

Debian:
Install the following:
apt install -y libexpat1-dev
apt install -y libgl1-mesa-dev
apt install -y libglu1-mesa-dev
apt install -y libxt-dev
apt install -y xorg-dev
apt install -y build-essential
apt install -y libharfbuzz-dev

CentOS:
Install the following:
yum groupinstall -y "Development Tools"
yum install -y expat-devel
yum install -y libXmu-devel
yum install -y mesa-libGL-devel
yum install -y mesa-libGLU-devel


1) Install TOPAS:
You will only need to do this part one time for each new TOPAS version.

macOS 11.7 or later:
Download the following file from the TOPAS Code Repository web page:
topas_3_9_macOS11_7.zip
Double-click on the downloaded file to unzip it.
Move the resulting directory so that you have /Applications/topas
Before you can run TOPAS for the first time,
you need to tell the macOS Gatekeeper that TOPAS is OK to run.
You do this by right clicking (that is, Control click) on the file: topas/bin/topas
and selecting Open.
This will not actually start TOPAS successfully, but it will tell Gatekeeper that you approve of running topas.
After that, you can run topas from the terminal window in the usual way that is explained in step 3 below.

Debian9 or Debian10:
Download the following two files from the TOPAS Code Repository web page:
topas_3_9_debian9.tar.gz.part_1
topas_3_9_debian9.tar.gz.part_2
Combine them using the "cat" command and then unpack the result as follows:
cat topas_3_9_debian9.tar.gz.part_* > topas_3_9_debian9.tar.gz
tar -zxvf topas_3_9_debian9.tar.gz
Move the result so that you have ~/topas

CentOS7:
Download the following two files from the TOPAS Code Repository web page:
topas_3_9_centos7.tar.gz.part_1
topas_3_9_centos7.tar.gz.part_2
Combine them using the "cat" command and then unpack the result as follows:
cat topas_3_9_centos7.tar.gz.part_* > topas_3_9_centos7.tar.gz
tar -zxvf topas_3_9_centos7.tar.gz
Move the result so that you have ~/topas

CentOS8:
Download the following two files from the TOPAS Code Repository web page:
topas_3_9_centos8.tar.gz.part_1
topas_3_9_centos8.tar.gz.part_2
Combine them using the "cat" command and then unpack the result as follows:
cat topas_3_9_centos8.tar.gz.part_* > topas_3_9_centos8.tar.gz
tar -zxvf topas_3_9_centos8.tar.gz
Move the result so that you have ~/topas


2) Install Data Files:
You may only need to do this one time for your first TOPAS version.

In most cases, when we upgrade TOPAS you do not need to upgrade these Data files.
The TOPAS release notes will tell you if you actually need to update these Data files.
(it only needs doing when we change the underlying Geant4 release inside of TOPAS).

The only part of Geant4 that you need to install are the data files.
You do not need to download or build any other part of Geant4 since the necessary
Geant4 libraries and header files are already included in TOPAS.

Download and Install Geant4 Data files into your /Applications/G4Data directory.

macOS:
You do this by issuing the following commands from a Terminal window:
mkdir /Applications/G4Data
cd /Applications/G4Data
curl -O https://geant4-data.web.cern.ch/geant4-data/datasets/G4NDL.4.6.tar.gz
curl -O https://geant4-data.web.cern.ch/geant4-data/datasets/G4EMLOW.7.13.tar.gz
curl -O https://geant4-data.web.cern.ch/geant4-data/datasets/G4PhotonEvaporation.5.7.tar.gz
curl -O https://geant4-data.web.cern.ch/geant4-data/datasets/G4RadioactiveDecay.5.6.tar.gz
curl -O https://geant4-data.web.cern.ch/geant4-data/datasets/G4PARTICLEXS.3.1.1.tar.gz
curl -O https://geant4-data.web.cern.ch/geant4-data/datasets/G4SAIDDATA.2.0.tar.gz
curl -O https://geant4-data.web.cern.ch/geant4-data/datasets/G4ABLA.3.1.tar.gz
curl -O https://geant4-data.web.cern.ch/geant4-data/datasets/G4INCL.1.0.tar.gz
curl -O https://geant4-data.web.cern.ch/geant4-data/datasets/G4PII.1.3.tar.gz
curl -O https://geant4-data.web.cern.ch/geant4-data/datasets/G4ENSDFSTATE.2.3.tar.gz
curl -O https://geant4-data.web.cern.ch/geant4-data/datasets/G4RealSurface.2.2.tar.gz
curl -O https://geant4-data.web.cern.ch/geant4-data/datasets/G4TENDL.1.3.2.tar.gz
tar -zxf G4NDL.4.6.tar.gz
tar -zxf G4EMLOW.7.13.tar.gz
tar -zxf G4PhotonEvaporation.5.7.tar.gz
tar -zxf G4RadioactiveDecay.5.6.tar.gz
tar -zxf G4PARTICLEXS.3.1.1.tar.gz
tar -zxf G4SAIDDATA.2.0.tar.gz
tar -zxf G4ABLA.3.1.tar.gz
tar -xzf G4INCL.1.0.tar.gz
tar -zxf G4PII.1.3.tar.gz
tar -zxf G4ENSDFSTATE.2.3.tar.gz
tar -zxf G4RealSurface.2.2.tar.gz
tar -zxf G4TENDL.1.3.2.tar.gz

Linux:
You do this by issuing the following commands from a Terminal window:
mkdir ~/G4Data
cd ~/G4Data
wget -4 https://geant4-data.web.cern.ch/geant4-data/datasets/G4NDL.4.6.tar.gz
wget -4 https://geant4-data.web.cern.ch/geant4-data/datasets/G4EMLOW.7.13.tar.gz
wget -4 https://geant4-data.web.cern.ch/geant4-data/datasets/G4PhotonEvaporation.5.7.tar.gz
wget -4 https://geant4-data.web.cern.ch/geant4-data/datasets/G4RadioactiveDecay.5.6.tar.gz
wget -4 https://geant4-data.web.cern.ch/geant4-data/datasets/G4PARTICLEXS.3.1.1.tar.gz
wget -4 https://geant4-data.web.cern.ch/geant4-data/datasets/G4SAIDDATA.2.0.tar.gz
wget -4 https://geant4-data.web.cern.ch/geant4-data/datasets/G4ABLA.3.1.tar.gz
wget -4 https://geant4-data.web.cern.ch/geant4-data/datasets/G4INCL.1.0.tar.gz
wget -4 https://geant4-data.web.cern.ch/geant4-data/datasets/G4PII.1.3.tar.gz
wget -4 https://geant4-data.web.cern.ch/geant4-data/datasets/G4ENSDFSTATE.2.3.tar.gz
wget -4 https://geant4-data.web.cern.ch/geant4-data/datasets/G4RealSurface.2.2.tar.gz
wget -4 https://geant4-data.web.cern.ch/geant4-data/datasets/G4TENDL.1.3.2.tar.gz
tar -zxf G4NDL.4.6.tar.gz
tar -zxf G4EMLOW.7.13.tar.gz
tar -zxf G4PhotonEvaporation.5.7.tar.gz
tar -zxf G4RadioactiveDecay.5.6.tar.gz
tar -zxf G4PARTICLEXS.3.1.1.tar.gz
tar -zxf G4SAIDDATA.2.0.tar.gz
tar -zxf G4ABLA.3.1.tar.gz
tar -xzf G4INCL.1.0.tar.gz
tar -zxf G4PII.1.3.tar.gz
tar -zxf G4ENSDFSTATE.2.3.tar.gz
tar -zxf G4RealSurface.2.2.tar.gz
tar -zxf G4TENDL.1.3.2.tar.gz


3) Set up the environment:
You will need to do this every time you open a fresh Terminal window.

You may choose to put this setup information into one of your startup files,
but we recommend you only do that if you are very comfortable with such things,
as you may accidentally affect other processes.

macOS:
export TOPAS_G4_DATA_DIR=/Applications/G4Data
export QT_QPA_PLATFORM_PLUGIN_PATH=/Applications/topas/Frameworks

Linux Bourne shell:
export TOPAS_G4_DATA_DIR=~/G4Data

Linux C shell:
setenv TOPAS_G4_DATA_DIR ~/G4Data


4) Run TOPAS:
When using the commands below, remember to replace "/Applications/topas"
with the actual path to where you put your copy of TOPAS.

To run a single example:
cd to the directory that the example is in, then run topas from there, for example:
cd /Applications/topas/examples/SpecialComponents
/Applications/topas/bin/topas MultiLeafCollimator_sequence.txt

To test TOPAS with DICOM:
Unzip the example DICOM directories in examples/Patient
cd /Applications/topas/examples/Patient
/Applications/topas/bin/topas ViewAbdomen.txt

To test TOPAS with the Qt Graphical User Interface:
cd /Applications/topas/examples/Graphics
/Applications/topas/bin/topas QtTest.txt

That's it. Unless you need to add C++ Extensions to your TOPAS, you are done with setup!



The rest of these instructions are only required if you want to add C++ Extensions.
If the pre-built TOPAS has all the features you need, you do not need these steps.

5) To add User Extensions:

5a) Get an Appropriate Compiler:

macOS:
Install the Xcode compiler, version 11.7 or newer, from:
https://developer.apple.com/xcode/downloads/

Newest Macs with the M1 chip:
TOPAS has been built for Intel chips.
To run it on the newer M1 Macs, you will need Apple's adapter tool: Rosetta 2.
Install this by:
sudo /usr/sbin/softwareupdate --install-rosetta --agree-to-license

Debian9:  generally comes with appropriate g++ --version 6.3.0
Debian10: generally comes with appropriate g++ --version 8.3.0
CentOS8:  generally comes with appropriate g++ --version 8.2.1
Some research clusters may have their default compilers set to some different g++ version.
In that case, you may need to issue some command provided by your system administrators to change compilers.

CentOS7: generally comes with a version of g++ that is too old --version 4.8.5
If you are maintaining your own CentOS7 system, you may use the following:
yum install -y centos-release-scl
yum install -y devtoolset-7
Then, each time you open a terminal window, activate this new compiler by doing:
source /opt/rh/devtoolset-7/enable


5b) Get CMake:
You will need a tool called CMake (version 3.9 or newer).
Type "which cmake" to see if you already have this tool
and "cmake -version" to see what version you may have.

If you need to install CMake, install it as follows:

macOS:
Get the binary distribution at:
http://www.cmake.org/cmake/resources/software.html
Run the cmake.app
and follow the instructions in CMake's menu item: "Tools"... "How to Install for Command Line Use"

Debian9 or CentOS7:
Build a new CMake version from source as follows:
cd /usr
wget https://cmake.org/files/v3.9/cmake-3.9.6-Linux-x86_64.sh -P /usr/
chmod 755 /usr/cmake-3.9.6-Linux-x86_64.sh
./cmake-3.9.6-Linux-x86_64.sh
Answer y to accept the license.
Answer n to putting this new cmake into a subdirectory.

Debian10:
apt install -y cmake

CentOS8:
yum install -y cmake


5c) Build your New TOPAS:
Place your TOPAS extension code into a directory that is NOT inside of the topas directory.
macOS:   we suggest /Applications/topas_extensions
Linux: we suggest ~/topas_extensions

Newest Macs with the M1 chip:
cd /Applications/topas
unzip Geant4Headers.zip
cmake -DTOPAS_EXTENSIONS_DIR=/Applications/topas_extensions -DCMAKE_OSX_ARCHITECTURES=x86_64
make

Other Macs:
cd /Applications/topas
unzip Geant4Headers.zip
cmake -DTOPAS_EXTENSIONS_DIR=/Applications/topas_extensions
make

Linux:
cd ~/topas
unzip Geant4Headers.zip
cmake -DTOPAS_EXTENSIONS_DIR=~/topas_extensions
make

For every extension file that was correctly found, cmake will print out a message such as:
SomeFileName is a Component for SomeComponentName
SomeFileName is a Scorer for SomeScorerName
etc.

If one of your extension files was not listed in the messages from cmake,
check again that the file was correctly contained in your extensions directory.
Also check that the first line of your cc file had the correct syntax.
Compare that first line to one of the provided examples.
The first line must follow the expected pattern precisely,
including exact spacing and exact usage of upper and lower case.

Don't worry about any "Warning" messages. Only worry about messages that say "Error".
If the build worked, you will have a new topas executable.

CMake caches the name of your extensions directory and watches for subsequent changes there.
If you make changes to any of the extensions you were already including
(such as while you are debugging your extensions), you just need to re-run
make

If you also add new files to your extensions directory, then you will need to re-run
both cmake and make.
