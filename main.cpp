#include <vtkActor.h>
#include <vtkLight.h>
#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkPLYReader.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkCameraPass.h>
#include <vtkCubeSource.h>
#include <vtkNamedColors.h>
#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>
#include <vtkSequencePass.h>
#include <vtkRenderWindow.h>
#include <vtkShadowMapPass.h>
#include <vtkOpenGLTexture.h>
#include <vtkPolyDataMapper.h>
#include <vtkCallbackCommand.h>
#include <vtkLightCollection.h>
#include <vtksys/SystemTools.hxx>
#include <vtkRendererCollection.h>
#include <vtkShadowMapBakerPass.h>
#include <vtkRenderPassCollection.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSphereSource.h>

#include "array"
#include "math.h"

#define vtkSPtr vtkSmartPointer
#define vtkSPtrNew(Var, Type) vtkSPtr<Type> Var = vtkSPtr<Type>::New();

using namespace std;

#define PI 3.14159265
vtkSPtrNew(renderer, vtkRenderer);
vtkSPtrNew(renderWindow, vtkRenderWindow);
vtkSPtrNew(light, vtkLight);
float x = 0.875;
float y = 1.6125;
float z = 1;

/*
 * Global Variables for light control
 */
static double ligthPos[3] = {0, 0, 0};
static double angle = 45.0;
static double oRadius = 1.0; // Orbit Radius 
static vtkLight *oLigth = nullptr; //orbit Light 
static vtkRenderer *gRenderer = nullptr; // global renderer to update on ligth change 


/*
 * Custom function for key event
 * */
void KeypressCallbackFunction(vtkObject *caller, unsigned long eventId, void *clientData, void *callData) {
	vtkRenderWindowInteractor* iren =
		static_cast<vtkRenderWindowInteractor*>(caller);
	string key = iren->GetKeySym();

	if (key == "8")
		x = x + 0.010;
	if (key == "2")
		x = x - 0.010;
	if (key == "4")
		y = y - 0.010;
	if (key == "6")
		y = y + 0.010;
	if (key == "1")
		z = z - 0.010;
	if (key == "9")
		z = z + 0.010;

	light->SetPosition(x, y, z);

	renderWindow->Render();

}

int main() {

	/*
	 * Define a Color object to use color palette
	 * https://htmlpreview.github.io/?https://github.com/Kitware/vtk-examples/blob/gh-pages/VTKNamedColorPatches.html
	 */
	vtkSPtrNew(colors, vtkNamedColors);

	/*
	 * Read Mesh file from the data directory
	 */
	vtkSPtrNew(ply_reader, vtkPLYReader);
	ply_reader->SetFileName ("../DataSources/spider.ply");
	ply_reader->Update ();

	/*	vtkSPtrNew(colors, vtkNamedColors);

	 * Storing the mesh into a polydata object for
	 */
	vtkSPtrNew(man_polyData, vtkPolyData);
	man_polyData = ply_reader->GetOutput ();

	/*
	 * Associate the mesh with a GPU drawable object
	 */
	vtkSPtrNew(man_mapper, vtkPolyDataMapper);
	man_mapper->SetInputData (man_polyData);

	/*
	 * Associate the 3D to an actor to manipulate
	 * and change the actor color and light properties
	 */
	vtkSPtrNew(man_actor, vtkActor);
	man_actor->SetMapper (man_mapper);
	// Phong   illumination :combination of the diffuse reflection of rough surfaces with the specular reflection of shiny surfaces
	
	//Set Ambient color : constant lighting an object gives even in the absence of strong light
	man_actor->GetProperty ()->SetDiffuseColor (colors->GetColor3d ("FireBrick").GetData ());
	man_actor->GetProperty ()->SetDiffuse (0.6);

	// Set Diffuse Lighting relies on light direction and the surface normal.
	

	/*
	 * Get the mesh bounding box  and deform it to create the base
	 * returns xmin[0],xmax[1], ymin[2],ymax[3], zmin[4],zmax[5]
	 */
	std::array<double, 6> bounds;
	//man_polyData->GetBounds (bounds.data ());

	//std::array<double, 3> man_bb; // bounding box of man_mesh
	//man_bb[ 0 ] = bounds[ 1 ] - bounds[ 0 ];
	//man_bb[ 1 ] = bounds[ 3 ] - bounds[ 2 ];
	//man_bb[ 2 ] = bounds[ 5 ] - bounds[ 4 ];
	//double widen_by = 2;
	//double thickness = man_bb[ 2 ];

	/*
	* Add a plane to cast the shadows on
    */
	//vtkSPtrNew(plane, vtkCubeSource);
	//plane->SetCenter ((bounds[ 1 ] + bounds[ 0 ]) / 2.0,
	//                   bounds[ 2 ],
	//                  (bounds[ 5 ] + bounds[ 4 ]) / 2.0);
	//plane->SetXLength (bounds[ 1 ] - bounds[ 0 ] + (man_bb[ 0 ] * widen_by));
	//plane->SetZLength (bounds[ 5 ] - bounds[ 4 ] + (man_bb[ 2 ] * widen_by));
	//plane->SetYLength (thickness/2);


	//vtkSPtrNew(planeMapper, vtkPolyDataMapper);
	//planeMapper->SetInputConnection (plane->GetOutputPort ());

	//vtkSPtrNew(planeActor, vtkActor);
	//planeActor->SetMapper (planeMapper);
	//planeActor->RotateY (45);
	//planeActor->GetProperty ()->SetInterpolationToPhong ();

	//planeActor->GetProperty ()->SetAmbientColor (colors->GetColor3d ("Silver").GetData ());
	//planeActor->GetProperty ()->SetAmbient (0.7);

	//planeActor->GetProperty ()->SetDiffuseColor (colors->GetColor3d ("DarkGray").GetData ());
	//planeActor->GetProperty ()->SetDiffuse (0.6);


	renderer->RemoveAllLights();
	renderer->SetBackground (colors->GetColor3d ("PowderBlue").GetData ());
	renderer->AddActor (man_actor);
	//renderer->AddActor (planeActor);

	vtkNew<vtkLight> orbit_light;
	orbit_light->SetFocalPoint(0, 0, 0);
	orbit_light->SetPosition(0.875, 1.6125, 1);
	renderer->AddLight (orbit_light);
	gRenderer = renderer;


    // set the vtkShadowMapPass and vtkSequencePass
	vtkNew<vtkShadowMapPass> shadows;
	vtkNew<vtkSequencePass> seq;

	vtkNew<vtkRenderPassCollection> passes;
	passes->AddItem(shadows->GetShadowMapBakerPass());
	passes->AddItem(shadows);
	seq->SetPasses(passes);

	vtkSPtrNew(cameraP, vtkCameraPass);
	cameraP->SetDelegatePass (seq);


	// tell the renderer to use our render pass pipeline
	renderer.GetPointer ()->SetPass (cameraP);
	renderer->GetActiveCamera ()->SetPosition (0, 1, 2);
	renderer->ResetCamera ();

	vtkSPtrNew(keypressCallback, vtkCallbackCommand);
	keypressCallback->SetCallback (KeypressCallbackFunction);

	light->SetFocalPoint(1.875, 0.6125, 0);
	light->SetPosition(x, y, z);
	renderer->AddLight(light);

	renderWindow->AddRenderer (renderer);
	renderWindow->SetMultiSamples (64);
	renderWindow->SetSize (1500, 1000);

	vtkSPtrNew(interactor, vtkRenderWindowInteractor);
	interactor->SetRenderWindow (renderWindow);
	interactor->AddObserver (vtkCommand::KeyPressEvent, keypressCallback);

	renderWindow->Render ();
	interactor->Start ();

	return EXIT_SUCCESS;
}
