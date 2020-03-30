import numpy as np
import matplotlib.pyplot as plt
import argparse
import math
import glob, os


#os.chdir(".")
Path="/home/ilker/Desktop/Analysis/Second_Data/"
os.chdir(Path)
FileCount=0
Files=[]
print "Possible Files:"
print "Index   File Name"
for file in glob.glob("*.txt"):
    print(str(FileCount) +"--> " + file)
    Files.append(file)
    FileCount+=1

Index=int(raw_input("Enter the Index: "))
Name=Files[Index]

diskradius=16

CenterX=-3
CenterY=8

f=open(Name,"r")
File=[]
Title=[]
x=[]
y=[]
count=0

# get the true positions
for line in f:
    ReadLine = line.strip("\n").split("\t")
    if(count==0):
        Title.extend(ReadLine)
    else:
        if len(ReadLine)>2:
            File.append(ReadLine[0])
            x.append(int(ReadLine[1])-CenterX)
            y.append(int(ReadLine[2])-CenterY)
        else:
            continue

    count+=1
f.close()

# Draw the circle here
r =diskradius
theta = np.linspace(0, 2*np.pi, 100)
x1 = r*np.cos(theta)
x2 = r*np.sin(theta)

fig, ax = plt.subplots(1)

ax.plot(x1, x2)
ax.set_aspect(1)

plt.xlim(-diskradius-1,diskradius+1)
plt.ylim(-diskradius-1,diskradius+1)

#plt.grid(linestyle='-')
Name=Name.strip(".txt")
plt.title('Position of the Points has taken on the disk', fontsize=12)


for xs,ys,Fs in zip(x,y,File):
    label=Fs
    plt.annotate(label,  # this is the text
                 (xs, ys),  # this is the point to label
                 textcoords="offset points",  # how to position the text
                 xytext=(0, 10),  # distance from text to points (x,y)
                 ha='center')  # horizontal alignment can be left, right or center
plt.scatter(x, y, marker='o',s=30*4)
plt.savefig(Path + "pictures/Position_"+ Name +".png")

plt.show()




