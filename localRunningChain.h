
TChain* CreateAODChain(
  const char* aDataDir = "AODfiles.txt", 
  Int_t aRuns          = 20, 
  Int_t offset         = 0, 
  Bool_t addFileName   = kFALSE,
  const char* friends  = "",
  const char* check    = 0)
{
  // creates chain of files in a given directory or file containing a list.
  // In case of directory the structure is expected as:
  // <aDataDir>/<dir0>/AliAOD.root
  // <aDataDir>/<dir1>/AliAOD.root
  // ...
  //
  // if addFileName is true the list only needs to contain the directories that contain the AliAODs.root files
  // if check is != 0 the files that work are written back into the textfile with the name check

  if (!aDataDir)
    return 0;

  Long_t id, size, flags, modtime;
  if (gSystem->GetPathInfo(aDataDir, &id, &size, &flags, &modtime))
  {
    printf("%s not found.\n", aDataDir);
    return 0;
  }

  TChain* chain = new TChain("aodTree");
  TChain* chainFriend = 0;

  if (strcmp(friends, "") != 0) chainFriend = new TChain("aodTree");

  if (flags & 2)
  {
    TString execDir(gSystem->pwd());
    TSystemDirectory* baseDir = new TSystemDirectory(".", aDataDir);
    TList* dirList            = baseDir->GetListOfFiles();
    Int_t nDirs               = dirList->GetEntries();
    gSystem->cd(execDir);

    Int_t count = 0;

    for (Int_t iDir=0; iDir<nDirs; ++iDir)
    {
      TSystemFile* presentDir = (TSystemFile*) dirList->At(iDir);
      if (!presentDir || !presentDir->IsDirectory() || strcmp(presentDir->GetName(), ".") == 0 || strcmp(presentDir->GetName(), "..") == 0)
        continue;

      if (offset > 0)
      {
        --offset;
        continue;
      }

      if (count++ == aRuns)
        break;

      TString presentDirName(aDataDir);
      presentDirName += "/";
      presentDirName += presentDir->GetName();

      chain->Add(presentDirName + "/AliAOD.root/aodTree");
      if (chainFriend) chainFriend->Add(presentDirName + "/" + friends + "/aodTree");
    }
  }
  else
  {
    // Open the input stream
    ifstream in;
    in.open(aDataDir);

    ofstream outfile;
    if (check)
      outfile.open(check);

    Int_t count = 0;

    // Read the input list of files and add them to the chain
    TString line;
    while (in.good())
    {
      in >> line;

      if (line.Length() == 0)
        continue;

      if (offset > 0)
      {
        offset--;
        continue;
      }

      if (count++ == aRuns)
        break;

      TString aodFile(line);

      if (line.EndsWith(".zip")) {
	aodFile += "#AliAOD.root";
      }
      else if (addFileName) {
        aodFile += "/AliAOD.root";
      }
      
      if (check)
      {
        TFile* file = TFile::Open(aodFile);
        if (!file)
          continue;
        file->Close();
 
        outfile << line.Data() << endl;
        printf("%s\n", line.Data());
      }        
        
      // add aod file
      chain->Add(aodFile);

      if (chainFriend) {
        TString friendFileName(aodFile);
        friendFileName.ReplaceAll("AliAOD.root", friends);
        chainFriend->Add(friendFileName);
      }
    }
    
    in.close();
    
    if (check) {
      outfile.close();
    }
  }

  if (chainFriend) {
    chain->AddFriend(chainFriend);
  }

  return chain;
}

TChain* CreateESDChain(
  const char* aDataDir = "ESDfiles.txt", 
  Int_t aRuns          = 20, 
  Int_t offset         = 0, 
  Bool_t addFileName   = kFALSE, 
  Bool_t addFriend     = kFALSE, 
  const char* check    = 0
)
{
  // creates chain of files in a given directory or file containing a list.
  // In case of directory the structure is expected as:
  // <aDataDir>/<dir0>/AliESDs.root
  // <aDataDir>/<dir1>/AliESDs.root
  // ...
  //
  // if addFileName is true the list only needs to contain the directories that contain the AliESDs.root files
  // if addFriend is true a file AliESDfriends.root is expected in the same directory and added to the chain as friend
  // if check is != 0 the files that work are written back into the textfile with the name check

  if (!aDataDir)
    return 0;

  Long_t id, size, flags, modtime;
  if (gSystem->GetPathInfo(aDataDir, &id, &size, &flags, &modtime))
  {
    printf("%s not found.\n", aDataDir);
    return 0;
  }

  TChain* chain = new TChain("esdTree");
  TChain* chainFriend = 0;
  
  if (addFriend)
    chainFriend = new TChain("esdFriendTree");

  if (flags & 2)
  {
    TString execDir(gSystem->pwd());
    TSystemDirectory* baseDir = new TSystemDirectory(".", aDataDir);
    TList* dirList            = baseDir->GetListOfFiles();
    Int_t nDirs               = dirList->GetEntries();
    gSystem->cd(execDir);

    Int_t count = 0;

    for (Int_t iDir=0; iDir<nDirs; ++iDir)
    {
      TSystemFile* presentDir = (TSystemFile*) dirList->At(iDir);
      if (!presentDir || !presentDir->IsDirectory() || strcmp(presentDir->GetName(), ".") == 0 || strcmp(presentDir->GetName(), "..") == 0)
        continue;

      if (offset > 0)
      {
        --offset;
        continue;
      }

      if (count++ == aRuns)
        break;

      TString presentDirName(aDataDir);
      presentDirName += "/";
      presentDirName += presentDir->GetName();

      chain->Add(presentDirName + "/AliESDs.root/esdTree");
    }
  }
  else
  {
    // Open the input stream
    ifstream in;
    in.open(aDataDir);

    ofstream outfile;
    if (check)
      outfile.open(check);

    Int_t count = 0;

    // Read the input list of files and add them to the chain
    TString line;
    while (in.good())
    {
      in >> line;

      if (line.Length() == 0)
        continue;

      if (offset > 0)
      {
        offset--;
        continue;
      }

      if (count++ == aRuns)
        break;

      TString esdFile(line);

      if (addFileName)
        esdFile += "/AliESDs.root";

      if (esdFile.EndsWith(".zip"))
	esdFile += "#AliESDs.root";
        
      TString esdFileFriend(esdFile);
      esdFileFriend.ReplaceAll("AliESDs.root", "AliESDfriends.root");
        
      if (check)
      {
        TFile* file = TFile::Open(esdFile);
        if (!file)
          continue;
        file->Close();
        
        if (chainFriend)
        {
          TFile* file = TFile::Open(esdFileFriend);
          if (!file)
            continue;
          file->Close();
        }
        
        outfile << line.Data() << endl;
        printf("%s\n", line.Data());
      }        
        
        // add esd file
      chain->Add(esdFile);

        // add file
      if (chainFriend)
        chainFriend->Add(esdFileFriend);
    }

    in.close();
    
    if (check)
      outfile.close();
  }
  
  if (chainFriend)
    chain->AddFriend(chainFriend);

  return chain;
}


