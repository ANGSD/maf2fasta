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

char **input_data = (char **)malloc(2*sizeof(char*));
char **data = (char **)malloc(2*sizeof(char*));
char *table=NULL;
int startPos = 0;

const char *hg18 = "hg38";
const char *panTro2 = "panTro4";
const char *delims = ". \n\t";
const char *chr = NULL;

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
    if(!strcmp(hg18,tok)){//its a hg18 sequence
      chr = strdup(strtok(NULL,delims));
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
  if(!strcmp(hg18,id))
    return 0;
  else if(!strcmp(panTro2,id))
    return 1;
  else
    return -1;
}


void mergingOcean(int startRead){
  if(input_data[0]==NULL)
    fprintf(stderr,"shit happens\n");

  if(input_data[1]==NULL){//the case of no abe sequence
    int gap =0;
    for(uint i=0;i<strlen(input_data[0]);i++){
      char v = input_data[0][i];
      // if(v!='a'&& v!='c'&&v!='g'&&v!='t'){
	if(v=='-'){
	  gap++;
	  //	  fprintf(stderr,"gap is: %d\n",gap);
	}else
	  data[0][startRead+i-gap] = table[input_data[0][i]];//DRAGON maybe not offsetting
	//}
    }
  }
  else{//we have a abe sequence
    int gap =0;
    for(uint i=0;i<strlen(input_data[0]);i++){
      char v = input_data[0][i];
      // if(v!='a'&& v!='c'&&v!='g'&&v!='t'){
      if(v=='-'){
	gap++;
	//	fprintf(stderr,"gap is: %d\n",gap);}
      }else
	for(int j=0;j<2;j++)
	  data[j][startRead+i-gap] = table[input_data[j][i]];//DRAGON maybe not offsetting
    }
    //}      
  }

  for(int i=0;i<2;i++)
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
FILE *sfp = fopen("start","w");

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
  for(int i=0;i<2;i++){
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
      if(id==0){
	startRead = atoi(tok);
	fprintf(sfp,"%d\n",startRead);
	fflush(sfp);
      }
      //fflush(stdout);
      for(int i=0;i<3;i++)
	tok = strtok(NULL,delims);
      
      input_data[id] = strdup(strtok(NULL,delims));
    }
  }  
  // fprintf(stderr,"after main loop nlines: %d processed\n",pos);
  for(int i=0;i<refLen;i++)
    fprintf(stdout,"%s\t%d\t%c\t%c\n",chr,i+1,data[0][i],data[1][i]);

  //cleanup
  gzclose(fp);
  for(int i=0;i<2;i++)
    free(data[i]);
  free(data);
  free(input_data);
}
