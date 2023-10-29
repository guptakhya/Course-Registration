{\rtf1\ansi\ansicpg1252\cocoartf2639
\cocoatextscaling0\cocoaplatform0{\fonttbl\f0\fswiss\fcharset0 Helvetica;}
{\colortbl;\red255\green255\blue255;}
{\*\expandedcolortbl;;}
\paperw11900\paperh16840\margl1440\margr1440\vieww14200\viewh8760\viewkind0
\pard\tx566\tx1133\tx1700\tx2267\tx2834\tx3401\tx3968\tx4535\tx5102\tx5669\tx6236\tx6803\pardirnatural\partightenfactor0

\f0\fs24 \cf0 # Makefile for compiling the Servers and Client C++ codes for the USC Web Registration System.\
# To create this Makefile, https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html was referred\
\
# compiler command for C++\
CPP = g++\
\
# Compiler flag for storing output in the following filename\
OFLAG  = -o\
\
# default case - compile all the files\
default: all\
\
# compiles all the files (servers and client) in the provided order\
all:  serverM.o serverC.o serverEE.o serverCS.o client.o\
\
# Compiles the serverM.cpp code\
\pard\tx566\tx1133\tx1700\tx2267\tx2834\tx3401\tx3968\tx4535\tx5102\tx5669\tx6236\tx6803\pardirnatural\partightenfactor0
\cf0 serverM.o:  serverM.cpp \
\pard\tx566\tx1133\tx1700\tx2267\tx2834\tx3401\tx3968\tx4535\tx5102\tx5669\tx6236\tx6803\pardirnatural\partightenfactor0
\cf0 	$(CPP) $(OFLAG) serverM serverM.cpp\
\
\pard\tx566\tx1133\tx1700\tx2267\tx2834\tx3401\tx3968\tx4535\tx5102\tx5669\tx6236\tx6803\pardirnatural\partightenfactor0
\cf0 # Compile and execute serverM that was created in the previous step\
serverM:  serverM.o\
	./serverM\
\pard\tx566\tx1133\tx1700\tx2267\tx2834\tx3401\tx3968\tx4535\tx5102\tx5669\tx6236\tx6803\pardirnatural\partightenfactor0
\cf0 \
\pard\tx566\tx1133\tx1700\tx2267\tx2834\tx3401\tx3968\tx4535\tx5102\tx5669\tx6236\tx6803\pardirnatural\partightenfactor0
\cf0 # Compiles the serverC.cpp code\
serverC.o:  serverC.cpp \
	$(CPP) $(OFLAG) serverC serverC.cpp\
\
# Compile and execute serverC that was created in the previous step\
serverC:  serverC.o\
	./serverC\
\
# Compiles the serverEE.cpp code\
serverEE.o:  serverEE.cpp \
	$(CPP) $(OFLAG) serverEE serverEE.cpp\
\
# Compile and execute serverM that was created in the previous step\
serverEE:  serverEE.o\
	./serverEE\
\
# Compiles the serverCS.cpp code\
serverCS.o:  serverCS.cpp \
	$(CPP) $(OFLAG) serverCS serverCS.cpp\
\
# Compile and execute serverCS that was created in the previous step\
serverCS:  serverCS.o\
	./serverCS\
\
# Compiles the client.cpp code\
client.o:  client.cpp \
	$(CPP) $(OFLAG) client client.cpp\
\
# Compile and execute client that was created in the previous step\
client:  client.o\
	./client\
\pard\tx566\tx1133\tx1700\tx2267\tx2834\tx3401\tx3968\tx4535\tx5102\tx5669\tx6236\tx6803\pardirnatural\partightenfactor0
\cf0 \
# Removes the executable files and the backup (*~) files created to start from scratch\
clean: \
	$(RM) serverM serverC serverEE serverCS client *~}
