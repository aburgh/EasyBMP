/*************************************************
*                                                *
*  EasyBMP Cross-Platform Windows Bitmap Library *
*                                                *
*  Author: Paul Macklin                          *
*   email: macklin01@users.sourceforge.net       *
* support: http://easybmp.sourceforge.net        *
*                                                *
*          file: EasyBMP.cpp                     *
*    date added: 03-31-2006                      *
* date modified: 12-01-2006                      *
*       version: 1.06                            *
*                                                *
*   License: BSD (revised/modified)              *
* Copyright: 2005-6 by the EasyBMP Project       *
*                                                *
* description: Actual source file                *
*                                                *
*************************************************/

#include "EasyBMP.h"
#include <exception>
#include <fstream>

using namespace std;

/* These functions are defined in EasyBMP.h */

static bool g_exceptions = true;

bool BMP::exceptions(void)       { return g_exceptions; }
void BMP::exceptions(bool flag)  { g_exceptions = flag; }

/* These functions are defined in EasyBMP_DataStructures.h */

int IntPow(int base, int exponent)
{
	int output = 1;
	for (int i = 0; i < exponent; i++) output *= base;
	return output;
}

BMFH::BMFH()
{
	bfType = 19778;
	bfReserved1 = 0;
	bfReserved2 = 0;
}

void BMFH::SwitchEndianess(void)
{
	bfType = FlipWORD(bfType);
	bfSize = FlipDWORD(bfSize);
	bfReserved1 = FlipWORD(bfReserved1);
	bfReserved2 = FlipWORD(bfReserved2);
	bfOffBits = FlipDWORD(bfOffBits);
	return;
}

BMIH::BMIH()
{
	biPlanes = 1;
	biCompression = 0;
	biXPelsPerMeter = DefaultXPelsPerMeter;
	biYPelsPerMeter = DefaultYPelsPerMeter;
	biClrUsed = 0;
	biClrImportant = 0;
}

void BMIH::SwitchEndianess(void)
{
	biSize = FlipDWORD(biSize);
	biWidth = FlipDWORD(biWidth);
	biHeight = FlipDWORD(biHeight);
	biPlanes = FlipWORD(biPlanes);
	biBitCount = FlipWORD(biBitCount);
	biCompression = FlipDWORD(biCompression);
	biSizeImage = FlipDWORD(biSizeImage);
	biXPelsPerMeter = FlipDWORD(biXPelsPerMeter);
	biYPelsPerMeter = FlipDWORD(biYPelsPerMeter);
	biClrUsed = FlipDWORD(biClrUsed);
	biClrImportant = FlipDWORD(biClrImportant);
	return;
}

void BMIH::display(void)
{
	cout << "biSize: " << (int) biSize << endl
		 << "biWidth: " << (int) biWidth << endl
		 << "biHeight: " << (int) biHeight << endl
		 << "biPlanes: " << (int) biPlanes << endl
		 << "biBitCount: " << (int) biBitCount << endl
		 << "biCompression: " << (int) biCompression << endl
		 << "biSizeImage: " << (int) biSizeImage << endl
		 << "biXPelsPerMeter: " << (int) biXPelsPerMeter << endl
		 << "biYPelsPerMeter: " << (int) biYPelsPerMeter << endl
		 << "biClrUsed: " << (int) biClrUsed << endl
		 << "biClrImportant: " << (int) biClrImportant << endl << endl;
}

void BMFH::display(void)
{
	cout << "bfType: " << (int) bfType << endl
		 << "bfSize: " << (int) bfSize << endl
		 << "bfReserved1: " << (int) bfReserved1 << endl
		 << "bfReserved2: " << (int) bfReserved2 << endl
		 << "bfOffBits: " << (int) bfOffBits << endl << endl;
}

/* These functions are defined in EasyBMP_BMP.h */

struct ReadBuf : public std::streambuf
{
	ReadBuf(char* s, std::size_t n)
	{
		setg(s, s, s + n);
	}
};

bool SafeFread(char* buffer, int size, int number, istream& in)
{
	if (in.eof()) return false;
	streamsize bytesRequested = size * number;
	in.read(buffer, bytesRequested);

	return in.gcount() == bytesRequested;
}


RGBApixel BMP::GetPixel(int i, int j) const
{
	bool err = false;
	if (i < 0) { i = 0; err = true; }
	if (j < 0) { j = 0; err = true; }
	if (i >= Width)  { i = Width - 1; err = true; }
	if (j >= Height) { j = Height - 1; err = true; }
	if (err and g_exceptions) {
		throw invalid_argument("EasyBMP: attempted to access non-existent pixel");
	}
	return Pixels[i][j];
}

bool BMP::SetPixel( int i, int j, RGBApixel NewPixel )
{
	Pixels[i][j] = NewPixel;
	return true;
}


bool BMP::SetColor( int ColorNumber , RGBApixel NewColor )
{
	if ( BitDepth != 1 and BitDepth != 4 and BitDepth != 8 )
	{
		if (g_exceptions) throw invalid_argument("EasyBMP::SetColor: cannot change color table for a BMP object that lacks a color table.");
		return false;
	}
	if (not Colors)	{
		if (g_exceptions) {
			throw invalid_argument("EasyBMP::SetColor: the color table is not defined.");
		}
		return false;
	}
	if (ColorNumber >= TellNumberOfColors()) {
		if (g_exceptions) {
			throw invalid_argument(string("EasyBMP::SetColor: requested color number ") + to_string(ColorNumber) +
								   " is outside the allowed range [0," + to_string(TellNumberOfColors()-1) +
								   "]. Ignoring request to set this color.");
		}
		return false;
	}
	Colors[ColorNumber] = NewColor;
	return true;
}

// RGBApixel BMP::GetColor( int ColorNumber ) const
RGBApixel BMP::GetColor(int ColorNumber)
{
	RGBApixel Output;
	Output.Red   = 255;
	Output.Green = 255;
	Output.Blue  = 255;
	Output.Alpha = 0;

	if ( BitDepth != 1 and BitDepth != 4 and BitDepth != 8 )
	{
		if (g_exceptions) {
			throw runtime_error("EasyBMP::GetColor: image bit depth does not support having a color table.");
		}
		return Output;
	}
	if (not Colors)	{
		if (g_exceptions) {
			throw runtime_error("EasyBMP::GetColor: no color table is defined.");
		}
		return Output;
	}
	if (ColorNumber >= TellNumberOfColors()) {
		if (g_exceptions) {
			throw runtime_error("EasyBMP::GetColor: requested color number " + to_string(ColorNumber) +
								" is outside the allowed range [0," + to_string(TellNumberOfColors() - 1) + "].");
		}
		return Output;
	}
	Output = Colors[ColorNumber];
	return Output;
}

BMP::BMP()
{
	Width = 1;
	Height = 1;
	BitDepth = 24;
	Pixels = new RGBApixel* [Width];
	Pixels[0] = new RGBApixel [Height];
	Colors = nullptr;

	XPelsPerMeter = 0;
	YPelsPerMeter = 0;

	MetaData1 = nullptr;
	SizeOfMetaData1 = 0;
	MetaData2 = nullptr;
	SizeOfMetaData2 = 0;
}

// BMP::BMP( const BMP& Input )
BMP::BMP(BMP& Input)
{
	// first, make the image empty.

	Width = 1;
	Height = 1;
	BitDepth = 24;
	Pixels = new RGBApixel* [Width];
	Pixels[0] = new RGBApixel [Height];
	Colors = nullptr;
	XPelsPerMeter = 0;
	YPelsPerMeter = 0;

	MetaData1 = nullptr;
	SizeOfMetaData1 = 0;
	MetaData2 = nullptr;
	SizeOfMetaData2 = 0;

	// now, set the correct bit depth

	SetBitDepth(Input.TellBitDepth());

	// set the correct pixel size

	SetSize(Input.TellWidth(), Input.TellHeight());

	// set the DPI information from Input

	SetDPI(Input.TellHorizontalDPI(), Input.TellVerticalDPI());

	// if there is a color table, get all the colors

	if (BitDepth == 1 or BitDepth == 4 or BitDepth == 8)
	{
		for (int k = 0; k < TellNumberOfColors() ; k++)
		{
			SetColor(k, Input.GetColor(k));
		}
	}

	// get all the pixels

	for (int j = 0; j < Height; j++) {
		for (int i = 0; i < Width; i++) {
			Pixels[i][j] = Input(i,j);
		//   Pixels[i][j] = Input.GetPixel(i,j); // *Input(i,j);
		}
	}
}

BMP::~BMP()
{
	for (int i = 0; i < Width; i++) delete [] Pixels[i];
	delete [] Pixels;
	delete [] Colors;
	delete [] MetaData1;
	delete [] MetaData2;
}

RGBApixel& BMP::operator()(int i, int j)
{
	bool Warn = false;
	if (i < 0 )      { i = 0; Warn = true; }
	if (j < 0 )	     { j = 0; Warn = true; }
	if (i >= Width)  { i = Width - 1; Warn = true; }
	if (j >= Height) { j = Height - 1; Warn = true; }
	if (Warn and g_exceptions) {
		throw invalid_argument("EasyBMP: attempted to access non-existent pixel");
	}
	return Pixels[i][j];
}

// int BMP::TellBitDepth( void ) const
int BMP::TellBitDepth(void) { return BitDepth; }

// int BMP::TellHeight( void ) const
int BMP::TellHeight(void) { return VerticalFlip ? -Height : Height; }
int BMP::AbsHeight(void) { return Height; }

// int BMP::TellWidth( void ) const
int BMP::TellWidth(void) { return HorizontalFlip ? -Width : Width; }
int BMP::AbsWidth(void) { return Width; }

// int BMP::TellNumberOfColors( void ) const
int BMP::TellNumberOfColors(void)
{
	int output = IntPow(2, BitDepth);
	if (BitDepth == 32) { output = IntPow(2, 24); }
	return output;
}

bool BMP::SetBitDepth(int NewDepth)
{
	if (NewDepth != 1 and NewDepth != 4 and
		NewDepth != 8 and NewDepth != 16 and
		NewDepth != 24 and NewDepth != 32)
	{
		if (g_exceptions) {
			throw invalid_argument("EasyBMP::SetBitDepth: unsupported bit depth " + to_string(NewDepth));
		}
		return false;
	}

	BitDepth = NewDepth;
	delete [] Colors;
	int NumberOfColors = IntPow(2, BitDepth);
	if (BitDepth == 1 or BitDepth == 4 or BitDepth == 8) {
		Colors = new RGBApixel [NumberOfColors];
	}
	else {
		Colors = nullptr;
	}
	if (BitDepth == 1 or BitDepth == 4 or BitDepth == 8) {
		CreateStandardColorTable();
	}

	return true;
}

bool BMP::SetSize(int NewWidth , int NewHeight )
{
	if (NewWidth == 0 or NewHeight == 0)
	{
		if (g_exceptions) {
			throw invalid_argument("EasyBMP::SetSize: width or height must be non-zero");
		}
		return false;
	}

	int i, j;

	for (i = 0; i < Width; i++) delete [] Pixels[i];
	delete [] Pixels;

	if (NewWidth < 0)
		HorizontalFlip = true;
	if (NewHeight < 0)
		VerticalFlip = true;

	Width = abs(NewWidth);
	Height = abs(NewHeight);
	Pixels = new RGBApixel* [ Width ];

	for (i = 0; i < Width; i++)
		Pixels[i] = new RGBApixel[Height];

	for (i = 0; i < Width ; i++)
	{
		for (j = 0; j < Height ; j++)
		{
			Pixels[i][j].Red = 255;
			Pixels[i][j].Green = 255;
			Pixels[i][j].Blue = 255;
			Pixels[i][j].Alpha = 0;
		}
	}

	return true;
}

bool BMP::WriteToFile(const string& FileName)
{
	if (not EasyBMPcheckDataSize()) {
		if (g_exceptions) {
			throw runtime_error(string("EasyBMP::WriteToFile: data types are wrong size! ") +
								"You may need to mess with EasyBMP_DataTypes.h to fix these errors, and then recompile. " +
								"All 32-bit and 64-bit machines should be supported, however.");
		}
		return false;
	}

	FILE* fp = fopen(FileName.c_str(), "wb");
	if (not fp) {
		if (g_exceptions) {
			throw runtime_error("EasyBMP::WriteToFile: cannot open file " + FileName + " for output.");
		}
		return false;
	}

	// some preliminaries

	double dBytesPerPixel = ( (double) BitDepth ) / 8.0;
	double dBytesPerRow = dBytesPerPixel * (Width + 0.0);
	dBytesPerRow = ceil(dBytesPerRow);

	int BytePaddingPerRow = 4 - ( (int) (dBytesPerRow) ) % 4;
	if (BytePaddingPerRow == 4)
		BytePaddingPerRow = 0;

	double dActualBytesPerRow = dBytesPerRow + BytePaddingPerRow;

	double dTotalPixelBytes = Height * dActualBytesPerRow;

	double dPaletteSize = 0;
	if (BitDepth == 1 or BitDepth == 4 or BitDepth == 8) {
		dPaletteSize = IntPow(2,BitDepth)*4.0;
	}

	// leave some room for 16-bit masks
	if (BitDepth == 16) dPaletteSize = 3*4;

	double dTotalFileSize = 14 + 40 + dPaletteSize + dTotalPixelBytes;

	// write the file header
	BMFH bmfh;
	bmfh.bfSize = (ebmpDWORD) dTotalFileSize;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = (ebmpDWORD) (14+40+dPaletteSize);

	if (IsBigEndian()) bmfh.SwitchEndianess();

	fwrite( (char*) &(bmfh.bfType), sizeof(ebmpWORD), 1, fp );
	fwrite( (char*) &(bmfh.bfSize), sizeof(ebmpDWORD), 1, fp );
	fwrite( (char*) &(bmfh.bfReserved1), sizeof(ebmpWORD), 1, fp );
	fwrite( (char*) &(bmfh.bfReserved2), sizeof(ebmpWORD), 1, fp );
	fwrite( (char*) &(bmfh.bfOffBits), sizeof(ebmpDWORD), 1, fp );

	// write the info header
	BMIH bmih;
	bmih.biSize = 40;
	bmih.biWidth = HorizontalFlip ? -Width : Width;
	bmih.biHeight = VerticalFlip ? -Height : Height;
	bmih.biPlanes = 1;
	bmih.biBitCount = BitDepth;
	bmih.biCompression = 0;
	bmih.biSizeImage = (ebmpDWORD) dTotalPixelBytes;

	if (XPelsPerMeter) bmih.biXPelsPerMeter = XPelsPerMeter;
	else bmih.biXPelsPerMeter = DefaultXPelsPerMeter;

	if (YPelsPerMeter) bmih.biYPelsPerMeter = YPelsPerMeter;
	else bmih.biYPelsPerMeter = DefaultYPelsPerMeter;

	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;

	// indicates that we'll be using bit fields for 16-bit files
	if (BitDepth == 16 ) bmih.biCompression = 3;

	if (IsBigEndian()) bmih.SwitchEndianess();

	fwrite( (char*) &(bmih.biSize), sizeof(ebmpDWORD), 1, fp );
	fwrite( (char*) &(bmih.biWidth), sizeof(ebmpDWORD), 1, fp );
	fwrite( (char*) &(bmih.biHeight), sizeof(ebmpDWORD), 1, fp );
	fwrite( (char*) &(bmih.biPlanes), sizeof(ebmpWORD), 1, fp );
	fwrite( (char*) &(bmih.biBitCount), sizeof(ebmpWORD), 1, fp );
	fwrite( (char*) &(bmih.biCompression), sizeof(ebmpDWORD), 1, fp );
	fwrite( (char*) &(bmih.biSizeImage), sizeof(ebmpDWORD), 1, fp );
	fwrite( (char*) &(bmih.biXPelsPerMeter), sizeof(ebmpDWORD), 1, fp );
	fwrite( (char*) &(bmih.biYPelsPerMeter), sizeof(ebmpDWORD), 1, fp );
	fwrite( (char*) &(bmih.biClrUsed), sizeof(ebmpDWORD), 1, fp);
	fwrite( (char*) &(bmih.biClrImportant), sizeof(ebmpDWORD), 1, fp);

	// write the palette
	if (BitDepth == 1 or BitDepth == 4 or BitDepth == 8) {
		int NumberOfColors = IntPow(2,BitDepth);

		// if there is no palette, create one
		if (not Colors)	{
			Colors = new RGBApixel[NumberOfColors];
			CreateStandardColorTable();
		}

		for (int n = 0; n < NumberOfColors; n++ ) {
			fwrite((char*) &(Colors[n]), 4, 1, fp);
		}
	}

	// write the pixels
	int i, j, row, col;
	if (BitDepth != 16)	{
		ebmpBYTE* Buffer;
		int BufferSize = (int) ( (Width * BitDepth) / 8.0 );
		while (8 * BufferSize < Width * BitDepth) BufferSize++;
		while (BufferSize % 4) BufferSize++;

		Buffer = new ebmpBYTE [BufferSize];
		for (j = 0; j < BufferSize; j++) Buffer[j] = 0;

		j=Height - 1;

		while (j > -1) {
			// If the image has a negative height, then the pixel buffer is 
			// stored top to bottom rather than bottom to top.
			row = VerticalFlip ? Height -1 -j : j;

			bool Success = false;
			if (BitDepth == 32) Success = Write32bitRow( Buffer, BufferSize, row );
			if (BitDepth == 24) Success = Write24bitRow( Buffer, BufferSize, row );
			if (BitDepth == 8 ) Success = Write8bitRow( Buffer, BufferSize, row );
			if (BitDepth == 4 ) Success = Write4bitRow( Buffer, BufferSize, row );
			if (BitDepth == 1 ) Success = Write1bitRow( Buffer, BufferSize, row );
			if (Success) {
				int BytesWritten = (int) fwrite((char*) Buffer, 1, BufferSize, fp);
				if ( BytesWritten != BufferSize ) Success = false;
			}
			if (!Success) {
				if (g_exceptions) {
					delete [] Buffer;
					throw runtime_error("EasyBMP::WriteToFile: could not write proper amount of data.");
				}
				j = -1;
			}
			j--;
		}

		delete [] Buffer;
	}

	if (BitDepth == 16) {
		// write the bit masks

		ebmpWORD BlueMask = 31;    // bits 12-16
		ebmpWORD GreenMask = 2016; // bits 6-11
		ebmpWORD RedMask = 63488;  // bits 1-5
		ebmpWORD ZeroWORD;

		if (IsBigEndian()) RedMask = FlipWORD(RedMask);
		fwrite( (char*) &RedMask, 2, 1, fp );
		fwrite( (char*) &ZeroWORD, 2, 1, fp );

		if (IsBigEndian()) GreenMask = FlipWORD(GreenMask);
		fwrite( (char*) &GreenMask, 2, 1, fp );
		fwrite( (char*) &ZeroWORD, 2, 1, fp );

		if (IsBigEndian()) BlueMask = FlipWORD(BlueMask);
		fwrite( (char*) &BlueMask, 2, 1, fp );
		fwrite( (char*) &ZeroWORD, 2, 1, fp );

		int DataBytes = Width * 2;
		int PaddingBytes = (4 - DataBytes % 4) % 4;

		// write the actual pixels
		for (j = Height - 1; j >= 0 ; j--) {
			// write all row pixel data
			i = 0;
			int WriteNumber = 0;
			while (WriteNumber < DataBytes) {
				ebmpWORD TempWORD;

				// If the image has a negative height, then the pixel buffer is 
				// stored top to bottom rather than bottom to top.
				row = VerticalFlip ? Height -1 -j : j; 
				col = HorizontalFlip ? Width -1 -i : i;

				ebmpWORD RedWORD = (ebmpWORD) ((Pixels[col][row]).Red / 8);
				ebmpWORD GreenWORD = (ebmpWORD) ((Pixels[col][row]).Green / 4);
				ebmpWORD BlueWORD = (ebmpWORD) ((Pixels[col][row]).Blue / 8);

				TempWORD = (RedWORD << 11) + (GreenWORD << 5) + BlueWORD;
				if (IsBigEndian()) TempWORD = FlipWORD( TempWORD );

				fwrite( (char*) &TempWORD , 2, 1, fp);
				WriteNumber += 2;
				i++;
			}
			// write any necessary row padding
			WriteNumber = 0;
			while (WriteNumber < PaddingBytes) {
				ebmpBYTE TempBYTE;
				fwrite( (char*) &TempBYTE , 1, 1, fp);
				WriteNumber++;
			}
		}

	}

	fclose(fp);
	return true;
}

bool BMP::ReadFromStream(istream& in)
{
	// read the file header

	BMFH bmfh;
	bool NotCorrupted = true;

	NotCorrupted &= SafeFread((char*) &(bmfh.bfType), sizeof(ebmpWORD), 1, in);

	bool IsBitmap = false;

	if (IsBigEndian()     and bmfh.bfType == 16973) IsBitmap = true;
	if (not IsBigEndian() and bmfh.bfType == 19778) IsBitmap = true;

	if (!IsBitmap) {
		if (g_exceptions) {
			throw runtime_error("EasyBMP::ReadFromStream: not a Windows BMP file");
		}
		return false;
	}

	NotCorrupted &= SafeFread((char*) &(bmfh.bfSize), sizeof(ebmpDWORD), 1, in);
	NotCorrupted &= SafeFread((char*) &(bmfh.bfReserved1), sizeof(ebmpWORD), 1, in);
	NotCorrupted &= SafeFread((char*) &(bmfh.bfReserved2), sizeof(ebmpWORD), 1, in);
	NotCorrupted &= SafeFread((char*) &(bmfh.bfOffBits), sizeof(ebmpDWORD), 1, in);

	if (IsBigEndian()) bmfh.SwitchEndianess();

	// read the info header

	BMIH bmih;

	NotCorrupted &= SafeFread((char*) &(bmih.biSize), sizeof(ebmpDWORD), 1, in);
	NotCorrupted &= SafeFread((char*) &(bmih.biWidth), sizeof(ebmpDWORD), 1, in);
	NotCorrupted &= SafeFread((char*) &(bmih.biHeight), sizeof(ebmpDWORD), 1, in);
	NotCorrupted &= SafeFread((char*) &(bmih.biPlanes), sizeof(ebmpWORD), 1, in);
	NotCorrupted &= SafeFread((char*) &(bmih.biBitCount), sizeof(ebmpWORD), 1, in);

	NotCorrupted &= SafeFread((char*) &(bmih.biCompression), sizeof(ebmpDWORD), 1, in);
	NotCorrupted &= SafeFread((char*) &(bmih.biSizeImage), sizeof(ebmpDWORD), 1, in);
	NotCorrupted &= SafeFread((char*) &(bmih.biXPelsPerMeter), sizeof(ebmpDWORD), 1, in);
	NotCorrupted &= SafeFread((char*) &(bmih.biYPelsPerMeter), sizeof(ebmpDWORD), 1, in);
	NotCorrupted &= SafeFread((char*) &(bmih.biClrUsed), sizeof(ebmpDWORD), 1, in);
	NotCorrupted &= SafeFread((char*) &(bmih.biClrImportant), sizeof(ebmpDWORD), 1, in);

	if (IsBigEndian()) bmih.SwitchEndianess();

	// a safety catch: if any of the header information didn't read properly, abort
	// future idea: check to see if at least most is self-consistent

	if (not NotCorrupted) {
		SetSize(1, 1);
		SetBitDepth(1);
		if (g_exceptions) {
			throw runtime_error("EasyBMP::ReadFromStream: file is corrupted");
		}
		return false;
	}

	XPelsPerMeter = bmih.biXPelsPerMeter;
	YPelsPerMeter = bmih.biYPelsPerMeter;

	// if bmih.biCompression 1 or 2, then the file is RLE compressed

	if (bmih.biCompression == 1 or bmih.biCompression == 2) {
		SetSize(1, 1);
		SetBitDepth(1);
		if (g_exceptions) {
			throw runtime_error("EasyBMP::ReadFromStream: file is (RLE) compressed. "
								"EasyBMP does not support compression.");
		}
		return false;
	}

	// if bmih.biCompression > 3, then something strange is going on
	// it's probably an OS2 bitmap file.

	if (bmih.biCompression > 3) {
		SetSize(1, 1);
		SetBitDepth(1);
		if (g_exceptions) {
			throw runtime_error("EasyBMP::ReadFromStream: file is an unsupported format."
								"(bmih.biCompression = " + to_string(bmih.biCompression) + "). "
								"The file may be an old OS2 bitmap or corrupted.");
		}
		return false;
	}

	if (bmih.biCompression == 3 and bmih.biBitCount != 16) {
		SetSize(1, 1);
		SetBitDepth(1);
		if (g_exceptions) {
			throw runtime_error("EasyBMP::ReadFromStream: file uses bit fields and is not a 16-bit file. This is not supported.");
		}
		return false;
	}

	// set the bit depth

	int TempBitDepth = (int) bmih.biBitCount;
	if (TempBitDepth != 1  and TempBitDepth != 4  and TempBitDepth != 8 and
		TempBitDepth != 16 and TempBitDepth != 24 and TempBitDepth != 32)
	{
		SetSize(1, 1);
		SetBitDepth(1);
		if (g_exceptions) {
			throw runtime_error("EasyBMP::ReadFromStream: unrecognized bit depth.");
		}
		return false;
	}
	SetBitDepth((int) bmih.biBitCount);

	// set the size

	if ((int) bmih.biWidth <= 0) {
		// Only negative height currently supported
		SetSize(1, 1);
		SetBitDepth(1);
		if (g_exceptions) {
			throw runtime_error("EasyBMP::ReadFromStream: negative width parameter.");
		}
		return false;
	}
	SetSize((int) bmih.biWidth , (int) bmih.biHeight);

	// some preliminaries

	double dBytesPerPixel = ((double) BitDepth ) / 8.0;
	double dBytesPerRow = dBytesPerPixel * (Width + 0.0);
	dBytesPerRow = ceil(dBytesPerRow);

	int BytePaddingPerRow = 4 - ((int) (dBytesPerRow)) % 4;
	if (BytePaddingPerRow == 4) BytePaddingPerRow = 0;

	// if < 16 bits, read the palette

	if (BitDepth < 16) {
		// determine the number of colors specified in the
		// color table

		int NumberOfColorsToRead = ((int) bmfh.bfOffBits - 54 ) / 4;
		if (NumberOfColorsToRead > IntPow(2, BitDepth)) NumberOfColorsToRead = IntPow(2, BitDepth);

		int n;
		for (n = 0; n < NumberOfColorsToRead; n++)
		{
			SafeFread((char*) &(Colors[n]), 4, 1, in);
		}
		for (n = NumberOfColorsToRead; n < TellNumberOfColors(); n++)
		{
			RGBApixel WHITE;
			WHITE.Red = 255;
			WHITE.Green = 255;
			WHITE.Blue = 255;
			WHITE.Alpha = 0;
			SetColor(n, WHITE);
		}
	}

	// skip blank data if bfOffBits so indicates

	int BytesToSkip = bmfh.bfOffBits - 54;;
	if (BitDepth < 16) BytesToSkip -= 4*IntPow(2,BitDepth);
	if (BitDepth == 16 and bmih.biCompression == 3) BytesToSkip -= 3*4;
	if (BytesToSkip < 0 ) BytesToSkip = 0;
	if (BytesToSkip > 0 and BitDepth != 16) {
//		if (g_exceptions) {
//			throw runtime_error("EasyBMP::ReadFromStream: extra meta data detected");
//			// " Data will be skipped."
//		}
		ebmpBYTE* TempSkipBYTE;
		TempSkipBYTE = new ebmpBYTE[BytesToSkip];
		SafeFread((char*) TempSkipBYTE, BytesToSkip, 1, in);
		delete [] TempSkipBYTE;
	}

	// This code reads 1, 4, 8, 24, and 32-bpp files
	// with a more-efficient buffered technique.

	int i, j, col, row;
	if (BitDepth != 16) {
		int BufferSize = (int) ((Width * BitDepth) / 8.0 );
		while (8 * BufferSize < Width * BitDepth) BufferSize++;
		while (BufferSize % 4) BufferSize++;
		unique_ptr<ebmpBYTE> Buffer(new ebmpBYTE[BufferSize]);
		j = Height - 1;
		while (j > -1) {
			in.read((char*) Buffer.get(), BufferSize);
			if (in.gcount() < BufferSize) {
				j = -1;
				if (g_exceptions) {
					throw runtime_error("EasyBMP::ReadFromStream: could not read proper amount of data.");
				}
			}
			else {
				
				// If the image has a negative height, then the pixel buffer is 
				// stored top to bottom rather than bottom to top.
				row = VerticalFlip ? Height -1 -j : j;

				bool Success = false;
				if (BitDepth == 1 ) Success = Read1bitRow( Buffer.get(), BufferSize, row);
				if (BitDepth == 4 ) Success = Read4bitRow( Buffer.get(), BufferSize, row);
				if (BitDepth == 8 ) Success = Read8bitRow( Buffer.get(), BufferSize, row);
				if (BitDepth == 24) Success = Read24bitRow(Buffer.get(), BufferSize, row);
				if (BitDepth == 32) Success = Read32bitRow(Buffer.get(), BufferSize, row);
				if (not Success) {
					if (g_exceptions) {
						throw runtime_error("EasyBMP::ReadFromStream: could not read enough pixel data.");
					}
					j = -1;
				}
			}
			j--;
		}
	}

	if (BitDepth == 16)
	{
		int DataBytes = Width * 2;
		int PaddingBytes = (4 - DataBytes % 4) % 4;

		// set the default mask

		ebmpWORD BlueMask = 31; // bits 12-16
		ebmpWORD GreenMask = 992; // bits 7-11
		ebmpWORD RedMask = 31744; // bits 2-6

		// read the bit fields, if necessary, to
		// override the default 5-5-5 mask

		if (bmih.biCompression != 0) {
			// read the three bit masks

			ebmpWORD TempMaskWORD;
			//ebmpWORD ZeroWORD;

			SafeFread((char*) &RedMask, 2, 1, in);
			if (IsBigEndian()) RedMask = FlipWORD(RedMask);
			SafeFread((char*) &TempMaskWORD, 2, 1, in);

			SafeFread((char*) &GreenMask, 2, 1, in);
			if (IsBigEndian()) GreenMask = FlipWORD(GreenMask);
			SafeFread((char*) &TempMaskWORD, 2, 1, in);

			SafeFread( (char*) &BlueMask, 2, 1, in);
			if (IsBigEndian()) BlueMask = FlipWORD(BlueMask);
			SafeFread((char*) &TempMaskWORD, 2, 1, in);
		}

		// read and skip any meta data

		if (BytesToSkip > 0) {
//			if (g_exceptions) {
//				throw runtime_error("EasyBMP::ReadFromStream: extra meta data detected");
//			}
			ebmpBYTE* TempSkipBYTE;
			TempSkipBYTE = new ebmpBYTE[BytesToSkip];
			SafeFread((char*) TempSkipBYTE, BytesToSkip, 1, in);
			delete [] TempSkipBYTE;
		}

		// determine the red, green and blue shifts

		int GreenShift = 0;
		ebmpWORD TempShiftWORD = GreenMask;
		while (TempShiftWORD > 31) { TempShiftWORD = TempShiftWORD>>1; GreenShift++; }
		int BlueShift = 0;
		TempShiftWORD = BlueMask;
		while (TempShiftWORD > 31) { TempShiftWORD = TempShiftWORD>>1; BlueShift++; }
		int RedShift = 0;
		TempShiftWORD = RedMask;
		while (TempShiftWORD > 31) { TempShiftWORD = TempShiftWORD>>1; RedShift++; }

		// read the actual pixels

		for (j = Height - 1; j >= 0; j--) {
			i = 0;
			int ReadNumber = 0;
			while (ReadNumber < DataBytes) {
				ebmpWORD TempWORD;
				SafeFread((char*) &TempWORD, 2, 1, in);
				if (IsBigEndian()) TempWORD = FlipWORD(TempWORD);
				ReadNumber += 2;

				ebmpWORD Red = RedMask & TempWORD;
				ebmpWORD Green = GreenMask & TempWORD;
				ebmpWORD Blue = BlueMask & TempWORD;

				ebmpBYTE BlueBYTE = (ebmpBYTE) 8 * (Blue >> BlueShift);
				ebmpBYTE GreenBYTE = (ebmpBYTE) 8 * (Green >> GreenShift);
				ebmpBYTE RedBYTE = (ebmpBYTE) 8 * (Red >> RedShift);

				row = VerticalFlip   ? Height -1 -j : j;
				col = HorizontalFlip ? Width  -1 -i : i;

				(Pixels[col][row]).Red = RedBYTE;
				(Pixels[col][row]).Green = GreenBYTE;
				(Pixels[col][row]).Blue = BlueBYTE;

				i++;
			}
			ReadNumber = 0;
			while (ReadNumber < PaddingBytes) {
				ebmpBYTE TempBYTE;
				SafeFread((char*) &TempBYTE, 1, 1, in);
				ReadNumber++;
			}
		}
	}

	return true;
}


bool BMP::ReadFromFile(const string& FileName)
{
	if (!EasyBMPcheckDataSize()) {
		if (g_exceptions) {
			throw runtime_error("EasyBMP::ReadFromStream: Data types are wrong size! "
								"You may need to mess with EasyBMP_DataTypes.h to fix these errors, and then recompile. "
								"All 32-bit and 64-bit machines should be supported, however.");
		}
		return false;
	}

	try {
		ifstream stream(FileName, ios::binary);
		if (not stream) {
			if (g_exceptions) {
				throw runtime_error("EasyBMP::ReadFromFile: cannot open file " + FileName + " for input.");
			}
			return false;
		}

		return ReadFromStream(stream);
	}
	catch (const exception& e) {
		SetBitDepth(1);
		SetSize(1, 1);
		throw_with_nested(runtime_error("EasyBMP::ReadFromFile: failed to read file '" + FileName + "'"));
	}
	return false;
}

bool BMP::ReadFromBuffer(const unsigned char *buffer, size_t size)
{
	// No need to catch exceptions for a buffer since we can't add useful info

	// Dropping const with cast should be OK.
	ReadBuf streambuffer((char*) buffer, size);
	istream stream(&streambuffer);

	return ReadFromStream(stream);
}


bool BMP::CreateStandardColorTable( void )
{
	if (BitDepth != 1 and BitDepth != 4 and BitDepth != 8) {
		if (g_exceptions) {
			throw runtime_error("EasyBMP: attempted to create color table at a bit depth "
								"that does not require a color table.");
		}
		return false;
	}

	if (BitDepth == 1) {
		int i;
		for (i = 0; i < 2; i++)	{
			Colors[i].Red = i * 255;
			Colors[i].Green = i * 255;
			Colors[i].Blue = i * 255;
			Colors[i].Alpha = 0;
		}
		return true;
	}

	if (BitDepth == 4) {
		int i = 0;
		int j, k, ell;

		// simplify the code for the first 8 colors
		for (ell = 0; ell < 2; ell++) {
			for (k = 0; k < 2 ; k++) {
				for (j = 0; j < 2; j++)	{
					Colors[i].Red = j * 128;
					Colors[i].Green = k * 128;
					Colors[i].Blue = ell * 128;
					i++;
				}
			}
		}

		// simplify the code for the last 8 colors
		for (ell = 0; ell < 2; ell++) {
			for (k = 0; k < 2; k++) {
				for (j = 0; j < 2 ; j++) {
					Colors[i].Red = j * 255;
					Colors[i].Green = k * 255;
					Colors[i].Blue = ell * 255;
					i++;
				}
			}
		}

		// overwrite the duplicate color
		i = 8;
		Colors[i].Red = 192;
		Colors[i].Green = 192;
		Colors[i].Blue = 192;

		for (i = 0; i < 16; i++) Colors[i].Alpha = 0;
		return true;
	}

	if (BitDepth == 8) {
		int i = 0;
		int j, k, ell;

		// do an easy loop, which works for all but colors
		// 0 to 9 and 246 to 255
		for (ell = 0; ell < 4; ell++) {
			for (k = 0; k < 8; k++) {
				for (j = 0; j < 8; j++) {
					Colors[i].Red = j * 32;
					Colors[i].Green = k * 32;
					Colors[i].Blue = ell * 64;
					Colors[i].Alpha = 0;
					i++;
				}
			}
		}

		// now redo the first 8 colors
		i = 0;
		for (ell = 0; ell < 2; ell++) {
			for (k = 0; k < 2; k++) {
				for (j = 0; j < 2; j++) {
					Colors[i].Red = j * 128;
					Colors[i].Green = k * 128;
					Colors[i].Blue = ell * 128;
					i++;
				}
			}
		}

		// overwrite colors 7, 8, 9
		i = 7;
		Colors[i].Red = 192;
		Colors[i].Green = 192;
		Colors[i].Blue = 192;
		i++; // 8
		Colors[i].Red = 192;
		Colors[i].Green = 220;
		Colors[i].Blue = 192;
		i++; // 9
		Colors[i].Red = 166;
		Colors[i].Green = 202;
		Colors[i].Blue = 240;

		// overwrite colors 246 to 255
		i = 246;
		Colors[i].Red = 255;
		Colors[i].Green = 251;
		Colors[i].Blue = 240;
		i++; // 247
		Colors[i].Red = 160;
		Colors[i].Green = 160;
		Colors[i].Blue = 164;
		i++; // 248
		Colors[i].Red = 128;
		Colors[i].Green = 128;
		Colors[i].Blue = 128;
		i++; // 249
		Colors[i].Red = 255;
		Colors[i].Green = 0;
		Colors[i].Blue = 0;
		i++; // 250
		Colors[i].Red = 0;
		Colors[i].Green = 255;
		Colors[i].Blue = 0;
		i++; // 251
		Colors[i].Red = 255;
		Colors[i].Green = 255;
		Colors[i].Blue = 0;
		i++; // 252
		Colors[i].Red = 0;
		Colors[i].Green = 0;
		Colors[i].Blue = 255;
		i++; // 253
		Colors[i].Red = 255;
		Colors[i].Green = 0;
		Colors[i].Blue = 255;
		i++; // 254
		Colors[i].Red = 0;
		Colors[i].Green = 255;
		Colors[i].Blue = 255;
		i++; // 255
		Colors[i].Red = 255;
		Colors[i].Green = 255;
		Colors[i].Blue = 255;

		return true;
	}
	return true;
}

void BMP::SetDPI(int HorizontalDPI, int VerticalDPI)
{
	XPelsPerMeter = (int) (HorizontalDPI * 39.37007874015748);
	YPelsPerMeter = (int) (  VerticalDPI * 39.37007874015748);
}

// int BMP::TellVerticalDPI( void ) const
int BMP::TellVerticalDPI( void )
{
	if (not YPelsPerMeter) YPelsPerMeter = DefaultYPelsPerMeter;
	return (int) (YPelsPerMeter / (double) 39.37007874015748);
}

// int BMP::TellHorizontalDPI( void ) const
int BMP::TellHorizontalDPI( void )
{
	if (not XPelsPerMeter) XPelsPerMeter = DefaultXPelsPerMeter;
	return (int) (XPelsPerMeter / (double) 39.37007874015748);
}

/* These functions are defined in EasyBMP_VariousBMPutilities.h */

BMFH GetBMFH(const string& szFileNameIn)
{
	BMFH bmfh;

	FILE* fp;
	fp = fopen(szFileNameIn.c_str(), "rb");

	if (not fp) {
		if (g_exceptions) {
			throw runtime_error("EasyBMP::GetBMFH: cannot initialize from file " + szFileNameIn + ". " +
								"File cannot be opened or does not exist.");
		}
		bmfh.bfType = 0;
		return bmfh;
	}

	fread((char*) &(bmfh.bfType), sizeof(ebmpWORD), 1, fp);
	fread((char*) &(bmfh.bfSize), sizeof(ebmpDWORD), 1, fp);
	fread((char*) &(bmfh.bfReserved1), sizeof(ebmpWORD), 1, fp);
	fread((char*) &(bmfh.bfReserved2), sizeof(ebmpWORD), 1, fp);
	fread((char*) &(bmfh.bfOffBits), sizeof(ebmpDWORD), 1, fp);

	fclose(fp);

	if (IsBigEndian()) bmfh.SwitchEndianess();

	return bmfh;
}

BMIH GetBMIH(const string& szFileNameIn)
{
	BMFH bmfh;
	BMIH bmih;

	FILE* fp = fopen(szFileNameIn.c_str(), "rb");
	if (not fp) {
		if (g_exceptions) {
			throw runtime_error("EasyBMP::GetBMIH: cannot initialize from file " + szFileNameIn + ". "
								"File cannot be opened or does not exist.");
		}
		return bmih;
	}

	// read the bmfh, i.e., first 14 bytes (just to get it out of the way);

	ebmpBYTE TempBYTE;
	for (int i = 14; i > 0; i--) fread((char*) &TempBYTE, sizeof(ebmpBYTE), 1, fp);

	// read the bmih

	fread((char*) &(bmih.biSize), sizeof(ebmpDWORD), 1, fp);
	fread((char*) &(bmih.biWidth), sizeof(ebmpDWORD), 1, fp);
	fread((char*) &(bmih.biHeight), sizeof(ebmpDWORD), 1, fp);
	fread((char*) &(bmih.biPlanes), sizeof(ebmpWORD), 1, fp);

	fread((char*) &(bmih.biBitCount), sizeof(ebmpWORD), 1, fp);
	fread((char*) &(bmih.biCompression), sizeof(ebmpDWORD), 1, fp);
	fread((char*) &(bmih.biSizeImage), sizeof(ebmpDWORD), 1, fp);
	fread((char*) &(bmih.biXPelsPerMeter), sizeof(ebmpDWORD), 1, fp);

	fread((char*) &(bmih.biYPelsPerMeter), sizeof(ebmpDWORD), 1, fp);
	fread((char*) &(bmih.biClrUsed), sizeof(ebmpDWORD), 1, fp);
	fread((char*) &(bmih.biClrImportant), sizeof(ebmpDWORD), 1, fp);

	fclose(fp);

	if (IsBigEndian()) bmih.SwitchEndianess();

	return bmih;
}

void DisplayBitmapInfo(const string& szFileNameIn)
{
	FILE* fp = fopen(szFileNameIn.c_str(),"rb");
	if (not fp) {
		if (g_exceptions) {
			throw runtime_error("EasyBMP::DisplayBitmapInfo: cannot initialize from file " + szFileNameIn + ". "
								"File cannot be opened or does not exist.");
		}
		return;
	}
	fclose(fp);

	// don't duplicate work! Just use the functions from above!

	BMFH bmfh = GetBMFH(szFileNameIn);
	BMIH bmih = GetBMIH(szFileNameIn);

	cout << "File information for file " << szFileNameIn
		 << ":" << endl << endl;

	cout << "BITMAPFILEHEADER:" << endl
		 << "bfType: " << bmfh.bfType << endl
		 << "bfSize: " << bmfh.bfSize << endl
		 << "bfReserved1: " << bmfh.bfReserved1 << endl
		 << "bfReserved2: " << bmfh.bfReserved2 << endl
		 << "bfOffBits: " << bmfh.bfOffBits << endl << endl;

	cout << "BITMAPINFOHEADER:" << endl
		 << "biSize: " << bmih.biSize << endl
		 << "biWidth: " << (int) bmih.biWidth << endl
		 << "biHeight: " << (int) bmih.biHeight << endl
		 << "biPlanes: " << bmih.biPlanes << endl
		 << "biBitCount: " << bmih.biBitCount << endl
		 << "biCompression: " << bmih.biCompression << endl
		 << "biSizeImage: " << bmih.biSizeImage << endl
		 << "biXPelsPerMeter: " << bmih.biXPelsPerMeter << endl
		 << "biYPelsPerMeter: " << bmih.biYPelsPerMeter << endl
		 << "biClrUsed: " << bmih.biClrUsed << endl
		 << "biClrImportant: " << bmih.biClrImportant << endl << endl;
}

int GetBitmapColorDepth(const string& szFileNameIn)
{
	BMIH bmih = GetBMIH(szFileNameIn);
	return (int) bmih.biBitCount;
}

void PixelToPixelCopy(BMP& From, int FromX, int FromY,
                      BMP& To, int ToX, int ToY)
{
	To(ToX,ToY) = From(FromX,FromY);
}

void PixelToPixelCopyTransparent(BMP& From, int FromX, int FromY,
                                 BMP& To, int ToX, int ToY,
                                 RGBApixel& Transparent)
{
	if (From(FromX,FromY).Red != Transparent.Red or
		From(FromX,FromY).Green != Transparent.Green or
		From(FromX,FromY).Blue != Transparent.Blue)
	{
		To(ToX,ToY) = From(FromX,FromY);
	}
}

void RangedPixelToPixelCopy(BMP& From, int FromL , int FromR, int FromB, int FromT,
                            BMP& To, int ToX, int ToY )
{
	// make sure the conventions are followed
	if (FromB < FromT )
	{ int Temp = FromT; FromT = FromB; FromB = Temp; }

	// make sure that the copied regions exist in both bitmaps
	if (FromR >= abs(From.TellWidth())) FromR = abs(From.TellWidth()) - 1;
	if (FromL < 0) FromL = 0;

	if (FromB >= abs(From.TellHeight())) FromB = abs(From.TellHeight()) - 1;
	if (FromT < 0) FromT = 0;

	if (ToX + (FromR - FromL) >= abs(To.TellWidth()))  FromR = abs(To.TellWidth())  - 1 + FromL - ToX;
	if (ToY + (FromB - FromT) >= abs(To.TellHeight())) FromB = abs(To.TellHeight()) - 1 + FromT - ToY;

	int i, j;
	for (j = FromT; j <= FromB; j++) {
		for (i = FromL; i <= FromR; i++) {
			PixelToPixelCopy(From, i, j,
							 To, ToX + (i - FromL), ToY + (j - FromT));
		}
	}
}

void RangedPixelToPixelCopyTransparent(
     BMP& From, int FromL , int FromR, int FromB, int FromT,
     BMP& To, int ToX, int ToY,
     RGBApixel& Transparent)
{
	// make sure the conventions are followed
	if (FromB < FromT)	{ int Temp = FromT; FromT = FromB; FromB = Temp; }

	// make sure that the copied regions exist in both bitmaps
	if (FromR >= abs(From.TellWidth()))	{ FromR = abs(From.TellWidth()) - 1; }
	if (FromL < 0) { FromL = 0; }

	if (FromB >= abs(From.TellHeight()))	{ FromB = abs(From.TellHeight()) - 1; }
	if (FromT < 0) { FromT = 0; }

	if (ToX + (FromR - FromL) >= abs(To.TellWidth()))  { FromR = abs(To.TellWidth()) - 1 + FromL - ToX; }
	if (ToY + (FromB - FromT) >= abs(To.TellHeight())) { FromB = abs(To.TellHeight()) - 1 + FromT - ToY; }

	int i, j;
	for (j = FromT; j <= FromB; j++) {
		for (i = FromL; i <= FromR; i++) {
			PixelToPixelCopyTransparent(From, i, j,
										To, ToX + (i - FromL), ToY + (j - FromT),
										Transparent);
		}
	}
}

bool CreateGrayscaleColorTable( BMP& InputImage )
{
	int BitDepth = InputImage.TellBitDepth();
	if (BitDepth != 1 and BitDepth != 4 and BitDepth != 8) {
		if (g_exceptions) {
			throw runtime_error("EasyBMP: attempted to create color table at a bit depth that does not require a color table.");
		}
		return false;
	}
	int i;
	int NumberOfColors = InputImage.TellNumberOfColors();

	ebmpBYTE StepSize = BitDepth != 1 ? (255 / (NumberOfColors - 1)) : 255;

	for (i = 0; i < NumberOfColors; i++ ) {
		ebmpBYTE TempBYTE = i * StepSize;
		RGBApixel TempColor;
		TempColor.Red = TempBYTE;
		TempColor.Green = TempBYTE;
		TempColor.Blue = TempBYTE;
		TempColor.Alpha = 0;
		InputImage.SetColor(i, TempColor);
	}
	return true;
}

bool BMP::Read32bitRow(ebmpBYTE* Buffer, int BufferSize, int Row)
{
	if (Width * 4 > BufferSize) return false;

	for (int i = 0; i < Width; i++) {
		int x = HorizontalFlip ? Width -1 -i : i;
		memcpy((char*) &(Pixels[x][Row]), (char*) Buffer+4 * i, 4);
	}
	return true;
}

bool BMP::Read24bitRow( ebmpBYTE* Buffer, int BufferSize, int Row )
{
	if (Width * 3 > BufferSize) return false;

	for (int i = 0; i < Width; i++) {
		int x = HorizontalFlip ? Width -1 -i : i;
		memcpy((char*) &(Pixels[x][Row]), Buffer + 3 * i, 3);
	}
	return true;
}

bool BMP::Read8bitRow(ebmpBYTE* Buffer, int BufferSize, int Row)
{
	if (Width > BufferSize) return false;

	for (int i = 0; i < Width; i++) {
		int Index = Buffer[i];
		int x = HorizontalFlip ? Width -1 -i : i;
		RGBApixel p = GetColor(Index);
		this->operator()(x,Row) = p;
	}
	return true;
}

bool BMP::Read4bitRow(ebmpBYTE* Buffer, int BufferSize, int Row)
{
	int Shifts[2] = {   4,  0 };
	int Masks[2]  = { 240, 15 };

	int i = 0;
	int j;
	int k = 0;

	if (Width > 2 * BufferSize) return false;
	while (i < Width) {
		j = 0;
		while (j < 2 and i < Width) {
			int Index = (int) ((Buffer[k] & Masks[j]) >> Shifts[j]);
			int x = HorizontalFlip ? Width -1 -i : i;
			this->operator()(x, Row) = GetColor(Index);
			i++; j++;
		}
		k++;
	}
	return true;
}

bool BMP::Read1bitRow(ebmpBYTE* Buffer, int BufferSize, int Row)
{
	int Shifts[8] = {  7,  6,  5,  4, 3, 2, 1, 0};
	int Masks[8]  = {128, 64, 32, 16, 8, 4, 2, 1};

	int i = 0;
	int j;
	int k = 0;

	if (Width > 8 * BufferSize) return false;
	while (i < Width) {
		j = 0;
		while (j < 8 and i < Width) {
			int Index = (int) ((Buffer[k] & Masks[j]) >> Shifts[j]);
			int x = HorizontalFlip ? Width -1 -i : i;
			this->operator()(x,Row) = GetColor(Index);
			i++; j++;
		}
		k++;
	}
	return true;
}

bool BMP::Write32bitRow(ebmpBYTE* Buffer, int BufferSize, int Row)
{
	if (Width * 4 > BufferSize) return false;

	for (int i = 0; i < Width; i++) {
		int col = HorizontalFlip ? Width -1 -i : i;
		memcpy((char*) Buffer + 4 * col, (char*) &(Pixels[col][Row]), 4);
	}
	return true;
}

bool BMP::Write24bitRow(ebmpBYTE* Buffer, int BufferSize, int Row)
{
	if (Width * 3 > BufferSize) return false;

	for (int i = 0; i < Width; i++) {
		int col = HorizontalFlip ? Width -1 -i : i;
		memcpy((char*) Buffer + 3 * col, (char*) &(Pixels[col][Row]), 3);
	}
	return true;
}

bool BMP::Write8bitRow(  ebmpBYTE* Buffer, int BufferSize, int Row )
{
	if (Width > BufferSize) return false;

	for (int i = 0; i < Width; i++)
	{
		int col = HorizontalFlip ? Width -1 -i : i;
		ebmpBYTE d = FindClosestColor(Pixels[col][Row]);
		Buffer[i] = d;
	}
	return true;
}

bool BMP::Write4bitRow(ebmpBYTE* Buffer, int BufferSize, int Row)
{
	if (Width > 2 * BufferSize) return false;

	static int PositionWeights[2] = { 16, 1 };

	int i = 0, j, k = 0;

	while (i < Width) {
		j = 0;
		int Index = 0;
		int col = HorizontalFlip ? Width -1 -i : i;
		while (j < 2 and i < Width) {
			Index += (PositionWeights[j] * (int) FindClosestColor(Pixels[col][Row]));
			i++; j++;
		}
		Buffer[k] = (ebmpBYTE) Index;
		k++;
	}
	return true;
}

bool BMP::Write1bitRow(ebmpBYTE* Buffer, int BufferSize, int Row)
{
	static int PositionWeights[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };

	if (Width > 8 * BufferSize) return false;

	int i = 0, j, k = 0;

	while (i < Width) {
		j = 0;
		int Index = 0;
		int col = HorizontalFlip ? Width -1 -i : i;
		while (j < 8 and i < Width) {
			Index += (PositionWeights[j] * (int) FindClosestColor( Pixels[col][Row] ));
			i++; j++;
		}
		Buffer[k] = (ebmpBYTE) Index;
		k++;
	}
	return true;
}

ebmpBYTE BMP::FindClosestColor(RGBApixel& input)
{
	int NumberOfColors = TellNumberOfColors();
	ebmpBYTE BestI = 0;
	int BestMatch = 999999;

	int i = 0;
	while (i < NumberOfColors) {
		RGBApixel Attempt = GetColor(i);
		int TempMatch = IntSquare((int) Attempt.Red - (int) input.Red)
			+ IntSquare( (int) Attempt.Green - (int) input.Green )
			+ IntSquare( (int) Attempt.Blue - (int) input.Blue );
		if (TempMatch < BestMatch) {
			BestI = (ebmpBYTE) i;
			BestMatch = TempMatch;
		}
		if (BestMatch < 1) { i = NumberOfColors; }
		i++;
	}
	return BestI;
}

bool EasyBMPcheckDataSize(void)
{
	bool ReturnValue = true;
	if (sizeof(ebmpBYTE) != 1) {
		if (g_exceptions) {
			throw runtime_error("EasyBMP::EasyBMPcheckDataSize: ebmpBYTE has the wrong size (" + to_string(sizeof(ebmpBYTE)) +
								" bytes), compared to the expected 1 byte value");
		}
		ReturnValue = false;
	}
	if (sizeof(ebmpWORD) != 2) {
		if (g_exceptions) {
			throw runtime_error("EasyBMP::EasyBMPcheckDataSize: ebmpWORD has the wrong size (" + to_string(sizeof(ebmpWORD)) +
								" bytes), compared to the expected 2 byte value");
		}
		ReturnValue = false;
	}
	if (sizeof(ebmpDWORD) != 4) {
		if (g_exceptions) {
			throw runtime_error("EasyBMP::EasyBMPcheckDataSize: ebmpDWORD has the wrong size (" + to_string(sizeof(ebmpDWORD)) +
								" bytes), compared to the expected 4 byte value");
		}
		ReturnValue = false;
	}
	return ReturnValue;
}

bool Rescale(BMP& InputImage, char mode, int NewDimension)
{
	int CapMode = toupper(mode);

	BMP OldImage(InputImage);

	if (CapMode != 'P' and
		CapMode != 'W' and
		CapMode != 'H' and
		CapMode != 'F')
	{
		if (g_exceptions) {
			throw runtime_error("EasyBMP::Rescale: unknown rescale mode " + to_string(mode) + " requested");
		}
		return false;
	}

	int NewWidth  = 0;
	int NewHeight = 0;

	int OldWidth = OldImage.TellWidth();
	int OldHeight= OldImage.TellHeight();

	if (CapMode == 'P')	{
		NewWidth = (int) floor( OldWidth * NewDimension / 100.0 );
		NewHeight = (int) floor( OldHeight * NewDimension / 100.0 );
	}
	if (CapMode == 'F') {
		if (OldWidth > OldHeight)
			CapMode = 'W';
		else
			CapMode = 'H';
	}

	if (CapMode == 'W') {
		double percent = (double) NewDimension / (double) OldWidth;
		NewWidth = NewDimension;
		NewHeight = (int) floor(OldHeight * percent);
	}
	if (CapMode == 'H') {
		double percent = (double) NewDimension / (double) OldHeight;
		NewHeight = NewDimension;
		NewWidth = (int) floor( OldWidth * percent );
	}

	if (NewWidth < 1) NewWidth = 1;
	if (NewHeight < 1 ) NewHeight = 1;

	InputImage.SetSize(NewWidth, NewHeight);
	InputImage.SetBitDepth(24);

	int I, J;
	double ThetaI,ThetaJ;

	for (int j = 0; j < NewHeight - 1; j++)	{
		ThetaJ = (double)(j * (OldHeight - 1.0)) / (double)(NewHeight - 1.0);
		J = (int) floor(ThetaJ);
		ThetaJ -= J;

		for (int i = 0; i < NewWidth - 1; i++) {
			ThetaI = (double)(i * (OldWidth - 1.0)) / (double)(NewWidth - 1.0);
			I = (int) floor(ThetaI);
			ThetaI -= I;

			InputImage(i, j).Red = (ebmpBYTE)
			( (1.0 - ThetaI - ThetaJ + ThetaI * ThetaJ) * (OldImage(I,J).Red)
			 + (ThetaI - ThetaI * ThetaJ) * (OldImage(I+1, J).Red)
			 + (ThetaJ - ThetaI * ThetaJ) * (OldImage(I, J+1).Red)
			 + (ThetaI * ThetaJ) * (OldImage(I+1, J+1).Red) );
			InputImage(i, j).Green = (ebmpBYTE)
			( (1.0 - ThetaI - ThetaJ + ThetaI * ThetaJ) * OldImage(I,J).Green
			 + (ThetaI - ThetaI * ThetaJ) * OldImage(I+1, J).Green
			 + (ThetaJ - ThetaI * ThetaJ) * OldImage(I, J+1).Green
			 + (ThetaI * ThetaJ) * OldImage(I+1, J+1).Green );
			InputImage(i, j).Blue = (ebmpBYTE)
			( (1.0 - ThetaI - ThetaJ + ThetaI * ThetaJ) * OldImage(I,J).Blue
			 + (ThetaI - ThetaI * ThetaJ) * OldImage(I+1, J).Blue
			 + (ThetaJ - ThetaI * ThetaJ) * OldImage(I, J+1).Blue
			 + (ThetaI * ThetaJ) * OldImage(I+1, J+1).Blue );
		}
		InputImage(NewWidth - 1, j).Red   = (ebmpBYTE) ((1.0 - ThetaJ) * (OldImage(OldWidth - 1, J).Red)   + ThetaJ * (OldImage(OldWidth-1, J+1).Red));
		InputImage(NewWidth - 1, j).Green = (ebmpBYTE)	((1.0 - ThetaJ) * (OldImage(OldWidth - 1, J).Green) + ThetaJ * (OldImage(OldWidth-1, J+1).Green));
		InputImage(NewWidth - 1, j).Blue  = (ebmpBYTE) ((1.0 - ThetaJ) * (OldImage(OldWidth - 1, J).Blue)  + ThetaJ * (OldImage(OldWidth-1, J+1).Blue));
	}

	for (int i = 0; i < NewWidth-1 ; i++) {
		ThetaI = (double)(i * (OldWidth - 1.0)) / (double)(NewWidth - 1.0);
		I = (int) floor(ThetaI);
		ThetaI -= I;
		InputImage(i, NewHeight - 1).Red   = (ebmpBYTE) ((1.0 - ThetaI) * (OldImage(I, OldHeight - 1).Red)   + ThetaI * (OldImage(I, OldHeight - 1).Red));
		InputImage(i, NewHeight - 1).Green = (ebmpBYTE) ((1.0 - ThetaI) * (OldImage(I, OldHeight - 1).Green) + ThetaI * (OldImage(I, OldHeight - 1).Green));
		InputImage(i, NewHeight - 1).Blue  = (ebmpBYTE) ((1.0 - ThetaI) * (OldImage(I, OldHeight - 1).Blue)  + ThetaI * (OldImage(I, OldHeight - 1).Blue));
	}

	InputImage(NewWidth - 1, NewHeight - 1) = OldImage(OldWidth - 1, OldHeight - 1);
	return true;
}
