/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
* can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement (which
* also govern the use of this file).  You may share a modified version of this
* file with another authorized licensee of Derivative's TouchDesigner software.
* Otherwise, no redistribution or sharing of this file, with or without
* modification, is permitted.
*/

#include "cinder/gl/gl.h"
#include "cinder/app/AppBase.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/Shader.h"
#include "TOP_CPlusPlusBase.h"

using namespace ci;
using namespace std;

class CinderCPlusPlusTOPApp : public TOP_CPlusPlusBase
{
public:

	CinderCPlusPlusTOPApp(const OP_NodeInfo *info);
	virtual ~CinderCPlusPlusTOPApp();

	virtual void		getGeneralInfo(TOP_GeneralInfo *) override;
	virtual bool		getOutputFormat(TOP_OutputFormat*) override;


	virtual void		execute(const TOP_OutputFormatSpecs*,
		OP_Inputs*,
		void* reserved) override;


	virtual int			getNumInfoCHOPChans() override;
	virtual void		getInfoCHOPChan(int index,
		OP_InfoCHOPChan *chan) override;

	virtual bool		getInfoDATSize(OP_InfoDATSize *infoSize) override;
	virtual void		getInfoDATEntries(int index,
		int nEntries,
		OP_InfoDATEntries *entries) override;

	virtual void		setupParameters(OP_ParameterManager *manager) override;
	virtual void		pulsePressed(const char *name) override;

private:

	// We don't need to store this pointer, but we do for the example.
	// The OP_NodeInfo class store information about the node that's using
	// this instance of the class (like its name).
	const OP_NodeInfo		*myNodeInfo;

	// In this example this value will be incremented each time the execute()
	// function is called, then passes back to the TOP 
	int						 myExecuteCount;
	double					 myRotation;

	bool								mShouldQuit;

	HWND wnd;

	HDC__ *mDC;
	HGLRC__ *mRC;

	ci::gl::ContextRef mCinderContext;

	mat4				mCubeRotation;
	ci::gl::FboRef		mFBO;
};

/*
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CinderCPlusPlusTOPApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void CinderCPlusPlusTOPApp::setup()
{
}

void CinderCPlusPlusTOPApp::mouseDown( MouseEvent event )
{
}

void CinderCPlusPlusTOPApp::update()
{
}

void CinderCPlusPlusTOPApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( CinderCPlusPlusTOPApp, RendererGl )
*/