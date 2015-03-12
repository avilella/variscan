/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'mp' 2.0                           */
/*                                                                          */
/*      Copyright (C) 2000 Remi Gribonval, Emmanuel Bacry and Javier Abadia.*/
/*      Copyright (C) 2001-2003 Remi Gribonval                              */
/*      email  : remi.gribonval@inria.fr                                    */
/*      email  : lastwave@cmap.polytechnique.fr                             */
/*                                                                          */
/*..........................................................................*/
/*                                                                          */
/*      This program is a free software, you can redistribute it and/or     */
/*      modify it under the terms of the GNU General Public License as      */
/*      published by the Free Software Foundation; either version 2 of the  */
/*      License, or (at your option) any later version                      */
/*                                                                          */
/*      This program is distributed in the hope that it will be useful,     */
/*      but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/*      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       */
/*      GNU General Public License for more details.                        */
/*                                                                          */
/*      You should have received a copy of the GNU General Public License   */
/*      along with this program (in a file named COPYRIGHT);                */
/*      if not, write to the Free Software Foundation, Inc.,                */
/*      59 Temple Place, Suite 330, Boston, MA  02111-1307  USA             */
/*                                                                          */
/*..........................................................................*/


#include "lastwave.h"
#include "mp_book.h"

/*
 * Write/Read a TFContent to/from a stream, either in binary or XML-like format.
 * When called with NULL tfContent in write mode, it writes an XML-like description of the format.
 * When called with NULL tfContent in read mode, it checks the presence of a XML-like description 
 * of the format compatible with this version. If not, an error is generated.
 */
static void ReadWriteStreamTFContent_2_0(ATFCONTENT tfContent,FILE *stream,char flagWrite,char flagBinary)
{
  int cnt;
  int  bufferSize = 63;
  char buffer[63];
  if(tfContent == NULL) {   // Checking or Writing Info
    if(flagWrite) {
      fprintf(stream,"<item> <id>tfcontent</id> <version>2.0</version> <content>\n");
      fprintf(stream,"<item> <id>x0</id> <type>float</type> </item>\n");
      fprintf(stream,"<item> <id>dx</id> <type>float</type> </item>\n");
      fprintf(stream,"<item> <id>signalSize</id> <type>unsigned long</type> </item>\n");
      fprintf(stream,"</content> </item>\n");
    } else {
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>tfcontent</id> <version>2.0</version> <content>\n")) Errorf("ReadWriteStreamTFContent : bad format beginning [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>x0</id> <type>float</type> </item>\n")) Errorf("ReadWriteStreamTFContent : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>dx</id> <type>float</type> </item>\n")) Errorf("ReadWriteStreamTFContent : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>signalSize</id> <type>unsigned long</type> </item>\n")) Errorf("ReadWriteStreamTFContent : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"</content> </item>\n")) Errorf("ReadWriteStreamTFContent : bad format ending [%s]",buffer);
    }
    return;
  }

  // TODO : Shall also read/write freqIdNyquist !!!
  if(flagBinary) {// Binary mode
    if(flagWrite) {  // Writing
      fwrite((char *)(&(tfContent->x0)),sizeof(float),1,stream);
      fwrite((char *)(&(tfContent->dx)),sizeof(float),1,stream);
      fwrite((char *)(&(tfContent->signalSize)),sizeof(unsigned long),1,stream);
    } else {         // Reading
      fread((char *)(&(tfContent->x0)),sizeof(float),1,stream);
      fread((char *)(&(tfContent->dx)),sizeof(float),1,stream);
      fread((char *)(&(tfContent->signalSize)),sizeof(unsigned long),1,stream);
    }
  } else {        // XML-like mode
    if(flagWrite) {  // Writing
      fprintf(stream,"<tfcontent> %g %g %ld</tfcontent>\n",tfContent->x0,tfContent->dx,tfContent->signalSize);
    } else {         // Reading
      fgets(buffer,bufferSize,stream);
      cnt = sscanf(buffer,"<tfcontent> %g %g %ld</tfcontent>\n",&(tfContent->x0),&(tfContent->dx),&(tfContent->signalSize));
      if(cnt != 3) Errorf("ReadWriteStreamTFContent : error while reading [%s]",buffer);
    }
  }

  if(!flagWrite) {
    CheckTFContent(tfContent);
  }
}

/*
 * Write/Read an Atom to/from a stream, either in binary or XML-like format.
 * When called with NULL atom in write mode, it writes an XML-like description of the format.
 * When called with NULL atom in read mode, it checks the presence of a XML-like description 
 * of the format compatible with this version. If not, an error is generated.
 */
static void ReadWriteStreamAtom_2_0(ATOM atom,FILE *stream,char flagWrite,char flagBinary)
{
  int cnt;

  int bufferSize = 255;
  char buffer[255];

  char windowShapeName[255];
  
  if(atom == NULL) {  // Checking or Writing Info
    if(flagWrite) {
      fprintf(stream,"<item> <id>atom</id> <version>2.0</version> <content>\n");
      fprintf(stream,"<item> <id>windowShape</id> <type>char</type> </item> \n");
      fprintf(stream,"<item> <id>windowSize</id> <type>unsigned long</type> </item>\n");
      fprintf(stream,"<item> <id>timeId</id> <type>float</type> </item>\n");
      fprintf(stream,"<item> <id>freqId</id> <type>float</type> </item>\n");
      fprintf(stream,"<item> <id>chirpId</id> <type>float</type> </item>\n");
      fprintf(stream,"<item> <id>coeffR</id> <type>float</type> </item>\n");
      fprintf(stream,"<item> <id>coeffI</id> <type>float</type> </item>\n");
      fprintf(stream,"<item> <id>realGG</id> <type>float</type> </item>\n");
      fprintf(stream,"<item> <id>imagGG</id> <type>float</type> </item>\n");
      fprintf(stream,"<item> <id>coeff2</id> <type>float</type> </item>\n");
      fprintf(stream,"<item> <id>cosPhase</id> <type>float</type> </item>\n");
      fprintf(stream,"<item> <id>sinPhase</id> <type>float</type> </item>\n");
      fprintf(stream,"</content> </item>\n");
    } else {
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>atom</id> <version>2.0</version> <content>\n")) Errorf("ReadWriteStreamAtom : bad format beginning [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>windowShape</id> <type>char</type> </item> \n")) Errorf("ReadWriteStreamAtom : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>windowSize</id> <type>unsigned long</type> </item>\n")) Errorf("ReadWriteStreamAtom : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>timeId</id> <type>float</type> </item>\n")) Errorf("ReadWriteStreamAtom : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>freqId</id> <type>float</type> </item>\n")) Errorf("ReadWriteStreamAtom : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>chirpId</id> <type>float</type> </item>\n")) Errorf("ReadWriteStreamAtom : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>coeffR</id> <type>float</type> </item>\n")) Errorf("ReadWriteStreamAtom : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>coeffI</id> <type>float</type> </item>\n")) Errorf("ReadWriteStreamAtom : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>realGG</id> <type>float</type> </item>\n")) Errorf("ReadWriteStreamAtom : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>imagGG</id> <type>float</type> </item>\n")) Errorf("ReadWriteStreamAtom : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>coeff2</id> <type>float</type> </item>\n")) Errorf("ReadWriteStreamAtom : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>cosPhase</id> <type>float</type> </item>\n")) Errorf("ReadWriteStreamAtom : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>sinPhase</id> <type>float</type> </item>\n")) Errorf("ReadWriteStreamAtom : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"</content> </item>\n")) Errorf("ReadWriteStreamAtom : bad format ending [%s]",buffer);
    }
    return;
  }

  if(flagBinary) {  // Binary mode
    if(flagWrite) {    // Writing
      fwrite((char *)(&(atom->windowShape)),sizeof(char),1,stream);
      fwrite((char *)(&(atom->windowSize)),sizeof(unsigned long),1,stream);
      fwrite((char *)(&(atom->timeId)),sizeof(float),1,stream);
      fwrite((char *)(&(atom->freqId)),sizeof(float),1,stream);
      fwrite((char *)(&(atom->chirpId)),sizeof(float),1,stream);
      fwrite((char *)(&(atom->coeffR)),sizeof(float),1,stream);
      fwrite((char *)(&(atom->coeffI)),sizeof(float),1,stream);
      fwrite((char *)(&(atom->realGG)),sizeof(float),1,stream);
      fwrite((char *)(&(atom->imagGG)),sizeof(float),1,stream);
      fwrite((char *)(&(atom->coeff2)),sizeof(float),1,stream);
      fwrite((char *)(&(atom->cosPhase)),sizeof(float),1,stream);
      fwrite((char *)(&(atom->sinPhase)),sizeof(float),1,stream);
    } else {           // Reading
      fread((char *)(&(atom->windowShape)),sizeof(char),1,stream);
      fread((char *)(&(atom->windowSize)),sizeof(unsigned long),1,stream);
      fread((char *)(&(atom->timeId)),sizeof(float),1,stream);
      fread((char *)(&(atom->freqId)),sizeof(float),1,stream);
      fread((char *)(&(atom->chirpId)),sizeof(float),1,stream);
      fread((char *)(&(atom->coeffR)),sizeof(float),1,stream);
      fread((char *)(&(atom->coeffI)),sizeof(float),1,stream);
      fread((char *)(&(atom->realGG)),sizeof(float),1,stream);
      fread((char *)(&(atom->imagGG)),sizeof(float),1,stream);
      fread((char *)(&(atom->coeff2)),sizeof(float),1,stream);
      fread((char *)(&(atom->cosPhase)),sizeof(float),1,stream);
      fread((char *)(&(atom->sinPhase)),sizeof(float),1,stream);
    }
  } else {         // XML-like mode
    if(flagWrite) {   // Writing
      fprintf(stream,"<atom> %s %ld %g %g %g %g %g %g %g %g %g %g </atom>\n",
	      WindowShape2Name(atom->windowShape),atom->windowSize,atom->timeId,atom->freqId,atom->chirpId,
	      atom->coeffR,atom->coeffI,atom->realGG,atom->imagGG,atom->coeff2,atom->cosPhase,atom->sinPhase);
    } else {         // Reading
      fgets(buffer,bufferSize,stream);
      cnt = sscanf(buffer,"<atom> %s %ld %g %g %g %g %g %g %g %g %g %g </atom>\n",
	     windowShapeName,&(atom->windowSize),&(atom->timeId),&(atom->freqId),&(atom->chirpId),
	     &(atom->coeffR),&(atom->coeffI),&(atom->realGG),&(atom->imagGG),&(atom->coeff2),&(atom->cosPhase),&(atom->sinPhase));
      if(cnt != 12) Errorf("ReadWriteStreamAtom : error while reading [%s]",buffer);
      atom->windowShape = Name2WindowShape(windowShapeName);
    }
  }

  if(!flagWrite) {
    atom->flagGGIsSet = YES;
    CheckAtom(atom);
  }
}

/*
 * Write/Read a Molecule to/from a stream, either in binary or XML-like format.
 * When reading, a non-NULL tfContent must be provided. When writing, it must be NULL.
 * When called with NULL molecule in write mode, it writes an XML-like description of the format.
 * When called with NULL molecule in read mode, it checks the presence of a XML-like description 
 * of the format compatible with this version. If not, an error is generated.
 */
static void ReadWriteStreamMolecule_2_0(MOLECULE molecule,FILE *stream,char flagWrite,char flagBinary,ATFCONTENT tfContent)
{
  int cnt;
  int bufferSize = 63;
  char buffer[63];

  static MOLECULE  tmpMolecule = NULL;

  unsigned short dim;
  unsigned short nChannels;

  unsigned short k;
  unsigned char  channel;
  ATOM atom;

  // Initialization (once only)
  if(tmpMolecule==NULL) tmpMolecule = NewMolecule();

  if(molecule == NULL) {  // Checking or Writing Info
    if(flagWrite) {
      fprintf(stream,"<item> <id>molecule</id> <version>2.0</version> <content>\n");
      fprintf(stream,"<item> <id>dim</id> <type>unsigned short</type> </item> \n");
      fprintf(stream,"<item> <id>nChannels</id> <type>unsigned char</type> </item>\n");
      ReadWriteStreamAtom_2_0(NULL,stream,YES,NO);
      fprintf(stream,"</content> </item>\n");
    } else {
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>molecule</id> <version>2.0</version> <content>\n")) Errorf("ReadWriteStreamMolecule : bad format beginning [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>dim</id> <type>unsigned short</type> </item> \n")) Errorf("ReadWriteStreamMolecule : bad format [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>nChannels</id> <type>unsigned char</type> </item>\n")) Errorf("ReadWriteStreamMolecule : bad format [%s]",buffer);
      ReadWriteStreamAtom_2_0(NULL,stream,NO,NO);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"</content> </item>\n")) Errorf("ReadWriteStreamMolecule : bad format ending [%s]",buffer);
    }
    return;
  }

  // Checking
  if(flagWrite && (tfContent != NULL))  Errorf("ReadWriteStreamMolecule : tfContent must be NULL when writing");
  if(!flagWrite && (tfContent == NULL)) Errorf("ReadWriteStreamMolecule : tfContent must be non-NULL when reading");


  if(!flagWrite)  ClearMolecule(molecule);

  // Read or write the dimension and number of channels
  if(flagBinary) {// Binary mode
    if(flagWrite) {  // Writing
      fwrite((char *)(&(molecule->dim)),sizeof(unsigned short),1,stream);
      fwrite((char *)(&(molecule->nChannels)),sizeof(unsigned char),1,stream);
    } else {         // Reading
      fread((char *)(&dim),sizeof(unsigned short),1,stream);
      fread((char *)(&nChannels),sizeof(unsigned char),1,stream);
    }
  } else {        // XML-like mode
    if(flagWrite) {  // Writing
      fprintf(stream,"<molecule> %hu %hu\n",molecule->dim,molecule->nChannels);
    } else {         // Reading
      fgets(buffer,bufferSize,stream);
      cnt = sscanf(buffer,"<molecule> %hu %hu\n",&dim,&nChannels);
      if(cnt != 2) Errorf("ReadWriteStreamMolecule : error while reading [%s]",buffer);
    }
  }

  // Read or write the atoms
  if(flagWrite) {  // Writing
    for(channel = 0; channel < molecule->nChannels; channel++) {
      for(k=0; k < molecule->dim; k++) {
	atom = GetMoleculeAtom(molecule,channel,k);
	ReadWriteStreamAtom_2_0(atom,stream,flagWrite,flagBinary);
      }      
    }
  } else {         // Reading
    SizeMolecule(molecule,nChannels*dim);
    // Read the first channel
    for(k=0; k < dim; k++) {
      atom = NewAtom();
      CopyFieldsTFContent(tfContent,atom);
      ReadWriteStreamAtom_2_0(atom,stream,flagWrite,flagBinary);
      AddAtom2Molecule(molecule,atom);
    }
    // Read the other channels if necessary
    for(channel = 1; channel < molecule->nChannels; channel++) {
      ClearMolecule(tmpMolecule);
      for(k=0; k < molecule->dim; k++) {
	atom = NewAtom();
	CopyFieldsTFContent(tfContent,atom);
	ReadWriteStreamAtom_2_0(atom,stream,flagWrite,flagBinary);
	AddAtom2Molecule(tmpMolecule,atom);
      }      
      AddChannel2Molecule(molecule,tmpMolecule);
    }
  }

  // Case when we need to read or write the terminating flag
  if(!flagBinary) {
    if(flagWrite) { // Writing
      fprintf(stream,"</molecule>\n");
    } else {        // Reading
      fgets(buffer,bufferSize,stream);
      sscanf(buffer,"</molecule>\n");
    }
  }
}

/*
 * Write/Read a Book from a stream, either in binary or XML-like format.
 * When called with NULL book in write mode, it writes an XML-like description of the format.
 * When called with NULL book in read mode, it checks the presence of a XML-like description 
 * of the format compatible with this version. If not, an error is generated.
 */
static void ReadWriteStreamBook_2_0(BOOK book,FILE *stream,char flagWrite,char flagBinary)
{
  int cnt;
  int bufferSize = 63;
  char buffer[63];

  unsigned long size;
  unsigned long n;
  MOLECULE molecule;

  if(book == NULL) { // Checking or Writing Info
    if(flagWrite) {
      fprintf(stream,"<item> <id>book</id> <version>2.0</version> <content>\n");
      fprintf(stream,"<item> <id>size</id> <type>unsigned long</type> </item> \n");
      ReadWriteStreamTFContent_2_0(NULL,stream,YES,NO);
      ReadWriteStreamMolecule_2_0(NULL,stream,YES,NO,NULL);
      fprintf(stream,"</content> </item>\n");
    } else {
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>book</id> <version>2.0</version> <content>\n")) Errorf("ReadWriteStreamBook : bad format beginning [%s]",buffer);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"<item> <id>size</id> <type>unsigned long</type> </item> \n")) Errorf("ReadWriteStreamBook : bad format [%s]",buffer);
      ReadWriteStreamTFContent_2_0(NULL,stream,NO,NO);
      ReadWriteStreamMolecule_2_0(NULL,stream,NO,NO,NULL);
      fgets(buffer,bufferSize,stream);
      if(strcmp(buffer,"</content> </item>\n")) Errorf("ReadWriteStreamBook : bad format beginning [%s]",buffer);
    }
    return;
  }

  // Read or write the tfContent
  if(!flagWrite) ClearBook(book);
  ReadWriteStreamTFContent_2_0((ATFCONTENT)book,stream,flagWrite,flagBinary);

  // Read or write the size
  if(flagBinary) {// Binary mode
    if(flagWrite) {  // Writing
      size = book->size;
      fwrite((char *)(&size),sizeof(unsigned long),1,stream);
    } else {         // Reading
      fread((char *)(&size),sizeof(unsigned long),1,stream);
    }
  } else {        // XML-like mode
    if(flagWrite) {  // Writing
      size = book->size;
      fprintf(stream,"<book> %lu\n",size);
    } else {         // Reading
      fgets(buffer,bufferSize,stream);
      cnt = sscanf(buffer,"<book> %lu\n",&size);
      if(cnt != 1) Errorf("ReadWriteStreamBook : error while reading [%s]",buffer);
    }
  }

  // Read or write the molecules
  if(flagWrite) {// Writing
    for(n=0; n < book->size; n++) {
      molecule = GetBookMolecule(book,n);
      ReadWriteStreamMolecule_2_0(molecule,stream,flagWrite,flagBinary,NULL);
    }      
  } else {       // Reading
    SizeBook(book,size);
    for(n=0; n < size; n++) {
      molecule = NewMolecule();
      ReadWriteStreamMolecule_2_0(molecule,stream,flagWrite,flagBinary,(ATFCONTENT)book);
      AddMolecule2Book(book,molecule);
    }
  }

  // Case when we need to read or write the terminating flag
  if(!flagBinary) {
    if(flagWrite) {// Writing
      fprintf(stream,"</book>\n");
    } else {       // Reading
      fgets(buffer,bufferSize,stream);
      sscanf(buffer,"</book>\n");
    }
  }
}

/*
 * Write/Read a Book from a stream, either in binary or XML-like format.
 */
void ReadWriteBook(BOOK book,FILE *stream,char flagWrite,char flagBinary)
{
  int cnt;
  int bufferSize = 63;
  char buffer[63];

  char binaryStr[63];

  // Read or write a header
  if(flagWrite) {
    fprintf(stream,"LastWave Header\n");
    // The binary mode first
    if(flagBinary) {
      if(IsCPULittleEndian) fprintf(stream,"<binary> little </binary>\n");
      else                  fprintf(stream,"<binary> big </binary>\n");
    } else                  fprintf(stream,"<binary> no </binary>\n");
    fprintf(stream,"End of Header\n");
  } else {
    fgets(buffer,bufferSize,stream);
    if(strcmp(buffer,"LastWave Header\n")) Errorf("ReadWriteBook : bad header beginning [%s]",buffer);
    // The binary mode first
    fgets(buffer,bufferSize,stream);
    cnt = sscanf(buffer,"<binary> %s </binary>\n",binaryStr);
    if(cnt!=1) Errorf("ReadWriteBook : bad binary format [%s]",binaryStr);
    if(!strcmp(binaryStr,"no")) {    // Case where we have to read in ASCII
      flagBinary = NO;
    } else if((!strcmp(binaryStr,"little") && IsCPULittleEndian) || (!strcmp(binaryStr,"big") && !IsCPULittleEndian)) {
      flagBinary = YES;
    } else {                        // Case where we have to convert between LittleEndian and BigEndian
      Errorf("ReadWriteBook : conversion between LittleEndian and BigEndian not implemented yet");
    }
    fgets(buffer,bufferSize,stream);
    if(strcmp(buffer,"End of Header\n")) Errorf("ReadWriteBook : bad format beginning [%s]",buffer);
  }
  ReadWriteStreamBook_2_0(NULL,stream,flagWrite,flagBinary);
  ReadWriteStreamBook_2_0(book,stream,flagWrite,flagBinary);
}

/*********************/
/*
 * 	Utilities for some old file formats
 */
/*********************/
static unsigned long ComputeStftMaxFreqId(unsigned long signalSize)
{
  unsigned short logMaxFreqId;
  unsigned long  maxFreqId;
  /* Checking argument */
  if(signalSize <= 2)  Errorf("ComputeStftMaxFreqId : signalSize %d is too small",signalSize);

  /*
   * The 'brute' logMaxFreqId is the largest 'n' 
   * such that 2^n \le signalSize 
   */
  logMaxFreqId = (int) (log((float) signalSize)/log(2.0)+.5);
  maxFreqId  =  1<<logMaxFreqId;
  if(maxFreqId > signalSize) {
    maxFreqId  /= 2;
    logMaxFreqId -= 1;
  }
  /* 
   * We do not want to get bigger than
   * the maximum allowed maxFreqId
   */
  if(maxFreqId > STFT_MAX_WINDOWSIZE) maxFreqId  = STFT_MAX_WINDOWSIZE;
  return(maxFreqId);
}


/******************************/
/* READ OLD BOOK FILES
/******************************/
extern ATOM   ReadAtomOld(BOOK book,char flagBinary,char windowShape,unsigned long forceMaxFreqId,char flagChirp,FILE * stream);
extern MOLECULE ReadMoleculeOld(BOOK book,char flagBinary,char windowShape,unsigned long forceMaxFreqId,char flagChirp,char flagHarmo,FILE * stream,float *pResEnergy);
void ReadBookOld(BOOK book,FILE *stream,unsigned long forceMaxFreqId,SIGNAL residualEnergy)
{
  struct aTFContent tfContent;
  int n;
  char str[255];
  
  char name[255];
  int flagBinary = NO;
  
  float dx;
  int signalSize;
  float signalEnergy;
  SIGNAL signal;
  
  int borderType;
  int windowShape;
  
  int oMin,oMax;
  int tNSON,fNSON;
  int flagNoFreq;
  
  char flagChirp = NO;
  char flagHarmo = NO;

  int flagMZPhase,flagHighRes,depthHighRes;
  int flagHarmonic;
  float freq0IdMin,freq0IdMax;
  int dimHarmo,iFMO;
  
  float energy;
  int size;
  float threshold;
  float resEnergy;
  int i;
  MOLECULE molecule;
  
  int cnt = 0;
  
  // Clear the book
  ClearBook(book);
  /* Let's try to read the header */
  cnt += fscanf(stream,"%[^\n] ",str);
  if (strcmp(str,"LastWave Header")) Errorf("Error in the header of the file");
  n = fscanf(stream,"Name : %[^\n] ",name);
  if(n != 1) Errorf("Name is missing in book header");
  cnt += n;
  /* The signal information*/
  cnt += fscanf(stream,"Signal dx        : %g ",&dx);
  cnt += fscanf(stream,"Signal size      : %d ",&signalSize);
  cnt += fscanf(stream,"Signal energy    : %g ",&signalEnergy);
  /* The dict information */
  cnt += fscanf(stream,"Decomp border type  %d ",&borderType);
  cnt += fscanf(stream,"Decomp atomWindow   %d ",&windowShape);
  cnt += fscanf(stream,"Decomp oMin  %d oMax  %d ",&oMin,&oMax);
  cnt += fscanf(stream,"Decomp tNSON %d fNSON %d ",&tNSON,&fNSON);
  cnt += fscanf(stream,"Decomp flagNoFreq   %d ",&flagNoFreq);
  cnt += fscanf(stream,"Decomp flagMZPhase   %d ",&flagMZPhase);
  cnt += fscanf(stream,"Decomp flagHighRes   %d ",&flagHighRes);
  cnt += fscanf(stream,"Decomp depthHighRes  %d ",&depthHighRes);
  cnt += fscanf(stream,"Decomp flagMolecular    %d ",&flagHarmonic);
  cnt += fscanf(stream,"Decomp freq0IdMin %g freq0IdMax %g ",&freq0IdMin,&freq0IdMax);
  cnt += fscanf(stream,"Decomp K    %d ",&dimHarmo);
  cnt += fscanf(stream,"Decomp iFMO %d ",&iFMO);
  cnt += fscanf(stream,"Book energy      %g ",&energy);
  cnt += fscanf(stream,"Book size        %d ",&size);
  cnt += fscanf(stream,"Book threshold   %g ",&threshold);
  cnt += fscanf(stream,"%[^\n] ",str);
  if(!strcmp(str,"BINARY"))     flagBinary = YES;
  else if(!strcmp(str,"ASCII")) flagBinary = NO;
  else  Errorf("Unknown book file type %s",str);
  cnt += fscanf(stream,"%[^\n] ",str);
  if(strcmp(str,"End of Header"))   Errorf("Error at the end of book header");
  
  /* Setting the TFContent */
  tfContent.signalSize = signalSize;
  tfContent.dx         = dx;
  tfContent.x0         = 0;
  CopyFieldsTFContent(&tfContent,book);

  // Initializing the 'residualEnergy'
  SizeSignal(residualEnergy,size+1,YSIG);
  ZeroSig(residualEnergy);
  residualEnergy->size = 0;

  residualEnergy->Y[0] = signalEnergy;
  residualEnergy->size++;
  
  
  if(flagBinary == NO) {
    /* Skipping two lines */
    cnt += fscanf(stream,"%[^\n]\n",str);
    if(!strcmp(str,"ATOMS : octave timeId freqId innerProdR innerProdI phase g2Cos2 energy coeff2")) flagChirp=NO;
    else if(!strcmp(str,"ATOMS : octave timeId freqId chirpId innerProdR innerProdI phase g2Cos2 realGG imagGG energy coeff2")) flagChirp=YES;
    else Errorf("ReadBookOld : WRONG ATOMS format : %s",str);

    cnt += fscanf(stream,"%[^\n]\n",str);
    if(!strcmp(str,"WORDS : dim energy resEnergy coeff2 status")) flagHarmo=YES;
    else Errorf("ReadBookOld : WRONG MOLECULE format : %s",str);
  }
  
  for(i = 0; i < size; i++) {
    molecule = ReadMoleculeOld(book,flagBinary,windowShape,forceMaxFreqId,flagChirp,flagHarmo,stream,&resEnergy);
    AddMolecule2Book(book,molecule);
    residualEnergy->Y[residualEnergy->size-1]=resEnergy;
    residualEnergy->size++;
  }
}

MOLECULE ReadMoleculeOld(BOOK book,char flagBinary,char windowShape,unsigned long forceMaxFreqId,char flagChirp,char flagHarmo,FILE * stream,float *pResEnergy)
{
  MOLECULE molecule;
  ATOM atom;
  int dim;
  int i;
  float energy;
  float resEnergy;
  float coeff2;
  int status;
  /* */
  int cnt;
  
  /* Checking argument */
  if(!WindowShapeIsOK(windowShape))  Errorf("ReadMoleculeOld : bad windowShape %d",windowShape);
  if(pResEnergy==NULL)               Errorf("ReadMoleculeOld : NULL pResEnergy");
  cnt = 0;
if(flagHarmo==NO) Errorf("ReadMoleculeOld : harmo=NO not implemented!");

  if(flagBinary==YES) { /* binary reading */
    cnt+= fread( (char *)(&dim), sizeof(int), 1, stream);
    cnt+= fread( (char *)(&energy), sizeof(float), 1, stream);
    cnt+= fread( (char *)(&resEnergy), sizeof(float), 1, stream);
    cnt+= fread( (char *)(&coeff2), sizeof(float), 1, stream);
    cnt+= fread( (char *)(&status), sizeof(int), 1, stream);
  }
  else {
    cnt += fscanf(stream,"%d %g %g %g %d\n",&dim,&energy,&resEnergy,&coeff2,&status);
  }
  
  if (cnt == EOF)   Errorf("End of file encountered in ReadMolecule");
  if (cnt != 5)     Errorf("Reading error in ReadMolecule");
  
  /* Reading atoms */
  molecule = NewMolecule();
  SizeMolecule(molecule,dim);
  for(i = 0; i < dim; i++) {
    atom = ReadAtomOld(book,flagBinary,windowShape,forceMaxFreqId,flagChirp,stream);
    if(atom==NULL) continue;
    AddAtom2Molecule(molecule,atom);
  }
  *pResEnergy= resEnergy;
  return(molecule);
}


ATOM ReadAtomOld(BOOK book,char flagBinary,char windowShape,unsigned long forceMaxFreqId,char flagChirp,FILE * stream)
{
  ATOM atom;
  int   octave;
  float timeId;
  float freqId;
  float chirpId = 0.0;
  
  float coeffR;
  float coeffI;
  float phase;
  
  float g2Cos2;
  float realGG = 0.0;
  float imagGG = 0.0;
  
  float energy;
  float coeff2;
  
  /* */
  int cnt;
  
  /* Checking arguments */
  if(book == NULL)                   Errorf("ReadAtomOld : NULL book");
  if(!WindowShapeIsOK(windowShape))  Errorf("ReadAtomOld : bad windowShape %d",windowShape);
  
  cnt = 0;
  /* binary version */
  if(flagBinary==YES) { 
    cnt+= fread( (char *)(&octave), sizeof(int), 1, stream);
    cnt+= fread( (char *)(&timeId), sizeof(float), 1, stream);
    cnt+= fread( (char *)(&freqId), sizeof(float), 1, stream);
    if(flagChirp)
    cnt+= fread( (char *)(&chirpId), sizeof(float), 1, stream);
    cnt+= fread( (char *)(&coeffR), sizeof(float), 1, stream);
    cnt+= fread( (char *)(&coeffI), sizeof(float), 1, stream);
    
    cnt+= fread( (char *)(&phase), sizeof(float), 1, stream);
    
    cnt+= fread( (char *)(&g2Cos2), sizeof(float), 1, stream);
    
    cnt+= fread( (char *)(&energy), sizeof(float), 1, stream);
    cnt+= fread( (char *)(&coeff2), sizeof(float), 1, stream);
  }
  else {
    if(flagChirp==NO) {
      cnt += fscanf(stream,"%d %g %g %g %g %g %g %g %g\n",&octave,&timeId,&freqId,&coeffR,&coeffI,&phase,&g2Cos2,&energy,&coeff2);
    } else {
      cnt += fscanf(stream,"%d %g %g %g %g %g %g %g %g %g %g %g\n",&octave,&timeId,&freqId,&chirpId,&coeffR,&coeffI,&phase,&g2Cos2,&realGG,&imagGG,&energy,&coeff2);
    }
  }
  
  if (cnt == EOF)   Errorf("End of file encountered in ReadAtomOld");
  if(flagChirp==NO) {
    if (cnt != 9)     Errorf("Reading error in ReadAtomOld");
  } else {
    if (cnt != 12)    Errorf("Reading error in ReadAtomOld");
  }
  // Case of NULL atoms
  if(octave==0) return(NULL);

  atom 	 = NewAtom();
  CopyFieldsTFContent(book,atom);
  atom->windowShape = windowShape;
  // Everything but Diracs
  if(octave >= 2) {
    atom->windowSize = 1<<octave;
    atom->timeId = timeId;
    // Converts freqId/chirpId from range [0,forceMaxFreqId/2] to range [0,GABOR_MAX_FREQID/2]
    if(!INRANGE(0,freqId,forceMaxFreqId/2)) 
      Warningf("ReadAtomOld : freqId %g > forceMaxFreqId/2 %d means that you probably under-estimated the true maxFreqId of the old book",
	       freqId,forceMaxFreqId/2);
    freqId = freqId*(GABOR_MAX_FREQID)/forceMaxFreqId;
    atom->freqId = freqId;
    if(fabsf(chirpId)>((float)forceMaxFreqId)/4) 
      Warningf("ReadAtomOld : |chirpId|= %g > forceMaxFreqId/4 %g means that you probably under-estimated the true maxFreqId of the old book",
	       chirpId,((float)forceMaxFreqId)/4);
    chirpId = chirpId*(GABOR_MAX_FREQID)/forceMaxFreqId;
    atom->chirpId = chirpId;
    
    atom->coeffR = coeffR;
    atom->coeffI = coeffI;
    atom->coeff2 = coeff2;    
    atom->cosPhase = cos(2*M_PI*(phase-(int)phase));
    atom->sinPhase = sin(2*M_PI*(phase-(int)phase));
    atom->realGG 	= realGG;
    atom->imagGG 	= imagGG;
    atom->flagGGIsSet = YES;
  }
  // Case of Dirac
  else {
    // One can check that a Dirac is exactly a 4-point Gabor atom 
    // at frequency 0.25 : w[n-timeId]cos(2*pi*(n-timeId)/4) because
    // for n<=timeId-2 and n>=timeId+2, w[n-timeId] = 0 
    // for n =timeId-1 and n =timeId+1, cos(2*pi*(n-timeId)/4)=0
    atom->windowSize = 4;
    atom->timeId     = timeId;
    atom->freqId     = GABOR_NYQUIST_FREQID/2;
    atom->chirpId    = 0.0;
    atom->coeffR     = coeffR;
    atom->coeffI     = coeffI;
    CastAtomReal(atom);
  }
  return(atom);
}


/* COMMAND */
void C_Book(char **argv)
{
  char *action;
  BOOK book = NULL;
  char *filename;
  FILE *stream;
  char opt;
  char flagBinary = NO; 

  SIGNAL decaySignal;
  unsigned long forceMaxFreqId;
 
 /* For I/O */
  
  /* Parsing command */
  argv = ParseArgv(argv,tWORD,&action,-1);
  // Write a book to a file, either in binary or in ascii
  if(!strcmp(action,"write")) {
    argv = ParseArgv(argv,tBOOK_,NULL,&book,tSTR,&filename,-1);
    if(book == NULL) book = GetBookCur();
    CheckBookNotEmpty(book);
    if ((stream = FOpen(filename, "w")) == NULL)  
      Errorf("Can't open file '%s' for writing book\n", filename);
    while( (opt = ParseOption(&argv)) ) {
      if(opt=='b') flagBinary = YES; /* for binary writing */
      else ErrorOption(opt);
    }
    NoMoreArgs(argv);
    
    ReadWriteBook(book,stream,YES,flagBinary);
    FClose(stream);	
    return;
  }
  // Read a book from a file, the binary/ascii format is determined from the header
  if(!strcmp(action,"read")) {
    argv = ParseArgv(argv,tBOOK_,NULL,&book,tSTR,&filename,0);
    if(book == NULL) book = GetBookCur();
    if ((stream = FOpen(filename, "r")) == NULL)  
      Errorf("Can't open file '%s' for reading book\n", filename);
    ReadWriteBook(book,stream,NO,flagBinary);
    FClose(stream);	
    return;
  }

  // Read a book from a file, the binary/ascii format is determined from the header
  if(!strcmp(action,"readold")) {
    argv = ParseArgv(argv,tBOOK_,NULL,&book,tSTR,&filename,tINT,&forceMaxFreqId,tSIGNAL,&decaySignal,-1);
    if(book == NULL) book = GetBookCur();
    NoMoreArgs(argv);
    
    if ((stream = FOpen(filename, "r")) == NULL) 
      Errorf("Cannot open file '%s' for reading book\n", filename);
    ReadBookOld(book,stream,forceMaxFreqId,decaySignal);
    FClose(stream);
    return;
  }
  
  Printf("Unknown action '%s'\n",action);
  ErrorUsage();
}



/* EOF */




