//Will support both ppm and pgm format files, in both 1 byte and 2 byte format.

#include <stdio.h>
#include <stdlib.h>


#define PGM 0
#define PPM 1

#define DEBUG 1

//TODO: Use unions instead of multiple struct options?
typedef struct
{
	int type;
	int width;
	int height;
	int max_val;
} img_header;

typedef struct
{
	//char bw;
	short bw;
	//TODO: change this? need extra space for values over 255 during transformation
} bw_pixel_1;

typedef struct
{
	short bw;
} bw_pixel_2;

typedef struct
{
	char red;
	char green;
	char blue;
} color_pixel_1;

typedef struct
{
	short red;
	short green;
	short blue;
} color_pixel_2;

typedef struct
{
	img_header header;
	//change this to use a union

	bw_pixel_1** pixels_bw;
	color_pixel_1 ** pixels_color;
} img_data;

int sobelFilter(img_data* inImg, img_data* outImg);
int processHeader(FILE* inpFile, img_data* inImg);
int processData (FILE* inpFile, img_data* inImg);
void outputImage(FILE* outFile, img_data* inImg);
char skipSpaces(FILE* inpFile);
void skipComment(FILE* inpFile);

int main(int argc, char* argv[])
{
	FILE* inpFile;
	FILE* outFile;
	img_data inImg;
	img_data outImg;
	int retVal;
	int row;
	
	//TODO: change the way that input/output is handled
	//take optional command line args, output by default to inputName_sobel
	
	//inpFile = fopen("images/lena.pgm", "r");
	//inpFile = fopen("images/mona_lisa.pgm", "r");
	inpFile = fopen("images/lion.pgm", "r");
	//inpFile = fopen("images/parrot_original.ppm", "r");
	if(inpFile == NULL)
	{
		printf("Error opening input file.\n");
		return 1;
	} 
	
	outFile = fopen("sobel_lion.pgm", "w");
	//outFile = fopen("sobel_parrot.ppm", "w");
	if(outFile == NULL)
	{
		printf("Error opening output file.\n");
		return 1;
	} 
	
	retVal = processHeader(inpFile, &inImg);
	if(retVal != 0)
	{
		return 1;
	}
	
	//TODO: Put the following stuff in a function call
	//Create the array of pixels for the input file
	if(inImg.header.type == PPM)
	{
		inImg.pixels_color = (color_pixel_1**)malloc(sizeof(color_pixel_1*) * inImg.header.height);
		for(row=0; row < inImg.header.height; row++)
		{
			//create each row
			inImg.pixels_color[row] = (color_pixel_1*)malloc(sizeof(color_pixel_1) * inImg.header.width);
		}
	}
	else
	{
		inImg.pixels_bw = (bw_pixel_1**)malloc(sizeof(bw_pixel_1*) * inImg.header.height);
		for(row=0; row < inImg.header.height; row++)
		{
			//create each row
			inImg.pixels_bw[row] = (bw_pixel_1*)malloc(sizeof(bw_pixel_1) * inImg.header.width);
		}
	}
	
	//TODO: what to do about color images? Convert to grayscale and then output?
	
	//Create the array of pixels for the output file
	//For now assume output file is always black and white.
	outImg.header.type = PGM;
	outImg.header.width = inImg.header.width;
	outImg.header.height = inImg.header.height;
	outImg.header.max_val = inImg.header.max_val;
	
	outImg.pixels_bw = (bw_pixel_1**)malloc(sizeof(bw_pixel_1*) * outImg.header.height);
	for(row=0; row < outImg.header.height; row++)
	{
		//create each row
		outImg.pixels_bw[row] = (bw_pixel_1*)malloc(sizeof(bw_pixel_1) * outImg.header.width);
	}
	
	retVal = processData(inpFile, &inImg);
	if(retVal != 0)
	{
		return 1;
	}
	
	if(DEBUG)
	{
		//to test, print out same image
		//outputImage(outFile, &inImg);
	}
	
	
	sobelFilter(&inImg, &outImg);
	
	//output image
	outputImage(outFile, &outImg);
	
	
	//TODO: free memory
	return 0;
}


int sobelFilter(img_data* inImg, img_data* outImg)
{
	int row;
	int col;
	int height;
	int width;
	int sum1;
	int sum2;
	int curMax = 0;
	
	if(DEBUG)
	{
		printf("Performing the Sobel filter.\n");
	}
	
	height = inImg->header.height;
	width = inImg->header.width;
	
	//Loop through the whole image
	for(row=0; row < height; row++)
	{
		for(col=0; col < width; col++)
		{
			//For each pixel in the image, do the following
			
			//If it's an edge.
			if((row==0) || (row==height-1) || (col==0) || (col==width-1))
			{
				//TODO: add support for color images
				//Just convert each pixel to grayscale on the fly
				outImg->pixels_bw[row][col].bw = 0;	//black edge
			}
			else
			{
				//TODO: change - to inverter, 2's to shifts, etc.
				sum1 = (-1 * inImg->pixels_bw[row-1][col-1].bw) + 
					   (1 * inImg->pixels_bw[row-1][col+1].bw) +
					   (-2 * inImg->pixels_bw[row][col-1].bw) + 
					   (2 * inImg->pixels_bw[row][col+1].bw) + 
					   (-1 * inImg->pixels_bw[row+1][col-1].bw) + 
					   (1 * inImg->pixels_bw[row+1][col+1].bw);
			
				sum2 = (-1 * inImg->pixels_bw[row-1][col-1].bw) + 
					   (-2 * inImg->pixels_bw[row-1][col].bw) +
					   (-1 * inImg->pixels_bw[row-1][col+1].bw) + 
					   (1 * inImg->pixels_bw[row+1][col-1].bw) + 
					   (2 * inImg->pixels_bw[row+1][col].bw) +
					   (1 * inImg->pixels_bw[row+1][col+1].bw);
				
				if(DEBUG)
				{
					//printf("Sum1: %d\n", sum1);
					//printf("Sum2: %d\n", sum2);
				}
				
				if(sum1 < 0)
				{
					sum1 = -sum1;
				}
				if(sum2 < 0)
				{
					sum2 = -sum2;
				}
				outImg->pixels_bw[row][col].bw = sum1 + sum2;
				if(sum1 + sum2 > curMax)
				{
					curMax = sum1 + sum2;
				}	
			}
		}
	}
	
	if(DEBUG)
	{
		printf("Max: %d.\n", curMax);
	}
	//Scale the values in the output image
	for(row=0; row < height; row++)
	{
		for(col=0; col < width; col++)
		{
			//TODO: can this line be optimized? For sure just do (*255/curMax) instead of a division every time.
			outImg->pixels_bw[row][col].bw = (outImg->pixels_bw[row][col].bw * 255) / curMax;
		}
	}
}


int processHeader(FILE* inpFile, img_data* inImg)
{
	char let1 = fgetc(inpFile);
	char let2 = fgetc(inpFile);
	char let3 = fgetc(inpFile);
	char dimension[5];
	
	if(let1 != 'P')
	{
		printf("Illegal first character in file: %c.\n", let1);
		return 1;
	}
	if(let2 == '6')
	{
		if(DEBUG)
		{
			printf("PPM image.\n");
		}
		inImg->header.type = PPM;
	}
	else if(let2 == '5')
	{
		if(DEBUG)
		{
			printf("PGM image.\n");
		}
		inImg->header.type = PGM;
	}
	else
	{
		printf("Illegal second character in file: %c.\n", let2);
		return 1;
	}
	if(let3 != 10)
	{
		printf("Illegal third character in file: %c.\n", let1);
		return 1;
	}
	
	
	//TODO: fix these next parts so it supports any size
	//For now assume it is 3 digits
	dimension[0] = skipSpaces(inpFile);
	dimension[1] = fgetc(inpFile);
	dimension[2] = fgetc(inpFile);
	dimension[3] ='\0';
	inImg->header.width = atoi(dimension);
	if(DEBUG)
	{
		printf("Image Width: %d.\n", inImg->header.width);
	}
	
	dimension[0] = skipSpaces(inpFile);
	dimension[1] = fgetc(inpFile);
	dimension[2] = fgetc(inpFile);
	dimension[3] ='\0';
	inImg->header.height = atoi(dimension);
	if(DEBUG)
	{
		printf("Image Height: %d.\n", inImg->header.height);
	}
	
	dimension[0] = skipSpaces(inpFile);
	dimension[1] = fgetc(inpFile);
	dimension[2] = fgetc(inpFile);
	dimension[3] ='\0';
	inImg->header.max_val = atoi(dimension);
	if(DEBUG)
	{
		printf("Image Max Val: %d.\n", inImg->header.max_val);
	}
	
	//Assume one space but fix this later
	fgetc(inpFile);
	
	return 0;
}

int processData (FILE* inpFile, img_data* inImg)
{
	int height;
	int width;
	int row;
	int col;
	
	height = inImg->header.height;
	width = inImg->header.width;
	
	for(row=0; row < height; row++)
	{
		for(col=0; col < width; col++)
		{
			if(inImg->header.type == PPM)
			{
				//TODO: error checking. for now assume correct amount of data.
				inImg->pixels_color[row][col].red = fgetc(inpFile);
				inImg->pixels_color[row][col].green = fgetc(inpFile);
				inImg->pixels_color[row][col].blue = fgetc(inpFile);	
			}
			else
			{
				inImg->pixels_bw[row][col].bw = fgetc(inpFile);
			}
		}
	}
	return 0;
}

//Output the image to the specified file
void outputImage(FILE* outFile, img_data* inImg)
{	
	//TODO:change the way the header is printed.
	
	//TODO:change the name of the second parameter

	int height;
	int width;
	int row;
	int col;
	char dimension[5];

	//magic number + newline
	if(inImg->header.type == PGM)
	{
		fputc('P', outFile);
		fputc('5', outFile);
		fputc(10, outFile);	
	}
	else
	{
		fputc('P', outFile);
		fputc('6', outFile);
		fputc(10, outFile);	
	}
	
	//width + newLine
	sprintf(dimension, "%d", inImg->header.width);
	fputs(dimension, outFile);
	fputc(10, outFile);
	
	//height + newLine
	sprintf(dimension, "%d", inImg->header.height);
	fputs(dimension, outFile);
	fputc(10, outFile);
	
	//maxval + newline
	sprintf(dimension, "%d", inImg->header.max_val);
	fputs(dimension, outFile);
	fputc(10, outFile);
	
	
	//data
	height = inImg->header.height;
	width = inImg->header.width;
	
	for(row=0; row < height; row++)
	{
		for(col=0; col < width; col++)
		{
			if(inImg->header.type == PPM)
			{
				fputc(inImg->pixels_color[row][col].red, outFile);
				fputc(inImg->pixels_color[row][col].green, outFile);
				fputc(inImg->pixels_color[row][col].blue, outFile);	
			}
			else
			{
				fputc(inImg->pixels_bw[row][col].bw, outFile);
			}
		}
	}
}

//Returns the first non-space character
char skipSpaces(FILE* inpFile)
{
	// TAB -> 9 (decimal)
	// LF -> 10 (decimal)
	// CR -> 13 (decimal)
	// SPACE -> 32 (decimal)
	// # -> 35
	
	int nextChar;
	while(1)
	{
		nextChar = fgetc(inpFile);
		if(nextChar == 35)
		{
			if(DEBUG)
			{
				printf("Found Comment. Skipping.\n");	
			}
			skipComment(inpFile);
		}
		else if((nextChar != 9) && (nextChar != 10) &&(nextChar != 13) &&(nextChar != 32))
		{
			break;
		}	
	}
	return nextChar;
}

void skipComment(FILE* inpFile)
{
	// LF -> 10
	int nextChar;
	while(1)
	{
		nextChar = fgetc(inpFile);
		if(nextChar == 10)
		{
			return;
		}
	}
}
