*Recursive RePair* is a grammar compressor for huge files with many repetitions. Recursive RePair uses Recursive Prefix-Free Parsing (PFP) along with RePair to create a grammar and rules. This work is based on BigRepair by Gagie et al.: https://gitlab.com/manzai/bigrepair

## Installation
* Download/Clone repository
* `bigrepair -h` (get usage instruction)

Note that `bigrepair` is a Python script so you need at least **Python 3.6** installed.

## Dependencies
*Recursive RePair* requires use of PFP from Marco Oliva: https://github.com/marco-oliva/pfp
Prior to running *Recursive RePair* run recursive PFP on the input file as shown below:

       pfp++ -t yeast.fasta -c
       pfp++ -i yeast.fasta.parse -c

This version of PFP also supports VCF files.

## Usage

To build a grammar for file *yeast.fasta* just type

       reRePair yeast.fasta

If no errors occur the files yeast.fasta.C and yeast.fasta.R are created.

To recover the original file, type

       reRePair -d yeast.fasta

this command will read the yeast.fasta.C and yeast.fasta.R files and produce a yeast.fasta.out file identical to the original input yeast.fasta. 

For more options type:

       reRePair -h


## Authors and acknowledgment


## References
