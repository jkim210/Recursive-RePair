extern "C" {
#include "xerrors.h"
}
pthread_mutex_t map_mutex = PTHREAD_MUTEX_INITIALIZER;

// struct shared via mt_parse
typedef struct {
  map<uint64_t,word_stats> *wordFreq; // shared dictionary
  Args *arg;       // command line input 
  long start, end; // input
  long skipped, parsed, words;  // output
  FILE *parse, *last, *sa;
} mt_data;


void *mt_parse(void *dx)
{
  // extract input data
  mt_data *d = (mt_data *) dx;
  Args *arg = d->arg;
  map<uint64_t,word_stats> *wordFreq = d->wordFreq;

  if(arg->verbose>1)
    printf("Scanning from %ld, size %ld\n",d->start,d->end-d->start);

  // open input file 
  ifstream f(arg->inputFileName);
  if(!f.is_open()) {
    perror(__func__);
    throw new std::runtime_error("Cannot open file " + arg->inputFileName);
  }

  // init buffers containing a single symbol 
  assert(arg->bytexsymb>0);
  uint8_t buffer[arg->bytexsymb], dollar[arg->bytexsymb]={0}; //note in c++ also {} is ok
  dollar[arg->bytexsymb-1] = Dollar;  // this is the generalized Dollar symbol  

  // prepare for parsing 
  f.seekg(d->start*arg->bytexsymb); // move to the beginning of assigned region
  d->skipped = d->parsed = d->words = 0;
  
  // init empty KR window
  KR_window krw(arg->w,arg->bytexsymb);
  // current word in the parsing
  ztring word;
  
  if(d->start==0) {  // no need to reach the next kr-window
    if(!arg->compress) 
      ztring_append(word,dollar,arg->bytexsymb);
  }
  else {   // skip some symbol to reach the next breaking window 
    while( true ) {
      f.read((char *)buffer,arg->bytexsymb);
      // if we could not read arg->bytexsymb, it is eof or error
      if(f.gcount()!=arg->bytexsymb) {
        if(f.gcount()==0) {
          if(f.eof()) break;
          else {cerr << "Invalid read at position " << d->start + d->skipped <<" Exiting....\n"; exit(1);}
        }
        else {cerr << "Incomplete symbol at position " << d->start + d->skipped <<" Exiting....\n"; exit(1);}
      }
      // if not bigEndian swap bytes as we compare byte sequences 
      if(arg->bytexsymb>1 && !arg->bigEndian)
        buffer_reverse(buffer,arg->bytexsymb);
      // if we are not simply compressing then we cannot accept 0,1,or 2
      if(!arg->compress && memcmp(buffer,dollar,arg->bytexsymb) <= 0) {
        cerr << "Invalid symbol at position " << d->start + d->skipped <<" Exiting...\n"; exit(1);
      }
      d->skipped++;
      if(d->start + d->skipped == d->end + arg->w) {f.close(); return NULL;} 
      // add new symbol to current word and check if we reached a splitting point 
      ztring_append(word,buffer,arg->bytexsymb);
      uint64_t hash = krw.addsymbol(buffer);
      if(hash%arg->p==0 && d->skipped >= arg->w) break;
    }
    d->parsed = arg->w;   // the kr-window is part of the next word
    d->skipped -= arg->w; // ... so w less chars have been skipped
    // keep only the last w chars 
    word.erase(word.begin(),word.begin() + (word.size() - arg->w*arg->bytexsymb));
  }
  // cout << "Skipped: " << d->skipped << endl;
  
  // there is some actual parsing to do
  uint64_t pos = d->start;             // ending position+1 in text of previous word
  if(pos>0) pos+= d->skipped+ arg->w;  // or 0 for the first word  
  if(arg->SAinfo) assert(IBYTES<=sizeof(pos)); // IBYTES bytes of pos are written to the sa info file 

  while( true ) {
    f.read((char *)buffer,arg->bytexsymb);
    // we must be able to read exactly arg.bytexsymb bytes otherwise the symbol is incomplete
    if(f.gcount()!=arg->bytexsymb) {
      if(f.gcount()==0) break;
      else {cerr << "Incomplete symbol at position " << pos <<" Exiting....\n"; exit(1);}
    }
    // if not bigEndian swap bytes as we want to compare byte sequences 
    if(arg->bytexsymb>1 && !arg->bigEndian)
      buffer_reverse(buffer,arg->bytexsymb);
    // if we are not simply compressing then we cannot accept 0,1,or 2
    if(!arg->compress && memcmp(buffer,dollar,arg->bytexsymb) <= 0) {
      cerr << "Invalid symbol at position " << pos <<" Exiting...\n"; exit(1);
    }
    // add new symbol to current word and check is we reached a splitting point 
    ztring_append(word,buffer,arg->bytexsymb);
    uint64_t hash = krw.addsymbol(buffer);
    d->parsed++;
    if(hash%arg->p==0 && d->parsed>arg->w) {
      // end of word, save it and write its full hash to the output file
      // pos is the ending position+1 of previous word and is updated in the next call
      save_update_word(*arg,word,*wordFreq,d->parse,d->last,d->sa,pos);
      d->words++;
      if(d->start+d->skipped+d->parsed>=d->end+arg->w) {f.close(); return NULL;}
    }    
  }
  // check that we really reached the end of the file
  if(!f.eof()) die("Error reading from input file (mt_parse)");
  // virtually add w null chars at the end of the file and add the last word in the dict
  for(int i=0;i<arg->w;i++)
    ztring_append(word,dollar,arg->bytexsymb);
  save_update_word(*arg,word,*wordFreq,d->parse,d->last,d->sa,pos);
  // close input file and return 
  f.close();
  return NULL;
}


// prefix free parse of file fnam. w is the window size, p is the modulus 
// use a KR-hash as the word ID that is written to the parse file
uint64_t mt_process_file(Args& arg, map<uint64_t,word_stats>& wf)
{
  // get input file size 
  ifstream f(arg.inputFileName, std::ifstream::ate);
  if(!f.is_open()) {
    perror(__func__);
    throw new std::runtime_error("Cannot open input file " +arg.inputFileName);
  }
  long size = f.tellg();
  f.close();
  // check size and trasform it to symbol units 
  if(size%arg.bytexsymb!=0) 
    throw new std::runtime_error("Input file size not multiple of symbol size");
  size = size/arg.bytexsymb;
    
  // prepare and execute threads 
  assert(arg.th>0);
  pthread_t t[arg.th];
  mt_data td[arg.th];
  for(int i=0;i<arg.th;i++) {
    td[i].wordFreq = &wf;
    td[i].arg = &arg;
    td[i].start = i*(size/arg.th); // range start
    td[i].end = (i+1==arg.th) ? size : (i+1)*(size/arg.th); // range end
    assert(td[i].end<=size);
    // open the 1st pass parsing file 
    td[i].parse = open_aux_file_num(arg.inputFileName.c_str(),EXTPARS0,i,"wb");
    if(!arg.compress) {
      // open output file containing the char at position -(w+1) of each word
      td[i].last = open_aux_file_num(arg.inputFileName.c_str(),EXTLST,i,"wb");  
      // if requested open file containing the ending position+1 of each word
      td[i].sa = arg.SAinfo ?open_aux_file_num(arg.inputFileName.c_str(),EXTSAI,i,"wb") : NULL;
    }
    else { td[i].last = td[i].sa = NULL;}
    xpthread_create(&t[i],NULL,&mt_parse,&td[i],__LINE__,__FILE__);
  }
  
  // wait for the threads to finish (in order) and close output files
  long tot_symb=0;
  for(int i=0;i<arg.th;i++) {
    xpthread_join(t[i],NULL,__LINE__,__FILE__);
    if(arg.verbose) {
      cout << "s:" << td[i].start << "  e:" << td[i].end << "  pa:";
      cout << td[i].parsed << "  sk:" << td[i].skipped << "  wo:" << td[i].words << endl;
    }
    // close thread-specific output files 
    fclose(td[i].parse);
    if(td[i].last) fclose(td[i].last);
    if(td[i].sa) fclose(td[i].sa);
    if(td[i].words>0) {
      // extra check
      assert(td[i].parsed>arg.w);
      tot_symb += td[i].parsed - (i!=0? arg.w: 0); //parsed - overlapping 
    }
    else assert(i>0); // the first thread must produce some words
  }
  assert(tot_symb==size);
  return size;   
}


