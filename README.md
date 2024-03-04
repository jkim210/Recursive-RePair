# Recursive RePair (Re<sup>2</sup>Pair)

Recursive RePair (Re<sup>2</sup>Pair) is a grammar based compression scheme designed for compressing large repetetive files. Re<sup>2</sup>Pair applies Recursive Prefix-Free Parsing (PFP) on the input file to parse it into phrases which are later processed by RePair. This work extends and is inspired by [BigRePair](https://gitlab.com/manzai/bigrepair) by Gagie et al. [1].

## Installation

To install Re<sup>2</sup>Pair, the following steps should be run.
```
git clone https://github.com/jkim210/Recursive-RePair.git
cd Recursive-RePair/src
make
cd ..
./reRePair -h
```

> [!NOTE]  
> `reRePair` is a Python script that requires at least **Python 3.6** installed.

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

1. Travis Gagie, Tomohiro I, Giovanni Manzini, Gonzalo Navarro, Hiroshi Sakamoto, Yoshimasa Takabatake: Rpair: Rescaling RePair with Rsync. Proc. SPIRE '19 CoRR abs/1906.00809 (2019)