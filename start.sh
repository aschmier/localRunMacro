#local running with this code requires testfiles downloaded and a testSampleESD.txt or testSampleAOD.txt text file with the input files stored in, for example pPb_5TeV/LHC16q/testSampleESD.txt

system="pPb_8TeV"

isMC=true
collsys="pPb" # pp, pPb, or PbPb
runPeriod="LHC18b9b"
dataType="AOD"
numLocalFiles=4
chunk=-1
nEventsProcessing=1234567890
firstEventProcessing=0
localCorrConfig="/home/austin/alice/EMCalConfig/correct_corrections/Config_pPbJets_MC.yaml"
minradius=2
maxradius=3
kSupportAliEventCuts=true
kIsPtHard=true
kIsHepMC=false
kOldPtHardBinHandling=false
kIsRun1=false
localFiles="/home/austin/alice/localRunMacro/testSampleAOD.txt"


mkdir -p $system/$runPeriod
cd $system/$runPeriod
aliroot -x -l -b -q '/home/austin/alice/localRunMacro/runLocalAnalysisROOT6.C('$isMC',"'$collsys'","'$runPeriod'","'$dataType'",'$numLocalFiles','$chunk','$nEventsProcessing','$firstEventProcessing',"'$localCorrConfig'",'$minradius','$maxradius','$kSupportAliEventCuts','$kIsPtHard','$kIsHepMC','$kOldPtHardBinHandling','$kIsRun1',"'$localFiles'")'
