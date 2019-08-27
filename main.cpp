#include <stdio.h>
#include <allegro5/allegro.h>
#include <iostream>
using namespace std;

int main()
{
	bool running;
	bool redraw;

	float FPS = 60;
	int SCREEN_W = 500;
	int SCREEN_H = 500;

	int lightMatrix[50][50];
	bool objectMatrix[50][50];

	const double lightIntensity = 10;
	double theta = 0;



	//ALLEGRO setup
	al_init();//starting allegro

	ALLEGRO_DISPLAY *display;
	ALLEGRO_EVENT_QUEUE *event_queue;
	ALLEGRO_TIMER *timer;

	//defining timer, display, and queue
	display = al_create_display(SCREEN_W, SCREEN_H);
	event_queue = al_create_event_queue();
	timer = al_create_timer(1.0 / FPS);

	//logging timer and display as items that can be in queue
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_timer_event_source(timer));

	al_flip_display();
	al_start_timer(timer);

	//BEGIN GAME LOOP
	while (1) {//runs until 'break' command is used
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);

		if (ev.type == ALLEGRO_EVENT_TIMER)//update section
		{
			redraw = true;
			//GameUpdate();
		}
		else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { break; }//window closing (makes 'X' functional)

		//render section
		if (redraw && al_is_event_queue_empty(event_queue))//if event_timer has run, as well as no events within keyboard input or anything else, render
		{
			redraw = false;
			al_clear_to_color(al_map_rgb(0, 0, 0));

			//al_draw_...
			al_flip_display();//essentially takes what takes all recent draw commands and makes them visible. 
		}
	}

	//closing out allegro components
	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
}