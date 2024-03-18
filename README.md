# Recursive RePair (Re<sup>2</sup>Pair)

Recursive RePair (Re<sup>2</sup>Pair) is a grammar based compression scheme designed for compressing large repetetive files. Re<sup>2</sup>Pair applies Recursive Prefix-Free Parsing (PFP) on the input file to parse it into phrases which are later processed by RePair. This work extends and is inspired by [BigRePair](https://gitlab.com/manzai/bigrepair) by Gagie et al. [1]. See our paper for more details on the theory for this software.

## Installation

To install Re<sup>2</sup>Pair, the following steps should be run.
```
git clone https://github.com/jkim210/Recursive-RePair.git
cd Recursive-RePair/src
make
cd ..
./reRePair -h
```

> [!IMPORTANT]  
> `reRePair` is a Python script that requires at least **Python 3.6** installed.

## Dependencies

In order to perform the initial recursive PFP step on the input file, we used this version of [PFP](https://github.com/marco-oliva/pfp) developed by Marco Oliva [2].

Assuming that you have installed the aforementioned version of PFP in the `Recursive-RePair` directory, the following steps are used to run recursive PFP on the test data provided in this repository.

> [!NOTE]  
> How PFP is called on the command line will differ depending on how and where it was installed.


1. Run PFP on the `yeast.fasta` file
```
./pfp++ -t data/yeast.fasta -w 10 -p 100 -c
````
This should produce the following file in the `data` directory: `yeast.fasta.dict`, `yeast.fasta.dicz`, `yeast.fasta.dicz.len`, `yeast.fasta.parse`

2. Run PFP on the `yeast.fasta.parse`
```
./pfp++ -i data/yeast.fasta.parse -w 10 -p 100 -c
```
This should produce the following file in the `data` directory: `yeast.fasta.parse.dict`, `yeast.fasta.parse.dicz`, `yeast.fasta.parse.dicz.len`, `yeast.fasta.parse.parse`       

> [!NOTE]  
>  This version of PFP also supports VCF files. To use VCF files, modify the first PFP command with the appropriate VCF settings.

## Usage

Assuming that recursive PFP was applied correctly earlier, to build the grammar for the `yeast.fasta` file just type on the command line:

```
./reRePair -i data/yeast.fasta -k
```

In the `data` directory, the following files `yeast.fasta.C` and `yeast.fasta.R` will be created if no errors occured.

To decompress and recover the original file, type on the command line:

```
./reRePair -d data/yeast.fasta -k
```

This command will read the `yeast.fasta.C` and `yeast.fasta.R` files and produce a `yeast.fasta.out` file that is identical to the original `yeast.fasta` file. 

To see all the options available, type on the command line:

```
./reRePair -h
```
## Citation

Please, if you use this tool in an academic setting cite the following paper:


## Authors

### Theoretical Results

* [Justin Kim](https://github.com/jkim210)
* [Rahul Varki](https://github.com/rvarki)
* Christina Boucher

### Experiments

* [Rahul Varki](https://github.com/rvarki)

### Implementation

* [Justin Kim](https://github.com/jkim210)


## References

1. Travis Gagie, Tomohiro I, Giovanni Manzini, Gonzalo Navarro, Hiroshi Sakamoto, Yoshimasa Takabatake: Rpair: Rescaling RePair with Rsync. Proc. SPIRE '19 CoRR abs/1906.00809 (2019)

2. Marco Oliva, Travis Gagie, and Christina Boucher. Recursive Prefix-Free Parsing for Building Big BWTs. In 2023 Data Compression Conference (DCC), pages 62–70. IEEE, 2023.
