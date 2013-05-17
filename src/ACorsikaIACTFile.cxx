#include "TSystem.h"

#include "ACorsikaIACTFile.h"

const Int_t ACorsikaIACTFile::kMaxArrays = 100;
const Int_t ACorsikaIACTFile::kMaxTelescopes = 1000;

ACorsikaIACTFile::ACorsikaIACTFile(Int_t bufferLength)
  : fBunches(0), fEventHeader(0), fRunHeader(0)
{
  fIOBuffer = allocate_io_buffer(0);
  fIOBuffer->max_length = bufferLength;
  fMaxPhotonBunches = 100000;
  for(Int_t i = 0; i < 4; i++){
    fTelescopePosition[i] = new Double_t[kMaxTelescopes];
  } // if
  fCorsikaInputs.text = 0;
  fCorsikaInputs.next = 0;
}

//_____________________________________________________________________________
ACorsikaIACTFile::~ACorsikaIACTFile()
{
  Close();
  for(Int_t i = 0; i < 4; i++){
    delete [] fTelescopePosition[i];
    fTelescopePosition[i] = 0;
  } // if
}

//_____________________________________________________________________________
void ACorsikaIACTFile::Close()
{
  if(not IsOpen()){
    return;
  } // if

  // Reset all variables
  fclose(fIOBuffer->input_file);
  fIOBuffer->input_file = 0;

  struct linked_string *xl, *xln;
  for(xl = &fCorsikaInputs; xl != 0; xl = xln){
    free(xl->text);
    xl->text = 0;
    xln = xl->next;
    xl->next = 0;
    if(xl != &fCorsikaInputs){
      free(xl);
    } // if
  } // xl

  fNumberOfTelescopes = 0;
  for(Int_t j = 0; j < 4; j++){
    for(Int_t i = 0; i < kMaxTelescopes; i++){
      fTelescopePosition[j][i] = 0;
    } // i
  } // j

  SafeDelete(fBunches);
  SafeDelete(fEventHeader);
  SafeDelete(fRunHeader);

  fFileName = "";
}

//_____________________________________________________________________________
Double_t ACorsikaIACTFile::GetTelescopeR(Int_t i) const
{
  if(0 <= i and i < 100){
    return fTelescopePosition[3][i];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Double_t ACorsikaIACTFile::GetTelescopeX(Int_t i) const
{
  if(0 <= i and i < 100){
    return fTelescopePosition[0][i];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Double_t ACorsikaIACTFile::GetTelescopeY(Int_t i) const
{
  if(0 <= i and i < 100){
    return fTelescopePosition[1][i];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Double_t ACorsikaIACTFile::GetTelescopeZ(Int_t i) const
{
  if(0 <= i and i < 100){
    return fTelescopePosition[2][i];
  } else {
    return atof("NaN");
  } // if
}

//_____________________________________________________________________________
Bool_t ACorsikaIACTFile::IsAllocated()
{
  if(fIOBuffer){
    return kTRUE;
  } else {
    return kFALSE;
  } // if
}

//_____________________________________________________________________________
Bool_t ACorsikaIACTFile::IsOpen()
{
  if(IsAllocated()){
    if(fIOBuffer->input_file){
      return kTRUE;
    }  // if
  } // if

  return kFALSE;
}

//_____________________________________________________________________________
void ACorsikaIACTFile::Open(const Char_t* fname)
{
  if(IsOpen()){
    fprintf(stderr, "Already open.\n");
    Close();
  } // if

  if(IsAllocated()){
    if((fIOBuffer->input_file = fopen(gSystem->ExpandPathName(fname), "r")) == 0){
      fprintf(stderr, "Cannot open the file.\n");
      return;
    } // if
  } // if

  fFileName = TString(fname);

  /*********************************
    == IACT data file structure ==
    IO_TYPE_MC_RUNH     \
    IO_TYPE_MC_INPUTCFG  | RUN Header
    IO_TYPE_MC_TELPOS   /
    IO_TYPE_MC_EVTH     \
    IO_TYPE_MC_TELOFF    | Event data (*)
    IO_TYPE_MC_TELARRAY  |
    IO_TYPE_MC_EVTE     /
    ...Repeat(*)
    IO_TYPE_MC_RUNE        Run end
  **********************************/

  // Check if block type is CORSIKA run header
  if(ReadNextBlock() == IO_TYPE_MC_RUNH){
    Float_t runh[273];
    read_tel_block(fIOBuffer, IO_TYPE_MC_RUNH, runh, 273);
    fRunHeader = new ACorsikaIACTRunHeader(runh);
  } else {
    fprintf(stderr, "The first header is not IO_TYPE_MC_RUNH.\n");
    Close();
    return;
  } // if

  // Check if block type is CORSIKA input configuration
  if(ReadNextBlock() == IO_TYPE_MC_INPUTCFG){
    read_input_lines(fIOBuffer, &fCorsikaInputs);
  } else {
    fprintf(stderr, "The second header is not IO_TYPE_MC_INPUTCFG.\n");
    Close();
    return;
  } // if

  // Check if block type is telescope information
  if(ReadNextBlock() == IO_TYPE_MC_TELPOS){
    read_tel_pos(fIOBuffer, kMaxTelescopes, &fNumberOfTelescopes,
                 fTelescopePosition[0], fTelescopePosition[1],
                 fTelescopePosition[2], fTelescopePosition[3]);
  } else {
    fprintf(stderr, "The third header is not IO_TYPE_MC_TELPOS.\n");
    Close();
    return;
  } // if
}

//_____________________________________________________________________________
void ACorsikaIACTFile::PrintInputCard() const
{
  if(fCorsikaInputs.text!=0){
    const struct linked_string *xl;
    printf("CORSIKA was run with the following input lines:\n");

    for(xl = &fCorsikaInputs; xl!=0; xl=xl->next){
      printf("   %s\n", xl->text);
    } // xl

    fflush(stdout);
  } // if
}

#define SET_FLAG(flag, key) (flag |= (ULong64_t(0x1) << (key - IO_TYPE_MC_BASE)))
#define HAS_FLAG(flag, key) Bool_t(flag &  (ULong64_t(0x1) << (key - IO_TYPE_MC_BASE)))

//_____________________________________________________________________________
Int_t ACorsikaIACTFile::ReadEvent(Int_t num)
{
  if(!IsOpen()){
    fprintf(stderr, "File is not open.\n");
    return -1;
  } // if
  
  if(fEventHeader){
    if(fEventHeader->GetEventNumber() == num){
      // Event is already read. Do nothing.
      return num;
    } else if(fEventHeader->GetEventNumber() > num){
      // Cannot access previous data block in this version.
      // eventio.c does not have such a function.
      return -1;
    } // if
  } // if

  ULong64_t flag = 0x0; // flag indicating what blocks we have read

  while(1) {
    Int_t numberOfArrays; // ICERML of CSCAT option
    Double_t timeOffset;  // Time offset from the first interaction
    Double_t xOffset[kMaxArrays]; // X offset of core locations from (0, 0)
    Double_t yOffset[kMaxArrays]; // Y offset of core locations from (0, 0)

    Int_t headerType = ReadNextBlock();
    if(headerType == -1){
      break;
    } // if
    switch(headerType) {
    case IO_TYPE_MC_EVTH:
      //fprintf(stderr, "IO_TYPE_MC_EVTH\n");
      flag = 0x0; // initialize when we read the event header
      Float_t evth[273];
      read_tel_block(fIOBuffer, IO_TYPE_MC_EVTH, evth, 273);
      // Data blocks were OK, but event number != num. So skip.
      if(evth[1] != num){
        break;
      } // if

      SafeDelete(fEventHeader);
      fEventHeader = new ACorsikaIACTEventHeader(evth);
      SET_FLAG(flag, IO_TYPE_MC_EVTH);
      break;

    case IO_TYPE_MC_TELOFF:
      //fprintf(stderr, "IO_TYPE_MC_TELOFF\n");
      if(HAS_FLAG(flag, IO_TYPE_MC_EVTH)){
        read_tel_offset(fIOBuffer, kMaxArrays, &numberOfArrays, &timeOffset, xOffset, yOffset);
        SET_FLAG(flag, IO_TYPE_MC_TELOFF);
      } // if
      break;

    case IO_TYPE_MC_EXTRA_PARAM:
      //fprintf(stderr, "IO_TYPE_MC_EXTRA_PARAM\n");
      // not implemented yet
      break;

    case IO_TYPE_MC_LONGI:
      //fprintf(stderr, "IO_TYPE_MC_LONGI\n");
      // not implemented yet
      break;

    case IO_TYPE_MC_TELARRAY:
    case IO_TYPE_MC_TELARRAY_HEAD:
    {
      if(headerType == IO_TYPE_MC_TELARRAY){
        //fprintf(stderr, "IO_TYPE_MC_TELARRAY\n");
      } else {
        //fprintf(stderr, "IO_TYPE_MC_TELARRAY_HEAD\n");
      } // if
      if(not (HAS_FLAG(flag, IO_TYPE_MC_EVTH) and HAS_FLAG(flag, IO_TYPE_MC_TELOFF))){
        break;
      } // if

      Int_t instanceNumberOfArrays;
      IO_ITEM_HEADER itemHeader;

      Int_t telIndividual;
      if(headerType == IO_TYPE_MC_TELARRAY){
        telIndividual = 0;
        begin_read_tel_array(fIOBuffer, &itemHeader, &instanceNumberOfArrays);
        SET_FLAG(flag, IO_TYPE_MC_TELARRAY);
      } else {
        telIndividual = 1;
        read_tel_array_head(fIOBuffer, &itemHeader, &instanceNumberOfArrays);
        SET_FLAG(flag, IO_TYPE_MC_TELARRAY_HEAD);
      } // if
      if(fEventHeader) {
        fEventHeader->SetMultipleUseHeader(numberOfArrays, timeOffset, xOffset, yOffset, instanceNumberOfArrays);
      } // if

      SafeDelete(fBunches);
      fBunches = new TTree("tree", "Photon tree of CORSIKA IACT output.");

      Int_t telNo, arrayNo;
      Float_t x, y, zem, time, cx, cy, cz, lambda, photons;
      Double_t totalPhotons;
      fBunches->Branch("telNo", &telNo, "telNo/I");
      fBunches->Branch("arrayNo", &arrayNo, "arrayNo/I");
      fBunches->Branch("x", &x, "x/F");
      fBunches->Branch("y", &y, "y/F");
      fBunches->Branch("zem", &zem, "z/F");
      fBunches->Branch("time", &time, "time/F");
      fBunches->Branch("cx", &cx, "cx/F");
      fBunches->Branch("cy", &cy, "cy/F");
      fBunches->Branch("cz", &cz, "cz/F");
      fBunches->Branch("lambda", &lambda, "lambda/F");
      fBunches->Branch("photons", &photons, "photons/F");

      struct bunch* bunches = (struct bunch*)malloc(fMaxPhotonBunches*sizeof(struct bunch));

      if(bunches == 0){
        fprintf(stderr, "Error in allocating memory for photon bunch array.\n");
        return -1;
      } // if

      for(Int_t i = 0; i < fNumberOfTelescopes; i++){
        if(not telIndividual){
          IO_ITEM_HEADER subItemHeader;
          subItemHeader.type = IO_TYPE_MC_PHOTONS;
          if(search_sub_item(fIOBuffer, &itemHeader, &subItemHeader) < 0){
            break;
          } // if
        } else {
          if(find_io_block(fIOBuffer, &fBlockHeader) != 0){
            break;
          } // if
          if(read_io_block(fIOBuffer, &fBlockHeader) != 0){
            break;
          } // if
          if(fBlockHeader.type == IO_TYPE_MC_TELARRAY_END){
            telIndividual = 0;
            break;
          } // if
          if(fBlockHeader.type != IO_TYPE_MC_PHOTONS){
            telIndividual = 0;
            break;
          } // if
        } // if

        Int_t nbunches;
        if(read_tel_photons(fIOBuffer, fMaxPhotonBunches, &arrayNo, &telNo,
                            &totalPhotons,  bunches, &nbunches) < 0){
          fprintf(stderr,"Error reading %d photon bunches\n",nbunches);
          continue;
        } // if

        if(arrayNo != instanceNumberOfArrays){
          // do nothing for now
        } // if
        if(i >= kMaxTelescopes or telNo < 0){
          fprintf(stderr, "Cannot process data for telescope #%d because only %d are configured.\n", i + 1, kMaxTelescopes);
          continue;
        } // if
      
        for(Int_t j = 0; j < nbunches; j++){
          x = bunches[j].x;
          y = bunches[j].y;
          zem = bunches[j].zem;
          time = bunches[j].ctime;
          cx = bunches[j].cx;
          cy = bunches[j].cy;
          cz = -1.*sqrt(1. - cx*cx - cy*cy);
          lambda = bunches[j].lambda;
          photons = bunches[j].photons;
          fBunches->Fill();
        } // j
      } // i

      free(bunches);

      if(fBlockHeader.type == IO_TYPE_MC_TELARRAY){
        end_read_tel_array(fIOBuffer, &itemHeader);
      } // if

      break;
    }
    case IO_TYPE_MC_EVTE:
      //fprintf(stderr, "IO_TYPE_MC_EVTE\n");
      break;

    case IO_TYPE_MC_RUNE:
      //fprintf(stderr, "IO_TYPE_MC_RUNE\n");
      break;

    default:
      break;

    } // switch

    if(HAS_FLAG(flag, IO_TYPE_MC_EVTH) and HAS_FLAG(flag, IO_TYPE_MC_TELOFF) and
       (HAS_FLAG(flag, IO_TYPE_MC_TELARRAY) or HAS_FLAG(flag, IO_TYPE_MC_TELARRAY_HEAD))){
      return num;
    } // if
  } // while

  return -1;
}

//_____________________________________________________________________________
Int_t ACorsikaIACTFile::ReadNextBlock()
{
  if(IsOpen()){
    if(find_io_block(fIOBuffer, &fBlockHeader) != 0){
      Close();
      return -1;
    } // if

    if(read_io_block(fIOBuffer, &fBlockHeader) != 0){
      Close();
      return -1;
    } // if
  } else {
    return -1;
  } // if

  return fBlockHeader.type;
}
