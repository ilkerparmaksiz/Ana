import glob, os
import functions as fun
import ROOT

from collections import Counter
import itertools

#os.chdir(".")
Path="/home/ilker/Desktop/Analysis/Second_Data/"
os.chdir(Path)
RootFile = "Analyzed2min.root"
diskradius = 15
CenterX = -3
CenterY = 8

#Graphs=zip(name,ex,ey,etheta,er)
Histograms={}
Created_Names=[]
NotWanted=["ErrX","ErrY","ErrR","ErrTh"]
AllVariables = ["X", "Y", "R", "Th", "ErrX", "ErrY", "ErrR", "ErrTh"]
Pos=2
Threshold=2
Intesity=5
Variables=2
TotalHisto=Pos*Threshold*Intesity*Variables


All = [[]]
CountData=[]
Title = []

if __name__ == "__main__":
    os.chdir(".")
    FileCount=0
    Files=[]
    print "Possible Files:"
    print "Index   File Name"
    for file in glob.glob("*.out"):
        print(str(FileCount) +"--> " + file)
        Files.append(file)
        FileCount+=1
    Index = int(raw_input("Enter the Index: "))
    Name = Files[Index]

    f = open(Name, "r")

    count=0

    # get the true positions
    del All[:]
    del CountData[:]

    for line in f:
        ReadLine = line.strip("\n").split(" ")
        if count==0:
            Title.extend(ReadLine)
        else:
            if len(ReadLine)>5:
                All.append(ReadLine)
            else:
                continue

        count+=1

    f.close()

    # Uncomment this for getting a picture for True Vs Reco position for each file
    """for i in All:
        fun.DrawCircle(i,CenterX,CenterY,diskradius)
    """
    f = ROOT.TFile(RootFile, 'UPDATE')
    fun.CreateNames(All,Created_Names,AllVariables)
    fun.initHistosAll(Created_Names,200,-200,200,NotWanted,Histograms)




    #Do the Calculations and fill it
    fun.CalandFill(All,CenterX,CenterY,Histograms,AllVariables,NotWanted,RootFile)
        #fun.DrawCircle(Row,CenterX,CenterY,diskradius)
    #fun.plotHistsh(h,Name,"ErrorEstimateX",RootFile)
    f.Write()
    f.Close()


