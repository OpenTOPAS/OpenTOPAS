#!/bin/csh -f
#
cd examples/TimeFeature
/Applications/tswork/topas-build/bin/topas BoxWithinBox.txt
/Applications/tswork/topas-build/bin/topas CameraRotateAndZoom.txt
/Applications/tswork/topas-build/bin/topas CylinderGrowingInPhi.txt
cd ../UCSFETF
/Applications/tswork/topas-build/bin/topas User_SOBP24_Viewer.txt
/Applications/tswork/topas-build/bin/topas User_Beamline_WC_Viewer.txt 
cd ../SpecialComponents
/Applications/tswork/topas-build/bin/topas QuadAndDipoleMagnets.txt
/Applications/tswork/topas-build/bin/topas PurgingMagnet_move.txt
/Applications/tswork/topas-build/bin/topas MultiLeafCollimator_sequence.txt
/Applications/tswork/topas-build/bin/topas RangeModulator_CurrentModulatedBeam.txt
cd ../Nozzle
/Applications/tswork/topas-build/bin/topas ScatteringNozzle_run.txt
/Applications/tswork/topas-build/bin/topas ScanningTargetMovingHorizontal.txt
cd ../FHBPTC
/Applications/tswork/topas-build/bin/topas FullSetupWithScattering_ZoomAndPan.txt
cd ../Patient
unzip Abdomen.zip
/Applications/tswork/topas-build/bin/topas ViewAbdomen.txt
rm -rf Abdomen
tar -zxf synthetic_lung.tar.bz2
/Applications/tswork/topas-build/bin/topas DoseTo4DCT.txt
rm -rf synthetic_lung
unzip DICOM_Box
/Applications/tswork/topas-build/bin/topas Implant.txt
rm -rf DICOM_Box
cd ../TimeFeature
/Applications/tswork/topas-build/bin/topas Darkening.txt
rm *.csv
/Applications/tswork/topas-build/bin/topas Logo.txt
cd ../..
rm examples/*/NbParticlesInTime.txt
