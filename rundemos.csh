#!/bin/csh
# TOPAS demo runner

set start_dir = `pwd`
set repo_root = ""
set script_path = "$0"

# Determine script directory unless sourced without a filename.
if ("$script_path" == "csh" || "$script_path" == "-csh" || "$script_path" == "tcsh") then
	set script_path = ""
endif

if ("$script_path" != "") then
	if ("$script_path" !~ */*) then
		set resolved = `which "$script_path" 2> /dev/null`
		if ($status == 0 && "$resolved" != "") then
			set script_path = "$resolved"
		endif
	endif
	set script_dir = "$script_path:h"
	if ("$script_dir" == "" || "$script_dir" == "$script_path") then
		set script_dir = "."
	endif
	if (-d "$script_dir") then
		pushd "$script_dir" > /dev/null
		set repo_root = `pwd`
		popd > /dev/null
	endif
endif

if ("$repo_root" == "" && $?TOPAS_HOME) then
	set repo_root = "$TOPAS_HOME"
endif

if ("$repo_root" == "") then
	set repo_root = "$start_dir"
endif

if (! -d "$repo_root/examples") then
	echo "Unable to locate TOPAS repository root from '$repo_root'."
	echo "Set TOPAS_HOME or run the script via an explicit path."
	exit 1
endif

set topas_cmd = ""
if ($?TOPAS_CMD) then
	set topas_cmd = "$TOPAS_CMD"
else
	set topas_lookup = `which topas 2> /dev/null`
	if ($status == 0 && "$topas_lookup" != "") then
		set topas_cmd = "$topas_lookup"
	endif
endif

if ("$topas_cmd" == "") then
	echo "Unable to locate the 'topas' executable."
	echo "Add it to your PATH or export TOPAS_CMD=/path/to/topas."
	exit 1
endif

set _demo_had_nonomatch = $?nonomatch
if (! $_demo_had_nonomatch) then
	set nonomatch
endif

set basic_examples = ( \
	AllParameterForms.txt \
	BatchJob1.txt \
	BatchJob2.txt \
	BatchJobShared.txt \
	BoxRotatedAroundItsCorner.txt \
	DistributedSourcePointsInShell.txt \
	DistributedSourcePointsInSphere.txt \
	DistributedSourcePointsInSphereGaussian.txt \
	DistributedSourcePointsInTwistedTubs.txt \
	DividedComponents.txt \
	DNAModelByRegions.txt \
	Emittance_Gaussian.txt \
	Emittance_Twiss.txt \
	EmModelByRegions.txt \
	EnvironmentSource.txt \
	ExtraSequence1.txt \
	ExtraSequence2.txt \
	FlatteningFilter.txt \
	Isotope.txt \
	LayeredMassGeometry.txt \
	OneBox.txt \
	OneBoxRotate.txt \
	OneBoxTranslate.txt \
	ParallelWorlds.txt \
	ParticleSourcesFromGroup.txt \
	PhysicsSetting.txt \
	QtShapeTest.txt \
	ShapeTestWithAllParameters.txt \
	ShapeTestWithOnlyRequiredParameters.txt \
	Spectrum.txt \
	TwoBeams.txt \
	VolumetricSource.txt \ \
	VoxelMaterialsInDividedComponents.txt )

set brachytherapy_case = ( \
	DoseTLE.txt )

set mvlinac_case = ( \
	VRT_HD.txt )

set timefeature_intro = ( \
	BoxWithinBox.txt \
	CameraRotateAndZoom.txt \
	CylinderGrowingInPhi.txt \
        Darkening.txt \
        Logo.txt )

set ucsf_cases = ( \
	User_SOBP24_Viewer.txt \
	User_Beamline_WC_Viewer.txt )

set special_cases = ( \
	QuadAndDipoleMagnets.txt \
	PurgingMagnet_move.txt \
	MultiLeafCollimator_sequence.txt \
	RangeModulator_CurrentModulatedBeam.txt )

set nozzle_cases = ( \
	ScatteringNozzle_run.txt \
	ScanningTargetMovingHorizontal.txt )

set base_cleanup = ( *.csv *.phsp *.header *.root *.bin *.dcm *.binheader *.html )
set demo_dirs = ( Basic Brachytherapy MVLinac TimeFeature UCSFETF SpecialComponents Nozzle )
set demo_titles = ( \
	"Basic demos" \
	"Brachytherapy demo" \
	"MVLinac with VRT demo" \
	"Time Feature demos (set 1)" \
	"UCSF ETF demos" \
	"Special component demos" \
	"Nozzle demos" )
set demo_cases = ( basic_examples brachytherapy_case mvlinac_case timefeature_intro ucsf_cases special_cases nozzle_cases )

set green = `tput setaf 2`
set red = `tput setaf 1`
set reset = `tput sgr0`

if (`locale charmap` == "UTF-8") then
	set check = "✅"
	set cross = "❌"
else
	set check = "[ Success ]"
	set cross = "[ FAIL ]"
endif

set exec_log = out
set total_count = 0
set success_count = 0
set fail_count = 0
set start_time = `date +%s`

echo "Running TOPAS demos using repository root: $repo_root"
echo "Using TOPAS executable: $topas_cmd"

cd "$repo_root/examples"
@ demo_index = 1
while ($demo_index <= $#demo_dirs)
	set current_dir = "$demo_dirs[$demo_index]"
	set current_title = "$demo_titles[$demo_index]"
	set current_cases_var = $demo_cases[$demo_index]
	set demo_path = "$repo_root/examples/$current_dir"

	if (! -d "$demo_path") then
		echo ""
		echo "======================================"
		echo "** $current_title **"
		echo "======================================"
		echo "Skipping $current_dir because directory is missing."
		@ demo_index++
		continue
	endif

	cd "$demo_path"
	echo ""
	echo "======================================"
	echo "** $current_title **"
	echo "======================================"

	eval set cases = '($'$current_cases_var')'

	foreach case ($cases)
		set case_dir = $case:h
		set case_file = $case:t
		if ("$case_dir" == "" || "$case_dir" == "$case") then
			set case_dir = "."
		endif
		pushd "$case_dir" > /dev/null
		echo "includeFile = $case_file" > run.txt
		echo 'b:Ts/UseQt = "False"' >> run.txt
		echo 'b:Ts/PauseBeforeQuit = "False"' >> run.txt
		echo 'b:Gr/Enable = "False"' >> run.txt
		echo "🚀 Running $case"
		"$topas_cmd" run.txt >&! $exec_log
		set matches = `awk '/Execution/ {count++} END {print count+0}' $exec_log`
		@ total_count++
		if ("$matches" == "1") then
			@ success_count++
			printf "   %s%s [ Success ]%s %s\n" "$green" "$check" "$reset" "$case"
		else
			@ fail_count++
			printf "   %s%s [ Fail ]%s %s\n" "$red" "$cross" "$reset" "$case"
		endif
		rm -f $exec_log run.txt
		find . -maxdepth 1 -type f \( \
			-name "*.csv" -o \
			-name "*.phsp" -o \
			-name "*.header" -o \
			-name "*.root" -o \
			-name "*.bin" -o \
			-name "*.dcm" -o \
			-name "*.binheader" -o \
			-name "*.html" \
		\) -delete
		popd > /dev/null
	end

	rm -f $base_cleanup $exec_log
	find "$demo_path" -maxdepth 1 -type f \( \
		-name "*.csv" -o \
		-name "*.phsp" -o \
		-name "*.header" -o \
		-name "*.root" -o \
		-name "*.bin" -o \
		-name "*.dcm" -o \
		-name "*.binheader" -o \
		-name "*.html" \
	\) -delete

	@ demo_index++
end

cd "$repo_root/examples/Patient"
echo ""
echo "======================================"
echo "** Patient demos **"
echo "======================================"

if (-f Abdomen.zip) then
	if (-d Abdomen) then
		rm -rf Abdomen
	endif
	echo "📦 Extracting Abdomen.zip"
	unzip -q -o Abdomen.zip
	if ($status != 0) then
		echo "Failed to extract Abdomen.zip"
		exit 1
	endif
else
	echo "Missing Abdomen.zip dataset in examples/Patient."
	exit 1
endif

foreach case ( ViewAbdomen.txt )
	set case_dir = $case:h
	set case_file = $case:t
	if ("$case_dir" == "" || "$case_dir" == "$case") then
		set case_dir = "."
	endif
	pushd "$case_dir" > /dev/null
	echo "includeFile = $case_file" > run.txt
	echo 'b:Ts/UseQt = "False"' >> run.txt
	echo 'b:Ts/PauseBeforeQuit = "False"' >> run.txt
	echo 'b:Gr/Enable = "False"' >> run.txt
	echo "🚀 Running $case"
	"$topas_cmd" run.txt >&! $exec_log
	set matches = `awk '/Execution/ {count++} END {print count+0}' $exec_log`
	@ total_count++
	if ("$matches" == "1") then
		@ success_count++
		printf "   %s%s [ Success ]%s %s\n" "$green" "$check" "$reset" "$case"
	else
		@ fail_count++
		printf "   %s%s [ Fail ]%s %s\n" "$red" "$cross" "$reset" "$case"
	endif
	rm -f $exec_log run.txt *.csv *.phsp *.header *.root *.bin 
	popd > /dev/null
end
rm -rf Abdomen

if (-f synthetic_lung.tar.bz2) then
	if (-d synthetic_lung) then
		rm -rf synthetic_lung
	endif
	echo "📦 Extracting synthetic_lung.tar.bz2"
	tar -xjf synthetic_lung.tar.bz2
	if ($status != 0) then
		echo "Failed to extract synthetic_lung.tar.bz2"
		exit 1
	endif
else
	echo "Missing synthetic_lung.tar.bz2 dataset in examples/Patient."
	exit 1
endif

foreach case ( DoseTo4DCT.txt )
	set case_dir = $case:h
	set case_file = $case:t
	if ("$case_dir" == "" || "$case_dir" == "$case") then
		set case_dir = "."
	endif
	pushd "$case_dir" > /dev/null
	echo "includeFile = $case_file" > run.txt
	echo 'b:Ts/UseQt = "False"' >> run.txt
	echo 'b:Ts/PauseBeforeQuit = "False"' >> run.txt
	echo 'b:Gr/Enable = "False"' >> run.txt
	echo "🚀 Running $case"
	"$topas_cmd" run.txt >&! $exec_log
	set matches = `awk '/Execution/ {count++} END {print count+0}' $exec_log`
	@ total_count++
	if ("$matches" == "1") then
		@ success_count++
		printf "   %s%s [ Success ]%s %s\n" "$green" "$check" "$reset" "$case"
	else
		@ fail_count++
		printf "   %s%s [ Fail ]%s %s\n" "$red" "$cross" "$reset" "$case"
	endif
	rm -f $exec_log run.txt *.csv *.phsp *.header *.root *.bin *.dcm
	popd > /dev/null
end
rm -rf synthetic_lung

if (-f DICOM_Box.zip) then
	if (-d DICOM_Box) then
		rm -rf DICOM_Box
	endif
	echo "📦 Extracting DICOM_Box.zip"
	unzip -q -o DICOM_Box.zip 
	if ($status != 0) then
		echo "Failed to extract DICOM_Box.zip"
		exit 1
	endif
else
	echo "Missing DICOM_Box.zip dataset in examples/Patient."
	exit 1
endif

foreach case ( Implant.txt )
	set case_dir = $case:h
	set case_file = $case:t
	if ("$case_dir" == "" || "$case_dir" == "$case") then
		set case_dir = "."
	endif
	pushd "$case_dir" > /dev/null
	echo "includeFile = $case_file" > run.txt
	echo 'b:Ts/UseQt = "False"' >> run.txt
	echo 'b:Ts/PauseBeforeQuit = "False"' >> run.txt
	echo 'b:Gr/Enable = "False"' >> run.txt
	echo "🚀 Running $case"
	"$topas_cmd" run.txt >&! $exec_log
	set matches = `awk '/Execution/ {count++} END {print count+0}' $exec_log`
	@ total_count++
	if ("$matches" == "1") then
		@ success_count++
		printf "   %s%s [ Success ]%s %s\n" "$green" "$check" "$reset" "$case"
	else
		@ fail_count++
		printf "   %s%s [ Fail ]%s %s\n" "$red" "$cross" "$reset" "$case"
	endif
	rm -f $exec_log run.txt *.csv *.phsp *.header *.root *.bin *.dcm
	popd > /dev/null
end
rm -rf DICOM_Box __MACOSX

rm -f *.csv *.phsp *.header *.root *.bin *.dcm $exec_log

rm -f "$repo_root"/examples/*/NbParticlesInTime.txt

cd "$start_dir"

set end_time = `date +%s`
@ elapsed = $end_time - $start_time

printf "\n📊  Regression completed in %d seconds (%.2f minutes)\n" $elapsed `echo "$elapsed / 60.0" | bc -l`
printf "   Cases run: %d | Succeeded: %d | Failed: %d\n" $total_count $success_count $fail_count

if (! $_demo_had_nonomatch) then
	unset nonomatch
endif
