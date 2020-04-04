import numpy as np
import matplotlib.pyplot as plt
import math
import ROOT
import sys
from os import walk
import os
sys.setrecursionlimit(10000)



#------------------------------------------------------------------------
def IntensityPlotGraph(Data):
    gr=ROOT.TGraph(len(x),x,y)
    Canvas=TCanvas(Name,CTitle,200,10,700,500)
    Canvas.SetFillColor(42)
    gr.SetLineColor(2)
    gr.SetLineWidth(4)
    gr.SetMarkerStyle(21)
    gr.GetXaxis().SetTitle(XTitle)
    gr.GetYaxis().SetTitle(YTitle)
    gr.Draw(Mode)
    f = ROOT.TFile(RootFile, 'UPDATE')
    Canvas.Write(Name)
    f.Close()

def initTGraphs(Names,NotWanted,Graphs):
    for name in Names:
        info=name.split("_")
        if info[3] in NotWanted:
            Graphs[name]=ROOT.TGraph(name,name)


# ------------------------------------------------------------------------
def initHistosAll(Names,nbin,Startbin,Endbin,NotWanted,Histograms):
    for name in Names:
         info=name.split("_")
         #if info[3] not in NotWanted:
         Histograms[name]=ROOT.TH1F(name, name, nbin, Startbin, Endbin)



# ------------------------------------------------------------------------
def plotHistsh(h,name,Title,RootFile):
    ROOT.gStyle.SetOptFit(1)

    # Draw
    c1 = ROOT.TCanvas(name, Title, 1000, 1000)
    h.SetMarkerStyle(8)
    h.SetMarkerSize(1)
    h.Draw("pe1")

    f = ROOT.TFile(RootFile, 'UPDATE')
    c1.Write()
    f.Close()

def CalandFill(All,CenterX,CenterY,Histograms,AllVariable,NotWanted,RootFile):

    #Calculations
    for Data in All:
        print (Data)
        #True Variables
        tx = float(Data[4])
        ty = float(Data[5])
        TTheta=math.tan(ty/tx)*(180/3.14)
        TR=math.sqrt((tx*tx)+(ty*ty))

        #Estimated Variables
        ex = float(Data[7]) + CenterX
        ey = float(Data[8]) + CenterY
        ETheta=math.tan(ey/ex)*(180/3.14)
        ER=math.sqrt((ex*ex)+(ey*ey))

        name = Data[1] + "_" + Data[2] + "_" + Data[3] + "_" + AllVariable[0]
        Histograms[name].Fill(ex)
        name = Data[1] + "_" + Data[2] + "_" + Data[3] + "_" + AllVariable[1]
        Histograms[name].Fill(ey)
        name = Data[1] + "_" + Data[2] + "_" + Data[3] + "_" + AllVariable[2]
        Histograms[name].Fill(ER)
        name = Data[1] + "_" + Data[2] + "_" + Data[3] + "_" + AllVariable[3]
        Histograms[name].Fill(ETheta)


        # Percentage Calculation
        """for name,x,y,theta in Graphs:
            name.append(name)
        """
        if ty==0:
            ty=1
        if tx==0:
            tx=1
        if TTheta==0:
            TTheta=1
        ErrorEstimateX=abs((ex-tx)/tx)*100
        ErrorEstimateY=abs((ey-ty)/ty)*100
        ErrorEstimateTheta=abs((ETheta-TTheta)/TTheta)*100
        ErrorEstimateR=abs((ER-TR)/TR)*100
        name = Data[1] + "_" + Data[2] + "_" + Data[3] + "_" + NotWanted[0]
        Histograms[name].Fill(ErrorEstimateX)
        name = Data[1] + "_" + Data[2] + "_" + Data[3] + "_" + NotWanted[1]
        Histograms[name].Fill(ErrorEstimateY)
        name = Data[1] + "_" + Data[2] + "_" + Data[3] + "_" + NotWanted[2]
        Histograms[name].Fill(ErrorEstimateR)
        name = Data[1] + "_" + Data[2] + "_" + Data[3] + "_" + NotWanted[3]
        Histograms[name].Fill(ErrorEstimateTheta)


# ------------------------------------------------------------------------
def CreateNames(All,Result,Variables):
    Names = []
    Intesity = []
    Ths = []

    for Row in All:
        if(Row[1] not in Names):
            Names.append(Row[1])
        if(Row[2] not in Intesity):
            Intesity.append(Row[2])
        if (Row[3] not in Ths):
            Ths.append(Row[3])


    for v in Variables:
        for n in Names:
            for th in Ths:
                for i in Intesity:
                    Result.append(n + "_" + i  +"_"+ th +"_" + v)


def DrawCircle(Data,CenterX,CenterY,diskradius):
    # Draw the circle here
    tx=float(Data[4])-CenterX
    ty=float(Data[5])-CenterY
    ex=float(Data[7])
    ey=float(Data[8])

    x=[tx,ex]
    y=[ty,ey]
    File=["TrueP","RecoP"]
    markers=["p","o"]
    Name=Data[1]
    Title=Data[0]+"_TrueVsReco"+"_"+Data[1]+"_"+Data[2]+"_"+Data[3]


    theta = np.linspace(0, 2 * np.pi, 100)
    x1 = diskradius * np.cos(theta)
    x2 = diskradius * np.sin(theta)

    fig, ax = plt.subplots()

    ax.plot(x1, x2)
    ax.set_aspect(1)

    plt.xlim(-diskradius - 1, diskradius + 1)
    plt.ylim(-diskradius - 1, diskradius + 1)

    plt.grid(linestyle='--')

    plt.title(Title, fontsize=12)
    for xs, ys, Fs,t in zip(x, y, File,markers):
        label = Fs

        plt.annotate(label,  # this is the text
                     (xs, ys),  # this is the point to label
                     textcoords="offset points",  # how to position the text
                     xytext=(0, 10),  # distance from text to points (x,y)
                     ha='center')  # horizontal alignment can be left, right or center
        ax.scatter([xs], [ys],marker=t,s=40*3)
    plt.savefig( "pictures/2min/" + Title + ".png")
    print (Title + " is created")

def ReadFiles(FilePath,FileExtension):
    Files={}
    QualDic={}
    QualFileNames=[]
    FileExtension=FileExtension.replace('.','')

    for (dirpath,dirnames,filenames) in walk(FilePath):
            Files[dirpath]=filenames
    for key in Files:
        for file in Files[key]:
            extension=file.split(".")
            if (extension[1]==FileExtension):
                QualFileNames.append(extension[0])
        if(len(QualFileNames)!=0):
            QualDic[key]=QualFileNames
    return QualDic

def NumofSIPMsandSpace (TablePath):
    Parts   = TablePath.split("/")
    SIPM    = Parts[2].replace("_sipm","")
    Space   = Parts[1].split("mm")
    Values  = ["t","t"]
    if(len(Space)>1):
        VSpace=float(Space[0])/10

    Values[0] = SIPM
    Values[1] = str(VSpace)
    return Values

def Analyze(FilePath,OutPath,Analyze,Reconst,pixelPath,TablePath,Radius,sipmGainPath,SaveFile,Cut,Th,NofCoinc,TrueFile):
    OutPath     = OutPath + "/"
    Parts       = FilePath.strip("/").split("/")
    LastItem    = len(Parts)-1
    Result      = Parts[LastItem].find('root')

    SaveFile    = str(SaveFile)
    Cut         = str(Cut)
    Th          = str(Th)
    NofCoinc    = str(NofCoinc)
    Values      = NumofSIPMsandSpace(TablePath)
    NSIPM       = Values[0]
    Space       = Values[1]
    CombinedValues=NSIPM + "_" + Space + "_" + str(Radius)


    if (Result==-1):
        FilePath    = FilePath + "/"
        Files=ReadFiles(FilePath,"root") # Collects the files to analyze with root extension
        TrueFile    = FilePath + TrueFile
        if (len(Files)==0):
            print ("There are no .root files available to analyze")
            return


        for Path in Files :
            print ("Total of  " + str(len(Files[Path])) + " Root Files Will be Analyzed!")
            print Path
            for file in Files[Path]:

                if(Analyze):
                    theCommand = 'root -l -q -b \'doAnalyze.C(\"'+Path+'\",\"' \
                                 +file+'\",\"' \
                                 +OutPath+'\",\"' \
                                 +sipmGainPath+'\",\"' \
                                 +SaveFile+'\",\"' \
                                 +Cut+'\", \"' \
                                 +CombinedValues+'\", \"' \
                                 +Th+'\", \"' \
                                 +NofCoinc+'\")\''

                    os.system(theCommand)

                if(Reconst):
                    theCommand = 'root -l -b -q \'doRecoToFile.c(\"'+pixelPath+'\", \"'+TablePath+'\", \"'+OutPath+'\", \"'+file+'\", \"'+TrueFile+'\", \"'+CombinedValues+'\")\''
                    os.system(theCommand)

    else:
        FileName=Parts[LastItem].replace('.root','')
        AnaPath=FilePath.replace(Parts[LastItem],'')
        TrueFile    = AnaPath + TrueFile

        if(Analyze):
            theCommand = 'root -l -q -b \'doAnalyze.C(\"'+AnaPath+'\",\"' \
                         +FileName+'\",\"' \
                         +OutPath+'\",\"' \
                         +sipmGainPath+'\",\"' \
                         +SaveFile+'\",\"' \
                         +Cut+'\", \"' \
                         +CombinedValues+'\", \"' \
                         +Th+'\", \"' \
                         +NofCoinc+'\")\''
            os.system(theCommand)

        if(Reconst):
            theCommand = 'root -l -b -q \'doRecoToFile.c(\"'+pixelPath+'\",\"'+TablePath+'\", \"'+OutPath+'\", \"'+FileName+'\", \"'+TrueFile+'\", \"'+CombinedValues+'\")\''
            os.system(theCommand)
