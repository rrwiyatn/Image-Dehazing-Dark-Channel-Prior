#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <string.h>
#include <sstream>
#include <iomanip>

using namespace std;
//
//Declare Functions
void readImage(char* FileName);
void minMatrix(double ImageR[], double ImageG[], double ImageB[],double minRGB[]);
void medianFilter(double minRGB[],double median_minRGB[]);
void swap(double& num1, double& num2);
void linearSelectionSort(double (&val1)[25]);
double findMax(double median_minRGB[]);
void transmit(double transmissionR[],double transmissionG[],double transmissionB[],double ImageR[], double ImageG[], double ImageB[],double a);
void radiance(double transmissionR[],double transmissionG[],double transmissionB[], double a, double radianceR[],double radianceG[],double radianceB[], double tmaxR, double tmaxG, double tmaxB);
void setZero(double a[]);
void writeImage(double radianceR[],double radianceG[],double radianceB[],int i, string s);
//Declare Constants
const int numFrames=1; //number of frames to be processed
const char* inputFolder="input/";//This should be the folder with all the images that we want to process
const char* outputFolder="output/";//This should be the folder where you want to store the output images
const int ImageWidth=420;
const int ImageHeight=286;
const double omega=0.95;
//
//Declare Arrays
double ImageR[ImageWidth*ImageHeight];
double ImageG[ImageWidth*ImageHeight];
double ImageB[ImageWidth*ImageHeight];

double minRGB[ImageWidth*ImageHeight];
double median_minRGB[ImageWidth*ImageHeight];

double transmissionR[ImageWidth*ImageHeight];
double transmissionG[ImageWidth*ImageHeight];
double transmissionB[ImageWidth*ImageHeight];

double radianceR[ImageWidth*ImageHeight];
double radianceG[ImageWidth*ImageHeight];
double radianceB[ImageWidth*ImageHeight];

int main ()
{
	for(int i=1;i<=numFrames;i++)//i should start from 1 to numFrames-1
	{
		setZero(ImageR);
		setZero(ImageG);
		setZero(ImageB);
		setZero(minRGB);
		setZero(median_minRGB);
		setZero(transmissionR);
		setZero(transmissionG);
		setZero(transmissionB);
		setZero(radianceR);
		setZero(radianceG);
		setZero(radianceB);
		char input[90]={};
		string s="";//for input
		string z="";//for output
		stringstream out;//
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
		//cout<<"Input:"<<input<<endl;

		//readImage(input);
		readImage("foggy-road2.ppm");
		minMatrix(ImageR, ImageG, ImageB, minRGB);
		medianFilter(minRGB,median_minRGB);
		double a=findMax(median_minRGB);
		transmit(transmissionR,transmissionG,transmissionB,ImageR,ImageG,ImageB,a);
		double tmaxR=findMax(transmissionR);
		double tmaxG=findMax(transmissionG);
		double tmaxB=findMax(transmissionB);
		if(tmaxR<0.1)
			tmaxR=0.1;
		if(tmaxG<0.1)
			tmaxG=0.1;
		if(tmaxB<0.1)
			tmaxB=0.1;
		//cout<<"A= "<<a<<endl;
		//cout<<"tmaxR= "<<tmaxR<<endl;
		//cout<<"tmaxG= "<<tmaxG<<endl;
		//cout<<"tmaxB= "<<tmaxB<<endl;
		radiance(transmissionR,transmissionG,transmissionB,a,radianceR,radianceG,radianceB,tmaxR,tmaxG,tmaxB);
		writeImage(radianceR,radianceG,radianceB,i,z);
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
	for(int i=0;i<38;i++)
		c = ImageIn.get();

	for(int i=0;i<ImageHeight*ImageWidth;i++)
	{
		//for color image
		c1 = int(ImageIn.get());//R
		c2 = int(ImageIn.get());//G
		c3 = int(ImageIn.get());//B

		//c1 = int((ImageIn.get())*0.299);//R
		//c2 = int((ImageIn.get())*0.587);//G
		//c3 = int((ImageIn.get())*0.114);//B
		//c = c1+c2+c3;//=grayscale weighted average


		//for grayscale image
		/*
		c1 = int(ImageIn.get());//R
		c2 = int(ImageIn.get());//G
		c3 = int(ImageIn.get());//B
		c = (c1+c2+c3)/3;
		*/

		ImageR[i]=c1;
		ImageG[i]=c2;
		ImageB[i]=c3;

	}
	ImageIn.close();
	return;
}

void minMatrix(double ImageR[], double ImageG[], double ImageB[],double minRGB[])
{
	for(int i=0;i<ImageHeight*ImageWidth;i++)
	{
		if(ImageR[i]<ImageG[i])
		{
			if(ImageR[i]<ImageB[i])
			{
				minRGB[i]=ImageR[i];
			}
			else
			{
				minRGB[i]=ImageB[i];
			}
		}
		else
		{
			if(ImageG[i]<ImageB[i])
			{
				minRGB[i]=ImageG[i];
			}
			else
			{
				minRGB[i]=ImageB[i];
			}
		}
	}
}

void medianFilter(double minRGB[],double median_minRGB[])
{
	//5x5 median filter
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
}

void swap(double& num1, double& num2)
{
   double temp = num2;
   num2 = num1;
   num1 = temp;
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
}

double findMax(double median_minRGB[])
{
	double temp=0;
	for(int i=0;i<ImageHeight*ImageWidth-1;i++)
	{
		if(median_minRGB[i]>temp)
		{
			temp=median_minRGB[i];
		}
	}
	return temp;
}

void transmit(double transmissionR[],double transmissionG[],double transmissionB[],double ImageR[], double ImageG[], double ImageB[],double a)
{
	for(int i=0;i<ImageHeight*ImageWidth;i++)
	{
		transmissionR[i]=(1-(omega*ImageR[i]/a));//*255.0;
		transmissionG[i]=(1-(omega*ImageG[i]/a));//*255.0;
		transmissionB[i]=(1-(omega*ImageB[i]/a));//*255.0;
	}
}

void radiance(double transmissionR[],double transmissionG[],double transmissionB[], double a, double radianceR[],double radianceG[],double radianceB[],double tmaxR, double tmaxG, double tmaxB)
{
	for(int i=0;i<ImageHeight*ImageWidth;i++)
	{
		radianceR[i]=((ImageR[i]-a)/tmaxR)+a;
		if(radianceR[i]>255) radianceR[i]=255;
		radianceG[i]=((ImageG[i]-a)/tmaxG)+a;
		if(radianceG[i]>255) radianceG[i]=255;
		radianceB[i]=((ImageB[i]-a)/tmaxB)+a;
		if(radianceB[i]>255) radianceB[i]=255;

		/*
		radianceR[i]=((transmissionR[i]-a)/tmaxR)+a;
		if(radianceR[i]>255) radianceR[i]=255;
		radianceG[i]=((transmissionG[i]-a)/tmaxG)+a;
		if(radianceG[i]>255) radianceG[i]=255;
		radianceB[i]=((transmissionB[i]-a)/tmaxB)+a;
		if(radianceB[i]>255) radianceB[i]=255;
		*/
	}
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
	//ofstream ImageOut (output, ios::binary);
	ofstream ImageOut ("foggy_road2_out.ppm", ios::binary);
	ImageOut << "P6" << endl << ImageWidth << " " << ImageHeight << " 255" << endl;
	char numR;
	char numG;
	char numB;
//
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


