﻿#include "Bitmap.h"

#include <fstream>
#include <iostream>
#include "formats/bitmap/BitmapFileHeader.h"
#include "formats/bitmap/BitmapInfoHeader.h"


using namespace ragii::image;
using namespace std;


unique_ptr<Bitmap> Bitmap::loadFromFile(string path)
{
	ifstream fs(path, ios::in | ios::binary);

	if (!fs)
	{
		cout << path << ": open failed." << endl;
		return nullptr;
	}

	auto fileSize = fs.seekg(0, ios::end).tellg();

	if (BitmapFileHeaderSize > fileSize)
	{
		cout << "invalid bitmap file header." << endl;
		return nullptr;
	}

	auto bmp = make_unique<Bitmap>();

	fs.seekg(0, ios::beg);
	fs.read(reinterpret_cast<char*>(&bmp->m_Header.File), BitmapFileHeaderSize);

	if (!((bmp->m_Header.File.Type & 0xff) == 'B') && (bmp->m_Header.File.Type >> 8) == 'M')
	{
		cout << "invalid bitmap file header." << endl;
		return nullptr;
	}

	auto remain = fileSize - static_cast<streampos>(BitmapFileHeaderSize);

	if (BitmapInfoHeaderSize > remain)
	{
		cout << "invalid bitmap info header." << endl;
		return bmp;
	}

	uint32_t infoSize = 0UL;
	fs.read(reinterpret_cast<char*>(&infoSize), sizeof(uint32_t));
	fs.seekg(-streamoff(sizeof(uint32_t)), ios::cur);

	if (infoSize == BitmapInfoHeaderSize)
	{
		fs.read(reinterpret_cast<char*>(&bmp->m_Header.Info), BitmapInfoHeaderSize);
	}

	remain -= infoSize;

	fs.seekg(streamoff(bmp->m_Header.File.OffBits), ios::beg);
	if (fs.bad())
	{
		return bmp;
	}

	uint32_t dataSize = fileSize - static_cast<streampos>(bmp->m_Header.File.OffBits);
	bmp->m_Data = make_unique<uint8_t[]>(dataSize);

	fs.read(reinterpret_cast<char*>(bmp->m_Data.get()), dataSize);
	fs.close();

	return bmp;
}

unique_ptr<Bitmap> Bitmap::create(int32_t width, int32_t height, int16_t bitCount)
{
	auto bmp = make_unique<Bitmap>();

	BitmapFileHeader file = {};
	file = {};
	file.Size = BitmapHeaderSize + width * height * bitCount / 8;
	file.Type = 'B' | ('M' << 8);
	file.OffBits = BitmapHeaderSize;
	memcpy(&bmp->m_Header.File, &file, BitmapFileHeaderSize);

	BitmapInfoHeader info = {};
	info.Size = BitmapInfoHeaderSize;
	info.BitCount = bitCount;
	info.Width = width;
	info.Height = height;
	info.Planes = 1;
	info.Compression = BI_RGB;
	info.SizeImage = width * height * bitCount / 8;
	memcpy(&bmp->m_Header.Info, &info, BitmapInfoHeaderSize);

	bmp->m_Data = make_unique<uint8_t[]>(bmp->m_Header.Info.SizeImage);

	return bmp;
}

void Bitmap::save(string path)
{
	auto& info = m_Header.Info;

	ofstream fs(path, ios::out | ios::binary);
	fs.write(reinterpret_cast<char*>(&m_Header.File), BitmapFileHeaderSize);
	fs.write(reinterpret_cast<char*>(&m_Header.Info), BitmapInfoHeaderSize);
	fs.write(reinterpret_cast<char*>(m_Data.get()), info.Width * info.Height * info.BitCount / 8);
	fs.flush();
	fs.close();
}

const BitmapHeader& Bitmap::getHeader() const
{
	return m_Header;
}

unique_ptr<uint8_t[]>& Bitmap::getData()
{
	return m_Data;
}

const unique_ptr<uint8_t[]>& Bitmap::getData() const
{
	return m_Data;
}

int32_t Bitmap::getWidth() const
{
	return m_Header.Info.Width;
}

int32_t Bitmap::getHeight() const
{
	return m_Header.Info.Height;
}

int16_t Bitmap::getBitCount() const
{
	return m_Header.Info.BitCount;
}

