#!/bin/csh
# TOPAS demo runner modeled after the TOPAS-nBio workflow.

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

set fhbptc_cases = ( \
	FullSetupWithScattering_ZoomAndPan.txt )

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
set start_time = `date +%s`

echo "Running TOPAS demos using repository root: $repo_root"
echo "Using TOPAS executable: $topas_cmd"

cd "$repo_root/examples/TimeFeature"
echo ""
echo "======================================"
echo "** Time Feature demos (set 1) **"
echo "======================================"
foreach case ($timefeature_intro)
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
	if ("$matches" == "1") then
		printf "   %s%s [ Success ]%s %s\n" "$green" "$check" "$reset" "$case"
	else
		printf "   %s%s [ Fail ]%s %s\n" "$red" "$cross" "$reset" "$case"
	endif
	rm -f $exec_log run.txt *.csv *.phsp *.header *.root *.bin *.xyz *.sdd
	popd > /dev/null
end
rm -f *.csv *.phsp *.header *.root *.bin *.xyz *.sdd $exec_log

cd "$repo_root/examples/UCSFETF"
echo ""
echo "======================================"
echo "** UCSF ETF demos **"
echo "======================================"
foreach case ($ucsf_cases)
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
	if ("$matches" == "1") then
		printf "   %s%s [ Success ]%s %s\n" "$green" "$check" "$reset" "$case"
	else
		printf "   %s%s [ Fail ]%s %s\n" "$red" "$cross" "$reset" "$case"
	endif
	rm -f $exec_log run.txt *.csv *.phsp *.header *.root *.bin *.xyz *.sdd
	popd > /dev/null
end
rm -f *.csv *.phsp *.header *.root *.bin *.xyz *.sdd $exec_log

cd "$repo_root/examples/SpecialComponents"
echo ""
echo "======================================"
echo "** Special component demos **"
echo "======================================"
foreach case ($special_cases)
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
	if ("$matches" == "1") then
		printf "   %s%s [ Success ]%s %s\n" "$green" "$check" "$reset" "$case"
	else
		printf "   %s%s [ Fail ]%s %s\n" "$red" "$cross" "$reset" "$case"
	endif
	rm -f $exec_log run.txt *.csv *.phsp *.header *.root *.bin *.xyz *.sdd
	popd > /dev/null
end
rm -f *.csv *.phsp *.header *.root *.bin *.xyz *.sdd $exec_log

cd "$repo_root/examples/Nozzle"
echo ""
echo "======================================"
echo "** Nozzle demos **"
echo "======================================"
foreach case ($nozzle_cases)
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
	if ("$matches" == "1") then
		printf "   %s%s [ Success ]%s %s\n" "$green" "$check" "$reset" "$case"
	else
		printf "   %s%s [ Fail ]%s %s\n" "$red" "$cross" "$reset" "$case"
	endif
	rm -f $exec_log run.txt *.csv *.phsp *.header *.root *.bin *.xyz *.sdd
	popd > /dev/null
end
rm -f *.csv *.phsp *.header *.root *.bin *.xyz *.sdd $exec_log

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
	if ("$matches" == "1") then
		printf "   %s%s [ Success ]%s %s\n" "$green" "$check" "$reset" "$case"
	else
		printf "   %s%s [ Fail ]%s %s\n" "$red" "$cross" "$reset" "$case"
	endif
	rm -f $exec_log run.txt *.csv *.phsp *.header *.root *.bin *.xyz *.sdd
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
	if ("$matches" == "1") then
		printf "   %s%s [ Success ]%s %s\n" "$green" "$check" "$reset" "$case"
	else
		printf "   %s%s [ Fail ]%s %s\n" "$red" "$cross" "$reset" "$case"
	endif
	rm -f $exec_log run.txt *.csv *.phsp *.header *.root *.bin *.xyz *.sdd *.dcm
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
	if ("$matches" == "1") then
		printf "   %s%s [ Success ]%s %s\n" "$green" "$check" "$reset" "$case"
	else
		printf "   %s%s [ Fail ]%s %s\n" "$red" "$cross" "$reset" "$case"
	endif
	rm -f $exec_log run.txt *.csv *.phsp *.header *.root *.bin *.xyz *.sdd *.dcm
	popd > /dev/null
end
rm -rf DICOM_Box __MACOSX

rm -f *.csv *.phsp *.header *.root *.bin *.xyz *.sdd *.dcm $exec_log

rm -f "$repo_root"/examples/*/NbParticlesInTime.txt

cd "$start_dir"

set end_time = `date +%s`
@ elapsed = $end_time - $start_time

printf "\n📊  Regression completed in %d seconds (%.2f minutes)\n" $elapsed `echo "$elapsed / 60.0" | bc -l`

if (! $_demo_had_nonomatch) then
	unset nonomatch
endif
