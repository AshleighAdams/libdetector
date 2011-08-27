#include "string.h"
#include <iostream>
#include <fstream>
#include <exception>

#define _USE_MATH_DEFINES
#include <cmath>

#define DETECTOR_INTERNAL
#include "../include/libdetector.h"
using namespace Detector;
using namespace std;

CDetectorImage* CDetectorImage::FromFile(char* File)
{
	char buffer[128];
	ifstream file (File, ios::in|ios::binary);
	
	if(!file.is_open()) return NULL;
	
	// Make sure the header is correct
	char head[3];
	file.read(head, 3);
	
	if(head[0] != 'X' || head[1] != 'D' || head[2] != 'I')
	{
		file.close();
		return NULL;
	}
	
	
	imagesize_t size;
	
	file.read(buffer, sizeof(int));
	size.width = atoi(buffer);
	
	file.read(buffer, sizeof(int));
	size.height = atoi(buffer);
	
	CDetectorImage* ret = new CDetectorImage(size);
	
	int datsize = (size.height * size.width) * 3; // 3 bytes
	unsigned char* data = new unsigned char[datsize];
	file.read((char*)data, datsize);
	
	pixel_t* pix;
	int pos;
	XY_LOOP(size.width, size.height)
	{
		pos = (x + y * size.width) * 3;
		pix = ret->Pixel(x,y);
		pix->r = (unsigned char)data[pos];
		pix->g = (unsigned char)data[pos + 1];
		pix->b = (unsigned char)data[pos + 2];
	}
	
	delete [] data;
	file.close();
	return ret;
}

bool CDetectorImage::Save(char* File)
{
	ofstream file (File, ios::out|ios::binary);
	
	if(!file.is_open()) return false;
	
	file.put('X');
	file.put('D');
	file.put('I');
	
	file.write((const char*)&m_sSize.width, sizeof(int));
	file.write((const char*)&m_sSize.height, sizeof(int));
	
	int datsize = (m_sSize.height * m_sSize.width) * 3;
	file.write((const char*)m_psPixels, datsize);
	file.close();
	
	return true;
}

CDetectorImage::CDetectorImage( int Width, int Height )
{
	this->Refrence();
	m_sSize.width = Width;
	m_sSize.height = Height;
	m_psPixels = new pixel_t[Width * Height];
}

CDetectorImage::CDetectorImage( imagesize_t size )
{
	this->Refrence();
	m_sSize.width = size.width;
	m_sSize.height = size.height;
	m_psPixels = new pixel_t[size.width * size.height];
}

CDetectorImage::~CDetectorImage()
{
	delete[] m_psPixels;
}

CDetectorImage* CDetectorImage::Exclusive()
{
	if( m_iRefrenceCount == 1 )
		return this;
	imagesize_t selfsize = this->GetSize();
	int size_total = ( selfsize.width * selfsize.height ) * 3; // 3 bytes per pixel;
	CDetectorImage* ptr = new CDetectorImage( selfsize );
	memcpy( ptr->m_psPixels, this->m_psPixels, size_total );
	this->UnRefrence();
	return ptr;
}

pixel_t* CDetectorImage::Pixel( int x, int y )
{
	return &m_psPixels[x + y * m_sSize.width];
}

imagesize_t CDetectorImage::GetSize()
{
	return m_sSize;
}

void CDetectorImage::DrawColor(color_t Color)
{
	m_DrawColor.r = Color.r;
	m_DrawColor.g = Color.g;
	m_DrawColor.b = Color.b;
}

#define SETPIX(_x_,_y_)\
	SETPIX_pix = Pixel(_x_,_y_);\
	SETPIX_pix->r = m_DrawColor.r;\
	SETPIX_pix->g = m_DrawColor.g;\
	SETPIX_pix->b = m_DrawColor.b
pixel_t* SETPIX_pix;

#define RESTRAIN(_x_) _x_ = max(0.f, min(1.f, _x_))

void CDetectorImage::DrawBox(position_t& a, position_t& b)
{
	RESTRAIN(a.x);
	RESTRAIN(a.y);

	RESTRAIN(b.x);
	RESTRAIN(b.y);

	int startx  = a.x * m_sSize.width + 1;
	int starty  = a.y * m_sSize.height;

	int endx	= b.x * m_sSize.width;
	int endy	= b.y * m_sSize.height + 1;

	startx = min(m_sSize.width -1, startx);
	endy = min(m_sSize.height -1, endy);

	for (int x = startx; x < endx; x++)
	{
		SETPIX(x,starty);
		SETPIX(x, endy);
	}
	for (int y = starty; y < endy; y++)
	{
		SETPIX(startx, y);
		SETPIX(endx, y);
	}
}

void CDetectorImage::DrawTarget(CTrackedObject* pObj)
{
	ssize_t size = pObj->Size();
	position_t a = pObj->Position();
	position_t b;

	b.x = a.x + size.w;
	b.y = a.y + size.h;

	DrawBox(a, b);
}

void CDetectorImage::DrawTarget(target_t* pTarget)
{
	position_t a;
	position_t b;

	a.x = pTarget->x;
	a.y = pTarget->y;

	b.x = pTarget->x + pTarget->width;
	b.y = pTarget->y + pTarget->height;

	DrawBox(a, b);
}

void CDetectorImage::DrawLine(position_t& a, position_t& b)
{
	RESTRAIN(a.x);
	RESTRAIN(a.y);

	RESTRAIN(b.x);
	RESTRAIN(b.y);

	int	 x0 = a.x * m_sSize.width,
			y0 = a.y * m_sSize.height,
			x1 = b.x * m_sSize.width,
			y1 = b.y * m_sSize.height;
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);

	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;

	int err = dx - dy;
	int e2;
	while(true)
	{
		SETPIX(x0,y0);
		if(x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if(e2 > -dy)
		{
			err -= dy;
			x0 += sx;
		}
		if(e2 < dx)
		{
			err += dx;
			y0 += sy;
		}
	}
}


// TODO: Put files in here
/*
function line(x0, y0, x1, y1)
   dx := abs(x1-x0)
   dy := abs(y1-y0)
   if x0 < x1 then sx := 1 else sx := -1
   if y0 < y1 then sy := 1 else sy := -1
   err := dx-dy

   loop
	 setPixel(x0,y0)
	 if x0 = x1 and y0 = y1 exit loop
	 e2 := 2*err
	 if e2 > -dy then
	   err := err - dy
	   x0 := x0 + sx
	 end if
	 if e2 <  dx then
	   err := err + dx
	   y0 := y0 + sy
	 end if
   end loop
   */
