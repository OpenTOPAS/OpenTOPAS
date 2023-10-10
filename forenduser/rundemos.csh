#!/bin/csh -f
#
cd examples/TimeFeature
../../bin/topas BoxWithinBox.txt
../../bin/topas CameraRotateAndZoom.txt
../../bin/topas CylinderGrowingInPhi.txt
cd ../UCSFETF
../../bin/topas User_SOBP24_Viewer.txt
../../bin/topas User_Beamline_WC_Viewer.txt
cd ../SpecialComponents
../../bin/topas QuadAndDipoleMagnets.txt
../../bin/topas PurgingMagnet_move.txt
../../bin/topas MultiLeafCollimator_sequence.txt
../../bin/topas RangeModulator_CurrentModulatedBeam.txt
cd ../Nozzle
../../bin/topas ScatteringNozzle_run.txt
../../bin/topas ScanningTargetMovingHorizontal.txt
cd ../Patient
unzip Abdomen.zip
../../bin/topas ViewAbdomen.txt
rm -rf Abdomen
tar -zxf synthetic_lung.tar.bz2
../../bin/topas DoseTo4DCT.txt
rm -rf synthetic_lung
unzip DICOM_Box
../../bin/topas Implant.txt
rm -rf DICOM_Box
cd ../TimeFeature
../../bin/topas Darkening.txt
rm *.csv
../../bin/topas Logo.txt
cd ../..
rm examples/*/NbParticlesInTime.txt
