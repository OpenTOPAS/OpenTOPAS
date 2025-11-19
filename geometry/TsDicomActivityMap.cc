//
// ********************************************************************
// *                                                                  *
// * Copyright 2024 The TOPAS Collaboration                           *
// * Copyright 2022 The TOPAS Collaboration                           *
// *                                                                  *
// * Permission is hereby granted, free of charge, to any person      *
// * obtaining a copy of this software and associated documentation   *
// * files (the "Software"), to deal in the Software without          *
// * restriction, including without limitation the rights to use,     *
// * copy, modify, merge, publish, distribute, sublicense, and/or     *
// * sell copies of the Software, and to permit persons to whom the   *
// * Software is furnished to do so, subject to the following         *
// * conditions:                                                      *
// *                                                                  *
// * The above copyright notice and this permission notice shall be   *
// * included in all copies or substantial portions of the Software.  *
// *                                                                  *
// * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,  *
// * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES  *
// * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND         *
// * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT      *
// * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,     *
// * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
// * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR    *
// * OTHER DEALINGS IN THE SOFTWARE.                                  *
// *                                                                  *
// ********************************************************************
//

#include "TsDicomActivityMap.hh"
#include "TsParameterManager.hh"
#include "TsDicomPatient.hh"

#include "G4Box.hh"
#include "G4UIcommand.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"

#include "gdcmDirectory.h"
#include "gdcmImageReader.h"
#include "gdcmIPPSorter.h"
#include "gdcmScanner.h"
#include "gdcmAttribute.h"
#include "gdcmRescaler.h"


TsDicomActivityMap::TsDicomActivityMap(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM,
									   TsGeometryManager* gM, TsVGeometryComponent* parentComponent,
									   G4VPhysicalVolume* parentVolume, G4String& name)
: TsBox(pM, eM, mM, gM, parentComponent, parentVolume, name)
{
	fDicomDirectory = fPm->GetStringParameter(GetFullParmName("DicomDirectory"));
	fCountThreshold = 0;
	if (fPm->ParameterExists(GetFullParmName("LowerCountThreshold")))
		fCountThreshold = fPm->GetIntegerParameter(GetFullParmName("LowerCountThreshold"));
	fNumberOfSlices = 0;
	fVoxelCountX = 0;
	fVoxelCountY = 0;
	fVoxelSizeX = 0;
	fVoxelSizeY = 0;
	fVoxelSizeZ = 0;
}

TsDicomActivityMap::~TsDicomActivityMap() {}

G4VPhysicalVolume* TsDicomActivityMap::Construct()
{
	BeginConstruction();
	ReadImage();

	G4double halfLengthX = std::min(fParentComponent->GetFullWidth(0)/2., fFullWidths[0]/2.);
	G4double halfLengthY = std::min(fParentComponent->GetFullWidth(1)/2., fFullWidths[1]/2.);
	G4double halfLengthZ = std::min(fParentComponent->GetFullWidth(2)/2., fFullWidths[2]/2.);
	G4Box* envelope = new G4Box(fName, halfLengthX, halfLengthY, halfLengthZ);
	fEnvelopeLog = CreateLogicalVolume(envelope);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	InstantiateChildren();
	return fEnvelopePhys;
}

void TsDicomActivityMap::UpdateForSpecificParameterChange(G4String parameter)
{
	TsBox::UpdateForSpecificParameterChange(parameter);
}

void TsDicomActivityMap::ReadImage()
{
	G4cout << "TOPAS will read DICOM NM image for activity map from: " << fDicomDirectory << G4endl;

	// Define changeable parameters so DICOM frame of reference can be used in parameter files
	G4String transName, transValue;
	transName  = "dc:Ge/" + fName + "/DicomOriginX";
	transValue = G4UIcommand::ConvertToString((fTransFirstVoxelCenterRelToDicom.x() - fTransFirstVoxelCenterRelToComponentCenter.x()) / mm) + " mm";
	fPm->AddParameter(transName, transValue);
	transName  = "dc:Ge/" + fName + "/DicomOriginY";
	transValue = G4UIcommand::ConvertToString((fTransFirstVoxelCenterRelToDicom.y() - fTransFirstVoxelCenterRelToComponentCenter.y()) / mm) + " mm";
	fPm->AddParameter(transName, transValue);
	transName  = "dc:Ge/" + fName + "/DicomOriginZ";
	transValue = G4UIcommand::ConvertToString((fTransFirstVoxelCenterRelToDicom.z() - fTransFirstVoxelCenterRelToComponentCenter.z()) / mm) + " mm";
	fPm->AddParameter(transName, transValue);


	// Set up scanner
	gdcm::Scanner scanner;

	// Modality and acquisition tags in dicom
	gdcm::Tag const modalityTag = gdcm::Tag(0x08, 0x60);
	scanner.AddTag(modalityTag);

	gdcm::Tag const acquisitionTag = gdcm::Tag(0x20, 0x12);
	scanner.AddTag(acquisitionTag);

	// Reading directory
	gdcm::Directory dir;
	if (!dir.Load(fDicomDirectory)) {
		G4cout << "Unable to read the specified DICOM directory." << G4endl;
		G4cout << "It may be that you just forgot to unzip the relevant DICOM dataset." << G4endl;
		fPm->AbortSession(1);
	}
	std::vector<std::string> allFilenames = dir.GetFilenames();
	if (allFilenames.size() == 0) {
		G4cout << "Found no files in the specified DICOM directory." << G4endl;
		G4cout << "It may be that you just forgot to unzip the relevant DICOM dataset." << G4endl;
		fPm->AbortSession(1);
	}

	scanner.Scan(allFilenames);

	G4String* modalityTags;
	G4int nModalityTags;
	if (fPm->ParameterExists(GetFullParmName("DicomModalityTags"))) {
		modalityTags = fPm->GetStringVector(GetFullParmName("DicomModalityTags"));
		nModalityTags = fPm->GetVectorLength(GetFullParmName("DicomModalityTags"));
	} else {
		modalityTags = new G4String[1];
		modalityTags[0] = "NM";
		modalityTags[1] = "PT";
		nModalityTags = 2;
	}
	std::vector<std::string> imageFilenames;
	std::vector<std::string> imageFilenamesOneModality;

	for (G4int iModalityTag = 0; iModalityTag < nModalityTags; iModalityTag++)
	{
		imageFilenamesOneModality = scanner.GetAllFilenamesFromTagToValue(modalityTag, modalityTags[iModalityTag]);
		imageFilenames.insert(imageFilenames.end(), imageFilenamesOneModality.begin(),imageFilenamesOneModality.end());
	}
	if (imageFilenames.size() == 0) {
		G4cout << "The specified DICOM directory did not contain any files of the desired modalities" << G4endl;
        G4cout << "that were indicated in " << GetFullParmName("DicomModalityTags") << G4endl;
		G4cout << "These were: " << G4endl;
		for (G4int iModalityTag = 0; iModalityTag < nModalityTags; iModalityTag++)
			G4cout << "  " << modalityTags[iModalityTag] << G4endl;
		fPm->AbortSession(1);
	}
	G4int nSlices = imageFilenames.size();
	std::vector<std::string> sortedImageFilenames;
	if (nSlices > 1)
	{
		// Use sorter to sort list of files in order of Z slice position
		gdcm::IPPSorter sorter;
		sorter.SetComputeZSpacing(false);
		sorter.Sort(imageFilenames);
		sortedImageFilenames = sorter.GetFilenames();
	}
	else sortedImageFilenames = imageFilenames;
	// Get DICOM positions from parent (TsDicomPatient)
	TsDicomPatient* dicomCT = (TsDicomPatient*)fParentComponent;
	G4Point3D originCTRelToDicom = dicomCT->GetTransFirstVoxelCenterRelToDicom();
	G4Point3D originCTRelToComponent = dicomCT->GetTransFirstVoxelCenterRelToComponentCenter();
	G4Point3D translation = originCTRelToComponent - originCTRelToDicom;

	for (G4int iSlice = 0; iSlice < nSlices; iSlice++)
	{
		// Preparing reader, image and dataset
		gdcm::ImageReader* reader;
		gdcm::Image* image;
		gdcm::DataSet* dataSet;

		G4bool dumpImagingValues = (fPm->ParameterExists(GetFullParmName("DumpImagingValues")) && fPm->GetBooleanParameter(GetFullParmName("DumpImagingValues")));
		reader = new gdcm::ImageReader();
		reader->SetFileName(sortedImageFilenames[iSlice].c_str());
		if (!reader->Read())
		{
			G4cout << "Failed on attempt to read data from DICOM file: " << sortedImageFilenames[0] << G4endl;
			fPm->AbortSession(1);
		}

		dataSet = &reader->GetFile().GetDataSet();

		// If modality is NM, read image position from detector information sequence
		if (modalityTags[0] == "NM")
		{
			gdcm::Tag const detInfoSeqTag = gdcm::Tag(0x0054, 0x0022);
			gdcm::SequenceOfItems* detInfoSeq = dataSet->GetDataElement(detInfoSeqTag).GetValueAsSQ();
			G4int itemN = 1;
			gdcm::Attribute<0x0020, 0x0032> atPos;
			G4int numItems = detInfoSeq->GetNumberOfItems();
			while (itemN <= numItems)
			{
				gdcm::Item item = detInfoSeq->GetItem(itemN);
				if (item.FindDataElement(atPos.GetTag()))
				{
					atPos.SetFromDataElement(item.GetDataElement(atPos.GetTag()));
					fTransFirstVoxelCenterRelToDicom = G4Point3D(atPos.GetValues()[0], atPos.GetValues()[1], atPos.GetValues()[2]);
				}
				itemN++;
			}

			gdcm::Attribute<0x54, 0x81> atNSlices;
			atNSlices.SetFromDataElement(dataSet->GetDataElement(atNSlices.GetTag()));
			fNumberOfSlices = atNSlices.GetValue();
		}
		else if (modalityTags[0] == "PT")
		{
			gdcm::Attribute<0x0020, 0x0032> atPos;
			atPos.SetFromDataElement(dataSet->GetDataElement(atPos.GetTag()));
			fTransFirstVoxelCenterRelToDicom = G4Point3D(atPos.GetValues()[0], atPos.GetValues()[1], atPos.GetValues()[2]);

			fNumberOfSlices = nSlices;
		}
		else
		{
			G4cout << "The specified DICOM modality is not supported." << G4endl;
			fPm->AbortSession(1);
		}
		// Read slice thickness and number of slices
		gdcm::Attribute<0x18, 0x50> atThickness;
		atThickness.SetFromDataElement(dataSet->GetDataElement(atThickness.GetTag()));
		fVoxelSizeZ = atThickness.GetValue();

		fFullWidths[2] = fNumberOfSlices * fVoxelSizeZ;

		// Read image
		image = &reader->GetImage();

		// Get number of pixels and spacing in each direction
		fVoxelCountX = image->GetDimensions()[0];
		fVoxelCountY = image->GetDimensions()[1];
		fVoxelSizeX = image->GetSpacing()[0];
		fVoxelSizeY = image->GetSpacing()[1];

		fFullWidths[0] = fVoxelCountX * fVoxelSizeX;
		fFullWidths[1] = fVoxelCountY * fVoxelSizeY;

		// Read the pixels
		//gdcm::PixelFormat pixelFormat = image->GetPixelFormat();

		G4double slope = image->GetSlope();
		G4double intercept = image->GetIntercept();
		G4int bufferLength = image->GetBufferLength();
		char* bufferIn = new char[bufferLength];
		image->GetBuffer(bufferIn);

		// Assuming the input pixel format is INT16, modify this if your input format is different
		short* inputValues = reinterpret_cast<short*>(bufferIn);
		G4int numValues = bufferLength / sizeof(short);

		// Create a buffer for the rescaled float values
		float* ImagingValues = new float[numValues];
		// Manually apply the rescaling
		for (G4int i = 0; i < numValues; ++i) {
			ImagingValues[i] = static_cast<float>(inputValues[i]) * slope + intercept;
		}

		G4int iVoxel = 0;
		if (nSlices == 1)
		{
			for (G4int iz = 1; iz <= fNumberOfSlices; iz++)
			{
				for (G4int iy = 1; iy <= fVoxelCountY; iy++)
				{
					for (G4int ix = 1; ix <= fVoxelCountX; ix++)
					{
						fRawImageShort.push_back((signed short)ImagingValues[iVoxel]);
						G4double xposDcm = fTransFirstVoxelCenterRelToDicom[0] + (ix - 1) * fVoxelSizeX;
						G4double yposDcm = fTransFirstVoxelCenterRelToDicom[1] + (iy - 1) * fVoxelSizeY;
						G4double zposDcm = fTransFirstVoxelCenterRelToDicom[2] + (iz - 1) * fVoxelSizeZ;
						G4double xpos = xposDcm + translation[0];
						G4double ypos = yposDcm + translation[1];
						G4double zpos = zposDcm + translation[2];
						if ((float)ImagingValues[iVoxel] >= fCountThreshold)
						{
							if (dumpImagingValues)
								G4cout << "Voxel: (" << ix - 1 << ", " << iy - 1 << ", " << iz - 1 << "), DICOM position: (" << xposDcm << ", " << yposDcm << ", " << zposDcm << "); patient position: " << xpos << ", " << ypos << ", " << zpos << ") has value: " << (signed short)ImagingValues[iVoxel] << G4endl;
							fSourcePositions.push_back(G4Point3D(xpos, ypos, zpos));
							fSourceCounts.push_back((float)ImagingValues[iVoxel]);
							fSourcePosRelToDicom.push_back(G4Point3D(xposDcm, yposDcm, zposDcm));
						}
						iVoxel++;
					}
				}
			}
		}
		else
		{
			G4double zposDcm = fTransFirstVoxelCenterRelToDicom[2];
			G4double zpos = zposDcm + translation[2];
			for (G4int iy = 1; iy <= fVoxelCountY; iy++)
			{
				for (G4int ix = 1; ix <= fVoxelCountX; ix++)
				{
					fRawImageShort.push_back((signed short)ImagingValues[iVoxel]);
					G4double xposDcm = fTransFirstVoxelCenterRelToDicom[0] + (ix - 1) * fVoxelSizeX;
					G4double yposDcm = fTransFirstVoxelCenterRelToDicom[1] + (iy - 1) * fVoxelSizeY;
					G4double xpos = xposDcm + translation[0];
					G4double ypos = yposDcm + translation[1];
					if ((float)ImagingValues[iVoxel] >= fCountThreshold)
					{
						fSourcePositions.push_back(G4Point3D(xpos, ypos, zpos));
						fSourceCounts.push_back((float)ImagingValues[iVoxel]);
						fSourcePosRelToDicom.push_back(G4Point3D(xposDcm, yposDcm, zposDcm));
					}
					iVoxel++;
				}
			}
		}

		delete[] bufferIn;
		delete reader;
	}
}
