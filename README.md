*Recursive RePair* is a grammar compressor for huge files with many repetitions. Recursive RePair uses Recursive Prefix-Free Parsing (PFP) along with RePair to create a grammar and rules. This work is based on BigRepair by Gagie et al.: https://gitlab.com/manzai/bigrepair

## Installation
* Download/Clone repository
* `bigrepair -h` (get usage instruction)

Note that `bigrepair` is a Python script so you need at least **Python 3.6** installed.


## Usage

To build a grammar for file *yeast.fasta* just type

       reRePair yeast.fasta

If no errors occur the files yeast.fasta.C and yeast.fasta.R are created.

To recover the original file, type

       bigrepair -d yeast.fasta

this command will read the yeast.fasta.C and yeast.fasta.R files and produce a yeast.fasta.out file identical to the original input yeast.fasta. 


## Authors and acknowledgment


## References
