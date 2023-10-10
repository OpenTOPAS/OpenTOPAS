#
# Continuous Integration Tests

add_test(NAME AllParameterForms.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas AllParameterForms.txt)

add_test(NAME BatchJob1.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas BatchJob1.txt)

add_test(NAME BatchJob2.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas BatchJob2.txt)

add_test(NAME BoxRotatedAroundItsCorner.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas BoxRotatedAroundItsCorner.txt)

add_test(NAME DistributedSourcePointsInShell.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas DistributedSourcePointsInShell.txt)

add_test(NAME DistributedSourcePointsInSphere.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas DistributedSourcePointsInSphere.txt)

add_test(NAME DistributedSourcePointsInSphereGaussian.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas DistributedSourcePointsInSphereGaussian.txt)

add_test(NAME DistributedSourcePointsInTwistedTubs.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas DistributedSourcePointsInTwistedTubs.txt)

add_test(NAME DividedComponents.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas DividedComponents.txt)

add_test(NAME DNAModelByRegions.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas DNAModelByRegions.txt)

add_test(NAME Emittance_Gaussian.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas Emittance_Gaussian.txt)

add_test(NAME Emittance_Twiss.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas Emittance_Twiss.txt)

add_test(NAME EmModelByRegions.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas EmModelByRegions.txt)

add_test(NAME EnvironmentSource.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas EnvironmentSource.txt)

add_test(NAME ExtraSequences.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas ExtraSequences.txt)

add_test(NAME FlatteningFilter.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas FlatteningFilter.txt)

add_test(NAME Isotope.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas Isotope.txt)

add_test(NAME LayeredMassGeometry.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas LayeredMassGeometry.txt)

add_test(NAME OneBox.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas OneBox.txt)

add_test(NAME OneBoxRotate.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas OneBoxRotate.txt)

add_test(NAME OneBoxTranslate.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas OneBoxTranslate.txt)

add_test(NAME ParallelWorlds.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas ParallelWorlds.txt)

add_test(NAME ParticleSourcesFromGroup.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas ParticleSourcesFromGroup.txt)

add_test(NAME PhysicsSetting.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas PhysicsSetting.txt)

add_test(NAME QtShapeTest.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas QtShapeTest.txt)

add_test(NAME ShapeTestWithAllParameters.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas ShapeTestWithAllParameters.txt)

add_test(NAME ShapeTestWithOnlyRequiredParameters.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas ShapeTestWithOnlyRequiredParameters.txt)

add_test(NAME Spectrum.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas Spectrum.txt)

add_test(NAME TwoBeams.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas TwoBeams.txt)

add_test(NAME VolumetricSource.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas VolumetricSource.txt)

add_test(NAME VoxelMaterialsInDividedComponents.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Basic
         COMMAND ../../build/topas VoxelMaterialsInDividedComponents.txt)

add_test(NAME DoseTLE.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Brachytherapy
         COMMAND ../../build/topas DoseTLE.txt)

add_test(NAME HDRSource.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Brachytherapy
         COMMAND ../../build/topas HDRSource.txt)

add_test(NAME HDRSourceInApplicator.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Brachytherapy
         COMMAND ../../build/topas HDRSourceInApplicator.txt)

add_test(NAME LDRSource.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Brachytherapy
         COMMAND ../../build/topas LDRSource.txt)

add_test(NAME MainTxHead.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/MVLinac
         COMMAND ../../build/topas MainTxHead.txt)

add_test(NAME VRT_1.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/MVLinac
         COMMAND ../../build/topas VRT_1.txt)

add_test(NAME VRT_2.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/MVLinac
         COMMAND ../../build/topas VRT_2.txt)

add_test(NAME VRT_3.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/MVLinac
         COMMAND ../../build/topas VRT_3.txt)

#add_test(NAME VRT_HD.txt
#	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/MVLinac
#         COMMAND ../../build/topas VRT_HD.txt)

add_test(NAME ScanningNozzle.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Nozzle
         COMMAND ../../build/topas ScanningNozzle.txt)

add_test(NAME ScanningStationaryTarget.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Nozzle
         COMMAND ../../build/topas ScanningStationaryTarget.txt)

add_test(NAME ScanningTargetMovingHorizontal.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Nozzle
         COMMAND ../../build/topas ScanningTargetMovingHorizontal.txt)

add_test(NAME ScanningTargetMovingInDepth.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Nozzle
         COMMAND ../../build/topas ScanningTargetMovingInDepth.txt)

add_test(NAME ScatteringNozzle_run.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Nozzle
         COMMAND ../../build/topas ScatteringNozzle_run.txt)

add_test(NAME OpticalPhotonCount.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Optical
         COMMAND ../../build/topas OpticalPhotonCount.txt)

add_test(NAME PixelatedDetector.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Optical
         COMMAND ../../build/topas PixelatedDetector.txt)

add_test(NAME PlasticScintillator.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Optical
         COMMAND ../../build/topas PlasticScintillator.txt)

add_test(NAME Rotating_Surfaces.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Optical
         COMMAND ../../build/topas Rotating_Surfaces.txt)

add_test(NAME WavelengthShifter.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Optical
         COMMAND ../../build/topas WavelengthShifter.txt)

add_test(NAME TestOutcomeModel.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Outcome
         COMMAND ../../build/topas TestOutcomeModel.txt)

add_test(NAME TestRestoreModel.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Outcome
         COMMAND ../../build/topas TestRestoreModel.txt)

add_test(NAME Applicator.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Patient
         COMMAND bash -c "unzip Abdomen.zip ; ../../build/topas Applicator.txt")

add_test(NAME DoseTo4DCT.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Patient
         COMMAND bash -c "tar -xf synthetic_lung.tar.bz2 ; ../../build/topas DoseTo4DCT.txt")

add_test(NAME DoseToCT.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Patient
         COMMAND bash -c "unzip DICOM_Box.zip ; ../../build/topas DoseToCT.txt")

add_test(NAME Implant.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Patient
         COMMAND ../../build/topas Implant.txt)

add_test(NAME PatientInIEC_1.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Patient
         COMMAND ../../build/topas PatientInIEC_1.txt)

add_test(NAME PatientInIEC_2.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Patient
         COMMAND ../../build/topas PatientInIEC_2.txt)

add_test(NAME PatientInIEC_2_time_feature.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Patient
         COMMAND ../../build/topas PatientInIEC_2_time_feature.txt)

add_test(NAME PatientInIEC_3.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Patient
         COMMAND ../../build/topas PatientInIEC_3.txt)

add_test(NAME TwoDicoms.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Patient
         COMMAND ../../build/topas TwoDicoms.txt)

add_test(NAME ViewAbdomen_rtdose.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Patient
         COMMAND ../../build/topas ViewAbdomen_rtdose.txt)

add_test(NAME ViewAbdomen.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Patient
         COMMAND ../../build/topas ViewAbdomen.txt)

add_test(NAME XCAT.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Patient
         COMMAND ../../build/topas XCAT.txt)

add_test(NAME MultiRun_Write_ASCII.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/PhaseSpace
         COMMAND ../../build/topas MultiRun_Write_ASCII.txt)

add_test(NAME WriteASCII.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/PhaseSpace
         COMMAND ../../build/topas WriteASCII.txt)

add_test(NAME ReadASCII.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/PhaseSpace
         COMMAND ../../build/topas ReadASCII.txt)

add_test(NAME WriteBinary.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/PhaseSpace
         COMMAND ../../build/topas WriteBinary.txt)

add_test(NAME ReadBinary.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/PhaseSpace
         COMMAND ../../build/topas ReadBinary.txt)

add_test(NAME WriteLimited.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/PhaseSpace
         COMMAND ../../build/topas WriteLimited.txt)

add_test(NAME ReadLimited.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/PhaseSpace
         COMMAND ../../build/topas ReadLimited.txt)

add_test(NAME WriteIonsASCII.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/PhaseSpace
         COMMAND ../../build/topas WriteIonsASCII.txt)

add_test(NAME WriteROOT.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/PhaseSpace
         COMMAND ../../build/topas WriteROOT.txt)

add_test(NAME ChargeInSphere.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas ChargeInSphere.txt)

add_test(NAME Complex.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas Complex.txt)

add_test(NAME Dose.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas Dose.txt)

add_test(NAME DoseInVoxelMaterials.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas DoseInVoxelMaterials.txt)

add_test(NAME DoseToMediumVsWater.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas DoseToMediumVsWater.txt)

add_test(NAME DoseVolumeHistogram.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas DoseVolumeHistogram.txt)

add_test(NAME EnergyAndDose.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas EnergyAndDose.txt)

add_test(NAME EnergyDepositBinnedByEnergy.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas EnergyDepositBinnedByEnergy.txt)

add_test(NAME EnergyDivisions.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas EnergyDivisions.txt)

add_test(NAME EnergyInBinnedCylinder.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas EnergyInBinnedCylinder.txt)

add_test(NAME Filters.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas Filters.txt)

add_test(NAME FilterByInteractionCount.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas FilterByInteractionCount.txt)

add_test(NAME FoilToBox.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas FoilToBox.txt)

add_test(NAME FoilToCylinder.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas FoilToCylinder.txt)

add_test(NAME FoilToSphere.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas FoilToSphere.txt)

add_test(NAME Gated.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas Gated.txt)

add_test(NAME GeometryDivisions.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas GeometryDivisions.txt)

add_test(NAME Histograms.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas Histograms.txt)

add_test(NAME Inactive.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas Inactive.txt)

add_test(NAME Ion.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas Ion.txt)

add_test(NAME Origin.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas Origin.txt)

add_test(NAME OriginCount.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas OriginCount.txt)

add_test(NAME ParallelBoxRebinned.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas ParallelBoxRebinned.txt)

add_test(NAME SparsifyAndSingleIndex.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas SparsifyAndSingleIndex.txt)

add_test(NAME SplitByTimeFeature.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas SplitByTimeFeature.txt)

add_test(NAME Surfaces.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas Surfaces.txt)

add_test(NAME VoxelMaterials.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/Scoring
         COMMAND ../../build/topas VoxelMaterials.txt)

add_test(NAME CAD.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/SpecialComponents
         COMMAND ../../build/topas CAD.txt)

add_test(NAME DipoleMagnet.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/SpecialComponents
         COMMAND ../../build/topas DipoleMagnet.txt)

add_test(NAME MultiLeafCollimator_sequence.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/SpecialComponents
         COMMAND ../../build/topas MultiLeafCollimator_sequence.txt)

add_test(NAME MultiWire_Chamber.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/SpecialComponents
         COMMAND ../../build/topas MultiWire_Chamber.txt)

add_test(NAME Propeller_ContinuousRotation.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/SpecialComponents
         COMMAND ../../build/topas Propeller_ContinuousRotation.txt)

add_test(NAME Propeller_StepRotation.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/SpecialComponents
         COMMAND ../../build/topas Propeller_StepRotation.txt)

add_test(NAME PurgingMagnet_move.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/SpecialComponents
         COMMAND ../../build/topas PurgingMagnet_move.txt)

add_test(NAME QuadAndDipoleMagnets.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/SpecialComponents
         COMMAND ../../build/topas QuadAndDipoleMagnets.txt)

add_test(NAME QuadInMovingNozzle.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/SpecialComponents
         COMMAND ../../build/topas QuadInMovingNozzle.txt)

add_test(NAME QuadrupoleMagnet.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/SpecialComponents
         COMMAND ../../build/topas QuadrupoleMagnet.txt)

add_test(NAME RangeModulator_ConstantBeam.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/SpecialComponents
         COMMAND ../../build/topas RangeModulator_ConstantBeam.txt)

add_test(NAME RangeModulator_CurrentModulatedBeam.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/SpecialComponents
         COMMAND ../../build/topas RangeModulator_CurrentModulatedBeam.txt)

add_test(NAME RidgeFilter.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/SpecialComponents
         COMMAND ../../build/topas RidgeFilter.txt)

add_test(NAME RotatingMagnet.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/SpecialComponents
         COMMAND ../../build/topas RotatingMagnet.txt)

add_test(NAME UniformElectroMagneticField.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/SpecialComponents
         COMMAND ../../build/topas UniformElectroMagneticField.txt)

add_test(NAME BoxWithinBox.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/TimeFeature
         COMMAND ../../build/topas BoxWithinBox.txt)

add_test(NAME CameraRotateAndZoom.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/TimeFeature
         COMMAND ../../build/topas CameraRotateAndZoom.txt)

add_test(NAME ChangingKEFilterByTimeFeature.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/TimeFeature
         COMMAND ../../build/topas ChangingKEFilterByTimeFeature.txt)

add_test(NAME ColorChange.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/TimeFeature
         COMMAND ../../build/topas ColorChange.txt)

add_test(NAME CylinderGrowingInPhi.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/TimeFeature
         COMMAND ../../build/topas CylinderGrowingInPhi.txt)

add_test(NAME Darkening.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/TimeFeature
         COMMAND ../../build/topas Darkening.txt)

add_test(NAME Logo.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/TimeFeature
         COMMAND ../../build/topas Logo.txt)

add_test(NAME RasterScanOnMovingTarget.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/TimeFeature
         COMMAND ../../build/topas RasterScanOnMovingTarget.txt)

add_test(NAME RotateResizeRecolor.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/TimeFeature
         COMMAND ../../build/topas RotateResizeRecolor.txt)

add_test(NAME RotatingCylinderWithPhiCut.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/TimeFeature
         COMMAND ../../build/topas RotatingCylinderWithPhiCut.txt)

add_test(NAME Rotation.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/TimeFeature
         COMMAND ../../build/topas Rotation.txt)

add_test(NAME RunRandom_Mode.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/TimeFeature
         COMMAND ../../build/topas RunRandom_Mode.txt)

add_test(NAME RunSequential_Mode.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/TimeFeature
         COMMAND ../../build/topas RunSequential_Mode.txt)

add_test(NAME User_Beamline_WC_Viewer.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/UCSFETF
         COMMAND ../../build/topas User_Beamline_WC_Viewer.txt)

add_test(NAME User_BP_R28_WaterPhantom.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/UCSFETF
         COMMAND ../../build/topas User_BP_R28_WaterPhantom.txt)

add_test(NAME User_SOBP24_R28_WaterPhantom.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/UCSFETF
         COMMAND ../../build/topas User_SOBP24_R28_WaterPhantom.txt)

add_test(NAME User_SOBP24_Viewer.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/UCSFETF
         COMMAND ../../build/topas User_SOBP24_Viewer.txt)

add_test(NAME CrossSectionEnhancement.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/VarianceReduction
         COMMAND ../../build/topas CrossSectionEnhancement.txt)

add_test(NAME ForcedInteraction.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/VarianceReduction
         COMMAND ../../build/topas ForcedInteraction.txt)

add_test(NAME GeometricalParticleSplit.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/VarianceReduction
         COMMAND ../../build/topas GeometricalParticleSplit.txt)

add_test(NAME ImportanceSampling.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/VarianceReduction
         COMMAND ../../build/topas ImportanceSampling.txt)

add_test(NAME ImportanceSamplingMassGeometry.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/VarianceReduction
         COMMAND ../../build/topas ImportanceSamplingMassGeometry.txt)

add_test(NAME ImportanceSamplingSecondaryBiasing.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/VarianceReduction
         COMMAND ../../build/topas ImportanceSamplingSecondaryBiasing.txt)

add_test(NAME SecondaryBiasing.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/VarianceReduction
         COMMAND ../../build/topas SecondaryBiasing.txt)

add_test(NAME WeightWindow.txt
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples/VarianceReduction
         COMMAND ../../build/topas WeightWindow.txt)

add_test(NAME Field_01
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Field_01.txt)

add_test(NAME Field_02
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Field_02.txt)

add_test(NAME Field_03
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Field_03.txt)

add_test(NAME Geometry_01
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Geometry_01.txt)

add_test(NAME Geometry_01A
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Geometry_01A.txt)

add_test(NAME Geometry_02
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Geometry_02.txt)

add_test(NAME Geometry_03
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Geometry_03.txt)

add_test(NAME Optical_Scintillator
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Optical_Scintillator.txt)

add_test(NAME Optical_Surfaces
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Optical_Surfaces.txt)

add_test(NAME Optical_WLS
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Optical_WLS.txt)

add_test(NAME Parameter_01
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Parameter_01.txt)

add_test(NAME PhaseSpace_01A
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_01A.txt)

add_test(NAME PhaseSpace_01B
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_01B.txt)

add_test(NAME PhaseSpace_01C
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_01C.txt)

add_test(NAME PhaseSpace_01D
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_01D.txt)

add_test(NAME PhaseSpace_01E
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_01E.txt)

add_test(NAME PhaseSpace_01F
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_01F.txt)

add_test(NAME PhaseSpace_01G
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_01G.txt)

add_test(NAME PhaseSpace_02A
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_02A.txt)

add_test(NAME PhaseSpace_02B
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_02B.txt)

add_test(NAME PhaseSpace_02C
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_02C.txt)

add_test(NAME PhaseSpace_02D
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_02D.txt)

add_test(NAME PhaseSpace_02E
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_02E.txt)

add_test(NAME PhaseSpace_03A
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_03A.txt)

add_test(NAME PhaseSpace_03B
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_03B.txt)

add_test(NAME PhaseSpace_03C
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_03C.txt)

add_test(NAME PhaseSpace_03D
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_03D.txt)

add_test(NAME PhaseSpace_03E
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas PhaseSpace_03E.txt)

add_test(NAME Primary_01
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Primary_01.txt)

add_test(NAME Primary_02
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Primary_02.txt)

add_test(NAME Primary_02A
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Primary_02A.txt)

add_test(NAME Primary_02B
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Primary_02B.txt)

add_test(NAME Primary_02C
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Primary_02C.txt)

add_test(NAME Scoring_01
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_01.txt)

add_test(NAME Scoring_01A
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_01A.txt)

add_test(NAME Scoring_01B
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_01B.txt)

add_test(NAME Scoring_01C
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_01C.txt)

add_test(NAME Scoring_01D
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_01D.txt)

add_test(NAME Scoring_02
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_02.txt)

add_test(NAME Scoring_03
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_03.txt)

add_test(NAME Scoring_03A
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_03A.txt)

add_test(NAME Scoring_04
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_04.txt)

add_test(NAME Scoring_05
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_05.txt)

add_test(NAME Scoring_05A
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_05A.txt)

add_test(NAME Scoring_05B
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_05B.txt)

add_test(NAME Scoring_05C
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_05C.txt)

add_test(NAME Scoring_06
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_06.txt)

add_test(NAME Scoring_06A
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_06A.txt)

add_test(NAME Scoring_06B
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_06B.txt)

add_test(NAME Scoring_07
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas Scoring_07.txt)

add_test(NAME TimeFeature_01
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas TimeFeature_01.txt)

add_test(NAME TimeFeature_02
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas TimeFeature_02.txt)

add_test(NAME TimeFeature_03
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas TimeFeature_03.txt)

add_test(NAME TimeFeature_04
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas TimeFeature_04.txt)

add_test(NAME TimeFeature_05
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas TimeFeature_05.txt)

add_test(NAME vrt_CutByRegions
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas vrt_CutByRegions.txt)

add_test(NAME vrt_GeometricalParticleSplit
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas vrt_GeometricalParticleSplit.txt)

add_test(NAME vrt_KillOtherParticles
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas vrt_KillOtherParticles.txt)

add_test(NAME vrt_SecondaryBiasing_Directional
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas vrt_SecondaryBiasing_Directional.txt)

add_test(NAME vrt_SecondaryBiasing_Uniform
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas vrt_SecondaryBiasing_Uniform.txt)

add_test(NAME vrt_Shielding_ImpSamp_Mass
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas vrt_Shielding_ImpSamp_Mass.txt)

add_test(NAME vrt_Shielding_ImpSamp_Parallel
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas vrt_Shielding_ImpSamp_Parallel.txt)

add_test(NAME vrt_Shielding_WeightW_Mass
	 WORKING_DIRECTORY /home/runner/work/topas/topas/tests
         COMMAND ../build/topas vrt_Shielding_WeightW_Mass.txt)

add_test(NAME ShowOutputCSV
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples
         COMMAND bash -c "ls -la /home/runner/work/topas/topas/examples/*/*.csv")

add_test(NAME ShowOutputBinHeader
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples
         COMMAND bash -c "ls -la /home/runner/work/topas/topas/examples/*/*.binheader")

add_test(NAME ShowOutputBin
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples
         COMMAND bash -c "ls -la /home/runner/work/topas/topas/examples/*/*.bin")

add_test(NAME ShowOutputHeader
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples
         COMMAND bash -c "ls -la /home/runner/work/topas/topas/examples/*/*.header")

add_test(NAME ShowOutputPHSP
	 WORKING_DIRECTORY /home/runner/work/topas/topas/examples
         COMMAND bash -c "ls -la /home/runner/work/topas/topas/examples/*/*.phsp")
