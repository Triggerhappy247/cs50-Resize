/**
 * Copies a BMP piece by piece, just because.
 */
       
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize n infile outfile\n");
        return 1;
    }
    // remember scaling size
    float scale = atof(argv[1]);
    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];

    // open input file 
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }
    
    // Create new FILEHEADER and INFOHEADER for new image
    BITMAPFILEHEADER bf2 = bf;
    BITMAPINFOHEADER bi2 = bi;
    
    bi2.biWidth = fabs(bi.biWidth * scale);
    bi2.biHeight = - fabs(bi.biHeight * scale);
    int padding2 = (4 - (bi2.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    bi2.biSizeImage = ((sizeof(RGBTRIPLE) * bi2.biWidth) + padding2)
                      * abs(bi2.biHeight);
    bf2.bfSize = bi2.biSizeImage 
               + sizeof(BITMAPINFOHEADER)
               + sizeof(BITMAPFILEHEADER);             
    
    // write outfile's BITMAPFILEHEADER
    fwrite(&bf2, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi2, sizeof(BITMAPINFOHEADER), 1, outptr);

    // determine padding for scanlines
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    
    // Declare a 2 scanlines of RGBTriples
    RGBTRIPLE *row1 = (RGBTRIPLE*) malloc(bi.biWidth*sizeof(RGBTRIPLE));
    RGBTRIPLE *row2 = (RGBTRIPLE*) malloc(bi2.biWidth*sizeof(RGBTRIPLE));
    int m;
    // iterate over infile's scanlines
    if(floor(scale) == scale)
    {
        for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
        {
            // Read one entire row from image
            fread(row1, sizeof(RGBTRIPLE), bi.biWidth, inptr);
            m = 0;
            for (int j = 0; j < bi.biWidth; j++)
            {
                for(float l = 0; l < scale; l++)
                {
                    row2[m++] = row1[j];
                }
            }
        
            for(float l = 0; l < scale; l++)
            {
                fwrite(row2, sizeof(RGBTRIPLE), bi2.biWidth, outptr);
            }
            // skip over padding, if any
            fseek(inptr, padding, SEEK_CUR);

            // then add it back (to demonstrate how)
            for (int k = 0; k < padding2; k++)
            {
                fputc(0x00, outptr);
            }
        }
    }
    else
    {
        printf("Still working on that part");
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);
    
    // Free memory
    free(row1);
    free(row2);

    // success
    return 0;
}
