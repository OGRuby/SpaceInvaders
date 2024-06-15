#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_native_dialog.h>

#define num_aliens_per_row 19 // Liczba kosmitów na rząd
#define alien_size 27 // Rozmiar kosmitów
#define Width_sreen 1280 // Szerokość
#define Height_sreen 720 // Wysokość
#define MAX_SCORES 100

typedef struct {
    char nazwa[50];
    int punkty;
} Gracz;

// Struktura do przechowywania wyniku
typedef struct {
    char name[50];
    int score;
} Score;

// Inicjalizacja funkcji
void poruszanie(int* graczX, int prawaBanda, int lewaBanda);
bool collision(int graczX, int graczY, int w1, int h1, int x2, int y2, int w2, int h2);
void Insert_Name(ALLEGRO_DISPLAY* display, ALLEGRO_FONT* font, char* player_name);
void save_score(const char* filename, const char player_name[50], int score);
void wyswietl_10_najlepszych(Gracz tablica_wynikow[], int liczba_wynikow, ALLEGRO_FONT* font);
int odczytaj_wyniki(Gracz tablica_wynikow[], int max_wynikow);
void Print_score(Gracz* gracz);


int main() {
    // Utworzenie znaczników
    ALLEGRO_DISPLAY* display = NULL;
    ALLEGRO_EVENT_QUEUE* queue = NULL;
    ALLEGRO_TIMER* timer = NULL;
    ALLEGRO_BITMAP* player = NULL;
    ALLEGRO_BITMAP* enemy1 = NULL;
    ALLEGRO_BITMAP* enemy2 = NULL;
    ALLEGRO_BITMAP* enemy3 = NULL;
    ALLEGRO_BITMAP* tlo1 = NULL;
    ALLEGRO_BITMAP* bok1 = NULL;
    ALLEGRO_BITMAP* bok2 = NULL;
    ALLEGRO_SAMPLE* gameTheme = NULL;
    ALLEGRO_SAMPLE_INSTANCE* gameThemeInstance = NULL;
    ALLEGRO_SAMPLE* shootTheme = NULL;
    ALLEGRO_SAMPLE_INSTANCE* shootThemeInstance = NULL;
    ALLEGRO_SAMPLE* invaderKillTheme = NULL;
    ALLEGRO_SAMPLE_INSTANCE* invaderKillThemeInstance = NULL;

    ALLEGRO_FONT* font = NULL;
    ALLEGRO_FONT* fontLogo = NULL;
    ALLEGRO_EVENT event; // Utworzenie zmiennej do obsługi event'ów

    enum direction { UP, DOWN, LEFT, RIGHT };

    al_init(); // Inicjalizacja Allegro
    al_init_acodec_addon(); // Inicjalizacja addon'ów obsługujących muzykę
    al_init_primitives_addon(); // Inicjalizacja addon'ów obsługujących rysowanie figur
    al_init_image_addon(); // Inicjalizacja addon'ów obsługujących obrazy
    al_init_font_addon(); // Inicjalizacja addon'ów obsługujących czcionki oraz pisanie
    al_init_ttf_addon(); // ^

    // Przypisanie wartości znacznikom
    display = al_create_display(Width_sreen, Height_sreen); // Stworzenie okna
    queue = al_create_event_queue(); // Stworzenie kolejki wydarzeń
    timer = al_create_timer(1.0 / 60); // Stworzenie timera (60 kl/s)
    font = al_load_ttf_font("ttf/ARCADE_I.ttf", 24, 0); // Załadowanie czcionki
    fontLogo = al_load_ttf_font("ttf/ARCADE_I.ttf", 64, 0); // Załadowanie czcionki

    // Instalacja klawiatury, myszy i audio
    al_install_audio();
    al_install_keyboard();


    // Rejestracja rodzajów event'ów
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    al_start_timer(timer); // Uruchomienie timera

    // Obsługa plików audio
    al_reserve_samples(3);
    gameTheme = al_load_sample("wav/gameTheme.wav");
    gameThemeInstance = al_create_sample_instance(gameTheme);
    al_attach_sample_instance_to_mixer(gameThemeInstance, al_get_default_mixer());
    al_set_sample_instance_gain(gameThemeInstance, 1);
    al_set_sample_instance_playmode(gameThemeInstance, ALLEGRO_PLAYMODE_LOOP);

    shootTheme = al_load_sample("wav/shoot.wav");
    shootThemeInstance = al_create_sample_instance(shootTheme);
    al_attach_sample_instance_to_mixer(shootThemeInstance, al_get_default_mixer());
    al_set_sample_instance_gain(shootThemeInstance, 1);
    al_set_sample_instance_playmode(gameThemeInstance, ALLEGRO_PLAYMODE_ONCE);

    invaderKillTheme = al_load_sample("wav/invaderkilled.wav");
    invaderKillThemeInstance = al_create_sample_instance(invaderKillTheme);
    al_attach_sample_instance_to_mixer(invaderKillThemeInstance, al_get_default_mixer());
    al_set_sample_instance_gain(invaderKillThemeInstance, 0.7);
    al_set_sample_instance_playmode(invaderKillThemeInstance, ALLEGRO_PLAYMODE_ONCE);

    // Zmienne z granicami okna oraz pola gry
    int rightSide = al_get_display_width(display), leftSide = 0;
    int bottomSide = al_get_display_height(display), topSide = 0;
    int prawaBanda = rightSide - rightSide / 4, lewaBanda = leftSide + rightSide / 4;

    // Załadowanie sprite'ów gracza oraz przeciwnika
    player = al_load_bitmap("img/player.png");
    enemy1 = al_load_bitmap("img/enemy1.png");
    enemy2 = al_load_bitmap("img/enemy2.png");
    enemy3 = al_load_bitmap("img/enemy3.png");
    tlo1 = al_load_bitmap("img/tlo1.png");
    bok1 = al_load_bitmap("img/bok1.png");
    bok2 = al_load_bitmap("img/bok2.png");
    al_convert_mask_to_alpha(player, al_map_rgb(0, 0, 0)); // Utworzenie kanału alfa na kolorze białym
    al_convert_mask_to_alpha(enemy1, al_map_rgb(0, 0, 0)); // ^

    float skala_szerokosci = 15.0 / 420.0;  // Współczynnik skalowania szerokości
    float skala_wysokosci = skala_szerokosci * (300.0 / 420.0); // Współczynnik skalowania wysokości

    float x = rightSide / 2, y = bottomSide - 10; // Pozycja startowa gracza (środek)
    // Pozycja i rozmiar przeciwnika
    int enemyWidth = al_get_bitmap_width(enemy2);
    int enemyHeight = al_get_bitmap_height(enemy2);
    int enX = 0, enY = 0;


    int num_rows = 6; // Liczba rzędów kosmitów
    int alive_row = num_rows - 1; // żyjące rzędy
    int dead = 0;

    int num_aliens = num_aliens_per_row * num_rows;// Całkowita liczba kosmitów
    int aliens_counter = num_aliens;

    // Dynamiczna alokacja tablic pozycji i życia kosmitów
    float* alien_x = (float*)malloc(num_aliens * sizeof(float));
    float* alien_y = (float*)malloc(num_aliens * sizeof(float));
    bool* alive = (bool*)malloc(num_aliens * sizeof(bool));

    float movespeed_alien = 0.9; // Prędkość kosmitów
    int dir2 = LEFT; // Deklaracja ruchu kosmitów

    // Pozycja i rozmiar gracza
    int graczX = x - 16, graczY = y - 32;
    int graczWidth = al_get_bitmap_width(player);
    int graczHeight = al_get_bitmap_height(player);

    // Zmienne przechowujące życia i wynik gracza
    int zycia = 3;
    char zyciaText[20];
    int wynik = 0;
    char wynikText[20];
    char player_name[50];
    const char* filename = "scores.txt";


    int bullet_x = -10, bullet_y = -10; // Pozycja pocisku
    bool bullet_fired = false; // Flaga informująca, czy pocisk został wystrzelony

    




    int oknoGry = 0;
    int wyborMenu = 1;
    int wyborTrudnosci = 1;
    // Główna pętla gry

    bool running = true;
    while (running) {

        // Możliwość zamknięcia programu poprzez kliknięcie X
        al_wait_for_event(queue, &event);
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            running = false;

        if (oknoGry == 0)
        {

            if (event.type == ALLEGRO_EVENT_TIMER) {

                al_draw_scaled_bitmap(tlo1, 0, 0, al_get_bitmap_width(tlo1), al_get_bitmap_height(tlo1), 0, 0, rightSide, Height_sreen, 0);
                al_draw_text(fontLogo, al_map_rgb(0, 255, 0), rightSide / 2, topSide + 128, ALLEGRO_ALIGN_CENTER, "SPACE INVADERS");
                if (wyborMenu == 1)
                    al_draw_text(font, al_map_rgb(255, 255, 255), rightSide / 2, bottomSide / 2 - 34, ALLEGRO_ALIGN_CENTER, ">START");
                else
                    al_draw_text(font, al_map_rgb(0, 255, 0), rightSide / 2, bottomSide / 2 - 34, ALLEGRO_ALIGN_CENTER, "START");

                if (wyborMenu == 2)
                    al_draw_text(font, al_map_rgb(255, 255, 255), rightSide / 2, bottomSide / 2, ALLEGRO_ALIGN_CENTER, ">BEST SCORE");
                else
                    al_draw_text(font, al_map_rgb(0, 255, 0), rightSide / 2, bottomSide / 2, ALLEGRO_ALIGN_CENTER, "BEST SCORE");

                if (wyborMenu == 3)
                    al_draw_text(font, al_map_rgb(255, 255, 255), rightSide / 2, bottomSide / 2 + 34, ALLEGRO_ALIGN_CENTER, ">QUIT");
                else
                    al_draw_text(font, al_map_rgb(0, 255, 0), rightSide / 2, bottomSide / 2 + 34, ALLEGRO_ALIGN_CENTER, "QUIT");


            }
            if (wyborMenu > 1)
            {
                if (event.type == ALLEGRO_EVENT_KEY_DOWN)
                {
                    if (event.keyboard.keycode == ALLEGRO_KEY_UP)
                        wyborMenu--;
                }
            }
            if (wyborMenu < 3)
            {
                if (event.type == ALLEGRO_EVENT_KEY_DOWN)
                {
                    if (event.keyboard.keycode == ALLEGRO_KEY_DOWN)
                        wyborMenu++;
                }
            }
            if (event.type == ALLEGRO_EVENT_KEY_DOWN)
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_ENTER)
                    switch (wyborMenu)
                    {
                    case 1:

                        oknoGry = 1;
                        break;
                    case 2:
                        oknoGry = 3;
                        break;
                    case 3:
                        running = false;
                        break;
                    default:
                        break;
                    }
            }
            // Inicjalizacja pozycji kosmitów
            for (int row = 0; row < num_rows; ++row) {
                for (int i = 0; i < num_aliens_per_row; ++i) {
                    int idx = row * num_aliens_per_row + i;
                    alien_x[idx] = i * 30 + lewaBanda;
                    alien_y[idx] = 25 + row * 40;
                    alive[idx] = true;
                }
            }
            wynik = 0;
            zycia = 3;
            movespeed_alien = 0.8;

        }
        if (oknoGry == 1)
        {
            if (event.type == ALLEGRO_EVENT_TIMER) {

                al_draw_scaled_bitmap(tlo1, 0, 0, al_get_bitmap_width(tlo1), al_get_bitmap_height(tlo1), 0, 0, rightSide, Height_sreen, 0);
                al_draw_text(fontLogo, al_map_rgb(0, 255, 0), rightSide / 2, topSide + 128, ALLEGRO_ALIGN_CENTER, "POZIOM  TRUDNOSCI");
                if (wyborTrudnosci == 1)
                    al_draw_text(font, al_map_rgb(255, 255, 255), rightSide / 2, bottomSide / 2 - 34, ALLEGRO_ALIGN_CENTER, ">LATWY");
                else
                    al_draw_text(font, al_map_rgb(0, 255, 0), rightSide / 2, bottomSide / 2 - 34, ALLEGRO_ALIGN_CENTER, "LATWY");

                if (wyborTrudnosci == 2)
                    al_draw_text(font, al_map_rgb(255, 255, 255), rightSide / 2, bottomSide / 2, ALLEGRO_ALIGN_CENTER, ">TRUDNY");
                else
                    al_draw_text(font, al_map_rgb(0, 255, 0), rightSide / 2, bottomSide / 2, ALLEGRO_ALIGN_CENTER, "TRUDNY");




            }
            if (wyborTrudnosci > 1)
            {
                if (event.type == ALLEGRO_EVENT_KEY_DOWN)
                {
                    if (event.keyboard.keycode == ALLEGRO_KEY_UP)
                        wyborTrudnosci--;
                }
            }
            if (wyborTrudnosci < 2)
            {
                if (event.type == ALLEGRO_EVENT_KEY_DOWN)
                {
                    if (event.keyboard.keycode == ALLEGRO_KEY_DOWN)
                        wyborTrudnosci++;
                }
            }

            if (event.type == ALLEGRO_EVENT_KEY_DOWN)
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT)

                    switch (wyborTrudnosci)
                    {
                    case 1:

                        Insert_Name(display, font, player_name, tlo1,rightSide);
                        oknoGry = 2;

                        break;
                    case 2:
                        movespeed_alien *= 2;
                        Insert_Name(display, font, player_name, tlo1,rightSide);
                        oknoGry = 2;
                        break;

                    default:
                        break;
                    }



            }
            if (event.type == ALLEGRO_EVENT_KEY_DOWN)
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                    oknoGry = 0;
            }
        }
        if (oknoGry == 2)
        {

            sprintf_s(wynikText, sizeof(wynikText), "Wynik: %d", wynik); // Przekształcenie int na tablicę znaków
            sprintf_s(zyciaText, sizeof(zyciaText), "Zycia: %d", zycia); // ^
            al_play_sample_instance(gameThemeInstance); // Odpalenie muzyki w tle




            if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                if (event.keyboard.keycode == ALLEGRO_KEY_SPACE) { // Wystrzelenie pocisku
                    if (!bullet_fired) { // Sprawdza istnienie pocisku (strzela tylko jeden na raz)
                        bullet_x = graczX + 17; // Koordynaty pocisku
                        bullet_y = graczY + 40;
                        bullet_fired = true;
                        al_play_sample_instance(shootThemeInstance);
                    }
                }
            }

            // Poruszanie pocisku
            if (bullet_fired) {
                bullet_y -= 12;
                if (bullet_y < 0) { // Sprawdza, czy dotarł do krawędzi
                    bullet_fired = false;
                }
                else { // Sprawdź kolizję z kosmitami
                    for (int i = 0; i < num_aliens; ++i) {
                        if (alive[i]) {
                            if (bullet_x > alien_x[i] && bullet_x < alien_x[i] + alien_size+5) {
                                if (bullet_y > alien_y[i] && bullet_y < alien_y[i] + alien_size) {
                                    bullet_fired = false;
                                    alive[i] = false;
                                    aliens_counter--;
                                    if (i < 38)
                                        wynik += 3;
                                    else if (i >= 38 && i < 76)
                                        wynik += 2;
                                    else if (i >= 76)
                                        wynik++;
                                    al_play_sample_instance(invaderKillThemeInstance);
                                }
                            }
                        }
                    }
                }
            }

            poruszanie(&graczX, prawaBanda, lewaBanda); // Poruszanie gracza

            // Sprawdzanie czy linia kozmitów żyje
            if (alive_row >= 0)
            {

                for (int i = 0; i < num_aliens_per_row; ++i)
                {
                    int check = alive_row * num_aliens_per_row + i;
                    if (alive[check] == false)
                    {
                        dead = dead + 1;
                    }
                }

                if (dead == num_aliens_per_row)
                {
                    --alive_row;
                }
                else
                {
                    dead = 0;
                }
            }
            else
            {
                for (int row = 0; row < num_rows; ++row) {
                    for (int i = 0; i < num_aliens_per_row; ++i) {
                        int idx = row * num_aliens_per_row + i;
                        alien_x[idx] = i * 80 + lewaBanda;
                        alien_y[idx] = 10 + row * 40;
                        alive[idx] = true;
                    }
                }
            }

            // Poruszanie automatycznie kosmitów
            for (int i = 0; i < num_aliens; ++i) {
                if (alive[i]) {
                    if (graczY-10 > alien_y[alive_row * num_aliens_per_row]) {
                        if (dir2 == LEFT) {
                            if (alien_x[i] < prawaBanda - alien_size) { // Sprawdza, czy dotarł do krawędzi
                                alien_x[i] += movespeed_alien;
                            }
                            else {
                                dir2 = RIGHT; // Zmiana kierunku
                                for (int j = 0; j < num_aliens; ++j) {
                                    alien_y[j] += 20; // Opadnięcie
                                }
                            }
                        }
                        else if (dir2 == RIGHT) {
                            if (alien_x[i] > lewaBanda) { // Sprawdza, czy dotarł do krawędzi
                                alien_x[i] -= movespeed_alien;
                            }
                            else {
                                dir2 = LEFT; // Zmiana kierunku
                                for (int j = 0; j < num_aliens; ++j) {
                                    alien_y[j] += 20; // Opadnięcie
                                }
                            }
                        }
                    }
                    else {
                        --zycia;

                        for (int row = 0; row < num_rows; ++row) {
                            for (int i = 0; i < num_aliens_per_row; ++i) {
                                int idx = row * num_aliens_per_row + i;
                                alien_y[idx] = 10 + row * 40; // Powrót
                            }
                        }
                    }
                }
            }
            if (num_aliens == 0) {
                save_score(filename, player_name, wynik);
                oknoGry = 5;
            }
            if (event.type == ALLEGRO_EVENT_TIMER) {
                al_draw_bitmap(bok1, 0,0,0);
                al_draw_bitmap(bok2, prawaBanda, 0, 0);
                al_draw_scaled_bitmap(tlo1, 0, 0, al_get_bitmap_width(tlo1), al_get_bitmap_height(tlo1),lewaBanda, 0, prawaBanda - lewaBanda, Height_sreen,  0);
                al_draw_line(prawaBanda, topSide, prawaBanda, bottomSide, al_map_rgb(0, 0, 0), 3); // Zaznaczenie granic
                al_draw_line(lewaBanda, topSide, lewaBanda, bottomSide, al_map_rgb(0, 0, 0), 3); // ^
                al_draw_bitmap(player, graczX, graczY, 0); // Rysowanie sprite'a gracza

                for (int i = 0; i < num_aliens; ++i) { // Kosmita
                    if (alive[i]) {
                        if(i<38)
                            al_draw_bitmap(enemy3, alien_x[i], alien_y[i], 0);
                        else if(i >= 38 && i<76)
                            al_draw_bitmap(enemy2, alien_x[i], alien_y[i], 0);
                        else if(i>=76)
                            al_draw_bitmap(enemy1, alien_x[i], alien_y[i], 0);
                    }
                }

                if (bullet_fired) { // Pocisk
                    al_draw_filled_rectangle(bullet_x, bullet_y, bullet_x + 3, bullet_y + 12, al_map_rgb(255, 255, 255));

                }

                al_draw_text(font, al_map_rgb(255, 255, 255), 10, bottomSide - 50, ALLEGRO_ALIGN_LEFT, player_name);
                al_draw_text(font, al_map_rgb(255, 255, 255), 10, 30, ALLEGRO_ALIGN_LEFT, zyciaText); // Aktualizowanie żyć
                al_draw_text(font, al_map_rgb(255, 255, 255), 10, 70, ALLEGRO_ALIGN_LEFT, wynikText); // Aktualizowanie wyniku

                if (zycia == 0) {
                    save_score(filename, player_name, wynik);
                    oknoGry = 4;
                }
                if (aliens_counter == 0) {
                    save_score(filename, player_name, wynik);
                    oknoGry = 5;
                }

                if (event.type == ALLEGRO_EVENT_KEY_DOWN)
                {
                    if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                        oknoGry = 0;
                }

            }
        }
        if (oknoGry == 3)
        {
            if (event.type == ALLEGRO_EVENT_TIMER) {
                al_draw_scaled_bitmap(tlo1, 0, 0, al_get_bitmap_width(tlo1), al_get_bitmap_height(tlo1), 0, 0, rightSide, Height_sreen, 0);

                Print_score(fontLogo, font, rightSide, topSide);

            }
            if (event.type == ALLEGRO_EVENT_KEY_DOWN)
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                    oknoGry = 0;
            }
        }
        if (oknoGry == 4)
        {
            if (event.type == ALLEGRO_EVENT_TIMER) {
                al_draw_scaled_bitmap(tlo1, 0, 0, al_get_bitmap_width(tlo1), al_get_bitmap_height(tlo1), 0, 0, rightSide, Height_sreen, 0);
                al_draw_text(fontLogo, al_map_rgb(0, 255, 0), rightSide / 2, topSide + 128, ALLEGRO_ALIGN_CENTER, "Przegrana");
            }
            if (event.type == ALLEGRO_EVENT_KEY_DOWN)
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                    oknoGry = 0;
            }
        }
        if (oknoGry == 5)
        {
            if (event.type == ALLEGRO_EVENT_TIMER) {
                al_draw_scaled_bitmap(tlo1, 0, 0, al_get_bitmap_width(tlo1), al_get_bitmap_height(tlo1), 0, 0, rightSide, Height_sreen, 0);
                al_draw_text(fontLogo, al_map_rgb(0, 255, 0), rightSide / 2, topSide + 128, ALLEGRO_ALIGN_CENTER, "Wygrana");
            }
            if (event.type == ALLEGRO_EVENT_KEY_DOWN)
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                    oknoGry = 0;
            }
        }
        al_flip_display(); // Przesłanie bieżącej klatki do ekranu
    }


    // Zwolnienie miejsca
    free(alien_x);
    free(alien_y);
    free(alive);
    al_destroy_display(display);
    al_uninstall_keyboard();

    al_destroy_timer(timer);
    al_destroy_sample(gameTheme);
    al_destroy_sample_instance(gameThemeInstance);
    al_destroy_sample(shootTheme);
    al_destroy_sample_instance(shootThemeInstance);
    al_destroy_sample(invaderKillTheme);
    al_destroy_sample_instance(invaderKillThemeInstance);

    al_uninstall_audio();
    al_destroy_font(font);
    return 0;
}

void poruszanie(int* graczX, int prawaBanda, int lewaBanda) {
    ALLEGRO_KEYBOARD_STATE keyState;
    al_get_keyboard_state(&keyState);
    if (*graczX + 40 <= prawaBanda) { // Blokada wyjścia poza pole rozgrywki
        if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT)) {
            if (al_key_down(&keyState, ALLEGRO_KEY_LCTRL)) {
                *graczX += 8; // "Sprint" w prawo
            }
            else {
                *graczX += 4; // Zwykły ruch w prawo
            }
        }
    }
    if (*graczX >= lewaBanda) { // Blokada wyjścia poza pole rozgrywki
        if (al_key_down(&keyState, ALLEGRO_KEY_LEFT)) {
            if (al_key_down(&keyState, ALLEGRO_KEY_LCTRL)) {
                *graczX -= 8; // "Sprint" w lewo
            }
            else {
                *graczX -= 4; // Zwykły ruch w lewo
            }
        }
    }
}


void Insert_Name(ALLEGRO_DISPLAY* ekran, ALLEGRO_FONT* czcionka, char* player_name, ALLEGRO_BITMAP* tlo1,int rightSide) {
    ALLEGRO_EVENT_QUEUE* kolejka_zdarzen = al_create_event_queue();
    al_register_event_source(kolejka_zdarzen, al_get_keyboard_event_source());

    bool wprowadzanie = true;
    char nazwa[50] = "";

    while (wprowadzanie) {
        ALLEGRO_EVENT zdarzenie;
        al_wait_for_event(kolejka_zdarzen, &zdarzenie);

        if (zdarzenie.type == ALLEGRO_EVENT_KEY_DOWN) {
            int kod_klawisza = zdarzenie.keyboard.keycode;

            if (kod_klawisza == ALLEGRO_KEY_ENTER) {
                wprowadzanie = false;
            }
            else if (kod_klawisza == ALLEGRO_KEY_BACKSPACE) {
                if (strlen(nazwa) > 0) {
                    nazwa[strlen(nazwa) - 1] = '\0';
                }
            }
            else if (kod_klawisza >= ALLEGRO_KEY_A && kod_klawisza <= ALLEGRO_KEY_Z) {
                char nowy_znak = 'A' + (kod_klawisza - ALLEGRO_KEY_A);
                if (strlen(nazwa) < sizeof(nazwa) - 1) {
                    int len = strlen(nazwa);
                    nazwa[len] = nowy_znak;
                    nazwa[len + 1] = '\0';
                }
            }
        }

        al_draw_scaled_bitmap(tlo1, 0, 0, al_get_bitmap_width(tlo1), al_get_bitmap_height(tlo1), 0, 0, rightSide, Height_sreen, 0);
        al_draw_text(czcionka, al_map_rgb(255, 255, 255), Width_sreen / 2, Height_sreen / 2 - 20,
            ALLEGRO_ALIGN_CENTER, "Wprowadz swoja nazwe:");
        al_draw_text(czcionka, al_map_rgb(255, 255, 255), Width_sreen / 2, Height_sreen / 2 + 20,
            ALLEGRO_ALIGN_CENTER, nazwa);
        al_flip_display();
    }


    al_destroy_event_queue(kolejka_zdarzen);
    strcpy_s(player_name, 49, nazwa);
    player_name[49] = '\0'; // Upewnij się, że string jest zakończony null
}




void save_score(const char* filename, const char player_name[50], int score) {
    FILE* file = fopen(filename, "a"); // Otwieramy plik w trybie dołączania
    if (file == NULL) {
        perror("Nie można otworzyć pliku do zapisu");
        return;
    }
    fprintf(file, "%s %d\n", player_name, score);
    fclose(file);
}

void wyswietl_10_najlepszych(Gracz tablica_wynikow[], int liczba_wynikow, ALLEGRO_FONT* fontLogo, ALLEGRO_FONT* font, int rightSide, int topSide) {
    ALLEGRO_COLOR color = al_map_rgb(0, 255, 0);
    int y = 100;



    al_draw_textf(fontLogo, color, rightSide / 2, topSide + 64, ALLEGRO_ALIGN_CENTER, "Najlepsze wyniki:");

    for (int i = 0; i < 10 && i < liczba_wynikow; i++) {
        al_draw_textf(font, color, rightSide / 2, topSide + 172+y, ALLEGRO_ALIGN_CENTER, "%d. %s - %d", i + 1, tablica_wynikow[i].nazwa, tablica_wynikow[i].punkty);
        y += 30;
    }

    al_flip_display();
}

// Funkcja odczytująca istniejące wyniki
int odczytaj_wyniki(Gracz tablica_wynikow[], int max_wynikow) {
    FILE* plik;
    int liczba_wynikow = 0;

    if (fopen_s(&plik, "scores.txt", "r") == 0) {
        while (fscanf_s(plik, "%s %d", tablica_wynikow[liczba_wynikow].nazwa, (unsigned)_countof(tablica_wynikow[liczba_wynikow].nazwa), &tablica_wynikow[liczba_wynikow].punkty) != EOF) {
            liczba_wynikow++;
            if (liczba_wynikow >= max_wynikow) // Zapobiegamy przekroczeniu rozmiaru tablicy
                break;
        }
        fclose(plik);
    }

    return liczba_wynikow;
}

// Funkcja odczytująca wyświetlająca
void Print_score(ALLEGRO_FONT* fontLogo, ALLEGRO_FONT* font, int rightSide, int topSide) {
    FILE* plik;
    Gracz tablica_wynikow[MAX_SCORES]; // Zdefiniuj rozmiar tablicy wyników
    int liczba_wynikow = 0;
    int i;

    // Odczytujemy istniejące wyniki
    liczba_wynikow = odczytaj_wyniki(tablica_wynikow, MAX_SCORES);


    // Sortujemy wyniki malejąco
    for (i = 0; i < liczba_wynikow - 1; i++) {
        for (int j = 0; j < liczba_wynikow - i - 1; j++) {
            if (tablica_wynikow[j].punkty < tablica_wynikow[j + 1].punkty) {
                Gracz temp = tablica_wynikow[j];
                tablica_wynikow[j] = tablica_wynikow[j + 1];
                tablica_wynikow[j + 1] = temp;
            }
        }
    }

    // Zapisujemy posortowane wyniki do pliku
    if (fopen_s(&plik, "scores.txt", "w") == 0) { // Użyj trybu "w" (zastępowanie)
        for (i = 0; i < liczba_wynikow && i < MAX_SCORES; i++) {
            fprintf(plik, "%s %d\n", tablica_wynikow[i].nazwa, tablica_wynikow[i].punkty);
        }
        fclose(plik);
    }

    // Wyświetlamy 10 najlepszych wyników
    wyswietl_10_najlepszych(tablica_wynikow, liczba_wynikow, fontLogo, font, rightSide, topSide);
}
