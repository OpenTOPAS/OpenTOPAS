# Brachytherapy package for TOPAS

This package has been developed by the Medical Physics Research Group at Laval University [https://physmed.fsg.ulaval.ca](https://physmed.fsg.ulaval.ca) with the close collaboration of the TOPAS developer team.

***Collaborators***: Francisco Berumen, Audran Poher, Yunzhi Ma, Jose Ramos Mendez, Joseph Perl, and Luc Beaulieu

For any presentation or publication using the track-length estimator or brachytherapy related calculation, please cite the following published works
- Francisco Berumen, Yunzhi Ma, José Ramos-Méndez, Joseph Perl, and Luc Beaulieu. "Validation of the TOPAS Monte Carlo toolkit for HDR brachytherapy simulations", Brachytherapy (2021) https://doi.org/10.1016/j.brachy.2020.12.007
- Audran Poher, Francisco Berumen, Yunzhi Ma, Joseph Perl, and Luc Beaulieu. "Characterization of LDR brachytherapy sources using the TOPAS Monte Carlo toolkit", COMP annual meeting 2021.

The Brachytherapy TOPAS package includes
- 3 HDR sources (Flexisource, TG186, MicroSelectronV2)
- 12 LDR sources (IsoRay CS-1 Rev2, Bard STM1251, Best 2301, IsoAid IAI-125A, OncoSeed 6711, selectSeed 130.002, SL-125, Theragenics AgX100, Best 2335, IsoAid IAPd-103A, TheraSeed 200 Heavy, TheraSeed 200 Light)
- A track-length estimator (TLE) for dose scoring
- 4 examples

Note that for each source a self-contained parameter file is provided. In such a way, the desired source can be imported with the includeFile method. For some sources, the layered mass geometry method was used, and the corresponding physics parameter is also given at the end.

HDRsource.txt and LDRSource.txt files provide a quick example on how to import a single source and visualize it, comment/uncomment the includeFile line accordingly.

For both Theraseed200 models, the user can define the materials of left and right caps.

The HDRSourceInApplicator.txt file presents the TG186 source inside the TG186 applicator. 

The DoseTLE.txt file score the dose of the previous setup using the TLE. Note that the TLE extension must be compiled before using it. The TLE requires the mass absorption coefficients which are given to TOPAS in the InputFile parameter "Muen.dat". 

***References***

Track Length Estimator:
1.	H Afsharpour et al., "ALGEBRA: ALgorithm for the heterogeneous dosimetry based on GEANT4 for BRAchytherapy", Physics in Medicine and Biology, 57(11), 3273–3280, 2012. https://doi.org/10.1088/0031-9155/57/11/3273
2.	Jeffrey F. Williamson, "Monte Carlo evaluation of kerma at a point for photon transport problems", Medical Physics 14, 567 (1987). https://doi.org/10.1118/1.596069

Ir-192 sources:
TG186 source and applicator
1.	F Ballester et al., "A generic high-dose rate 192Ir brachytherapy source for evaluation of model-based dose calculations beyond the TG-43 formalism", Medical Physics, Vol. 42, No. 6, June 2015. https://doi.org/10.1118/1.4921020 
2.	Y Ma et al., "A generic TG-186 shielded applicator for commissioning model-based dose calculation algorithms for high-dose-rate 192Ir brachytherapy", Medical Physics, 44 (11), November 2017. https://doi.org/10.1002/mp.12459 

Micro selectron V2
1.	"Nucletron, microSelectron-v2, HDR", Department of Physics, Carleton Laboratory for Radiotherapy Physics, consulted September 2019, https://physics.carleton.ca/clrp/seed_database/Ir192_HDR/microSelectron_v2 
2.	G. M. Daskalov et al., "Monte Carlo-aided dosimetry of a new high dose-rate brachytherapy source", Med Phys 25, 2200 - 2208, 1998. https://doi.org/10.1118/1.598418 

Flexisource
1.	"Isodose Control, Flexisource, HDR | Department of Physics", consulted July 28th 2020, https://physics.carleton.ca/clrp/egs_brachy/seed_database/Ir192_HDR/Flexisource
2.	D. Granero et al., "A Dosimetric Study on the High Dose Rate Flexisource", Medical Physics 33, no 12 (2006): 4578‑82, https://doi.org/10.1118/1.2388154

I-125 sources:
IsoAid, LLC, Advantage I-125, IAI-125A
1.	Mark J. Rivard et al., "Supplement to the 2004 Update of the AAPM Task Group No. 43 Report", Medical Physics 34, no 6Part1 (2007): 2187‑2205, https://doi.org/10.1118/1.2736790
2.	Timothy D. Solberg et al., "Dosimetric parameters of three new solid core I‐125 brachytherapy sources", Journal of Applied Clinical Medical Physics 3, no 2 (2002): 119‑34, https://doi.org/10.1120/jacmp.v3i2.2576
3.	"IsoAid, LLC, Advantage I-125, IAI-125A | Department of Physics", consulted July 8th 2020, https://physics.carleton.ca/clrp/egs_brachy/seed_database/I125/Advantage_IAI-125A

Mills Bio. Pharm., ProstaSeed, 125SL
1.	“Mills Bio. Pharm., ProstaSeed, 125SL | Department of Physics”, consulted June 26th 2020, https://physics.carleton.ca/clrp/egs_brachy/seed_database/I125/ProstaSeed_125SL
2.	Zuofeng Li, "Monte Carlo Calculations of Dosimetry Parameters of the Urocor Prostaseed Source", Medical Physics 29, no 6 (2002): 1029‑34, https://doi.org/10.1118/1.1478559

Nucletron SelectSeed 130.002
1.	Karaiskos et al , "Monte Carlo dosimetry of the selectSeed 125-I interstitial brachytherapy seed", Med. Phys., 28 , 1753—1760 (2001). https://doi.org/10.1118/1.1384460 
2.	G. Anagnostopoulos et al , "Thermoluminescent dosimetry of the selectSeed 125-I interstitial brachytherapy seed", Med. Phys., 29 , 709-716, (2002). https://doi.org/10.1118/1.1469631 
3.	Mark J. Rivard et al., "Supplement 2 for the 2004 update of the AAPM Task Group No. 43 Report: Joint recommendations by the AAPM and GEC-ESTRO", Medical Physics 44 (9), e297-e338. https://doi.org/10.1002/mp.12430 

Theragenics Co., I-Seed I-125, AgX100
1.	"Theragenics Co., I-Seed I-125, AgX100 | Department of Physics", consulted June 22th 2020, https://physics.carleton.ca/clrp/egs_brachy/seed_database/I125/ISeed_AgX100
2.	Firas Mourtada, Justin Mikell, et Geoffrey Ibbott, "Monte Carlo Calculations of AAPM Task Group Report No. 43 Dosimetry Parameters for the 125I I-Seed AgX100 Source Model", Brachytherapy 11, no 3 (2012): 237‑44, https://doi.org/10.1016/j.brachy.2011.06.002 

Bard Urological Division, 125Implant Seeds, STM1251
1.	"Bard Urological Division, 125Implant Seeds, STM1251 | Department of Physics", consulted July 6th 2020, https://physics.carleton.ca/clrp/egs_brachy/seed_database/I125/STM_1251
2.	Assen S. Kirov et Jeffrey F. Williamson, "Monte Carlo-Aided Dosimetry of the Source Tech Medical Model STM1251 I-125 Interstitial Brachytherapy Source", Medical Physics 28, no 5 (2001): 764‑72, https://doi.org/10.1118/1.1367280
3.	Assen S. Kirov et Jeffrey F. Williamson, "Erratum: “Monte Carlo-Aided Dosimetry of the Source Tech Medical Model STM1251 I-125 Interstitial Brachytherapy Source” [Med. Phys. 28, 764–772 (2001)]", Medical Physics 29, no 2 (2002): 262‑63, https://doi.org/10.1118/1.1446107

Best Industries, Best I-125, 2301
1.	"Best Industries, Best I-125, 2301 | Department of Physics", consulted June 19th 2020, https://physics.carleton.ca/clrp/egs_brachy/seed_database/I125/Best_2301 
2.	Keith T Sowards et Ali S Meigooni, "A Monte Carlo Evaluation of the Dosimetric Characteristics of the Best® Model 2301 125I Brachytherapy Source", Applied Radiation and Isotopes 57, no 3 (2002): 327‑33, https://doi.org/10.1016/S0969-8043(02)00124-0
3.	Mark J. Rivard et al., "Update of AAPM Task Group No. 43 Report: A Revised AAPM Protocol for Brachytherapy Dose Calculations", Medical Physics 31, no 3 (2004): 633‑74, https://doi.org/10.1118/1.1646040

Amersham, OncoSeed, 6711
1.	"Amersham, OncoSeed, 6711 | Department of Physics", consulted July 9th 2020, https://physics.carleton.ca/clrp/egs_brachy/seed_database/I125/OncoSeed_6711
2.	Habib Safigholi et al., "Update of the CLRP TG-43 Parameter Database for Low-Energy Brachytherapy Sources", Medical Physics n/a, no n/a: 43, consulted June 26th 2020, https://doi.org/10.1002/mp.14249
3.	James Dolan, Zuofeng Li, et Jeffrey F. Williamson, "Monte Carlo and Experimental Dosimetry of an 125-I Brachytherapy Seed", Medical Physics 33, no 12 (2006): 4675‑84, https://doi.org/10.1118/1.2388158

Pd-103 sources:
Best Industries, BestPd-103, 2335
1.	"Best Industries, BestPd-103, 2335 | Department of Physics", consulted July 16th 2020, https://physics.carleton.ca/clrp/egs_brachy/seed_database/Pd103/Best_2335
2.	Mark J. Rivard et al., "Supplement to the 2004 Update of the AAPM Task Group No. 43 Report", Medical Physics 34, no 6Part1 (2007): 2187‑2205, https://doi.org/10.1118/1.2736790

IsoAid, Advantage, IAPd-103A
1.	"IsoAid, Advantage, IAPd-103A | Department of Physics", consulted July 16th 2020, https://physics.carleton.ca/clrp/egs_brachy/seed_database/Pd103/Advantage_IAPd_103A
2.	Ali S. Meigooni et al., "Theoretical and Experimental Determination of Dosimetric Characteristics for ADVANTAGETM Pd-103 Brachytherapy Source", Applied Radiation and Isotopes 64, no 8 (2006): 881‑87, https://doi.org/10.1016/j.apradiso.2006.03.015
3.	Keith T. Sowards, "Monte Carlo dosimetric characterization of the IsoAid ADVANTAGE P103d brachytherapy source", Journal of Applied Clinical Medical Physics 8, no 2 (2007): 18‑25, https://doi.org/10.1120/jacmp.v8i2.2393

Theragenics Co., TheraSeed, 200
1.	"Theragenics Co., TheraSeed, 200 | Department of Physics ", consulted July 15th 2020, https://physics.carleton.ca/clrp/egs_brachy/seed_database/Pd103/TheraSeed_200
2.	James I. Monroe et Jeffrey F. Williamson, "Monte Carlo-Aided Dosimetry of the Theragenics TheraSeed® Model 200 Interstitial Brachytherapy Seed", Medical Physics 29, no 4 (2002): 609‑21, https://doi.org/10.1118/1.1460876

Cs-131 source: 
IsoRay Medical Inc., Proxcelan, CS-1 Rev2
1.	"IsoRay Medical Inc., Proxcelan, CS-1 Rev2 | Department of Physics", consulted July 17th 2020, https://physics.carleton.ca/clrp/egs_brachy/seed_database/Cs131_HDR/Proxcelan_CS1
2.	Mark J. Rivard, "Brachytherapy Dosimetry Parameters Calculated for a 131Cs Source", Medical Physics 34, no 2 (2007): 754‑62, https://doi.org/10.1118/1.2432162 
