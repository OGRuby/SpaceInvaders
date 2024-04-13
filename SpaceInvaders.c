#include <stdio.h>
#include <allegro5/allegro.h>//głowna biblioteka allegro
#include <allegro5/allegro_image.h>//oblsuga zdjec
#include <allegro5/allegro_audio.h>//obsluga audio
#include <allegro5/allegro_acodec.h>//^
#include <allegro5/allegro_primitives.h>//obsluga ksztaltow
#include <allegro5/allegro_font.h>//oblsuga czcionek i pisanie
#include <allegro5/allegro_ttf.h>//^
#include <allegro5/allegro_native_dialog.h>//obsluga wyskakujacych okienek powiadomien

#define num_aliens 6 // Liczba kosmitów
#define alien_size 15 // Rozmiar kosmitów

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

    enum direction { UP, DOWN, LEFT, RIGHT };

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

    int alien_x[num_aliens], alien_y[num_aliens]; // tablice pozycji kosmitów ( nie mogę przez zmienna [naprawić]) ?!?!
    bool alive[num_aliens]; // tablica żyć kosmitów ( nie mogę przez zmienna [naprawić]) ?!?!

    int movespeed_alien = 2; // prędkość kosmitów
    int dir2 = LEFT; // deklaracja ruchu kosmitów

    // Pozycja i rozmiar gracza
    int graczX = x - 16, graczY = y - 32;
    int graczWidth = al_get_bitmap_width(player);
    int graczHeight = al_get_bitmap_height(player);

    //zmienne przechowujace zycia i wynik gracz
    int zycia = 3;
    char zyciaText[20];
    int wynik = 0;
    char wynikText[20];
    char nick[100];

    // Pobierz nick od gracza
    getNick(nick);

    int bullet_x = -10, bullet_y = -10; // Pozycja pocisku
    bool bullet_fired = false; // Flaga informująca, czy pocisk został wystrzelony

    // Inicjalizacja pozycji kosmitów
    for (int i = 0; i < num_aliens; ++i) {
        alien_x[i] = i * 80 + lewaBanda;
        alien_y[i] = 10;
        alive[i] = true;
    }

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

        

        if (event.type == ALLEGRO_EVENT_KEY_UP)
        {
            ALLEGRO_KEYBOARD_STATE keystate;
            al_get_keyboard_state(&keystate);

            if (al_key_down(&keystate, ALLEGRO_KEY_SPACE)) // wystrzelenie pocisku
            {
                if (!bullet_fired) // sprawdza istnienie pocisku ( strzela tylko jeden na raz)
                {
                    bullet_x = graczX + 2; // kordynaty pocisku
                    bullet_y = graczY - 5;
                    bullet_fired = true;
                }
            }
        }

        // Poruszanie pocisku
        if (bullet_fired)
        {
            bullet_y -= 8; // zmniejszono wartość ruchu pocisku
            if (bullet_y < 0) // Sprawdza czy dotarł do krawędzi
            {
                bullet_fired = false;
            }
            else // Sprawdź kolizję z kosmitami
            {
                for (int i = 0; i < num_aliens; ++i) {
                    if (alive[i]) {
                        if (bullet_x > alien_x[i] && bullet_x < alien_x[i] + alien_size)
                        {
                            if (bullet_y > alien_y[i] && bullet_y < alien_y[i] + alien_size)
                            {
                                bullet_fired = false;
                                alive[i] = false;
                                movespeed_alien = movespeed_alien + 1; // co zabicie kozmita przyśpiesza
                            }
                        }
                    }
                }
            }
        }


        poruszanie(&graczX, prawaBanda, lewaBanda);//poruszanie gracza
        //wstepne ustawianie wyniku LPM
        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
        {
            wynik++;

        }

        // Poruszanie automatycznie kosmitów
        for (int i = 0; i < num_aliens; ++i) {
            if (alive[i]) 
            {
                if (graczY > alien_y[1])
                {
                    if (dir2 == LEFT)
                    {
                        if (alien_x[i] < prawaBanda - alien_size) // Sprawdza czy dotarł do krawędzi
                        {
                            alien_x[i] += movespeed_alien;
                        }
                        else
                        {
                            dir2 = RIGHT; // zmiana kierunku
                            for (int i = 0; i < num_aliens; ++i)
                            {
                                alien_y[i] += 25; // opadnięcie
                            }
                        }
                    }
                    else if (dir2 == RIGHT)
                    {
                        if (alien_x[i] > lewaBanda) // Sprawdza czy dotarł do krawędzi
                        {
                            alien_x[i] -= movespeed_alien;
                        }
                        else
                        {
                            dir2 = LEFT; // zmiana kierunku
                            for (int i = 0; i < num_aliens; ++i)
                            {
                                alien_y[i] += 25; // opadnięcie
                            }
                        }
                    }
                }
                else
                {
                    --zycia;
                    for (int i = 0; i < num_aliens; ++i)
                    {
                        alien_y[i] = 10; // powrot
                    }
                }
            }
        }

        if (event.type == ALLEGRO_EVENT_TIMER)
        {
            al_clear_to_color(al_map_rgb(255, 255, 255));//ustawienie koloru tła
            al_draw_line(prawaBanda, topSide, prawaBanda, bottomSide, al_map_rgb(0, 0, 0), 3);//zaznaczenie granic
            al_draw_line(lewaBanda, topSide, lewaBanda, bottomSide, al_map_rgb(0, 0, 0), 3);//^
            al_draw_bitmap(player, graczX, graczY, 0);//rysowanie sprite'a gracza
            

            for (int i = 0; i < num_aliens; ++i) //Kosmita
            {
                if (alive[i])
                {
                    al_draw_bitmap(enemy1,alien_x[i], alien_y[i],0);
                }
            }
            if (bullet_fired) // pocisk
            {
                al_draw_filled_rectangle(bullet_x, bullet_y, bullet_x + 6, bullet_y + 6, al_map_rgb(255, 0, 0));
            }
            al_flip_display();






            
            al_draw_text(font, al_map_rgb(0, 0, 0), 10, bottomSide - 50, ALLEGRO_ALIGN_LEFT, nick);

            al_draw_text(font, al_map_rgb(0, 0, 0), 10, 30, ALLEGRO_ALIGN_LEFT, zyciaText);//atkualizowanie zyc
            al_draw_text(font, al_map_rgb(0, 0, 0), 10, 70, ALLEGRO_ALIGN_LEFT, wynikText);//aktualizowanie wyniku
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
void getNick(char nick[])
{
    al_show_native_message_box(NULL, "Podaj swoj nick", "Nick", "", NULL, ALLEGRO_MESSAGEBOX_QUESTION);
    fgets(nick, 100, stdin); // Pobranie nicku z konsoli 
}
