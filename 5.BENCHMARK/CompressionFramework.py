import os
import sys
import subprocess
import time
import yaml
import statistics

class Corpuses:
	__AVAILABLE = {
		"CALGARY": {
			"PATH": "FILES\\CALGARY\\",
			"FILES": ["bib","book1","book2","geo","news","obj1","obj2","paper1","paper2","paper3","paper4","paper5","paper6","pic","progc","progl","progp","trans"]
		},
		"CANTERBURY": {
			"PATH": "FILES\\CANTERBURY\\",
			"FILES": ["alice29","asyoulik","cp","fields","grammar","kennedy","lcet10","plrabn12","ptt5","sum","xargs","bible","ecoli","world192"]
		},
		"PROTEIN": {
			"PATH": "FILES\\PROTEIN\\",
			"FILES": ["hi","hs","mj","sc"]
		},
		"SILESIA": {
			"PATH":"FILES\\SILESIA\\",
			"FILES": ["dickens","mozilla","mr","nci","ooffice","osdb","reymont","samba","sao","webster","xml","x-ray"]
		},
		"PIZZACHILLI":{
			"PATH":"FILES\\PIZZACHILLI\\",
			"FILES": ["DNA", "ENGLISH50MB", "ENGLISH100MB", "ENGLISH200MB", "ENGLISH1024MB", "PITCHES", "PROTEINS", "SOURCES", "XML"]
		},
		"DICTIONARIES":{
			"PATH":"FILES\\DICTIONARIES\\",
			"FILES": ["ENGLISH", "ENGLISH_EXT", "ENGLISH_EXT_2", "POLISH", "PORTUGUESE", "SPANISH"]
		}
	}

	def __init__(self):
		pass

	def requestCorpus(self, corpus):
		corpusFiles = []

		if not corpus in self.__AVAILABLE:
			return None

		for file in self.__AVAILABLE[corpus]["FILES"]:
			corpusFiles.append(self.__AVAILABLE[corpus]["PATH"] + file)

		return corpusFiles


class Compressors:
	__CORPUSES = None
	__COMPRESSOR = None
	__ROUNDS = None
	__AVAILABLE = {
		"LUISA": {
			"SKT": [("PATH", "EXE", "MARK", "FILE_IN", "FILE_OUT", "EXT", "ORDER", "HASH"), ("PATH", "EXE", "MARK", "FILE_IN", "EXT", "FILE_OUT")],
			"PATH": "EXE\\",
			"EXE": ["LUISA-AE-FREQUENCY-SWAP-UE.exe", "LUISA-AE-FREQUENCY-UE.exe", "LUISA-AE-SWAP-UE.exe", "LUISA-AE-MTF-UE.exe"],
			"MARK": ["-c", "-d"],
			"FILE_IN": "",
			"FILE_OUT": "",
			"EXT": ".lu"
		},

		"LUISA_FAST": {
			"SKT": [("PATH", "EXE", "MARK", "FILE_IN", "FILE_OUT", "EXT", "ORDER"), ("PATH", "EXE", "MARK", "FILE_IN", "EXT", "FILE_OUT")],
			"PATH": "EXE\\",
			"EXE": ["LUISA-FAST-FSE-FREQUENCY-SWAP-UE.exe", "LUISA-FAST-FSE-FREQUENCY-UE.exe", "LUISA-FAST-FSE-SWAP-UE.exe", "LUISA-FAST-FSE-MTF-UE.exe", "LUISA-FAST-HUFF0-FREQUENCY-SWAP-UE.exe", "LUISA-FAST-HUFF0-FREQUENCY-UE.exe", "LUISA-FAST-HUFF0-SWAP-UE.exe", "LUISA-FAST-HUFF0-MTF-UE.exe", "LUISA-FAST-FSE-FREQUENCY-SWAP-WUE.exe", "LUISA-FAST-FSE-FREQUENCY-WUE.exe", "LUISA-FAST-FSE-SWAP-WUE.exe", "LUISA-FAST-FSE-MTF-WUE.exe", "LUISA-FAST-HUFF0-FREQUENCY-SWAP-WUE.exe", "LUISA-FAST-HUFF0-FREQUENCY-WUE.exe", "LUISA-FAST-HUFF0-SWAP-WUE.exe", "LUISA-FAST-HUFF0-MTF-WUE.exe"],
			"MARK": ["-c", "-d"],
			"FILE_IN": "",
			"FILE_OUT": "",
			"EXT": ".lu"
		},
		"LUISA_FAST_EXTRA_MEMORY": {
			"SKT": [("PATH", "EXE", "MARK", "FILE_IN", "FILE_OUT", "EXT", "ORDER"), ("PATH", "EXE", "MARK", "FILE_IN", "EXT", "FILE_OUT")],
			"PATH": "EXE\\",
			"EXE": ["LUISA-FSE-FREQUENCY-UE-4X.exe", "LUISA-FSE-FREQUENCY-WUE-4X.exe", "LUISA-FSE-MTF-UE-4X.exe", "LUISA-FSE-SWAP-UE-4X.exe", "LUISA-HUFF0-FREQUENCY-UE-4X.exe"],
			"MARK": ["-c", "-d"],
			"FILE_IN": "",
			"FILE_OUT": "",
			"EXT": ".lu"
		},
		"LUISA_EXTRA_MEMORY": {
			"SKT": [("PATH", "EXE", "MARK", "FILE_IN", "FILE_OUT", "EXT", "ORDER", "HASH"), ("PATH", "EXE", "MARK", "FILE_IN", "EXT", "FILE_OUT")],
			"PATH": "EXE\\",
			"EXE": ["LUISA-AE-FREQUENCY-UE-4X.exe"],
			"MARK": ["-c", "-d"],
			"FILE_IN": "",
			"FILE_OUT": "",
			"EXT": ".lu"
		},
		"LUISA_FAST_MTF": {
			"SKT": [("PATH", "EXE", "MARK", "FILE_IN", "FILE_OUT", "EXT", "ORDER"), ("PATH", "EXE", "MARK", "FILE_IN", "EXT", "FILE_OUT")],
			"PATH": "EXE\\",
			"EXE": ["LUISA-FAST-FSE-MTF-UE.exe", "LUISA-FAST-HUFF0-MTF-UE.exe", "LUISA-FAST-FSE-MTF-WUE.exe", "LUISA-FAST-HUFF0-MTF-WUE.exe"],
			"MARK": ["-c", "-d"],
			"FILE_IN": "",
			"FILE_OUT": "",
			"EXT": ".lu"
		},

		"LUISA_PPMD_MODEL": {
			"SKT": [("PATH", "EXE", "MARK", "FILE_IN", "FILE_OUT", "EXT", "OPTIONS"), ("PATH", "EXE", "MARK", "FILE_IN", "EXT", "FILE_OUT")],
			"PATH": "EXE\\",
			"EXE": ["LUISA-PPMD-MODEL.exe"],
			"MARK": ["-c", "-d"],
			"FILE_IN": "",
			"FILE_OUT": "",
			"EXT": ".lu"
		},

		"LUISA_PPMD_MODEL_SV": {
			"SKT": [("PATH", "EXE", "MARK", "FILE_IN", "FILE_OUT", "EXT", "OPTIONS"), ("PATH", "EXE", "MARK", "FILE_IN", "EXT", "FILE_OUT")],
			"PATH": "EXE\\",
			"EXE": ["LUISA-PPMD-MODEL-SV.exe"],
			"MARK": ["-c", "-d"],
			"FILE_IN": "",
			"FILE_OUT": "",
			"EXT": ".lu"
		},

		"BSC": {
			"SKT": [("PATH", "EXE", "MARK", "FILE_IN", "FILE_OUT", "EXT", "OPTIONS"), ("PATH", "EXE", "MARK", "FILE_IN", "EXT", "FILE_OUT")],
			"PATH": "EXE\\",
			"EXE": ["BSC.exe"],
			"MARK": ["e", "d"],
			"FILE_IN": "",
			"FILE_OUT": "",
			"EXT": ".bs"
		},

		"P5": {
			"SKT": [("PATH", "EXE", "FILE_OUT", "EXT", "FILE_IN"), ("PATH", "EXE", "FILE_IN", "EXT")],
			"PATH": "EXE\\",
			"EXE": ["P5.exe"],
			"FILE_IN": "",
			"FILE_OUT": "",
			"EXT": ".p5"
		},

		"ZSTD": {
			"SKT": [("PATH", "EXE", "MARK", "FILE_IN"), ("PATH", "EXE", "MARK", "FILE_IN", "EXT")],
			"PATH": "EXE\\",
			"EXE": ["ZSTD.exe"],
			"MARK": ["-f -k", "-d -f -k"],
			"FILE_IN": "",
			"EXT": ".zst"
		},

		"BZIP2": {
			"SKT": [("PATH", "EXE", "MARK", "FILE_IN"), ("PATH", "EXE", "MARK", "FILE_IN", "EXT")],
			"PATH": "EXE\\",
			"EXE": ["BZIP2.exe"],
			"MARK": ["-f -k", "-d -f -k"],
			"FILE_IN": "",
			"EXT": ".bz2"
		},

		"GZIP": {
			"SKT": [("PATH", "EXE", "MARK", "FILE_IN"), ("PATH", "EXE", "MARK", "FILE_IN", "EXT")],
			"PATH": "EXE\\",
			"EXE": ["GZIP.exe"],
			"MARK": ["-f -k", "-d -f -k"],
			"FILE_IN": "",
			"EXT": ".gz"
		},

		"LZF": {
			"SKT": [("PATH", "EXE", "MARK", "FILE_IN"), ("PATH", "EXE", "MARK", "FILE_IN", "EXT")],
			"PATH": "EXE\\",
			"EXE": ["LZF.exe"],
			"MARK": ["-f -c", "-f -d"],
			"FILE_IN": "",
			"EXT": ".lzf"
		},

		"PPMD": {
			"SKT": [("PATH", "EXE", "MARK", "FILE_IN"), ("PATH", "EXE", "MARK", "FILE_IN", "EXT")],
			"PATH": "EXE\\",
			"EXE": ["PPMD.exe"],
			"MARK": ["e", "d"],
			"FILE_IN": "",
			"EXT": ".pmd"
		},

		"SR3": {
			"SKT": [("PATH", "EXE", "MARK", "FILE_IN", "FILE_OUT", "EXT"), ("PATH", "EXE", "MARK", "FILE_IN", "EXT", "FILE_OUT")],
			"PATH": "EXE\\",
			"EXE": ["SR3.exe"],
			"MARK": ["c", "d"],
			"FILE_IN": "",
			"FILE_OUT": "",
			"EXT": ".sr3"
		},

		"7ZIP": {
			"SKT": [("PATH", "EXE", "MARK", "FILE_OUT", "EXT", "FILE_IN"), ("PATH", "EXE", "MARK", "FILE_IN", "EXT")],
			"PATH": "EXE\\",
			"EXE": ["7Z.exe"],
			"MARK": ["a", "e"],
			"FILE_IN": "",
			"FILE_OUT": "",
			"EXT": ".7z"
		}
	}

	def __init__(self, COMPRESSOR, CONFIGURE, ROUNDS):
		self.__CORPUSES = Corpuses()

		for name in COMPRESSOR:
			if not name in self.__AVAILABLE:
				print("ERROR -> INVALID COMPRESSOR: " + name)
				exit()

			if name in CONFIGURE:
				if "MARK" in CONFIGURE[name]:
					self.__AVAILABLE[name]["MARK"][0] += " " + CONFIGURE[name]["MARK"] #Only for compression, saved in compressed file metadata before!
					del CONFIGURE[name]["MARK"]
				self.__AVAILABLE[name].update(CONFIGURE[name])

			for info in self.__AVAILABLE[name]["SKT"][0]:
				if not info in self.__AVAILABLE[name]:
					print("ERROR -> INFORMATION MISSING: " + name + " (" + info + ")")
					exit()

		self.__COMPRESSOR = COMPRESSOR
		self.__ROUNDS = ROUNDS

	def __compressCommand(self, file):
		commandLines = {}

		if self.__COMPRESSOR == None:
			print("ERROR -> COMPRESSOR WAS NOT CONFIGURED")
			exit()

		for name in self.__COMPRESSOR:
			commandLines[name] = {}
			self.__AVAILABLE[name]["FILE_IN"] = file

			if "FILE_OUT" in self.__AVAILABLE[name]:
				self.__AVAILABLE[name]["FILE_OUT"] = file

			for program in self.__AVAILABLE[name]["EXE"]:
				commandLines[name][program] = ""

				for element in self.__AVAILABLE[name]["SKT"][0]:	

					if element == "EXE":
						commandLines[name][program] += program
						commandLines[name][program] += " "
						continue
					
					if element == "MARK":
						commandLines[name][program] += str(self.__AVAILABLE[name][element][0])
						commandLines[name][program] += " "
						continue

					if element == "EXT":
						commandLines[name][program] = commandLines[name][program][:-1]
						
					commandLines[name][program] += str(self.__AVAILABLE[name][element])

					if element != "PATH":
						commandLines[name][program] += " "

		return commandLines

	def __decompressCommand(self, file):
		commandLines = {}

		if self.__COMPRESSOR == None:
			print("ERROR -> COMPRESSOR WAS NOT CONFIGURED")
			exit()

		for name in self.__COMPRESSOR:
			commandLines[name] = {}
			self.__AVAILABLE[name]["FILE_IN"] = file

			if "FILE_OUT" in self.__AVAILABLE[name]:
				self.__AVAILABLE[name]["FILE_OUT"] = file

			for program in self.__AVAILABLE[name]["EXE"]:
				commandLines[name][program] = ""

				for element in self.__AVAILABLE[name]["SKT"][1]:	

					if element == "EXE":
						commandLines[name][program] += program
						commandLines[name][program] += " "
						continue
					
					if element == "MARK":
						commandLines[name][program] += str(self.__AVAILABLE[name][element][1])
						commandLines[name][program] += " "
						continue

					if element == "EXT":
						commandLines[name][program] = commandLines[name][program][:-1]
						
					commandLines[name][program] += str(self.__AVAILABLE[name][element])

					if element != "PATH":
						commandLines[name][program] += " "

		return commandLines

	def compressFile(self, file):
		rawResults = {}

		os.system("xcopy \"" + file + "\" .\\ /Y")
		file = file.split("\\")[-1]
		commandSet = self.__compressCommand(file)
		for compressor in commandSet:
			rawResults[compressor] = {}
			for program in commandSet[compressor]:
				print("BENCHMARKING PROGRAM (COMP): " + program)
				rawResults[compressor][program] = {"UNCOMPRESSED":os.path.getsize(file), "COMPRESSED":None, "TIME":[]}
				subprocess.check_call(commandSet[compressor][program], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
				rawResults[compressor][program]["COMPRESSED"] = os.path.getsize(file + self.__AVAILABLE[compressor]["EXT"])
				os.remove(file + self.__AVAILABLE[compressor]["EXT"])
				for test in range(self.__ROUNDS):
					start = time.time()
					subprocess.check_call(commandSet[compressor][program], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
					rawResults[compressor][program]["TIME"].append(time.time() - start)
					os.remove(file + self.__AVAILABLE[compressor]["EXT"])
		os.remove(file)

		return rawResults


	def decompressFile(self, original):
		rawResults = {}
		
		os.system("xcopy \"" + original + "\" .\\ /Y")
		file = original.split("\\")[-1]
		compressSet = self.__compressCommand(file)
		decompressSet = self.__decompressCommand(file)
		for compressor in decompressSet:
			rawResults[compressor] = {}
			for program in decompressSet[compressor]:
				print("BENCHMARKING PROGRAM (DECOMP): " + program)
				rawResults[compressor][program] = {"UNCOMPRESSED":os.path.getsize(file), "COMPRESSED":None, "TIME":[]}
				subprocess.check_call(compressSet[compressor][program], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
				rawResults[compressor][program]["COMPRESSED"] = os.path.getsize(file + self.__AVAILABLE[compressor]["EXT"])
				os.remove(file)
				for test in range(self.__ROUNDS):
					start = time.time()
					subprocess.check_call(decompressSet[compressor][program], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
					rawResults[compressor][program]["TIME"].append(time.time() - start)
					os.system("del \"*" + file + "\"")
				os.remove(file + self.__AVAILABLE[compressor]["EXT"])
				os.system("xcopy \"" + original + "\" .\\ /Y")
		os.system("del \"*" + file + "\"")

		return rawResults

	def compressBatch(self, files):
		rawResults = {}

		for file in files:
			rawResults[file] = self.compressFile(file)

		return rawResults

	def decompressBatch(self, files):
		rawResults = {}

		for file in files:
			rawResults[file] = self.decompressFile(file)

		return rawResults

	def compressRequest(self, CORPUSES, FILES):
		rawResults = {}

		for corpus in CORPUSES:
			print ("COMPRESSING CORPUS: " + corpus)
			rawResults[corpus] = self.compressBatch(self.__CORPUSES.requestCorpus(corpus))
		if len(FILES) > 0:
			print ("COMPRESSING CORPUS: PERSONAL")
			rawResults["PERSONAL"] = self.compressBatch(FILES)

		return rawResults

	def decompressRequest(self, CORPUSES, FILES):
		rawResults = {}

		for corpus in CORPUSES:
			print ("DECOMPRESSING CORPUS: " + corpus)
			rawResults[corpus] = self.decompressBatch(self.__CORPUSES.requestCorpus(corpus))
		if len(FILES) > 0:
			print ("DECOMPRESSING CORPUS: PERSONAL")
			rawResults["PERSONAL"] = self.decompressBatch(FILES)

		return rawResults


class Requests:

	__ID = None
	__ROUNDS = None
	__TESTS = None
	__COMPRESSORS = None
	__CONFIGURE = None
	__CORPUSES = None
	__FILES = None

	__EXECUTION = None
	__STATUS = None

	def __init__(self, FILE):

		try:
			openFile = open(FILE, "r")
			request = yaml.safe_load(openFile)
			openFile.close()
		except:
			print("ERROR -> INVALID FILE: " + FILE)
			self.__STATUS = -1
			return

		if not "ID" in request or not "ROUNDS" in request or not "TESTS" in request:
			print("ERROR -> REQUEST IS NOT CORRECTLY DEFINED")
			self.__STATUS = -2
			return
		if not "COMPRESSORS" in request or not "CONFIGURE" in request:
			print("ERROR -> COMPRESSORS ARE NOT CORRECTLY DEFINED")
			self.__STATUS = -3
			return
		if not "CORPUSES" in request or not "FILES" in request:
			print("ERROR -> INPUTS ARE NOT CORRECTLY DEFINED")
			self.__STATUS = -4
			return

		if not isinstance(request["ID"], str) or not isinstance(request["ROUNDS"], int) or not isinstance(request["TESTS"], str):
			print("ERROR -> INVALID DATA IN REQUEST DEFINITION")
			self.__STATUS = -5
			return
		if not isinstance(request["COMPRESSORS"], list) or not isinstance(request["CONFIGURE"], dict):
			print("ERROR -> INVALID DATA IN COMPRESSORS DEFINITION")
			self.__STATUS = -6
			return
		if not isinstance(request["CORPUSES"], list) or not isinstance(request["FILES"], list):
			print("ERROR -> INVALID DATA IN INPUTS DEFINITION")
			self.__STATUS = -7
			return

		if request["ROUNDS"] <= 0:
			print("ERROR -> INVALID NUMBER IN ROUNDS ATTRIBUTE")
			self.__STATUS = -8
			return

		if request["TESTS"] != "COMPRESSION" and request["TESTS"] != "DECOMPRESSION" and request["TESTS"] != "ALL":
			print("ERROR -> INVALID KEY IN TESTS ATTRIBUTE")
			self.__STATUS = -9
			return

		for compressor in request["COMPRESSORS"]:
			if compressor not in ["LUISA", "LUISA_FAST", "LUISA_FAST_EXTRA_MEMORY", "LUISA_EXTRA_MEMORY", "LUISA_FAST_MTF", "LUISA_PPMD_MODEL", "LUISA_PPMD_MODEL_SV", "BSC", "P5", "ZSTD", "BZIP2", "GZIP", "LZF", "PPMD", "SR3", "7ZIP"]:
				print("ERROR -> INVALID COMPRESSOR IN COMPRESSORS LIST (" + compressor + ")")
				self.__STATUS = -10
				return

		for configure in request["CONFIGURE"]:
			if configure not in request["COMPRESSORS"]:
				print("ERROR -> INVALID CONFIGURE IN CONFIGURE DICTIONARY (" + configure + ")")
				self.__STATUS = -11
				return

		for corpus in request["CORPUSES"]:
			if corpus not in ["CALGARY", "CANTERBURY", "PROTEIN", "SILESIA", "PIZZACHILLI", "DICTIONARIES"]:
				print("ERROR -> INVALID CORPUS IN CORPUSES LIST (" + corpus + ")")
				self.__STATUS = -12
				return

		for file in request["FILES"]:
			if not os.path.isfile(file):
				print("ERROR -> INVALID FILE IN FILES LIST (" + file + ")")
				self.__STATUS = -13
				return

		self.__ID = request["ID"]
		self.__ROUNDS = request["ROUNDS"]
		self.__TESTS = request["TESTS"]
		self.__COMPRESSORS = request["COMPRESSORS"]
		self.__CONFIGURE = request["CONFIGURE"]
		self.__CORPUSES = request["CORPUSES"]
		self.__FILES = request["FILES"]

		self.__EXECUTION = Compressors(self.__COMPRESSORS, self.__CONFIGURE, self.__ROUNDS)
		self.__STATUS = 1

	def requestResults(self, results, test):

		resultFile = open(test + "[" + self.__ID + "].csv", "w+")  

		corpusList = list(results.keys())
		corpusList.sort()
		for corpus in corpusList:
			resultFile.write("++++====++++ " + corpus + " ++++====++++\n")

			fileList = list(results[corpus].keys())
			fileList.sort()
			for file in fileList:
				resultFile.write(file + "\n")

				for family in results[corpus][file]:
					resultFile.write(family + "\n")

					resultFile.write("EXECUTABLE;")
					for compressor in results[corpus][file][family]:
						resultFile.write(compressor + ";")						
					resultFile.write("\nUNCOMPRESSED;")
					for compressor in results[corpus][file][family]:
						resultFile.write(str(results[corpus][file][family][compressor]["UNCOMPRESSED"]) + ";")						
					resultFile.write("\nCOMPRESSED;")
					for compressor in results[corpus][file][family]:
						resultFile.write(str(results[corpus][file][family][compressor]["COMPRESSED"]) + ";")						
					resultFile.write("\nBPC;")
					for compressor in results[corpus][file][family]:
						resultFile.write(str(results[corpus][file][family][compressor]["COMPRESSED"] * 8 / results[corpus][file][family][compressor]["UNCOMPRESSED"]).replace(".", ",") + ";")						
					resultFile.write("\nMEAN TIME;")
					for compressor in results[corpus][file][family]:
						resultFile.write(str(statistics.mean(results[corpus][file][family][compressor]["TIME"])).replace(".", ",") + ";")						
					resultFile.write("\nSTD DEV TIME;")
					if (len(results[corpus][file][family][compressor]["TIME"]) > 1):
						for compressor in results[corpus][file][family]:
							resultFile.write(str(statistics.stdev(results[corpus][file][family][compressor]["TIME"])).replace(".", ",") + ";")						
						resultFile.write("\nSPEED (KB/S);")
					for compressor in results[corpus][file][family]:
						resultFile.write(str((results[corpus][file][family][compressor]["UNCOMPRESSED"] / 1024) / statistics.mean(results[corpus][file][family][compressor]["TIME"])).replace(".", ",") + ";")						
					resultFile.write("\n")

				resultFile.write("\n")

			resultFile.write("\n")

		resultFile.close()

	def requestExecute(self):

		if self.__TESTS == "COMPRESSION" or self.__TESTS == "ALL":
			self.requestResults(self.__EXECUTION.compressRequest(self.__CORPUSES, self.__FILES), "Compression")
		if self.__TESTS == "DECOMPRESSION" or self.__TESTS == "ALL":
			self.requestResults(self.__EXECUTION.decompressRequest(self.__CORPUSES, self.__FILES), "Decompression")

	def getStatus(self):

		return self.__STATUS


if len(sys.argv) < 2: 
	print("USAGE: .exe request_file_path1 ... request_file_pathN")
	exit()

for index in range(1, len(sys.argv)):

	benchmarking = Requests(sys.argv[index])
	if benchmarking.getStatus() < 0:
		continue

	print("\nSTART --------> FILE: " + sys.argv[index] + "\n")
	benchmarking.requestExecute()
	print("\nEND ----------> FILE: " + sys.argv[index] + "\n")

#os.system('shutdown /p /f')
