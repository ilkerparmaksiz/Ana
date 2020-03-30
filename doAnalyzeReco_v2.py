import argparse
import include.analyzer as analyzer
import include.reconstructor as reconstructor
import ROOT
import glob, os


# load list of root files 
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Analyze root files")
    parser.add_argument("-i", "--input", type=str, help="Input list of root files", required=True)
    parser.add_argument("-o", "--output", type=str, help="Output path for histograms and reconstruction results", required=True)
    parser.add_argument("-c", "--config", type=str, help="Path to reconstruction configuration", required=True)
    parser.add_argument("-t", "--trigsize", type=str, help="Trigger Size", required=True)
    parser.add_argument("-th", "--threshold", type=str, help="Threshold to filter the noise", required=True)
    parser.add_argument("-method", "--method", type=str, help="0 is the old way 1 is the new way", required=True)

    args = parser.parse_args()

    # create output file
    f = ROOT.TFile.Open(args.output, 'RECREATE')
    f.Close()

    # initalize
    DataFileName=raw_input("Enter the File Name to save the data  ")
    ana  = analyzer.analyzer(args.output)
    reco = reconstructor.reconstructor(args.config, args.output)
    TrigSize=int(args.trigsize)
    Th=int(args.threshold)
    Method=int(args.method)

    os.chdir(".")
    FileCount = 0
    Files = []
    # Get current working directory
    #Path=os.path.abspath(os.getcwd())
    #Path = "/home/ilker/Desktop/Analysis/1st_Data/"
    Path="/home/asaadi/Desktop/Analysis/Third_Data/"
    #os.chdir(Path)
    print "Choose file has true positions: "
    print "Index   File Name "

    for file in glob.glob(Path+"*.txt"):
        print(str(FileCount) + "--> " + file)
        Files.append(file)
        FileCount += 1

    Index = int(raw_input("Enter the Index: "))
    TrueFileName = Files[Index]

    f = open(TrueFileName, "r")
    count = 0
    ftitle = []
    file = []
    truex = []
    truey = []

    for line in f:

        readline = line.strip("\n").split("\t")
        if (count == 0):
            ftitle.extend(readline)
        else:
            if(len(readline)>2):
                file.append(readline[0])
                truex.append(readline[1])
                truey.append(readline[2])
            else:
                continue
        count+=1

    Trueinfo=zip(file,truex,truey)
    f.close()
    DataFileName=Path+DataFileName + ".out"
    File = open(DataFileName, "a+")
    File.write("EventID FileName Intensity CAENThreshold TrueX Truey TotalPE EstimateX EstimateY EstimateLY\n")
    File.close()
    # if just a single root file
    if args.input[-4:] == 'root':
        print ' '
        print 'Analyzing the file', args.input
        ana.analyze(theFile, event,TrigSize,reco,Th,Method,Trueinfo,DataFileName)
    else:
        # loop over the files and run the analyzer
        with open(args.input) as theFileList:

            event = 1

            for theFile in theFileList:
                theFile = theFile.replace('\r', '').replace('\n', '')

                print 'Analyzing the file', theFile
                ana.analyze(theFile, event,TrigSize,reco,Th,Method,Trueinfo,DataFileName)
