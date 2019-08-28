#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <iostream>
#include <vector>
#include <math.h>

using namespace std;
const double PI = 3.14159265;

class lightSource
{
public:
	lightSource(double x, double y);
	double x, y;
};

lightSource::lightSource(double x, double y)//constructor
{
	lightSource::x = x;
	lightSource::y = y;
}

int main()
{
	bool running;
	bool redraw = true;

	float FPS = 40;
	int SCREENSCALE = 750;

	const int matrixSize = 75;
	//const int screenSize = SCREENSCALE;
	double scaling = 1.00 / (SCREENSCALE / matrixSize);//current 0.1, scales console numbers to fit onto matrix.
	cout << scaling << endl;

	double lightMatrix[matrixSize][matrixSize];
	int objectMatrix[matrixSize][matrixSize];
	bool overlapMatrix[matrixSize][matrixSize];


	const double lightIntensity = 100.0;

	bool lMouseButton = false;
	bool lMouseButtonHeld = false;
	bool rMouseButton = false;
	bool rMouseButtonHeld = false;
	double mouseX = 0.0;
	double mouseY = 0.0;
	int dragX = 0;
	int dragY = 0;
	bool drawSelectionBox = false;

	enum objectID { EMPTY, FILLED, LIGHTSOURCE };

	std::vector <lightSource> lightSourceList;
	std::vector <lightSource>::iterator iter;

	//ALLEGRO setup
	al_init();//starting allegro
	al_init_primitives_addon();
	al_install_mouse();

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

	al_flip_display();
	al_start_timer(timer);

	for (int x = 0; x < matrixSize; x++) {//defining matrices
		for (int y = 0; y < matrixSize; y++) {
			lightMatrix[x][y] = 1.0;
			objectMatrix[x][y] = EMPTY;
			overlapMatrix[x][y] = false;
		}
	}

	//BEGIN GAME LOOP
	while (1) {//runs until 'break' command is used
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);

		if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
		{

			al_get_mouse_state(&state);
			if (state.buttons & 1)
				lMouseButton = true;
			if (state.buttons & 2)
				rMouseButton = true;

		}
		if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
		{
			al_get_mouse_state(&state);
			if (!state.buttons & 1)
				lMouseButton = false;
			if (!state.buttons & 2)
				rMouseButton = false;

		}
		if (ev.type == ALLEGRO_EVENT_MOUSE_AXES)//if allegro registers that mouse has been moved:
		{
			//update mouse x and y position
			mouseX = ev.mouse.x;
			mouseY = ev.mouse.y;
		}



		if (ev.type == ALLEGRO_EVENT_TIMER)//update section
		{

			redraw = true;

			if (rMouseButton)//if rMouseButton is clicked, generate a lightSource class and add to the vector
			{
				rMouseButton = false;
				int x = (mouseX*scaling);
				int y = (mouseY*scaling);

				if (objectMatrix[x][x] != FILLED) {//do not create light source within a filled space
					objectMatrix[x][y] = LIGHTSOURCE;

					lightSource lightSource(mouseX, mouseY);//
					lightSourceList.push_back(lightSource);
				}
			}
			//////////////////////////////////////////////////////////////////////////////////////////////////////HIGHLIGHTING TOOL CREATION
			if (lMouseButton && !lMouseButtonHeld)//when left mouse is initially clicked
			{
				dragX = mouseX;//track initial position of click
				dragY = mouseY;
				lMouseButtonHeld = true;
			}
			else if (lMouseButton && lMouseButtonHeld)//when leftmouse button is held down
			{
				drawSelectionBox = true;
			}
			else if (!lMouseButton && lMouseButtonHeld)//when left mouse button is initially released
			{

				//setting selected region to a block in objectMatrix

				double startingX = 0;
				double startingY = 0;
				double endingX = 0;
				double endingY = 0;

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

				//scaling to matrix
				startingX = startingX * scaling;
				startingY = startingY * scaling;
				endingX = endingX * scaling;
				endingY = endingY * scaling;

				//checking matrix for light sources before highlighting section	
				bool noLightSources = true;
				for (int a = startingX; a < endingX; a++) {
					for (int b = startingY; b < endingY; b++) {
						if (objectMatrix[a][b] == LIGHTSOURCE)
							noLightSources = false;

					}
				}
				if (noLightSources) {
					//running through matrix, setting to true (now that check has been performed)
					for (int a = startingX; a < endingX; a++) {
						for (int b = startingY; b < endingY; b++) {
							objectMatrix[a][b] = FILLED;

						}
					}
				}

				//resetting variables now that box is released
				lMouseButtonHeld = false;
				dragX = 0;
				dragY = 0;
				drawSelectionBox = false;

			}


			//updating lighting:

			//resetting lightMatrix
			for (int x = 0; x < matrixSize; x++) {
				for (int y = 0; y < matrixSize; y++) {
					lightMatrix[x][y] = 0.0;
				}
			}

			//going to each light source and seeing how it affects surrounding area
			for (iter = lightSourceList.begin(); iter != lightSourceList.end(); ++iter)
			{
				//double thetaIncrement = 0.85;
				double thetaIncrement = (atan2(scaling, (double) SCREENSCALE));
				cout << thetaIncrement << endl;
				for (double theta = 0.0; theta < 360.0; theta += thetaIncrement)//rotating around light source
				{

					double maxRadius = 175.0;//if this number goes beyond 255, the RGB values will go full circle and cause striped lighting.
					for (double radius = 0.0; radius < maxRadius; radius += 5.0) {
						double convertedTheta = theta;
						if (convertedTheta < 0)
							convertedTheta *= -1;
						else if (convertedTheta > 0)
							convertedTheta = 360 - convertedTheta;
						convertedTheta = -convertedTheta * (PI / 180);

						double x = ((iter)->x + (cos(convertedTheta)*radius))*scaling;
						double y = ((iter)->y + (sin(convertedTheta)*radius))*scaling;

						//exiting if coordinates exceed matrix (to prevent code instability by going beyond bounds of matrix)
						if (x >= matrixSize || x < 0 || y >= matrixSize || y < 0)
						{
							radius = maxRadius;//exit loop
						}
						else {
							if (objectMatrix[(int)x][(int)y] == FILLED)//if the space is filled, exit
							{
								overlapMatrix[(int)x][(int)y] = true;
								radius = maxRadius;//exit loop
							}

							else if (objectMatrix[(int)x][(int)y] != FILLED && !overlapMatrix[(int)x][(int)y]) {//if the space is not filled, and has not been accessed before
								overlapMatrix[(int)x][(int)y] = true;
								lightMatrix[(int)x][(int)y] += (255.0 - ((radius / maxRadius) * 255.0));
							}
						}

					}
				}

				for (int x = 0; x < matrixSize; x++) {
					for (int y = 0; y < matrixSize; y++) {
						overlapMatrix[x][y] = false;//refreshing overlap check before moving to next light source
					}
				}

			}//vector search end
			for (int x = 0; x < matrixSize; x++) {
				for (int y = 0; y < matrixSize; y++) {
					if (lightMatrix[x][y] > 255.0)//setting maximum light value to 255 to prevent striping
						lightMatrix[x][y] = 255.0;
				}
			}

		}//update section end


		else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { break; }//window closing (makes 'X' functional)

		//render section
		if (redraw && al_is_event_queue_empty(event_queue))//if event_timer has run, as well as no events within keyboard input or anything else, render
		{
			redraw = false;
			al_clear_to_color(al_map_rgb(0, 0, 0));

			//al_draw_...
			for (int x = 0; x < matrixSize; x++) {
				for (int y = 0; y < matrixSize; y++) {

					double lightLevel = lightMatrix[x][y];

					if (objectMatrix[x][y] == FILLED)
						al_draw_filled_rectangle(x*(SCREENSCALE / matrixSize), y*(SCREENSCALE / matrixSize), x*(SCREENSCALE / matrixSize) + matrixSize, y*(SCREENSCALE / matrixSize) + matrixSize,
							al_map_rgb(0, 0, 255));
					if (objectMatrix[x][y] == EMPTY || objectMatrix[x][y] == LIGHTSOURCE)
						al_draw_filled_rectangle(x*(SCREENSCALE / matrixSize), y*(SCREENSCALE / matrixSize), x*(SCREENSCALE / matrixSize) + matrixSize, y*(SCREENSCALE / matrixSize) + matrixSize, al_map_rgb(lightLevel, lightLevel, lightLevel));
				}
			}
			if (drawSelectionBox)
				al_draw_rectangle(dragX, dragY, mouseX, mouseY, al_map_rgb(255, 0, 0), 3);

			al_flip_display();//essentially takes what takes all recent draw commands and makes them visible. 
		}
	}

	//closing out allegro components
	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);

	return 0;
}