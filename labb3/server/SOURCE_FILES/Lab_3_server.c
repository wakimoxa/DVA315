#include <cairo/cairo.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <semaphore.h>
#include "wrapper.h"
#include <math.h>

#define DT 10
static void do_drawing(cairo_t *);
int x = 0;
int y = 0;
int x2 = 0;
GtkWidget *window;
GtkWidget *darea;
int width = 800;
int height = 600;

#define handle_error(msg) \
    do {perror(msg); exit(EXIT_FAILURE); } while(0)

planet_type * planet_list = NULL;
void calculate_planet_pos(planet_type *p1);
void * planet_thread (void*args);
//------------------MY FUNCTIONS---------------------//
void createPlanet(planet_type* pt);
void insertPlanet(planet_type* pt);
void removePlanet(planet_type* pt);
void * mq_reader();
//------------------MY FUNCTIONS---------------------//
pthread_t all_planet_thread;
pthread_mutex_t calc_lock;

void * mq_reader(){
    mqd_t message_queue;
    char * name = "/server_mq";
    //MQclose(&message_queue, name);
    MQcreate(&message_queue, name);

    struct mq_attr attr;

    if(mq_getattr(message_queue, &attr) == -1)
        handle_error("mq_getattr");


    while(1){
        //sleep(1);
        void *buffer = malloc(attr.mq_msgsize);
        int nr = MQread(message_queue, &buffer);
        printf("Server received %i bytes.\n", nr);
        //printf("Received msg: %s\n\n", (char*)buffer);
        planet_type* pt = (planet_type*) buffer;
        if(strncmp(pt->name, "END", 3) != 0)
        {
            insertPlanet(pt);
        }
        else
            break;
    }
    return NULL;
}

void insertPlanet(planet_type* pt){
    if(planet_list == NULL){
        planet_list = pt;
        return;
    }
    planet_type* current_pt;
    for(current_pt = planet_list; current_pt->next != NULL; current_pt = current_pt->next);
    current_pt->next = pt;
    pt->next = NULL;
    return;
}
void removePlanet(planet_type *pt){
    planet_type* current_pt;
    if(planet_list == pt){
        planet_list = pt->next;
        free(pt);
        return;
    }
    for(current_pt = planet_list; current_pt->next != pt && current_pt != NULL; current_pt = current_pt->next);
    if(current_pt == NULL){
        printf("Could not find planet to remove!\n");
        return;
    }
    current_pt->next = current_pt->next->next;
    free(pt);
}

void planetDied(char * msg, char*name){
    mqd_t mq;
    MQconnect(&mq, name);
    MQwrite(mq, (void*)msg);
    MQclose(&mq, name);
}



void * planet_thread (void*args)
{
	planet_type * this_planet = (planet_type *)args;
    //pthread_mutex_lock(&calc_lock);
	calculate_planet_pos(this_planet);
    //pthread_mutex_unlock(&calc_lock);
    if(this_planet->life < 0){
        char* name = (char*)malloc(sizeof(char)*100);
        char* msg = (char*)malloc(sizeof(char)*100);
        sprintf(name, "/mq_%s", this_planet->pid);
        sprintf(msg, "Your planet \"%s\" has died. ", this_planet->name);
        planetDied(msg, name);
        free(name);
        free(msg);
        removePlanet(this_planet);
    }
    if(this_planet->sx < 0 || this_planet->sy < 0 || this_planet->sx > width || this_planet->sy > height){
        char* name = (char*)malloc(sizeof(char)*100);
        char* msg = (char*)malloc(sizeof(char)*100);
        sprintf(name, "/mq_%s", this_planet->pid);
        sprintf(msg, "Your planet \"%s\" went out of bounds and has therefore died.", this_planet->name);
        planetDied(msg, name);
        free(name);
        free(msg);
        removePlanet(this_planet);
    }

    return NULL;
}
static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, //Draw event for cairo, will be triggered each time a draw event is executed
    gpointer user_data)
{
    if(planet_list != NULL){
        planet_type * current;
        for(current = planet_list; current != NULL; current = current->next){
            pthread_create(&all_planet_thread, NULL, planet_thread, (void*)current);
        }
        pthread_join(all_planet_thread, NULL);
    }
    do_drawing(cr); //Launch the actual draw method


    return FALSE; //Return something
}

static void on_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
    gtk_window_get_size (window, &width, &height);
    printf("Window size: (%i, %i)\n", width, height);
}
static void do_drawing(cairo_t *cr) //Do the drawing against the cairo surface area cr
{
    cairo_set_source_rgb(cr, 0, 0, 0); //Set RGB source of cairo, 0,0,0 = black
    cairo_select_font_face(cr, "Purisa",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_BOLD);
    //cairo_move_to(cr, 20, 30);
    //cairo_show_text(cr, "You probably do not want to debug using text output, but you can");
    //cairo_arc(cr, x,y,50,0,2*3.1415); //Create cairo shape: Parameters: Surface area, x pos, y pos, radius, Angle 1, Angle 2
    //cairo_fill(cr);
    //cairo_arc(cr, x2+100,0,25,0,2*3.1415); //These drawings are just examples, remove them once you understood how to draw your planets
    //cairo_fill(cr);
    //Printing planets should reasonably be done something like this:
    // --------- for all planets in list:
    // --------- cairo_arc(cr, planet.xpos, planet.ypos, 10, 0, 2*3.1415)
    // --------- cairo_fill(cr)
    //------------------------------------------Insert planet drawings below-------------------------------------------
    planet_type* current;
    for(current = planet_list; current != NULL; current = current->next){
        cairo_arc(cr, current->sx, current->sy, 10, 0, 2*3.1415);
        cairo_fill(cr);
    }



    //------------------------------------------Insert planet drawings Above-------------------------------------------

}
GtkTickCallback on_frame_tick(GtkWidget * widget, GdkFrameClock * frame_clock, gpointer user_data) //Tick handler to update the frame
{
    gdk_frame_clock_begin_updating (frame_clock); //Update the frame clock
    gtk_widget_queue_draw(darea); //Queue a draw event
    gdk_frame_clock_end_updating (frame_clock); //Stop updating frame clock
}

void calculate_planet_pos(planet_type *p1)  //Function for calculating the position of a planet, relative to all other planets in the system
{
    planet_type *current = planet_list; //Poiinter to head in the linked list
    //Variable declarations
    double Atotx = 0;
    double Atoty = 0;
    double x = 0;
    double y = 0;
    double r = 0;
    double a = 0;
    double ax = 0;
    double ay = 0;

    double G = 6.67259 * pow(10, -11); //Declaration of the gravitational constant
    while (current != NULL) //Loop through all planets in the list
    {
        if (p1 != current) //Only update variables according to properties of other planets
        {
            x = current->sx - p1->sx;
            y = current->sy - p1->sy;
            r = sqrt(pow(x, 2.0) + pow(y, 2.0));
            a = G * (current->mass / pow(r, 2.0));

            ay = a * (y / r);
            ax = a * (x / r);

            Atotx += ax;
            Atoty += ay;

        }
        current = current->next;
    }
    p1->vx = p1->vx + (Atotx * DT); //Update planet velocity, acceleration and life
    p1->vy = p1->vy + (Atoty * DT);
    p1->sx = p1->sx + (p1->vx * DT);
    p1->sy = p1->sy + (p1->vy * DT);
    p1->life -= 1;
}
int main(int argc, char *argv[]) //Main function
{
    //----------------------------------------Variable declarations should be placed below---------------------------------
	pthread_t mq_thread;
    pthread_mutex_init(&calc_lock, NULL);

    //----------------------------------------Variable declarations should be placed Above---------------------------------

    //GUI stuff, don't touch unless you know what you are doing, or if you talked to me
    gtk_init(&argc, &argv); //Initialize GTK environment
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL); //Create a new window which will serve as your top layer
    darea = gtk_drawing_area_new(); //Create draw area, which will be used under top layer window
    gtk_container_add(GTK_CONTAINER(window), darea); //add draw area to top layer window
    g_signal_connect(G_OBJECT(darea), "draw",
                    G_CALLBACK(on_draw_event), NULL); //Connect callback function for the draw event of darea
    g_signal_connect(window, "destroy", //Destroy event, not implemented yet, altough not needed
                    G_CALLBACK(gtk_main_quit), NULL);
    
    g_signal_connect(window, "size-allocate", G_CALLBACK(on_size_allocate), NULL);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); //Set position of window
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600); //Set size of window
    gtk_window_set_title(GTK_WINDOW(window), "GTK window"); //Title
    gtk_widget_show_all(window); //Show window
    gtk_widget_add_tick_callback(darea, on_frame_tick, NULL, 1); //Add timer callback functionality for darea
    //GUI stuff, don't touch unless you know what you are doing, or if you talked to me


    //-------------------------------Insert code for pthreads below------------------------------------------------
    //Create MQ_listener thread
    pthread_create(&mq_thread, NULL, mq_reader, NULL);
    //-------------------------------Insert code for pthreads above------------------------------------------------


    gtk_main();//Call gtk_main which handles basic GUI functionality
    return 0;
}

