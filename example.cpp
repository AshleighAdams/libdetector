// STL includes
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <string.h>
#include <vector>
#include <list>
#include <cmath>

// OpenCV includes
using namespace std; // OpenCV needs this else we get 500 errors....
#include "opencv/cv.h"
#include "opencv/highgui.h"

// Detector includes (must be after OpenCV)
#define DETECTOR_OPENCV
#include "libdetector/include/libdetector.h"

#define XY_LOOP_START(_x_,_y_,_endx_,_endy_) \
	for(int y = _y_; y < _endy_; y++)\
	for(int x = _x_; x < _endx_; x++)

// SFML and GWEN shit
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>

#include "Gwen/Gwen.h"
#include "Gwen/BaseRender.h"
#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"

#include "Gwen/Renderers/SFML.h"
#include "Gwen/Input/SFML.h"

#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"

#include "Gwen/Controls/WindowControl.h"
#include "Gwen/Controls/Button.h"
#include "Gwen/Controls/CheckBox.h"
#include "Gwen/Controls/TextBox.h"
#include "Gwen/Controls/ImagePanel.h"

using namespace Detector;

const int TrailLength = 40;

struct ObjectTrail
{
	int count;
	int current_position;
	position_t* positions;
};

ObjectTrail* Trails[1024];

void NewObject(CTrackedObject* Obj)
{
	cout << "Tracking new object! @ " << GetCurrentTime() << "\n";
	int idx = Obj->ID() % 1024;

	ObjectTrail* Trail = new ObjectTrail;
	Trail->count = 0;
	Trail->current_position = 0;
	Trail->positions = new position_t[TrailLength];

	Trails[idx] = Trail;
}

void LostObject(CTrackedObject* Obj)
{
	cout << "Lost object! @ " << GetCurrentTime() << "\n";
	int idx = Obj->ID() % 1024;
	delete [] Trails[idx]->positions;
	delete Trails[idx];
}

void Update(CTrackedObject* Obj, bool Simulated)
{
	int idx = Obj->ID() % 1024;
	ObjectTrail* Trail = Trails[idx];

	position_t pos = Obj->CenterPosition();

	Trail->positions[Trail->current_position].x = pos.x;
	Trail->positions[Trail->current_position].y = pos.y;

	Trail->current_position = (Trail->current_position + 1) % TrailLength;
	Trail->count = min(TrailLength, Trail->count + 1);
}

void DrawTrails(CDetectorImage* Img, CTrackedObject* Obj)
{
	int idx = Obj->ID() % 1024;
	ObjectTrail* Trail = Trails[idx];
	if(Trail->count <= 1) return;

	position_t last;
	int start = Trail->current_position;
	int i = start;
	bool doneone = false;
	while(true)
	{
		if(start == i && doneone) break;
		if(i >= Trail->count) i = 0;
		if(start == i && doneone) break;

		position_t pos = Trail->positions[i];

		if(doneone)
			Img->DrawLine(pos, last);

		doneone = true;
		last = pos;
		i++;
	}
}

CDetector* g_pDetector = NULL;
CObjectTracker* g_pTracker = NULL;

// Globals
bool 				g_bWriteFrame = false;
bool 				g_bExiting = false;
short 				g_Sensetivity;
double 				g_NextTime;
target_t* 			g_Targets[MAX_TARGETS];
int 				g_Count = 0;

color_t 			Red;
color_t 			Green;
color_t 			Blue;
color_t 			Orange;

CDetectorImage* 	g_pDetectorImage;
CBaseDescriptor*	g_pDesc;

// OpenCV
CvCapture* 			g_pCapture;

// GUI globals
Gwen::Controls::Canvas* 			g_pCanvas;
Gwen::Controls::WindowControl* 		g_pWindowCam;
Gwen::Controls::WindowControl* 		g_pWindowSet;
Gwen::Controls::ImagePanel*			g_pImage;
Gwen::Controls::CheckBoxWithLabel* 	g_pCeckBoxRecord;

void Init(imagesize_t size)
{
	Red.r = 255;
	Red.g = 0;
	Red.b = 0;

	Green.r = 0;
	Green.g = 255;
	Green.b = 0;

	Blue.r = 0;
	Blue.g = 0;
	Blue.b = 255;
	
	Orange.r = 255;
	Orange.g = 165;
	Orange.b = 0;
	
	g_pDetector = new CDetector(size);
	g_pDetector->SetDiffrenceThreshold(65.f);

	g_pDesc = new CBaseDescriptor();
	
	CDetectorImage* Person = CDetectorImage::FromFile("person.xdi");
	if(Person)
	{
		g_pDesc->LoadDescriptor(Person);
		Person->UnRefrence();
	}else cout << "Coudln't load person.xdi!\n";
	
	g_pDetector->SetDescriptor(g_pDesc);


	g_pTracker = new CObjectTracker();
	g_pTracker->SetEvent(EVENT_NEWTARG,	(void*)&NewObject);
	g_pTracker->SetEvent(EVENT_LOST,	(void*)&LostObject);
	g_pTracker->SetEvent(EVENT_UPDATE,	(void*)&Update);
}

bool ProccessFrame(CDetectorImage* Image)
{
	if(!g_pDetector) // Havn't loaded yet
		Init(Image->GetSize());
	
	g_pDetector->PushImage(Image);
	g_Count = g_pDetector->GetTargets(g_Targets);

	g_pTracker->PushTargets(g_Targets, g_Count);

	TrackedObjects Objs = *g_pTracker->GetTrackedObjects();


	Image->DrawColor(Red);
	
	for(int i = 0; i < g_Count; i++)
		Image->DrawTarget(g_Targets[i]);		

	Image->DrawColor(Green);
	
	bool ret = false;
	for(CTrackedObject* Obj : Objs)
	{
		ret = true;
		Image->DrawTarget(Obj);
		DrawTrails(Image, Obj);
	}
	
	if(g_Count > 0 && false)
	{
		target_t* targ = g_Targets[0];
		motion_t* mot = g_pDetector->GetMotionImage();
		
		int startx = mot->size.width * targ->x;
		int starty = mot->size.height * targ->y;
		
		int endx = mot->size.width * (targ->x + targ->width);
		int endy = mot->size.height * (targ->y + targ->height);
		
		int w = endx - startx;
		int h = endy - starty;
		
		CDetectorImage* img = new CDetectorImage(w, h);
		
		pixel_t* pix;
		int col;
		
		XY_LOOP_START(startx, starty, endx, endy)
		{
			col = PMOTION_XY(mot, x, y) > 0 ? 255 : 0;
			pix = img->Pixel(x - startx, y - starty);
			pix->r = col;
			pix->g = col;
			pix->b = col;
		}
		
		char* str = new char[255];
		sprintf(str, "save_%f.xdi", GetCurrentTime() );
		
		img->Save(str);
		img->UnRefrence();
		
		delete str;
	}
	

	Image->DrawColor(Blue);

	if(g_pDetector->GetNumberOfTargets() > 0)
	{
		position_t pos1;
		position_t pos2;

		for(int i = 0; i < 360; i++)
		{
			pos1.x = (float)i / 360.f;
			pos1.y = 0.f;
			if(i > 0)
			{
				pos1.y = 1.f - g_pDesc->m_Histogram.values[(i + g_pDesc->m_Histogram.highestvalue) % 360] * 0.25;
				Image->DrawLine(pos1, pos2);
			}
			pos2.x = pos1.x;
			pos2.y = pos1.y;
		}
	}
	position_t pos1;
	position_t pos2;
	
	Image->DrawColor(Orange);

	for(int i = 0; i < 360; i++)
	{
		pos1.x = (float)i / 360.f;
		pos1.y = 0.f;
		if(i > 0)
		{
			pos1.y = 1.f - g_pDesc->m_PersonHisto.values[(i + g_pDesc->m_PersonHisto.highestvalue) % 360] * 0.25;
			Image->DrawLine(pos1, pos2);
		}
		pos2.x = pos1.x;
		pos2.y = pos1.y;
	}
	return ret;
}

// All the camera stuffs
bool g_bCapThreadRunning = true;
bool g_bCapThreadClosed = false;

void CameraThread()
{
	IplImage* frame = cvQueryFrame( g_pCapture );
	if ( !frame )
	{
		fprintf( stderr, "ERROR: frame is null...\n" );
		g_bCapThreadClosed = true;
		return;
	}

	// let the cam initiate
	double sleeptime = GetCurrentTime();
	while(sleeptime + 5 > GetCurrentTime()) frame = cvQueryFrame( g_pCapture );

	CvSize imgSize;
	imgSize.width = frame->width;
	imgSize.height = frame->height;

	double fps = 10.0;
	CvVideoWriter *writer = cvCreateVideoWriter(
			"out.avi",
			CV_FOURCC('M', 'J', 'P', 'G'),
			fps,
			imgSize
			);
	while(g_bCapThreadRunning)
	{
		// Get one frame
		frame = cvQueryFrame( g_pCapture );
		if ( !frame )
		{
			fprintf( stderr, "ERROR: frame is null...\n" );
			break;
		}

		if(!g_pDetectorImage)
			g_pDetectorImage = new CDetectorImage(frame->width, frame->height);
		UpdateFrame(frame, g_pDetectorImage);

		bool WriteFrame = ProccessFrame(g_pDetectorImage);
		

		if(WriteFrame && g_bWriteFrame)
			cvWriteFrame(writer, frame);
			
		// Lets update the GUI capture thread texture thing
		sf::Image* tex = static_cast<sf::Image*>( g_pImage->_GetTexture()->data );
		sf::Color col;
		color_t* pix;
		XY_LOOP(frame->width, frame->height)
		{
			pix = g_pDetectorImage->Pixel(x, y);
			col.r = pix->r;
			col.g = pix->g;
			col.b = pix->b;
			tex->SetPixel(x, y, (const sf::Color)col);
		}
	}
	cvReleaseVideoWriter(&writer);
	g_bCapThreadClosed = true;
}

int main()
{
	g_pCapture = cvCaptureFromCAM( 1 );

	if ( !g_pCapture )
	{
		fprintf( stderr, "ERROR: capture is NULL \n" );
		return 1;
	}
	
	// Lower res = faster processing time.
	cvSetCaptureProperty(g_pCapture, CV_CAP_PROP_FRAME_WIDTH, 320);
	cvSetCaptureProperty(g_pCapture, CV_CAP_PROP_FRAME_HEIGHT, 240);

	// Init the Gwen GUI and SFML window
	sf::RenderWindow App( sf::VideoMode( 640, 480, 32 ), "Detector", sf::Style::Close );
	Gwen::Renderer::SFML GwenRenderer( App );
	Gwen::Skin::TexturedBase skin;
	skin.SetRender( &GwenRenderer );
	skin.Init( "DefaultSkin.png" );
	skin.SetDefaultFont( L"OpenSans.ttf", 11 );
	
	g_pCanvas = new Gwen::Controls::Canvas( &skin );
	
	g_pCanvas->SetSize( App.GetWidth(), App.GetHeight() );
	g_pCanvas->SetDrawBackground( true );
	g_pCanvas->SetBackgroundColor( Gwen::Color( 150, 170, 170, 255 ) );
	
	g_pWindowCam = new Gwen::Controls::WindowControl(g_pCanvas);
	g_pWindowCam->SetSize( 320, 240 + 25 );
	g_pWindowCam->SetClosable(false);
	g_pWindowCam->SetTitle("Camera");
	g_pWindowCam->SetPos(30, 30);
	
	g_pWindowSet = new Gwen::Controls::WindowControl(g_pCanvas);
	g_pWindowSet->SetSize( 200, 240 );
	g_pWindowSet->SetClosable(false);
	g_pWindowSet->SetTitle("Settings");
	g_pWindowSet->SetPos(355, 30);
	
	g_pCeckBoxRecord = new Gwen::Controls::CheckBoxWithLabel(g_pWindowSet);
	g_pCeckBoxRecord->Label()->SetText("Record Motion", false);
	//g_pCeckBoxRecord->Checkbox()->IsChecked();
	g_pCeckBoxRecord->SetPos(5, 5);
	
	g_pImage = new Gwen::Controls::ImagePanel(g_pWindowCam);
	g_pImage->SetImage("loading.png");
	g_pImage->SizeToContents();
	
	const sf::Image* tex = static_cast<sf::Image*>( g_pImage->_GetTexture()->data );
	if ( !tex )
	{
		cout << "ERROR LOADING 'loading.png'\n";
		return 1;
	}
	
	//sf::Color col = tex->GetPixel( x, y );
	
	Gwen::Input::SFML GwenInput;
	GwenInput.Initialize( g_pCanvas );
	
	// Now lets start the Capture thread
	thread thrd(CameraThread);

	// Show the image captured from the camera in the window and repeat
	while ( App.IsOpened() )
	{
		// Handle events
		sf::Event Event;
		while ( App.GetEvent(Event) )
		{
			if ((Event.Type == sf::Event::Closed) || ((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::Escape)))
			{
				App.Close();
				break;
			}
			GwenInput.ProcessMessage( Event );
		}
		
		App.Clear();
		
		g_pCanvas->RenderCanvas();
		App.Display();
		cvWaitKey(1);
	}
	
	cvReleaseCapture( &g_pCapture );
	return 0;
}
