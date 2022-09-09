#dataOrMC=data
#ESDOrAOD=ESD
dataOrMC=$1
ESDOrAOD=$2
energy=PbPb_5TeV

yearData=2018
periodData=LHC18r
pass=pass3 #pass1_CENT_woSDD #muon_calo_pass2 #pass1_FAST #pass1_CENT_woSDD
AODFILTERDATA=""

yearMC=2017
periodMC=LHC17g5c #LHC18f3_cent_woSDD_2
periodMCshort=LHC17g5c
AODFILTERMC="202"
PTHARDBIN=$3
RUN=297446
downloadFile=root_archive.zip


RUNDL=$RUN
passDL=$pass

if [ $dataOrMC = "MC" ]; then
    if [ $ESDOrAOD = "AODF" ]; then
        RUNDL="$RUN/AOD$AODFILTERMC"
        downloadFile=root_archive.zip
    fi
    mkdir -p $energy/$periodMCshort/$ESDOrAOD/$RUN
	for PARTS in $( alien_ls /alice/sim/$yearMC/$periodMC/$RUNDL/ )
	do
        if [ -f $energy/$periodMCshort/$ESDOrAOD/$RUN/$PARTS/$downloadFile ]; then
            echo "file $downloadFile has already been copied for run " $RUN "and part" $PARTS
        else
            mkdir $energy/$periodMCshort/$ESDOrAOD/$RUN/$PARTS
		    echo /alice/sim/$yearMC/$periodMC/$RUNDL/$PARTS/$downloadFile
		    alien_cp /alice/sim/$yearMC/$periodMC/$RUNDL/$PARTS/$downloadFile file:$energy/$periodMCshort/$ESDOrAOD/$RUN/$PARTS/$downloadFile
            unzip -t $energy/$periodMCshort/$ESDOrAOD/$RUN/$PARTS/$downloadFile
            md5sum $energy/$periodMCshort/$ESDOrAOD/$RUN/$PARTS/$downloadFile
        fi
	done
fi

if [ $dataOrMC = "JJMC" ]; then
    if [ $ESDOrAOD = "AODF" ]; then
        RUNDL="$RUN/AOD$AODFILTERMC"
        downloadFile=root_archive.zip
    fi
    if [ $ESDOrAOD = "AOD" ]; then
        RUNDL="$RUN/AOD"
        downloadFile=aod_archive.zip
    fi
    mkdir -p $energy/$periodMCshort/$ESDOrAOD/$RUN
	#for PTHARDBIN in $( alien_ls /alice/sim/$yearMC/$periodMC/ )
    #do
	    for PARTS in $( alien_ls /alice/sim/$yearMC/$periodMC/$PTHARDBIN/$RUNDL/ )
	    do
            if [ -f $energy/$periodMCshort/$ESDOrAOD/$RUN/$PTHARDBIN/$PARTS/$downloadFile ]; then
                echo "file $downloadFile has already been copied for run " $RUN "and part" $PARTS
            else
                mkdir -p $energy/$periodMCshort/$ESDOrAOD/$RUN/$PTHARDBIN/$PARTS
		        echo /alice/sim/$yearMC/$periodMC/$PTHARDBIN/$RUNDL/$PARTS/$downloadFile
		        alien_cp /alice/sim/$yearMC/$periodMC/$PTHARDBIN/$RUNDL/$PARTS/$downloadFile file:$energy/$periodMCshort/$ESDOrAOD/$RUN/$PTHARDBIN/$PARTS/$downloadFile
                unzip -t $energy/$periodMCshort/$ESDOrAOD/$RUN/$PTHARDBIN/$PARTS/$downloadFile
                md5sum $energy/$periodMCshort/$ESDOrAOD/$RUN/$PTHARDBIN/$PARTS/$downloadFile
            fi
	    done
    #done
fi


if [ $dataOrMC = "data" ]; then
    if [ $ESDOrAOD = "AOD" ]; then
        passDL="$pass/AOD"
        downloadFile=aod_archive.zip
    fi
    if [ $ESDOrAOD = "AODF" ]; then
        passDL="$pass/AOD$AODFILTERDATA"
        downloadFile=root_archive.zip
    fi
    mkdir -p $energy/$periodData/$pass/$ESDOrAOD/$RUN/
    for PARTS in $( alien_ls /alice/data/$yearData/$periodData/000$RUN/$passDL )
    do
        if [ -f $energy/$periodData/$pass/$ESDOrAOD/$RUN/$PARTS/$downloadFile ]; then
            echo "file $downloadFile has already been copied for run " $RUN "and part" $PARTS
        else
            mkdir $energy/$periodData/$pass/$ESDOrAOD/$RUN/$PARTS
            echo /alice/data/$yearData/$periodData/000$RUN/$passDL/$PARTS/$downloadFile
            alien_cp /alice/data/$yearData/$periodData/000$RUN/$passDL/$PARTS/$downloadFile file:$energy/$periodData/$pass/$ESDOrAOD/$RUN/$PARTS/$downloadFile
            unzip -t $energy/$periodData/$pass/$ESDOrAOD/$RUN/$PARTS/$downloadFile
            md5sum $energy/$periodData/$pass/$ESDOrAOD/$RUN/$PARTS/$downloadFile
        fi
    done
fi
