#include <stdio.h>
#include <allegro5/allegro.h>//głowna biblioteka allegro
#include <allegro5/allegro_image.h>//oblsuga zdjec
#include <allegro5/allegro_audio.h>//obsluga audio
#include <allegro5/allegro_acodec.h>//^
#include <allegro5/allegro_primitives.h>//obsluga ksztaltow
#include <allegro5/allegro_font.h>//oblsuga czcionek i pisanie
#include <allegro5/allegro_ttf.h>//^
#include <allegro5/allegro_native_dialog.h>//obsluga wyskakujacych okienek powiadomien

//inicjalizacja funkcji
void poruszanie(int* graczX, int prawaBanda, int lewaBanda);
bool collision(int graczX, int graczY, int w1, int h1, int x2, int y2, int w2, int h2);
void getNick(char nick[]);

int main()
{
    //utworzenie znaczkików
    ALLEGRO_DISPLAY* display = NULL;
    ALLEGRO_EVENT_QUEUE* queue = NULL;
    ALLEGRO_TIMER* timer = NULL;
    ALLEGRO_BITMAP* player = NULL;
    ALLEGRO_BITMAP* enemy1 = NULL;
    ALLEGRO_SAMPLE* sample = NULL;
    ALLEGRO_SAMPLE_INSTANCE* sampleInstance = NULL;
    ALLEGRO_FONT* font = NULL;
    ALLEGRO_EVENT event;//utworzenie zmiennej do obslugi event'ow

    al_init();//inicjalizacja allegro
    al_init_acodec_addon();//inicjalizacja addon'ow obslugujacych muzyke
    al_init_primitives_addon();//inicjalizacja addon'ow obslugujacych rysowanie figur
    al_init_image_addon();//inicjalizacja addon'ow obslugujacych obrazy
    al_init_font_addon();//inicjalizacja addon'ow obslugujacych czcionki oraz pisanie
    al_init_ttf_addon();//^

    //przypisanie wartości znacznikom
    display = al_create_display(1280, 720);//stworzenie okna
    queue = al_create_event_queue();//stworzenie kolejki wydarzen
    timer = al_create_timer(1.0 / 60);//stworzenie timera(60kl/s)
    font = al_load_ttf_font("ttf/ObelixProIt-cyr.ttf", 32, 0);//zaladowanie czcionki

    //instalacja klawiatury, enYszy i audio
    al_install_audio();
    al_install_keyboard();
    al_install_mouse();
    //rejestracja  rodzajow event'ow
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    al_start_timer(timer);//uruchomienie timer'a

    //obsluga plikow audio
    al_reserve_samples(1);
    sample = al_load_sample("wav/d-_-b.wav");
    sampleInstance = al_create_sample_instance(sample);
    al_attach_sample_instance_to_mixer(sampleInstance, al_get_default_mixer());
    al_set_sample_instance_gain(sampleInstance, 0.1);
    //zmienne z granicami okna oraz pola gry
    int rightSide = al_get_display_width(display), leftSide = 0;
    int bottomSide = al_get_display_height(display), topSide = 0;
    int prawaBanda = rightSide - rightSide / 4, lewaBanda = leftSide + rightSide / 4;
    //załadowanie sprite'ow gracza oraz przeciwnika
    player = al_load_bitmap("img/statek.png");
    enemy1 = al_load_bitmap("img/enemy1.png");
    al_convert_mask_to_alpha(player, al_map_rgb(255, 255, 255));//utworzenie kanału alfa na kolorze białym
    al_convert_mask_to_alpha(enemy1, al_map_rgb(255, 255, 255));//^

    float x = rightSide / 2, y = bottomSide - 10;//pozycja startowa gracza(srodek)
    // Pozycja i rozmiar przeciwnika
    int enemyWidth = al_get_bitmap_width(enemy1);
    int enemyHeight = al_get_bitmap_height(enemy1);
    int enX = 0, enY = 0;

    // Pozycja i rozmiar gracza
    int graczX = x - 16, graczY = y - 32;
    int graczWidth = al_get_bitmap_width(player);
    int graczHeight = al_get_bitmap_height(player);

    //zmienne przechowujace zycia, wynik i nazwe gracza
    int zycia = 3;
    char zyciaText[20];
    int wynik = 0;
    char wynikText[20];
    char nick[100];

    // Pobierz nick od gracza
    getNick(nick);

    //głowna petla gry
    bool running = true;
    while (running)
    {
        sprintf_s(wynikText, sizeof(wynikText), "Wynik: %d", wynik);//przeksztalcenie int na tablice znakow
        sprintf_s(zyciaText, sizeof(zyciaText), "Zycia: %d", zycia);//^
        al_play_sample_instance(sampleInstance);//odpalenie muzyki w tle

        //mozliwosc zamkniecia programu poprzez klikniecie X 
        al_wait_for_event(queue, &event);
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            running = false;

        //wstepne poruszanie sie przeciwnikiem dziki myszce
        if (event.type == ALLEGRO_EVENT_MOUSE_AXES)
        {
            enX = event.mouse.x;
            enY = event.mouse.y;
        }

        poruszanie(&graczX, prawaBanda, lewaBanda);//poruszanie gracza
        //wstepne ustawianie wyniku LPM
        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
        {
            wynik++;

        }

        if (event.type == ALLEGRO_EVENT_TIMER)
        {
            al_clear_to_color(al_map_rgb(255, 255, 255));//ustawienie koloru tła
            al_draw_line(prawaBanda, topSide, prawaBanda, bottomSide, al_map_rgb(0, 0, 0), 3);//zaznaczenie granic
            al_draw_line(lewaBanda, topSide, lewaBanda, bottomSide, al_map_rgb(0, 0, 0), 3);//^
            al_draw_bitmap(player, graczX, graczY, 0);//rysowanie sprite'a gracza
            al_draw_bitmap(enemy1, enX, enY, 0);//rysowanie sprite'a przeciwnika

            // Sprawdzenie kolizji
            if (collision(graczX, graczY, graczWidth, graczHeight,
                enX, enY, enemyWidth, enemyHeight)) {
                zycia--;
                al_set_mouse_xy(display, x, topSide + 20);

            }

            al_draw_text(font, al_map_rgb(0, 0, 0), 10, 30, ALLEGRO_ALIGN_LEFT, zyciaText);//atkualizowanie zyc
            al_draw_text(font, al_map_rgb(0, 0, 0), 10, 70, ALLEGRO_ALIGN_LEFT, wynikText);//aktualizowanie wyniku

            // Wyświetl nick gracza w lewym dolnym rogu
            al_draw_text(font, al_map_rgb(0, 0, 0), 10, bottomSide - 50, ALLEGRO_ALIGN_LEFT, nick);

            if (zycia == 0)
            {
                al_show_native_message_box(display, "Przegrana", ":/", "lipa", NULL, NULL);//wyswietlenie powiadomienia o przegranej
                running = false;//zamkniecie programu po przegranej
            }

            al_flip_display();//przesłanie bierzącej klatki do ekranu

        }

    }

    //zwolnienie miejsca
    al_destroy_display(display);
    al_uninstall_keyboard();
    al_uninstall_mouse();
    al_destroy_timer(timer);
    al_destroy_sample(sample);
    al_destroy_sample_instance(sampleInstance);
    al_uninstall_audio();
    al_destroy_font(font);
    return 0;
}

void poruszanie(int* graczX, int prawaBanda, int lewaBanda)//funkcja poruszania sie gracza
{
    ALLEGRO_KEYBOARD_STATE  keyState;
    al_get_keyboard_state(&keyState);
    if (*graczX + 40 <= prawaBanda)//blokada wyjscia poza pole rozgrywki
    {
        if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT))
        {
            if (al_key_down(&keyState, ALLEGRO_KEY_LCTRL))
            {
                *graczX += 5;//"sprint" w prawo

            }
            else
            {
                *graczX += 2;//zwykly ruch w prawo

            }
        }
    }
    if (*graczX >= lewaBanda)//blokada wyjscia poza pole rozgrywki
        if (al_key_down(&keyState, ALLEGRO_KEY_LEFT))
            if (al_key_down(&keyState, ALLEGRO_KEY_LCTRL))
            {
                *graczX -= 5;//"sprint" w lewo

            }
            else
            {
                *graczX -= 2;//zwykly ruch w prawo

            }
}

//funkcja wykrywajaca kolizje
bool collision(int obiekt1X, int obiekt1Y, int obiekt1Width, int obiekt1Height, int obiekt2X, int obiekt2Y, int obiekt2Width, int obiekt2Height) {
    return (obiekt1X< obiekt2X + obiekt2Width &&
        obiekt1X + obiekt1Width > obiekt2X &&
        obiekt1Y < obiekt2Y + obiekt2Height &&
        obiekt1Y + obiekt1Height > obiekt2Y);
}

// Funkcja do pobrania nicku od gracza

void getNick(char nick[])
{
    al_show_native_message_box(NULL, "Podaj swoj nick", "Nick", "", NULL, ALLEGRO_MESSAGEBOX_QUESTION);
    fgets(nick, 100, stdin); // Pobranie nicku z konsoli 
}

