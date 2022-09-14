#include "AliMCEventHandler.h"
#include "AliESDInputHandler.h"
#include "AliAODInputHandler.h"
#include "AliAnalysisAlien.h"
#include "AliAnalysisManager.h"

R__ADD_INCLUDE_PATH($ALICE_PHYSICS)
#include "localRunningChain.h"


//______________________________________________________________________________
void runLocalAnalysisROOT6(
    bool            isMC                        = true,
    TString         collsys                     = "", // pp, pPb, or PbPb
    TString         runPeriod                   = "",
    TString         dataType                    = "",
    UInt_t          numLocalFiles               = 50,
    int             chunk                       = -1,
    Long64_t        nEventsProcessing           = 1234567890,
    Long64_t        firstEventProcessing        = 0,
    TString         localCorrConfig             = "",
    int             minradius                   = 2,
    int             maxradius                   = 6,
    bool            kSupportAliEventCuts        = true,
    bool            kIsPtHard                   = true,
    bool            kIsHepMC                    = false,
    bool            kOldPtHardBinHandling       = false,
    bool            kIsRun1                     = true,
    TString         localFiles                  = ""
)
{
    cout << "Starting test" << endl;
    cout << dataType.Data() << " analysis chosen" << endl;

    // Create analysis manager
    AliAnalysisManager* mgr                     = new AliAnalysisManager("LocalAnalysisTaskRunning");

    // Check type of input and create handler for it
    if(dataType.Contains("AOD")){
        AliAODInputHandler* aodH                = new AliAODInputHandler();
        mgr->SetInputEventHandler(aodH);
    } else if(dataType.Contains("ESD")){
        AliESDInputHandler* esdH                = new AliESDInputHandler();
        mgr->SetInputEventHandler(esdH);
    } else {
        cout << "Data type not recognized! You have to specify ESD, AOD, or sESD!\n";
    }

    cout << "Using " << localFiles.Data() << " as input file list." << endl;

    // Create MC handler, if MC is demanded
    if (isMC && (!dataType.Contains("AOD")))
    {
        AliMCEventHandler* mcH                  = new AliMCEventHandler();
        mcH->SetPreReadMode(AliMCEventHandler::kLmPreRead);
        mcH->SetReadTR(kTRUE);
        mgr->SetMCtruthEventHandler(mcH);
    }

    // Define pt-hard bins in case of pt-hard analysis
    int knPthardBins;
    TArrayI kPtHardBinning;
    if(kIsPtHard){
        knPthardBins=21;
        kPtHardBinning.Set(22);
        int binning[]={0,5, 7, 9, 12, 16, 21, 28, 36, 45, 57, 70, 85, 99, 115, 132, 150, 169, 190, 212, 235,1000};
        for(Int_t bin=0;bin<22;bin++){ kPtHardBinning[bin]=binning[bin];}
    }

    // Define trigger names based on run period
    std::vector<TString> triggers;
    if(kIsRun1) triggers.insert(triggers.end(), {"EJE","EMC7","INT7"});
    else triggers.insert(triggers.end(), {"EJ1","EJ2","INT7"});

    AliTrackContainer::SetDefTrackCutsPeriod(runPeriod);

    // Configure analysis wagons
    // -----------------------------------------
    //                CDB CONNECT
    // -----------------------------------------
    AliTaskCDBconnect *taskCDB=reinterpret_cast<AliTaskCDBconnect*>(
        gInterpreter->ExecuteMacro("$ALICE_PHYSICS/PWGPP/PilotTrain/AddTaskCDBconnect.C(\"cvmfs:\")"));
    taskCDB->SetFallBackToRaw(kTRUE);

    // -----------------------------------------
    //   EMCAL CORRECTION FRAMEWORK
    // -----------------------------------------
    AliEmcalCorrectionTask *correctionTask=reinterpret_cast<AliEmcalCorrectionTask*>(
        gInterpreter->ExecuteMacro("$ALICE_PHYSICS/PWG/EMCAL/macros/AddTaskEmcalCorrectionTask.C()"));
    correctionTask->SelectCollisionCandidates(AliVEvent::kINT7);
    correctionTask->SetForceBeamType(AliEmcalCorrectionTask::kpp);
    if(isMC){
        AliEmcalCorrectionTask *correctionTaskConfig=reinterpret_cast<AliEmcalCorrectionTask*>(
            gInterpreter->ExecuteMacro(Form("$ALICE_PHYSICS/PWG/EMCAL/macros/ConfigureEmcalCorrectionTaskOnLEGOTrain.C(\"jets_%s_MC\")",collsys.Data())));
    }else{
        AliEmcalCorrectionTask *correctionTaskConfig=reinterpret_cast<AliEmcalCorrectionTask*>(
            gInterpreter->ExecuteMacro(Form("$ALICE_PHYSICS/PWG/EMCAL/macros/ConfigureEmcalCorrectionTaskOnLEGOTrain.C(\"jets_%s\")",collsys.Data())));
    }
    correctionTask->SetUserConfigurationFilename(localCorrConfig.Data());
    correctionTask->Initialize(true);

    // -----------------------------------------
    //            MC Particle Selection
    // -----------------------------------------
    if(isMC){
        AliEmcalRejectMCBackground *MCBGRejectionTask=reinterpret_cast<AliEmcalRejectMCBackground*>(
            gInterpreter->ExecuteMacro("$ALICE_PHYSICS/PWG/EMCAL/macros/AddTaskRejectMCBackground.C(\"MCParticlesNotRejected\",\"MCTracksNotRejected\",\"MCClustersNotRejected\",2,0)"));

        AliEmcalMCTrackSelector *MCParticleSelectorTask=reinterpret_cast<AliEmcalMCTrackSelector*>(
            gInterpreter->ExecuteMacro("$ALICE_PHYSICS/PWG/EMCAL/macros/AddTaskMCTrackSelector.C(\"mcparticlesSelected\",\"MCParticlesNotRejected\", kFALSE, kFALSE, -1, kFALSE)"));
        MCParticleSelectorTask->SetRejectPhotonMother(true);
    }

    // -----------------------------------------
    //            PHYSICS SELECTION
    // -----------------------------------------
    AliPhysicsSelectionTask *physSelTask=reinterpret_cast<AliPhysicsSelectionTask*>(
        gInterpreter->ExecuteMacro(Form("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C(kTRUE)")));

    // -----------------------------------------
    //               Trigger Maker
    // -----------------------------------------
    if(isMC){
        AliEmcalTriggerMakerTask *triggerMakerTask=reinterpret_cast<AliEmcalTriggerMakerTask*>(
            gInterpreter->ExecuteMacro("$ALICE_PHYSICS/PWG/EMCAL/macros/AddTaskEmcalTriggerMakerNew.C(\"EmcalTriggers\", \"\", \"\", kTRUE)"));
        triggerMakerTask->SetForceBeamType(AliAnalysisTaskEmcal::kpp);


        PWG::EMCAL::AliAnalysisTaskEmcalTriggerSelection *triggerSelTask=reinterpret_cast<PWG::EMCAL::AliAnalysisTaskEmcalTriggerSelection*>(
            gInterpreter->ExecuteMacro("$ALICE_PHYSICS/PWG/EMCAL/macros/AddEmcalTriggerSelectionTask.C()"));
        triggerSelTask->SetGlobalDecisionContainerName("EmcalTriggerDecision");
        triggerSelTask->AutoConfigure(runPeriod);
        triggerSelTask->SetForceBeamType(AliAnalysisTaskEmcal::kpp);

    }

    for(int r = minradius; r <= maxradius; r++){
        // -----------------------------------------
        //               Particle Level Jet Finder
        // -----------------------------------------
        AliEmcalJetTask *jetTaskParticle=reinterpret_cast<AliEmcalJetTask*>(
            gInterpreter->ExecuteMacro(Form("$ALICE_PHYSICS/PWGJE/EMCALJetTasks/macros/AddTaskEmcalJet.C( \"mcparticlesSelected\", \"\", AliJetContainer::antikt_algorithm, 0.%i, AliJetContainer::kFullJet, 0., 0., 0.01, AliJetContainer::E_scheme, \"Jet\", 0.1, kFALSE, kFALSE )",r)));
        jetTaskParticle->SelectCollisionCandidates(AliVEvent::kAny);
        jetTaskParticle->SetForceBeamType(AliAnalysisTaskEmcal::kpp);
        if(!kSupportAliEventCuts) jetTaskParticle->SetUseBuiltinEventSelection(true);

        // -----------------------------------------
        //               Detector Level Jet Finder
        // -----------------------------------------
        AliEmcalJetTask *jetTaskDetector=reinterpret_cast<AliEmcalJetTask*>(
            gInterpreter->ExecuteMacro(Form("$ALICE_PHYSICS/PWGJE/EMCALJetTasks/macros/AddTaskEmcalJet.C( \"MCTracksNotRejected\", \"MCClustersNotRejected\", AliJetContainer::antikt_algorithm, 0.%i, AliJetContainer::kFullJet, 0.15, 0.30, 0.01, AliJetContainer::E_scheme, \"Jet\", 0.1, kFALSE, kFALSE )",r)));
        jetTaskDetector->SelectCollisionCandidates(AliVEvent::kAny);
        jetTaskDetector->GetClusterContainer(0)->SetDefaultClusterEnergy(AliVCluster::kHadCorr);
        jetTaskDetector->GetClusterContainer(0)->SetClusHadCorrEnergyCut(0.3);
        jetTaskDetector->SetForceBeamType(AliAnalysisTaskEmcal::kpp);
        if(!kSupportAliEventCuts) jetTaskDetector->SetUseBuiltinEventSelection(true);

        // -----------------------------------------
        //               Jet Tagger
        // -----------------------------------------
        AliAnalysisTaskEmcalJetTagger *jetTaggerTask=reinterpret_cast<AliAnalysisTaskEmcalJetTagger*>(
            gInterpreter->ExecuteMacro(Form("$ALICE_PHYSICS/PWGJE/EMCALJetTasks/macros/AddTaskEmcalJetTagger.C(\"Jet_AKTFullR0%i0_MCTracksNotRejected_pT0150_MCClustersNotRejected_E0300_E_scheme\",\"Jet_AKTFullR0%i0_mcparticlesSelected_pT0000_E_scheme\",0.%i,\"\",\"\",\"MCTracksNotRejected\",\"MCClustersNotRejected\",\"EMCALFID\",\"\",AliVEvent::kAny)",r,r,r)));
        jetTaggerTask->SetNCentBins(1);
        if(!kSupportAliEventCuts) jetTaggerTask->SetUseBuiltinEventSelection(true);
        jetTaggerTask->SetIsPythia(kIsPtHard);
        if(kIsPtHard) jetTaggerTask->SetUsePtHardBinScaling(kTRUE);
        jetTaggerTask->SetJetTaggingType(AliAnalysisTaskEmcalJetTagger::kClosest);
        jetTaggerTask->SetJetTaggingMethod(AliAnalysisTaskEmcalJetTagger::kGeo);
        jetTaggerTask->SetTypeAcceptance(3);
        if(kIsPtHard){
            jetTaggerTask->SetNumberOfPtHardBins(knPthardBins);
            if(knPthardBins!=11) jetTaggerTask->SetUserPtHardBinning(kPtHardBinning);
            if(kOldPtHardBinHandling) jetTaggerTask->SetGetPtHardBinFromPath(kFALSE);
        }
        jetTaggerTask->SetMaxDistance(r*0.1);
        jetTaggerTask->SetExtraMargins(0., r*0.1);
        jetTaggerTask->SetReadPythiaCrossSectionFast(true);

        // -----------------------------------------
        //               Jet Energy Scale
        // -----------------------------------------
        PWGJE::EMCALJetTasks::AliAnalysisTaskEmcalJetEnergyScale *energyScaleTask=reinterpret_cast<PWGJE::EMCALJetTasks::AliAnalysisTaskEmcalJetEnergyScale*>(
            gInterpreter->ExecuteMacro(Form("$ALICE_PHYSICS/PWGJE/EMCALJetTasks/macros/AddTaskEmcalJetEnergyScale.C(AliJetContainer::kFullJet, AliJetContainer::E_scheme, AliVCluster::kHadCorr, 0.%i, false, \"mcparticlesSelected\", \"INT7\", \"nodownscalecorr\" )",r)));
        typedef PWGJE::EMCALJetTasks::AliAnalysisTaskEmcalJetEnergyScale::MCProductionType_t ProdType_t;
        ProdType_t prodtype = energyScaleTask->ConfigureMCDataset(runPeriod);
        if(prodtype == ProdType_t::kMCPythiaPtHard) {
            energyScaleTask->SetMCFilter();
            energyScaleTask->SetJetPtFactor(2.5);
        }
        if(prodtype == ProdType_t::kMCPythiaMB){
            // limit pt-hard range to 0-28 GeV in order to match with jet-jet production
            energyScaleTask->SetMaxPtHard(28.);
            // apply outlier cut as well on min. bias
            energyScaleTask->SetMCFilter();
            energyScaleTask->SetJetPtFactor(2.5);
        }
        if(kIsHepMC) energyScaleTask->SetCheckPtHardBin(false);
        if(!kSupportAliEventCuts) energyScaleTask->SetUseBuiltinEventSelection(true);
        energyScaleTask->ConfigureJetSelection(0., 0., 1000., 200., 200., 0.6);
        // Require same acceptance for part and det jet
        energyScaleTask->SetRequireSameAcceptance(true);
        energyScaleTask->SetForceBeamType(AliAnalysisTaskEmcal::kpp);
        energyScaleTask->SetDebugLevel(4);

        // -----------------------------------------
        //               Jet Energy Spectrum
        // -----------------------------------------
        for(int trig = 0; trig < triggers.size(); trig++){
            PWGJE::EMCALJetTasks::AliAnalysisTaskEmcalJetEnergySpectrum *spectrumTask=reinterpret_cast<PWGJE::EMCALJetTasks::AliAnalysisTaskEmcalJetEnergySpectrum*>(
                gInterpreter->ExecuteMacro(Form("$ALICE_PHYSICS/PWGJE/EMCALJetTasks/macros/AddTaskEmcalJetEnergySpectrum.C(kTRUE, AliJetContainer::kFullJet, AliJetContainer::E_scheme, AliVCluster::kHadCorr, 0.%i, \"mcparticlesSelected\", \"%s\", \"nodownscalecorr\")",r,triggers.at(trig).Data())));
            typedef PWGJE::EMCALJetTasks::AliAnalysisTaskEmcalJetEnergySpectrum::MCProductionType_t ProdType_t;
            ProdType_t prodtype = spectrumTask->ConfigureMCDataset(runPeriod);
            if(prodtype == ProdType_t::kMCPythiaPtHard || prodtype == ProdType_t::kMCHepMCPtHard) {
                spectrumTask->SetMCFilter();
                spectrumTask->SetJetPtFactor(2.5);
            }
            if(kOldPtHardBinHandling) spectrumTask->SetGetPtHardBinFromPath(kFALSE);
            spectrumTask->ConfigureDetJetSelection(0., 200., 200., 0.6);

            spectrumTask->SetReadPythiaCrossSectionFast(true);
            spectrumTask->SetFillHSparse(kTRUE);
            spectrumTask->SetRangeRun1(kIsRun1);
            // New cluster histogram
            spectrumTask->SetMakeClusterHistos1D(true);
            spectrumTask->SetEnergyDefinition(PWGJE::EMCALJetTasks::AliAnalysisTaskEmcalJetEnergySpectrum::kNonLinCorrEnergy);
            spectrumTask->SetForceBeamType(AliAnalysisTaskEmcal::kpp);
            //spectrumTask->
        }
    }

    AliLog::SetClassDebugLevel("PWGJE::EMCALJetTasks::AliAnalysisTaskEmcalJetEnergySpectrum", 4);


    if (!mgr->InitAnalysis()) return;
    mgr->PrintStatus();

    // LOCAL CALCULATION
    TChain* chain = 0;
    if (dataType.Contains("AOD")) {
        chain = CreateAODChain(localFiles.Data(), numLocalFiles,0,kFALSE,"AliAODGammaConversion.root");
        //chain = CreateAODChain(localFiles.Data(), numLocalFiles,0,kFALSE);
    } else {  // ESD
        chain = CreateESDChain(localFiles.Data(), numLocalFiles);
    }

    cout << endl << endl;
    cout << "****************************************" << endl;
    cout << "*                                      *" << endl;
    cout << "*            start analysis            *" << endl;
    cout << "*                                      *" << endl;
    cout << "****************************************" << endl;
    cout << endl << endl;

    // start analysis
    cout << "Starting LOCAL Analysis...";
    mgr->SetDebugLevel(0);
    mgr->StartAnalysis("local", chain, nEventsProcessing, firstEventProcessing);
    cout << endl << endl;
    cout << "****************************************" << endl;
    cout << "*                                      *" << endl;
    cout << "*             end analysis             *" << endl;
    cout << "*                                      *" << endl;
    cout << "****************************************" << endl;
    cout << endl << endl;
}
