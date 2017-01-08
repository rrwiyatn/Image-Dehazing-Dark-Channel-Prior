#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <string.h>
#include <sstream>
#include <iomanip>

using namespace std;

//Declare Constants
const int numFrames=1; //number of frames to be processed

//for batch/video processing (frame by frame)
const char* inputFolder="input/";//This should be the folder with all the images that we want to process
const char* outputFolder="output/";//This should be the folder where you want to store the output images
const int ImageWidth=1566;
const int ImageHeight=1056;
const double omega=0.95;

//Declare Arrays

//Input image: RGB channel
double ImageR[ImageWidth*ImageHeight];
double ImageG[ImageWidth*ImageHeight];
double ImageB[ImageWidth*ImageHeight];

//to store output after min filter
double minR[ImageWidth*ImageHeight];
double minG[ImageWidth*ImageHeight];
double minB[ImageWidth*ImageHeight];

//the dark channel of the image
double dark[ImageWidth*ImageHeight];

double transmissionR[ImageWidth*ImageHeight];
double transmissionG[ImageWidth*ImageHeight];
double transmissionB[ImageWidth*ImageHeight];
double transmission[ImageWidth*ImageHeight];

double radianceR[ImageWidth*ImageHeight];
double radianceG[ImageWidth*ImageHeight];
double radianceB[ImageWidth*ImageHeight];

//Declare Functions
void readImage(char* FileName);
void minFilter(double ImageR[], double ImageG[], double ImageB[],double (&minR)[ImageWidth*ImageHeight],double (&minG)[ImageWidth*ImageHeight],double (&minB)[ImageWidth*ImageHeight]);
void getDark(double minR[],double minG[],double minB[],double (&dark)[ImageWidth*ImageHeight]);
void medianFilter(double minRGB[],double (&median_minRGB)[ImageWidth*ImageHeight]);
void swap(double& num1, double& num2);
void linearSelectionSort(double (&val1)[25]);
void threshold(double src[],double (&dst)[ImageWidth*ImageHeight]);
double findMax(double mat[]);
double findMax(double mat[], int &index);
void transmit(double (&transmission)[ImageWidth*ImageHeight], double (&transmissionR)[ImageWidth*ImageHeight],double (&transmissionG)[ImageWidth*ImageHeight],double (&transmissionB)[ImageWidth*ImageHeight],double Ar, double Ag, double Ab, double ImageR[], double ImageG[], double ImageB[]);
void radiance(double transmission[], double Ar, double Ag, double Ab, double radianceR[],double radianceG[],double radianceB[], double ImageR[], double ImageG[], double ImageB[]);
void writeImage(double radianceR[],double radianceG[],double radianceB[],int i, string s);

int main ()
{
	for(int i=1;i<=numFrames;i++)//i should start from 1 to numFrames-1
	{
		//initialize everything to 0
		setZero(ImageR);
		setZero(ImageG);
		setZero(ImageB);
		setZero(minRGB);
		setZero(median_minRGB);
		setZero(CorrectedR);
		setZero(radianceR);
		setZero(radianceG);
		setZero(radianceB);
		
		char input[90]={};
		string s="";//for input
		string z="";//for output
		stringstream out;
		stringstream outz;
		out<<inputFolder<<i;
		outz<<outputFolder<<i;
		s=out.str();
		z=outz.str();
		char *FileName=new char[s.length()+1];
		strcpy(FileName, s.c_str());
		char *format=".ppm";
		strncpy(input,FileName,sizeof(input));
		strncat(input,format,sizeof(input));
		//readImage(input);//read images from input folder
		
		readImage("input.ppm");
		minFilter(ImageR,ImageG,ImageB,minR,minG,minB);//do min filter
		getDark(minR,minG,minB,dark);//get the dark channel
		
		int index;//to store index of the darkest pixel
		double A=findMax(dark, index);//global atmospheric light
		double Ar=minR[index];
		double Ag=minG[index];
		double Ab=minB[index];

		cout<<"A= "<<A<<endl;
		cout<<"i= "<<index<<endl;
		cout<<"Ar= "<<Ar<<endl;
		cout<<"Ag= "<<Ag<<endl;
		cout<<"Ab= "<<Ab<<endl;

		transmit(transmission,transmissionR,transmissionG,transmissionB,Ar,Ag,Ab,minR,minG,minB);//get transmission matrix
		radiance(transmission,Ar,Ag,Ab, radianceR, radianceG, radianceB, ImageR, ImageG, ImageB);//get radiance
		writeImage(radianceR,radianceG,radianceB,i,z);//output the image
	}
	return 0;
}

void readImage(char* FileName)
{
	int c;
	int c1;
	int c2;
	int c3;

	ifstream ImageIn (FileName, ios::binary);
	for(int i=0;i<40;i++)//limit is different for every image since using .ppm format
		c = ImageIn.get();

	for(int i=0;i<ImageHeight*ImageWidth;i++)
	{
		c1 = int(ImageIn.get());//R
		c2 = int(ImageIn.get());//G
		c3 = int(ImageIn.get());//B
		ImageR[i]=c1;
		ImageG[i]=c2;
		ImageB[i]=c3;
	}
	ImageIn.close();
	return;
}

void minFilter(double ImageR[], double ImageG[], double ImageB[],double (&minR)[ImageWidth*ImageHeight],double (&minG)[ImageWidth*ImageHeight],double (&minB)[ImageWidth*ImageHeight])
{
	//5x5 min filter
	double tempR[25];
	double tempG[25];
	double tempB[25];
	for(int i=0;i<ImageHeight;i++)
	{
		for(int j=0;j<ImageWidth;j++)
		{
			if(i<2||j<2||i>=ImageHeight-2||j>=ImageWidth-2)
			{
				minR[(ImageWidth*i)+j]=0;
				minG[(ImageWidth*i)+j]=0;
				minB[(ImageWidth*i)+j]=0;
			}
			else
			{
				//fill temp array
				for(int k=0;k<5;k++)
				{
					for(int l=0;l<5;l++)
					{
						tempR[k*5+l]=ImageR[(ImageWidth*(i-2))+(j-2)+(k*ImageWidth)+l];
						tempG[k*5+l]=ImageG[(ImageWidth*(i-2))+(j-2)+(k*ImageWidth)+l];
						tempB[k*5+l]=ImageB[(ImageWidth*(i-2))+(j-2)+(k*ImageWidth)+l];
					}
				}
				//fill sorted array
				linearSelectionSort(tempR);
				linearSelectionSort(tempG);
				linearSelectionSort(tempB);
				minR[(ImageWidth*i)+j]=tempR[0];
				minG[(ImageWidth*i)+j]=tempG[0];
				minB[(ImageWidth*i)+j]=tempB[0];
			}
		}
	}
	return;
}

void getDark(double minR[],double minG[],double minB[],double (&dark)[ImageWidth*ImageHeight])
{
	for(int i=0;i<ImageHeight*ImageWidth;i++)
	{
		if(minR[i]<=minG[i] && minR[i]<=minB[i])
			dark[i]=minR[i];
		else if(minG[i]<=minR[i] && minG[i]<=minB[i])
			dark[i]=minG[i];
		else if(minB[i]<=minR[i] && minB[i]<=minG[i])
			dark[i]=minB[i];
	}
	return;
}

void medianFilter(double minRGB[],double (&median_minRGB)[ImageWidth*ImageHeight])
{
	double temp[25];
	for(int i=2;i<ImageHeight-2;i++)
	{
		for(int j=2;j<ImageWidth-2;j++)
		{
			if(i<2||j<2||i>=ImageHeight-2||j>=ImageWidth-2)
			{
				median_minRGB[(ImageWidth*i)+j]=minRGB[(ImageWidth*i)+j];
			}
			else
			{
				//fill temp array
				for(int k=0;k<5;k++)
				{
					for(int l=0;l<5;l++)
					{
						temp[k*5+l]=minRGB[(ImageWidth*(i-2))+(j-2)+(k*ImageWidth)+l];
					}
				}
				//fill sorted array
				linearSelectionSort(temp);
				median_minRGB[(ImageWidth*i)+j]=temp[12];
			}
		}
	}
	return;
}

void swap(double& num1, double& num2)
{
   double temp = num2;
   num2 = num1;
   num1 = temp;
   return;
}

void linearSelectionSort(double (&val1)[25])
{
	   int i;
	   int j;
	   int min;
	   for(j = 0; j < 25; j++)
	   {
	      min = j;//assume the first number is the  minimum
	      for(i = j+1; i < 25; i++)
	      {
	         if (val1[i] <= val1[min])
	         {//if value at index i < value at middle index, index i become the minimum
	            min = i;
	         }
	      }
	      if (min != j)
	      {
	         swap(val1[min], val1[j]);//swap the value at index min and value at index j
	      }
	   }
	   return;
}

void threshold(double src[],double (&dst)[ImageWidth*ImageHeight])
{
	for(int i=0;i<ImageHeight*ImageWidth-1;i++)
	{
		if(src[i]>200)
		{
			dst[i]=0;
		}
		else if(src[i]<=200)
		{
			dst[i]=src[i];
		}
	}
	return;
}

double findMax(double mat[])
{
	double temp=0;
	for(int i=0;i<ImageHeight*ImageWidth-1;i++)
	{
		if(mat[i]>=temp)
			temp=mat[i];
	}
	return temp;
}

double findMax(double mat[], int &index)
{
	double temp=0;
	for(int i=0;i<ImageHeight*ImageWidth-1;i++)
	{
		if(mat[i]>=temp)
		{
			temp=mat[i];
			index=i;
		}
	}
	return temp;
}

void transmit(double (&transmission)[ImageWidth*ImageHeight], double (&transmissionR)[ImageWidth*ImageHeight],double (&transmissionG)[ImageWidth*ImageHeight],double (&transmissionB)[ImageWidth*ImageHeight],double Ar, double Ag, double Ab, double ImageR[], double ImageG[], double ImageB[])
{
	for(int i=0;i<ImageHeight*ImageWidth;i++)
	{
		transmissionR[i]=minR[i]/Ar;
		transmissionG[i]=minG[i]/Ag;
		transmissionB[i]=minB[i]/Ab;
		if(transmissionR[i]<=transmissionG[i] && transmissionR[i]<=transmissionB[i])
			transmission[i]=1-(omega*transmissionR[i]);
		else if(transmissionG[i]<=transmissionR[i] && transmissionG[i]<=transmissionB[i])
			transmission[i]=1-(omega*transmissionG[i]);
		else if(transmissionB[i]<=transmissionG[i] && transmissionB[i]<=transmissionR[i])
			transmission[i]=1-(omega*transmissionB[i]);
	}
	return;
}

void radiance(double transmission[], double Ar, double Ag, double Ab, double radianceR[],double radianceG[],double radianceB[], double ImageR[], double ImageG[], double ImageB[])
{
	for(int i=0;i<ImageHeight*ImageWidth;i++)
	{
		if(transmission[i]<0.1) transmission[i]=0.1;
		radianceR[i]=(((ImageR[i]-Ar)/(transmission[i])))+Ar;
		radianceG[i]=(((ImageG[i]-Ag)/(transmission[i]))+Ag);
		radianceB[i]=(((ImageB[i]-Ab)/(transmission[i]))+Ab);
		if(radianceR[i]>255) radianceR[i]=255;
		if(radianceG[i]>255) radianceG[i]=255;
		if(radianceB[i]>255) radianceB[i]=255;
	}
	return;
}

void setZero(double a[])
{
	for(int i=0;i<ImageHeight*ImageWidth;i++)
		a[i]=0;
	return;
}

void writeImage(double radianceR[],double radianceG[],double radianceB[],int i,string s)
{
	char *FileOut=new char[s.length()+1];
	strcpy(FileOut, s.c_str());
	char *format=".ppm";
	char output[70];
	strncpy(output,FileOut,sizeof(output));
	strncat(output,format,sizeof(output));
	//ofstream ImageOut (output, ios::binary);//only if you want to save the output to the output folder
	ofstream ImageOut ("output.ppm", ios::binary);
	ImageOut << "P6" << endl << ImageWidth << " " << ImageHeight << " 255" << endl;
	char numR;
	char numG;
	char numB;

	for(int i=0;i<ImageHeight*ImageWidth;i++)
	{
		numR=int(radianceR[i]);
		numG=int(radianceG[i]);
		numB=int(radianceB[i]);

		ImageOut << numR;
		ImageOut << numG;
		ImageOut << numB;
	}
	ImageOut.close();
	return;
}
