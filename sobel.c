//Will support both ppm and pgm format files, in both 1 byte and 2 byte format.
//Converts every image to grayscale as it is read in.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PGM 0
#define PPM 1

#define DEBUG 1

typedef struct
{
	int type;
	int width;
	int height;
	int max_val;
} img_header;

typedef struct
{
	//Use integers for extra space since values may overflow during transform
	int bw;
} bw_pixel;

typedef struct
{
	img_header header;
	bw_pixel** pixels_bw;
} img_data;

int sobelFilter(img_data* inImg, img_data* outImg);
void allocateImageSpace(img_data* inImg, img_data* outImg);
int processHeader(FILE* inpFile, img_data* inImg);
int processData (FILE* inpFile, img_data* inImg);
void outputImage(FILE* outFile, img_data* inImg);
char skipSpaces(FILE* inpFile);
void skipComment(FILE* inpFile);
int getDimension(FILE* inpFile);
int toGrayscale(int red, int green, int blue);

int main(int argc, char* argv[])
{
	FILE* inpFile;
	FILE* outFile;
	img_data inImg;
	img_data outImg;
	int retVal;
	char inFilename[100];
	
	//take optional command line args, output by default to "sobelOut.pgm"
	if(argc < 2) //use a test image
	{
		strncpy(inFilename, "images/lena.pgm", 20);
	}
	else
	{
		strncpy(inFilename, argv[1], 100);
		if(DEBUG)
		{
			printf("Using input file: %s.\n", inFilename);
		}
	}
	
	inpFile = fopen(inFilename, "r");
	if(inpFile == NULL)
	{
		printf("Error opening input file.\n");
		printf("Filename: %s.\n", inFilename);
		return 1;
	} 
	
	outFile = fopen("SobelOut.pgm", "w");
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
	
	allocateImageSpace(&inImg, &outImg);
	
	retVal = processData(inpFile, &inImg);
	if(retVal != 0)
	{
		return 1;
	}
	
	sobelFilter(&inImg, &outImg);
	outputImage(outFile, &outImg);
	
	//TODO: free memory
	return 0;
}

//Only supports black/white images
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
	
	//Loop through the whole image pixel by pixel
	for(row=0; row < height; row++)
	{
		for(col=0; col < width; col++)
		{	
			//If it's an edge.
			if((row==0) || (row==height-1) || (col==0) || (col==width-1))
			{
				outImg->pixels_bw[row][col].bw = 0;	//black edge
			}
			else
			{
				if(inImg->header.type == PPM)	//color images, should never happen
				{
					printf("This sobel filter function only supports black/white images.\n");
					return 1;
				}
				else	//bw images (All color images will have been converted to bw by now.)
				{
					//Optimized way with no multiplication
					sum1 = (-inImg->pixels_bw[row-1][col-1].bw) + 
						   (inImg->pixels_bw[row-1][col+1].bw) +
						   (-inImg->pixels_bw[row][col-1].bw - inImg->pixels_bw[row][col-1].bw) + 
						   (inImg->pixels_bw[row][col+1].bw + inImg->pixels_bw[row][col+1].bw) + 
						   (-inImg->pixels_bw[row+1][col-1].bw) + 
						   (inImg->pixels_bw[row+1][col+1].bw);
			
					sum2 = (-inImg->pixels_bw[row-1][col-1].bw) + 
						   (-inImg->pixels_bw[row-1][col].bw - inImg->pixels_bw[row-1][col].bw) +
						   (-inImg->pixels_bw[row-1][col+1].bw) + 
						   (inImg->pixels_bw[row+1][col-1].bw) + 
						   (inImg->pixels_bw[row+1][col].bw + inImg->pixels_bw[row+1][col].bw) +
						   (inImg->pixels_bw[row+1][col+1].bw);
					
					//Non-optimized method
					/*
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
					*/
				}
				
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
	return 0;
}

//All images will be stored as black/white, so only one int per pixel of space is allocated.
void allocateImageSpace(img_data* inImg, img_data* outImg)
{
	int row;
	//Create the array of pixels for the input file
	inImg->pixels_bw = (bw_pixel**)malloc(sizeof(bw_pixel*) * inImg->header.height);
	for(row=0; row < inImg->header.height; row++)
	{
		inImg->pixels_bw[row] = (bw_pixel*)malloc(sizeof(bw_pixel) * inImg->header.width);
	}
	
	//Create the array of pixels for the output file
	outImg->header.type = PGM;
	outImg->header.width = inImg->header.width;
	outImg->header.height = inImg->header.height;
	outImg->header.max_val = inImg->header.max_val;
	
	outImg->pixels_bw = (bw_pixel**)malloc(sizeof(bw_pixel*) * outImg->header.height);
	for(row=0; row < outImg->header.height; row++)
	{
		outImg->pixels_bw[row] = (bw_pixel*)malloc(sizeof(bw_pixel) * outImg->header.width);
	}
}

int processHeader(FILE* inpFile, img_data* inImg)
{
	char let1 = fgetc(inpFile);
	char let2 = fgetc(inpFile);
	char let3 = fgetc(inpFile);
	
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
	
	inImg->header.width = getDimension(inpFile);
	inImg->header.height = getDimension(inpFile);
	inImg->header.max_val = getDimension(inpFile);
	if(DEBUG)
	{
		printf("Image Width: %d.\n", inImg->header.width);
		printf("Image Height: %d.\n", inImg->header.height);
		printf("Image Max Val: %d.\n", inImg->header.max_val);
	}
	skipSpaces(inpFile);
	fseek(inpFile, -1, SEEK_CUR);	//Go back one byte so we don't lose a byte
	return 0;
}

int processData (FILE* inpFile, img_data* inImg)
{
	//For now this function doesn't do any error checking.
	
	int height;
	int width;
	int row;
	int col;
	int red;
	int green;
	int blue;
	
	height = inImg->header.height;
	width = inImg->header.width;
	
	for(row=0; row < height; row++)
	{
		for(col=0; col < width; col++)
		{
			if(inImg->header.type == PPM)
			{
				//convert to grayscale as values are read in.
				red = fgetc(inpFile);
				green = fgetc(inpFile);
				blue = fgetc(inpFile);
				inImg->pixels_bw[row][col].bw = toGrayscale(red, green, blue);
			}
			else
			{
				inImg->pixels_bw[row][col].bw = fgetc(inpFile);
			}
		}
	}
	
	inImg->header.type = PGM;  //Set the type to pgm since it is now a bw image.
	return 0;
}

//Output the image to the specified file. Only supports bw images.
void outputImage(FILE* outFile, img_data* inImg)
{
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
		printf("Color images not supported by this \"output image\" function.\n");
		return;
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
			//Must be a bw image by this point.
			fputc(inImg->pixels_bw[row][col].bw, outFile);
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

int getDimension(FILE* inpFile)
{
	// '0' -> 48 (decimal)
	// '9' -> 57 (decimal)
	
	char dimension[10];		//max of 10 digits
	int nextChar;
	int i;
	
	dimension[0] = skipSpaces(inpFile);	//Assume at least 1 digit is present
	
	for(i=1; i<10; i++)
	{
		nextChar = fgetc(inpFile);
		if(nextChar < 48 || nextChar > 57)
		{
			dimension[i] = '\0';	//Explicitly set the last char to null terminator.
			break;		//break if it's not a digit
		}
		else
		{
			dimension[i] = nextChar;
		}
	}

	fseek(inpFile, -1, SEEK_CUR);	//Go back one byte so we don't lose a byte
	return atoi(dimension);
}

int toGrayscale(int red, int green, int blue)
{
	//TODO: Can this be optimized at all?  Any way to avoid multiplication?
	return (0.3*red) + (0.59*green) + (0.11*blue);
}
