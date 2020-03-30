import multiprocessing as mt
import os
import time


def file1(theFile,output,theName):
    start=time.time()
    print ("Computing " + theFile + " Started ")
    theCommand = 'root -l -b \'doAnalyze.C(\"'+theFile+'\", \"'+output+'\", \"'+theName+'\")\''
    os.system(theCommand)
    endTime=time.time()
    print("Total Time : " + str(endTime-start))
def test(theFile):
    print ("Thread started!")
    start=time.time()
    print ("Computing " + theFile + " Started "  + str(start) )
    for i in range(1,10):
        time.sleep(1)
        print(i)
    endTime=time.time()
    print("Total Time : " + str(endTime-start))



counter=0
with open("filelist1.txt") as theFileList:
    for theFile in theFileList:
        theFile = theFile.replace('\r', '').replace('\n', '')
        theName = theFile.rsplit('/', 1)[1].rsplit('.', 1)[0]
        name="home/ilker/Desktop/output_" + str(counter) + ".root"
        mythread=mt.Process(target=file1,args=(theFile,name,theName)) # ...Instantiate a thread and pass a unique ID to it
        mythread.start()
        counter=counter+1
        time.sleep(5)

