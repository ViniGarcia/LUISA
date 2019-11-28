# LUISA: A Fast and Flexible PPM-based Compressor

*Status: Version 1.0*

### What is LUISA?

<p align="justify">LUISA is a prediction-based file compressor tool. This tool employs an innovative key/entropy encoding method to compress and decompress files of any type. LUISA relaxes the integration of the context tree models (as the ones used in Prediction by Partial Matching compressor) with the entropy encoding algorithms. To do that, instead of using the different context tree layers to find the probability of a particular symbol appearing, we create a generic key related to the symbol position in the same layers. Thus, blocks of keys are created and then provided to the entropic encoder to make the compression process. In this way, LUISA decouples the context model treats it as an independent task of the whole compressing/decompressing process. As a result of these context model decoupling, different entropic encoders can be naturally employed to compress the LUISA's keys. This feature makes LUISA, to best of our knowledge, the most interoperable prediction-based compressor.</p>

### How was it developed?

<p align="justify">LUISA has been developed in C programming language and uses already existing entropy encoders such as Huff0 (fast Huffman compressor), Finite State Entropy, and the traditional Arithmetic Encoding.</p>

### What does this repository provides?

<p align="justify">This repository provides several versions of the LUISA compressor. Here, you will find the Windows .exe files and their respective source codes and a CodeBlocks project in a .zip file. We separate the LUISA versions regarding most important characteristics, for example:</p>

| Feature 		   				| Versions                                                                    |
| ----------------------------- | --------------------------------------------------------------------------- |
| Entropy Encoder  				| Arithimetic Encoder (AE), Finite State Entropy (FSE), Fast Huffman (HUFF0)  |
| Keys Generation Optimizations | Update Exclusion (UE), Without Update Exclusion (WUE)						  |
| Implementation Type           | Traditional (LUISA), Fast (LUISA FAST)                                      |
| Context Tree Update   		| Frequency, Frequency-Swap, Move-to-Front (MTF), Swap  					  |

<p align="justify">These files are provided in three folders: "1.CLASSIC (With Update Exclusion)", "2.FAST (With Update Exclusion)", and "3.FAST (Without Update Exclusion)". Furthermore, we provide the source code of the most recent version of LUISA, which employs a PPMii context modeling [1]. This latest LUISA version is our state-of-art implementation and it is provided in "4.CURRENT (With Update Exclusion)" folder.</p>

### But... What is the difference among the versions?

<p align="justify">There are many actually... The entropy encoder will define a clear tradeoff between processing time and compression ratio. Typically, the AE gets the best compression ratio but requires extra time to do so. FSE is the halfway between AE and HUFF0. Finally, HUFF0 is the fastest entropy encoder but, typically, gets the worst compression ratio. Observe that the same tradeoff is found in the keys generation optimization versions. UE versions tend to achieve a better compression ratio, while WUE versions are mostly faster than UE.</p>

<p align="justify">The implementation type imposes a different tradeoff: memory usage vs processing time. The traditional implementation of LUISA uses linked lists to save and search in the context tree. The fast implementation, in turn, uses hash tables to create the context tree.</p>

<p align="justify">Finally, the context tree update does not create a clear tradeoff. Better compression ratios can be achieved by using the best update version to the input files type. For example, files with extremally fast content modifications and with context combinations present only in a section of the file (e.g., dictionaries) can benefit from MTF versions. Files with fast content modifications that happen in different sections (e.g., partially ordered files) can benefit from the swap and frequency-swap implementations. At last, files that use a particular set of symbols and have random context distributions (e.g., generic text files) are more adequate to the frequency implementation.</p>

<p align="justify">It is important to notice that our latest version ("4.CURRENT (With Update Exclusion)") employs a frequency oriented context update and the update exclusion technique. Furthermore, we enabled both the FSE and Huff0 compressors in the implementation to be configured on-demand through a flag. Other options, such as context tree memory, entropy encoder data block size, and context tree overflow recover mode, are also provided as flags to be configured on-demand.</p>

### Can I test the LUISA Compressor?

<p align="justify">Yes, for sure. To make it easier, we developed a testing framework located in the "5.BENCHMARK" folder. This framework is suited to Windows hosts and it is developed with Python 3 programming language. In there, we provide an easy way to benchmark several compressors with many well-known corpora. Also, we provide a set of results regarding the LUISA compressor in CSV files, in addition to the YAML scripts to reproduce their execution by using the framework.</p>

### Support

<p align="justify">Contact us towards git issues requests or by the e-mails vfulber@inf.ufsm.br and mergen@inf.ufsm.br.</p>

### LUISA Research Group

Vinícius Fülber Garcia (Federal University of Paraná)<br/>
Sérgio Luis Sardi Mergen (Federal University of Santa Maria)

### References

[1] D. Shkarin, "PPM: one step to practicality," Proceedings DCC 2002. Data Compression Conference, Snowbird, UT, USA, 2002, pp. 202-211. doi: 10.1109/DCC.2002.999958
