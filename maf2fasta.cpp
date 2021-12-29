/*
  This is a program to get monkey refs from multizXway datasets
  thorfinn.sand@gmail.com 29dec 2021
*/

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <zlib.h>

#define LENS 100000
#define NREFS 4
const char* refs[NREFS] = {"hg38","panTro4","ponAbe2","rheMac3"};

char **input_data = (char **)malloc(NREFS*sizeof(char*));
char **data = (char **)malloc(NREFS*sizeof(char*));
char *table=NULL;
int startPos = 0;



const char *delims = ". \n\t";
char *chr = NULL;

int getRefLen (const char *fname){
  gzFile fp = Z_NULL;
  fp = gzopen(fname,"r");
  assert(fp!=Z_NULL);
  char buf[LENS];
  int pos =0;
  int retval = -1;
  while(gzgets(fp,buf,LENS)) {
    pos++;
    if(buf[0]=='#'|| buf[0]=='a')
      continue;
    char *tok = strtok(buf,delims);
    tok = strtok(NULL,delims);
    if(!tok)
      break;
    if(!strcmp(refs[0],tok)){//its a reference sequence
      chr = strdup(strtok(NULL,delims));
      //loop below is not reference but discarding whitespaces
      for(int i=0;i<4;i++)
	tok= strtok(NULL,delims);
      gzclose(fp);
      return atoi(tok);
    }else{
      continue;

    }
  }
  if(fp!=Z_NULL)
    gzclose(fp);
  return -1;
}

int getId(const char*id){
  int ret =-1;
  for(int i=0;i<NREFS;i++)
    if(strcmp(refs[i],id)==0)
      ret=i;
  return ret;
}


void mergingOcean(int startRead){
  assert(input_data[0]!=NULL);

  for(int j=0;j<NREFS;j++){
    if(input_data[j]==NULL)
      continue;
    int gap =0;
    for(uint i=0;i<strlen(input_data[j]);i++){
      char v = input_data[0][i];
      // if(v!='a'&& v!='c'&&v!='g'&&v!='t'){
      if(v=='-'){
	gap++;
	//	fprintf(stderr,"gap is: %d\n",gap);}
      }else
	data[j][startRead+i-gap] = table[input_data[j][i]];//DRAGON maybe not offsetting
    }
  }

  for(int i=0;i<NREFS;i++)
    if(input_data[i]!=NULL){
      free(input_data[i]);
      input_data[i]=NULL;
    }
}

char *myClearAlloc(int i){
  //  fprintf(stderr,"myclearalloc : %d\n",i);
  char *ret = (char*) malloc(i);
  memset(ret,'N',i); 

  return ret;
}

void setTable(){
  table =(char *) calloc(130,1);
  table['a'] = 'A';
  table['c'] = 'C';
  table['g'] = 'G';
  table['t'] = 'T';
  table['A'] = 'A';
  table['C'] = 'C';
  table['G'] = 'G';
  table['T'] = 'T';
  table['-'] = 'N';
  table['N'] = 'N';
  table['n'] = 'N';
}

int main(int argc,char**argv){
  setTable();//generate a lookup table for a->A etc.
  int refLen = getRefLen(argv[1]);
  assert(refLen>0);
  fprintf(stderr,"%s: Length of reference: %d\n",chr,refLen);
  for(int i=0;i<NREFS;i++){
    data[i] =myClearAlloc(refLen);
    input_data[i]=NULL;
  }
  
  gzFile fp = Z_NULL;
  fp=gzopen(argv[1],"r");
  assert(fp!=Z_NULL);
  char buf[LENS];
  int pos =0;
  int startRead =0;

  while(gzgets(fp,buf,LENS)) {
    pos++;
    if(strlen(buf)==1){
      mergingOcean(startRead);
      continue;
    }
    if(buf[0]=='s'){
      char *tok = strtok(buf,delims);//this is now ''s
      int id = getId(strtok(NULL,delims));
      if(id==-1)
	continue;
      strtok(NULL,delims);//this is now chrID we dont use this
      tok = strtok(NULL,delims); //this is startpostions only using this if hg18 sequence
      if(id==0)
	startRead = atoi(tok);

      //loop below is not refs but discarding whitespaces
      for(int i=0;i<4;i++)
	tok = strtok(NULL,delims);
      input_data[id] = strdup(tok);
    }
  }  
  // fprintf(stderr,"after main loop nlines: %d processed\n",pos);
  fprintf(stdout,"Chromosome\tPosition");
  for(int i=0;i<NREFS;i++)
    fprintf(stdout,"\t%s",refs[i]);
  fprintf(stdout,"\n");
  for(int i=0;i<refLen;i++){
    fprintf(stdout,"%s\t%d",chr,i+1);
    for(int h=0;h<NREFS;h++)
      fprintf(stdout,"\t%c",data[h][i]);
    fprintf(stdout,"\n");
  }

  //cleanup
  gzclose(fp);
  for(int i=0;i<NREFS;i++)
    free(data[i]);
  free(data);
  free(input_data);
  if(chr)
    free(chr);
  if(table)
    free(table);
}
