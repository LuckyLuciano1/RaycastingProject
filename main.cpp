#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <fstream>
#include <string>

using namespace std;
const double PI = 3.14159265;//defining PI for later trig use

//class lightSource stores the position of a lightSOurce within the matrix for the raycasting system.
//lightSources are stored within the lightSourceList vector, so that each lightSource can be iterated through when raycasting. 
class lightSource
{
public:
	lightSource(double x, double y);
	double x, y;
};

lightSource::lightSource(double x, double y)//constructor for lightSource
{
	lightSource::x = x;
	lightSource::y = y;
}

const int matrixSize = 50;//global variables
int objectMatrix[matrixSize][matrixSize];
enum objectID { EMPTY, FILLED, LIGHTSOURCE };

//for use in drawing highlighted box that then 'fills in' console window
void DrawBox(int dragX, int dragY, double mouseX, double mouseY, double scaling);

int main()
{
	bool redraw = true;

	float FPS = 80;
	int SCREENSCALE = 500;
	double scaling = 1.00 / (SCREENSCALE / matrixSize);//scales console numbers to fit onto matrix slots.

	double maxLightRadius = 300.0;//if this number goes beyond 255, the RGB values will go full circle and cause striped lighting.

	double lightMatrix[matrixSize][matrixSize];//stores light values that will be rendered
	bool overlapMatrix[matrixSize][matrixSize];//tracks light sources accessing of slots in matrix, so that overlap within raycasting doews not occur.

	bool lMouseButton = false;
	bool lMouseButtonHeld = false;
	bool rMouseButton = false;
	bool rMouseButtonHeld = false;

	double mouseX = 0.0;//mouse position
	double mouseY = 0.0;
	int dragX = 0;//position of initial click when left mouse held down. (for use in highlighting tool)
	int dragY = 0;



	std::vector <lightSource> lightSourceList;
	std::vector <lightSource>::iterator iter;

	//ALLEGRO setup
	al_init();//starting allegro
	al_init_primitives_addon();//initializing basic allegro drawing commands
	al_install_mouse();//intitializing mouse functionality inn allegro

	//creating timer, state, display and event queue for allegro:
	ALLEGRO_DISPLAY *display;
	ALLEGRO_EVENT_QUEUE *event_queue;
	ALLEGRO_TIMER *timer;

	ALLEGRO_MOUSE_STATE state;

	//defining timer, display, and queue
	display = al_create_display(SCREENSCALE, SCREENSCALE);
	event_queue = al_create_event_queue();
	timer = al_create_timer(1.0 / FPS);

	//logging timer and display as items that can be in queue
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_register_event_source(event_queue, al_get_mouse_event_source());

	//starting display and timer
	al_flip_display();
	al_start_timer(timer);


	//opening .txt file for copying into objectMatrix
	fstream file;
	string fileOutput;
	file.open("data.txt");

	if (!file.is_open()) {
		cout << "Could not open file myfile.txt." << endl;
		return 1;
	}

	for (int x = 0; x < matrixSize; x++) {//defining starting values for matrices
		for (int y = 0; y < matrixSize; y++) {

			lightMatrix[x][y] = 1.0;
			overlapMatrix[x][y] = false;
			objectMatrix[x][y] = EMPTY;
		}
	}


	//file >> fileOutput;
	//cout << fileOutput << endl << endl;

	for (int y = 0; y < matrixSize; y++) {
		for (int x = 0; x < matrixSize; x++) {
			//cout << fileOutput; 
			//if (file.good()) {
				file >> fileOutput;
				//int convertedFileOutput = stoi(fileOutput,nullptr,10);
				//cout << fileOutput;
				objectMatrix[x][y] = stoi(fileOutput);//ya and x are reversed intentionally to counteract file output

				if (stoi(fileOutput) == LIGHTSOURCE)
				{
					cout << "flag" << endl;
					lightSource lightSource(x/scaling, y/scaling);//create lightSource class
					lightSourceList.push_back(lightSource);//store class within class vector
				}
			//}
			//else cout<<"file is not good"<<endl;
		}
	}

	//for (int x = 0; x < matrixSize; x++) {//defining matrices
	//	for (int y = 0; y < matrixSize; y++) {
	//		cout << objectMatrix[x][y];
	//	}
	//	cout << endl;
	//}

	file.close();//closing data file

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////BEGIN GAME LOOP
	while (1) {//runs until 'break' command is used

		ALLEGRO_EVENT ev;//set up input through allegro
		al_wait_for_event(event_queue, &ev);//wait for input (axes, l/r clicking, etc)

		if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)//if allegro registers that mouse had been 
		{
			al_get_mouse_state(&state);
			if (state.buttons & 1)//if left mouse button is held down
				lMouseButton = true;//track variable
			if (state.buttons & 2)//if right mouse button is held down
				rMouseButton = true;

		}
		if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)//if allegro registers that mouse has been moved:
		{
			al_get_mouse_state(&state);
			if (!state.buttons & 1)//if left mouse button is released
				lMouseButton = false;
			if (!state.buttons & 2)//if right mouse button is released
				rMouseButton = false;
		}
		if (ev.type == ALLEGRO_EVENT_MOUSE_AXES)//if allegro registers that mouse has been moved:
		{
			//update mouse x and y position
			mouseX = ev.mouse.x;
			mouseY = ev.mouse.y;
		}


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////UPDATE SECTION
		if (ev.type == ALLEGRO_EVENT_TIMER)
		{
			redraw = true;

			if (rMouseButton)//if right MouseButton is clicked, generate a lightSource class and add to the lightSource vector
			{
				rMouseButton = false;
				int x = (mouseX*scaling);
				int y = (mouseY*scaling);

				if (objectMatrix[x][x] != FILLED) {//do not create light source within a filled space
					objectMatrix[x][y] = LIGHTSOURCE;

					lightSource lightSource(mouseX, mouseY);//create lightSource class
					lightSourceList.push_back(lightSource);//store class within class vector
				}
			}
			//////////////////////////////////////////////////////////////////////////////////////////////////////HIGHLIGHTING TOOL CREATION
			if (lMouseButton && !lMouseButtonHeld)//when left mouse is initially clicked
			{
				dragX = mouseX;//track initial position of click
				dragY = mouseY;
				lMouseButtonHeld = true;
			}
			else if (!lMouseButton && lMouseButtonHeld)//when left mouse button is initially released, edit objectMatrix:
			{
				//setting selected region to a block in objectMatrix
				DrawBox(dragX, dragY, mouseX, mouseY, scaling);//sets all values of objectMatrix within dimensions of highlighted box to FILLED	

				//resetting variables now that box is released
				lMouseButtonHeld = false;
				dragX = 0;
				dragY = 0;

			}


			/////////////////////////////////////////////////////////////////////////////////////////////////////////////UPDATE LIGHTING:

			//resetting lightMatrix for recalculation
			for (int x = 0; x < matrixSize; x++) {
				for (int y = 0; y < matrixSize; y++) {
					lightMatrix[x][y] = 0.0;
				}
			}

			//going to each light source and seeing how it affects surrounding area


			//this lighting process:
			//1. checks a point that first expands in a line outwards from the light source
			//2. checks the point for if it has collided with a FILLED space in objectMatrix, and if it has not, updates point in lightMatrix based on distance from lightSource (further away = smaller number)
			//3. if the point has collided with FILLED space or exceeded bounds of screen, rotate trajectory of point by thetaIncrement, go back to lightSource, and restart prcess
			//4. by revolving completely around lightSource, this process creates a circular path that is blocked by FILLED spaces; in other wortds, a light source.
			for (iter = lightSourceList.begin(); iter != lightSourceList.end(); ++iter)
			{
				double thetaIncrement = atan2(matrixSize, 1.0);//distance from 

				for (double theta = 0.0; theta < 360.0; theta += thetaIncrement)//rotating around light source
				{
					for (double radius = 0.0; radius < maxLightRadius; radius += 5.0)//expanding light out from light source
					{
						//corrected theta value by first chaning the range, making negative, then converting it from degrees to radians:
						double convertedTheta = theta;
						if (convertedTheta < 0)
							convertedTheta *= -1;
						else if (convertedTheta > 0)
							convertedTheta = 360 - convertedTheta;
						convertedTheta = -convertedTheta * (PI / 180);

						//determine point using theta and distance from light source (vector math), then scale to matrix rather than concolse window
						double x = ((iter)->x + (cos(convertedTheta)*radius))*scaling;
						double y = ((iter)->y + (sin(convertedTheta)*radius))*scaling;

						//exiting if coordinates exceed matrix (to prevent code instability by going beyond bounds of matrix)
						if (x >= matrixSize || x < 0 || y >= matrixSize || y < 0)
						{
							radius = maxLightRadius;//exit line loop
						}
						else {
							//begin editing of lightMatrix if coordinates do not exceed matrix
							if (objectMatrix[(int)x][(int)y] == FILLED)//if the space is filled, exit
							{
								radius = maxLightRadius;//exit line loop
							}
							else if (objectMatrix[(int)x][(int)y] != FILLED && !overlapMatrix[(int)x][(int)y]) {//if the space is not filled, and has not been accessed before
								overlapMatrix[(int)x][(int)y] = true;//overlapMatrix prevents point in matrix being added to by the same light source more than once.
								lightMatrix[(int)x][(int)y] += (255.0 - ((radius / maxLightRadius) * 255.0));//adding light to lightMatrix based on (x,y)'s distance from lightSource. 255 is maximum due to that being brightest white in RGB value.
							}
						}

					}//expanding line for loop end
				}//rotating theta for loop end

				 //refreshing overlap check before moving to next light source:
				for (int x = 0; x < matrixSize; x++) {
					for (int y = 0; y < matrixSize; y++) {
						overlapMatrix[x][y] = false;
					}
				}

			}//lightSource vector end

			 //setting maximum light value to 255 to prevent striping:
			for (int x = 0; x < matrixSize; x++) {
				for (int y = 0; y < matrixSize; y++) {
					if (lightMatrix[x][y] > 255.0)
						lightMatrix[x][y] = 255.0;
				}
			}

		}//end update section
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////UPDATE SECTION END

		else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { break; }//window closing (makes 'X' functional)

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////RENDER SECTION BEGIN		
		if (redraw && al_is_event_queue_empty(event_queue))//if event_timer has run, as well as no events within keyboard input or anything else, render
		{
			redraw = false;
			al_clear_to_color(al_map_rgb(0, 0, 0));//clearing console window before new things are rendered on it

			//rendering onto console window by refrencing object/lightMatrix
			for (int x = 0; x < matrixSize; x++) {
				for (int y = 0; y < matrixSize; y++) {

					if (objectMatrix[x][y] == FILLED)//draw all filled boxes
						al_draw_filled_rectangle(x*(SCREENSCALE / matrixSize), y*(SCREENSCALE / matrixSize), x*(SCREENSCALE / matrixSize) + matrixSize, y*(SCREENSCALE / matrixSize) + matrixSize,
							al_map_rgb(0, 0, 255));
					if (objectMatrix[x][y] == EMPTY || objectMatrix[x][y] == LIGHTSOURCE)//draw all empty spaces 
						al_draw_filled_rectangle(x*(SCREENSCALE / matrixSize), y*(SCREENSCALE / matrixSize), x*(SCREENSCALE / matrixSize) + matrixSize, y*(SCREENSCALE / matrixSize) + matrixSize,
							al_map_rgb(lightMatrix[x][y], lightMatrix[x][y], lightMatrix[x][y]));//(RGB values taken from lightMatrix)
				}
			}

			if (lMouseButtonHeld)//rendering "highlight" box when left mouse is held down
				al_draw_rectangle(dragX, dragY, mouseX, mouseY, al_map_rgb(255, 0, 0), 3);

			al_flip_display();//essentially takes what takes all recent draw commands and makes them visible. 
		}//end render section
	}//end game loop

	//closing out allegro components
	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);

	return 0;
}//end main



void DrawBox(int dragX, int dragY, double mouseX, double mouseY, double scaling)
{
	double startingX = 0;
	double startingY = 0;
	double endingX = 0;
	double endingY = 0;


	//setting upper left cornner of box (that is defined by the given two points) to be the starting position (and lower right to be ending)
	//this will allow for the nested for loop to run through the objectMatrix without trouble.
	if ((dragX < mouseX) && (dragY < mouseY))//lower right corner
	{
		startingX = dragX;
		startingY = dragY;
		endingX = mouseX;
		endingY = mouseY;
	}
	else if ((dragX > mouseX) && (dragY < mouseY))//lower left corner
	{
		startingX = mouseX;
		startingY = dragY;
		endingX = dragX;
		endingY = mouseY;
	}
	else if ((dragX < mouseX) && (dragY > mouseY))//upper right corner
	{
		startingX = dragX;
		startingY = mouseY;
		endingX = mouseX;
		endingY = dragY;
	}
	else if ((dragX > mouseX) && (dragY > mouseY))//upper left corner
	{
		startingX = mouseX;
		startingY = mouseY;
		endingX = dragX;
		endingY = dragY;
	}

	//scaling console values to matrix slot coordinates
	startingX = startingX * scaling;
	startingY = startingY * scaling;
	endingX = endingX * scaling;
	endingY = endingY * scaling;

	//checking matrix for light sources before highlighting section	(so that a light source is not 'stuck' inside a box)
	bool noLightSources = true;
	for (int a = startingX; a < endingX; a++) {
		for (int b = startingY; b < endingY; b++) {
			if (objectMatrix[a][b] == LIGHTSOURCE)
				noLightSources = false;

		}
	}
	if (noLightSources) {
		//running through matrix, setting to FILLED (now that check has been performed)
		for (int a = startingX; a < endingX; a++) {
			for (int b = startingY; b < endingY; b++) {
				objectMatrix[a][b] = FILLED;
			}
		}
	}
}//end DrawBox function