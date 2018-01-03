/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
* can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement (which
* also govern the use of this file).  You may share a modified version of this
* file with another authorized licensee of Derivative's TouchDesigner software.
* Otherwise, no redistribution or sharing of this file, with or without
* modification, is permitted.
*/

#include "CinderCPlusPlusTOPApp.h"

using namespace ci;

// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

	DLLEXPORT
		int
		GetTOPAPIVersion(void)
	{
		// Always return TOP_CPLUSPLUS_API_VERSION in this function.
		return TOP_CPLUSPLUS_API_VERSION;
	}

	DLLEXPORT
		TOP_CPlusPlusBase*
		CreateTOPInstance(const OP_NodeInfo* info)
	{
		// Return a new instance of your class every time this is called.
		// It will be called once per TOP that is using the .dll
		return new CinderCPlusPlusTOPApp(info);
	}

	DLLEXPORT
		void
		DestroyTOPInstance(TOP_CPlusPlusBase* instance)
	{
		// Delete the instance here, this will be called when
		// Touch is shutting down, when the TOP using that instance is deleted, or
		// if the TOP loads a different DLL
		delete (CinderCPlusPlusTOPApp*)instance;
	}

};

CinderCPlusPlusTOPApp::CinderCPlusPlusTOPApp(const OP_NodeInfo* info) : myNodeInfo(info)
{
	mShouldQuit = false;
	myRotation = 0.0;
	myExecuteCount = 0;
	wnd = info->mainWindowHandle;
	mDC = wglGetCurrentDC();
	mRC = wglGetCurrentContext();

	gl::Environment::setCore();
	auto platformData = std::shared_ptr<gl::Context::PlatformData>(new gl::PlatformDataMsw(mRC, mDC));
	mCinderContext = gl::Context::createFromExisting(platformData);
	mCinderContext->makeCurrent();

	gl::enableDepthWrite();
	gl::enableDepthRead();
}

CinderCPlusPlusTOPApp::~CinderCPlusPlusTOPApp()
{

}

void
CinderCPlusPlusTOPApp::getGeneralInfo(TOP_GeneralInfo* ginfo)
{
	// Uncomment this line if you want the TOP to cook every frame even
	// if none of it's inputs/parameters are changing.
	ginfo->cookEveryFrame = true;
}

bool
CinderCPlusPlusTOPApp::getOutputFormat(TOP_OutputFormat* format)
{
	// In this function we could assign variable values to 'format' to specify
	// the pixel format/resolution etc that we want to output to.
	// If we did that, we'd want to return true to tell the TOP to use the settings we've
	// specified.
	// In this example we'll return false and use the TOP's settings
	return false;
}


void
CinderCPlusPlusTOPApp::execute(const TOP_OutputFormatSpecs* outputFormat,
	OP_Inputs* inputs,
	void* reserved)
{

	myExecuteCount++;

	double speed = inputs->getParDouble("Speed");


	double color1[3];
	double color2[3];

	inputs->getParDouble3("Color1", color1[0], color1[1], color1[2]);
	inputs->getParDouble3("Color2", color2[0], color2[1], color2[2]);


	myRotation += speed;

	int width = outputFormat->width;
	int height = outputFormat->height;

	int x = width / 2;
	int y = height / 2;
	mCubeRotation *= rotate(toRadians(0.2f), normalize(vec3(0, 1, 0)));
	gl::clear(Color(1, 0, 0));
	//unbind touch's FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//switch to Cinder's shared context
	mCinderContext->makeCurrent();
	if (!mFBO) {
		gl::Fbo::Format format;
		auto tType = outputFormat->colorBuffer0Type;
		auto index = outputFormat->colorBufferRB[0];
		format.attachment(GL_COLOR_ATTACHMENT0, gl::Texture2d::create(tType, index, outputFormat->width, outputFormat->height, true));
		mFBO = gl::Fbo::create(outputFormat->width, outputFormat->height, format);
	}

	{
		gl::ScopedFramebuffer fbScp(mFBO);
		gl::enableDepthWrite();
		gl::enableDepthRead();
		gl::clear(Color(1, .5, 0));

		// setup the viewport to match the dimensions of the FBO
		gl::ScopedViewport scpVp(ivec2(0), mFBO->getSize());

		// setup our camera to render the scene
		CameraPersp cam(mFBO->getWidth(), mFBO->getHeight(), 60.0f);
		cam.setPerspective(60, mFBO->getAspectRatio(), 1, 1000);
		cam.lookAt(vec3(2.8f, 1.8f, -2.8f), vec3(0));
		gl::setMatrices(cam);

		gl::ScopedGlslProg shaderScp(gl::getStockShader(gl::ShaderDef().color()));
		gl::color(Color(1.0f, 0.5f, 0.25f));

		gl::VertBatch vb(GL_TRIANGLES);
		vb.color(1, 0, 0);
		vb.vertex(outputFormat->width / 2, 50);
		vb.color(0, 1, 0);
		vb.vertex(outputFormat->width - 50, outputFormat->height - 50);
		vb.color(0, 0, 1);
		vb.vertex(50, outputFormat->height - 50);
		vb.draw();
		gl::color(Color::white());
	}

	wglMakeCurrent(mDC, mRC);

	//rebind touch's FBO and draw our FBO's texture
	glBindFramebuffer(GL_FRAMEBUFFER, outputFormat->FBOIndex);
	//glBindVertexArray(0);
}

int
CinderCPlusPlusTOPApp::getNumInfoCHOPChans()
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the TOP. In this example we are just going to send one channel.
	return 2;
}

void
CinderCPlusPlusTOPApp::getInfoCHOPChan(int index,
	OP_InfoCHOPChan* chan)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name = "executeCount";
		chan->value = (float)myExecuteCount;
	}

	if (index == 1)
	{
		chan->name = "rotation";
		chan->value = (float)myRotation;
	}
}

bool
CinderCPlusPlusTOPApp::getInfoDATSize(OP_InfoDATSize* infoSize)
{
	infoSize->rows = 2;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
CinderCPlusPlusTOPApp::getInfoDATEntries(int index,
	int nEntries,
	OP_InfoDATEntries* entries)
{
	// It's safe to use static buffers here because Touch will make it's own
	// copies of the strings immediately after this call returns
	// (so the buffers can be reuse for each column/row)
	static char tempBuffer1[4096];
	static char tempBuffer2[4096];

	if (index == 0)
	{
		// Set the value for the first column
		strcpy_s(tempBuffer1, "executeCount");
		entries->values[0] = tempBuffer1;

		// Set the value for the second column
		sprintf_s(tempBuffer2, "%d", myExecuteCount);
		entries->values[1] = tempBuffer2;
	}

	if (index == 1)
	{
		// Set the value for the first column
		strcpy_s(tempBuffer1, "rotation");
		entries->values[0] = tempBuffer1;

		// Set the value for the second column
		sprintf_s(tempBuffer2, "%g", myRotation);
		entries->values[1] = tempBuffer2;
	}
}

void
CinderCPlusPlusTOPApp::setupParameters(OP_ParameterManager* manager)
{
	// color 1
	{
		OP_NumericParameter	np;

		np.name = "Color1";
		np.label = "Color 1";

		for (int i = 0; i<3; i++)
		{
			np.defaultValues[i] = 1.0;
			np.minValues[i] = 0.0;
			np.maxValues[i] = 1.0;
			np.minSliders[i] = 0.0;
			np.maxSliders[i] = 1.0;
			np.clampMins[i] = true;
			np.clampMaxes[i] = true;
		}

		manager->appendRGB(np);
	}

	// color 2
	{
		OP_NumericParameter	np;

		np.name = "Color2";
		np.label = "Color 2";

		for (int i = 0; i<3; i++)
		{
			np.defaultValues[i] = 0.0;
			np.minValues[i] = 0.0;
			np.maxValues[i] = 1.0;
			np.minSliders[i] = 0.0;
			np.maxSliders[i] = 1.0;
			np.clampMins[i] = true;
			np.clampMaxes[i] = true;
		}

		manager->appendRGB(np);
	}

	// speed
	{
		OP_NumericParameter	np;

		np.name = "Speed";
		np.label = "Speed";
		np.defaultValues[0] = 1.0;
		np.minSliders[0] = -10.0;
		np.maxSliders[0] = 10.0;

		manager->appendFloat(np);
	}

	// pulse
	{
		OP_NumericParameter	np;

		np.name = "Reset";
		np.label = "Reset";

		manager->appendPulse(np);
	}

}

void
CinderCPlusPlusTOPApp::pulsePressed(const char* name)
{
	if (!strcmp(name, "Reset"))
	{
		myRotation = 0.0;
	}


}


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
