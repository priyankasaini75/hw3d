#include "App.h"
#include <algorithm>
#include "ChiliMath.h"
#include "imgui/imgui.h"
#include "ChiliUtil.h"
#include "Testing.h"
#include "PerfLog.h"
#include "TestModelProbe.h"
#include "Testing.h"
#include "Camera.h"
#include "Channels.h"

namespace dx = DirectX;

App::App( const std::string& commandLine )
	:
	commandLine( commandLine ),
	wnd( 1280,720,"DirectX Graphics Demo" ),
	scriptCommander( TokenizeQuoted( commandLine ) ),
	light( wnd.Gfx(),{ 10.0f,2.0f,0.0f } )
{
	
	//cameras.AddCamera( std::make_unique<Camera>( wnd.Gfx(),"A",dx::XMFLOAT3{ -13.5f,6.0f,3.5f },0.0f,PI / 2.0f ) );
	cameras.AddCamera( std::make_unique<Camera>( wnd.Gfx(),"B",dx::XMFLOAT3{ 10.f,30.f,-1.f },PI / 180.0f * 86.0f,PI / 180.0f * 6.0f ) );
	cameras.AddCamera(light.ShareCamera());
	

	cube.SetPos( { 9.5f,1.9f,7.5f } );
	cube.SetRotation(-PI / 180.0f * 25.0f, -PI / 180.0f * 144.0f, 0.f);
	
	teapot.SetRootTransform(
		dx::XMMatrixRotationY(-PI / 2.f) *
		dx::XMMatrixTranslation(-8.f, 10.f, 0.f)
	);
	
	
	sphere.SetRootTransform(
		dx::XMMatrixRotationY( PI / 2.f ) *
		dx::XMMatrixTranslation( 18.4f,0.5f,1.7f )
	);


	
	cube.LinkTechniques( rg );
	//cube2.LinkTechniques( rg );
	light.LinkTechniques( rg );
	//sponza.LinkTechniques( rg );
	teapot.LinkTechniques( rg );
	sphere.LinkTechniques( rg );
	cameras.LinkTechniques( rg );

	rg.BindShadowCamera( *light.ShareCamera() );
}

void App::HandleInput( float dt )
{
	while( const auto e = wnd.kbd.ReadKey() )
	{
		if( !e->IsPress() )
		{
			continue;
		}

		switch( e->GetCode() )
		{
		case VK_ESCAPE:
			if( wnd.CursorEnabled() )
			{
				wnd.DisableCursor();
				wnd.mouse.EnableRaw();
			}
			else
			{
				wnd.EnableCursor();
				wnd.mouse.DisableRaw();
			}
			break;
		case VK_F1:
			showDemoWindow = true;
			break;
		case VK_RETURN:
			savingDepth = true;
			break;
		}
	}

	if( !wnd.CursorEnabled() )
	{
		if( wnd.kbd.KeyIsPressed( 'W' ) )
		{
			cameras->Translate( { 0.0f,0.0f,dt } );
		}
		if( wnd.kbd.KeyIsPressed( 'A' ) )
		{
			cameras->Translate( { -dt,0.0f,0.0f } );
		}
		if( wnd.kbd.KeyIsPressed( 'S' ) )
		{
			cameras->Translate( { 0.0f,0.0f,-dt } );
		}
		if( wnd.kbd.KeyIsPressed( 'D' ) )
		{
			cameras->Translate( { dt,0.0f,0.0f } );
		}
		if( wnd.kbd.KeyIsPressed( 'R' ) )
		{
			cameras->Translate( { 0.0f,dt,0.0f } );
		}
		if( wnd.kbd.KeyIsPressed( 'F' ) )
		{
			cameras->Translate( { 0.0f,-dt,0.0f } );
		}
	}

	while( const auto delta = wnd.mouse.ReadRawDelta() )
	{
		if( !wnd.CursorEnabled() )
		{
			cameras->Rotate( (float)delta->x,(float)delta->y );
		}
	}
}

void App::DoFrame( float dt )
{
	wnd.Gfx().BeginFrame( 0.0f,0.0f,0.0f );
	light.Bind( wnd.Gfx(),cameras->GetMatrix() );
	rg.BindMainCamera( cameras.GetActiveCamera() );
		
	light.Submit( Chan::main );
	cube.Submit( Chan::main );
	//sponza.Submit( Chan::main );
	//cube2.Submit( Chan::main );
	teapot.Submit( Chan::main );
	sphere.Submit( Chan::main );
	cameras.Submit( Chan::main );

	//sponza.Submit( Chan::shadow );
	cube.Submit( Chan::shadow );
	//sponza.Submit( Chan::shadow );
	//cube2.Submit( Chan::shadow );
	teapot.Submit( Chan::shadow );
	sphere.Submit( Chan::shadow );

	rg.Execute( wnd.Gfx() );

	if( savingDepth )
	{
		rg.DumpShadowMap( wnd.Gfx(),"shadow.png" );
		savingDepth = false;
	}
	
	// imgui windows
	//static MP sponzeProbe{ "Sponza" };
	static MP teaProbe{ "teapot" };
	static MP sphereProbe{ "sphere" };
	//sponzeProbe.SpawnWindow( sponza );
	teaProbe.SpawnWindow( teapot );
	sphereProbe.SpawnWindow( sphere );
	cameras.SpawnWindow( wnd.Gfx() );
	light.SpawnControlWindow();
	ShowImguiDemoWindow();
	cube.SpawnControlWindow( wnd.Gfx(),"Cube 1" );
	
	rg.RenderWindows( wnd.Gfx() );
	//cube.Update(timer.Peek());
	//cameras->Rotate(1.0f, timer.Peek());
	teapot.UpdateTransform(timer.Peek(),"pitch",1.f);

	// present
	wnd.Gfx().EndFrame();
	rg.Reset();
}

void App::ShowImguiDemoWindow()
{
	if( showDemoWindow )
	{
		ImGui::ShowDemoWindow( &showDemoWindow );
	}
}

App::~App()
{}

int App::Go()
{
	while( true )
	{
		// process all messages pending, but to not block for new messages
		if( const auto ecode = Window::ProcessMessages() )
		{
			// if return optional has value, means we're quitting so return exit code
			return *ecode;
		}
		// execute the game logic
		const auto dt = timer.Mark() * speed_factor;
		HandleInput( dt );
		DoFrame( dt );
	}
}