#include "raylib.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#define NUM_MAX_ENEMIES 30
#define MYBROWN CLITERAL(Color){ 101, 56, 33, 255 }

typedef struct Time {
    int now;
    int countdown;
} Time;

typedef struct Player {
    Rectangle rec;
    Vector2 speed;
} Player;

typedef struct Enemy {
    Rectangle rec;
    Vector2 speed;
    Vector2 direction;
    Time time;
    bool active;
    Vector2 life; // x = init state, y = current state
} Enemy;

typedef struct Item {
    Rectangle rec;
    Vector2 speed;
    bool open;
    bool active;
    Time time;
} Item;

typedef struct Data {
    std::string name;
    double score;
    Data() : name("-"), score(0) {}
} Data;

typedef enum { TITLE, TUTORIAL, GAMEPLAY, ENDDAY_1, ENDDAY_2, NEWDAY, ENDING, SCORE } GameScreen;

//----------------------------------
//global variable
//----------------------------------

static Player player = { 0 };
static Enemy enemy[NUM_MAX_ENEMIES] = { 0 };
static Item item = { 0 };

static int activeEnemies = 10;
static int minSpeed = 3;
static int maxSpeed = 5;
static int maxLife = 1;
static int foodLife = 100;

static int brownPoint = 5;
static int greenPoint = 10;
static int redPoint = 15;
static int pointMultiply_Item = 1;
static int pointMultiply_Day = 1;

//----------------------------------
// Functions
//----------------------------------

static bool compareByScore(const Data &a, const Data &b) {
    return a.score > b.score;
}

static void randomEnemy(int min_enemy, int max_enemy, int min_speed, int max_speed, int max_life)
{
    for (int i = min_enemy; i < max_enemy; i++)
    {
        // Set enemy spawn point
        switch (GetRandomValue(0, 3))
        {
        case 0:
            enemy[i].rec.x = 0;
            enemy[i].rec.y = GetRandomValue(0, 900);
            break;
        case 1:
            enemy[i].rec.x = 1600;
            enemy[i].rec.y = GetRandomValue(0, 900);
            break;
        case 2:
            enemy[i].rec.y = 0;
            enemy[i].rec.x = GetRandomValue(0, 1600);
            break;
        case 3:
            enemy[i].rec.y = 900;
            enemy[i].rec.x = GetRandomValue(0, 1600);
            break;
        }
        
        // Set enemy speed
        enemy[i].speed.x = GetRandomValue(min_speed, max_speed);
        enemy[i].speed.y = GetRandomValue(min_speed, max_speed);

        // Set time
        enemy[i].time.now = 0;
        enemy[i].time.countdown = GetRandomValue(30, 180); // 0.5s - 3s

        // Active enemy
        enemy[i].active = true;

        // Set life
        enemy[i].life.x = GetRandomValue(1, max_life);
        enemy[i].life.y = enemy[i].life.x;
    }
}

static void effectCard(int number)
{
    switch (number)
    {
    case 0:
        //Increases player movement speed
        player.speed.x++;
        player.speed.y++;
        break;
    case 1:
        //Decreases enemy movement speed
        if (maxSpeed > 5) maxSpeed--;
        break;
    case 2:
        //Increases maximum health of food
        foodLife += 20;
        break;
    case 3:
        //Decreases maximum health of enemy
        if (maxLife > 1) maxLife--;
        break;
    case 4:
        //Double points in the next day
        pointMultiply_Day = 2;
        break;
    case 5:
        //Triple points in the next day
        pointMultiply_Day = 3;
        break;
    case 6:
        //Increases points received when killing brown flies
        brownPoint += 5;
        break;
    case 7:
        //Increases points received when killing green flies
        greenPoint += 5;
        break;
    case 8:
        //Increases points received when killing red flies
        redPoint += 5;
        break;
    case 9:
        //Decreases the number of enemies that appear
        if (activeEnemies > 10) activeEnemies -= 5;
        break;
    case 10:
        //Decreases player movement speed
        if (player.speed.x > 5 && player.speed.y > 5)
        {
            player.speed.x--;
            player.speed.y--;
        }
        break;
    case 11:
        //Increases enemy movement speed
        minSpeed++;
        maxSpeed++;
        break;
    case 12:
        //Decreases maximum health of food
        foodLife -= 20;
        break;
    case 13:
        //Increases maximum health of enemy
        maxLife++;
        break;
    case 14:
        //Increases the number of enemies that appear
        if (activeEnemies < 30) activeEnemies += 5;
        break;
    }
}

int main(void)
{
    //----------------------------------
    // Init game
    //----------------------------------

    // Init window
    const int screenWidth = 1600;
    const int screenHeight = 900;
    InitWindow(screenWidth, screenHeight, "tap tap bakery");

    // Initialize audio device
    InitAudioDevice();

    // Load game resources: textures
    Texture2D bg_pic = LoadTexture("resources/background.png");
    Texture2D title_pic = LoadTexture("resources/title.png");
    Texture2D tutorial_pic = LoadTexture("resources/tutorial.png");
    Texture2D food_pic = LoadTexture("resources/food.png");
    Texture2D food75_pic = LoadTexture("resources/food1.png");
    Texture2D food50_pic = LoadTexture("resources/food2.png");
    Texture2D food25_pic = LoadTexture("resources/food3.png");
    Texture2D food10_pic = LoadTexture("resources/food4.png");
    Texture2D item_pic = LoadTexture("resources/item.png");
    Texture2D boxtime_pic = LoadTexture("resources/boxtime.png");
    Texture2D boxcard_pic = LoadTexture("resources/boxcard.png");
    Texture2D topbrown_pic = LoadTexture("resources/topbrown.png");
    Texture2D toppink_pic = LoadTexture("resources/toppink.png");
    Texture2D topgreen_pic = LoadTexture("resources/topgreen.png");
    Texture2D enemybrown_pic = LoadTexture("resources/enemybrown.png");
    Texture2D enemygreen_pic = LoadTexture("resources/enemygreen.png");
    Texture2D enemyred_pic = LoadTexture("resources/enemyred.png");
    Texture2D player_pic = LoadTexture("resources/player.png");

    // Load game resources: sounds
    Sound hit_sound = LoadSound("resources/loud-switch-sound-from-an-old-boom-box-8438.mp3");
    Sound click_sound = LoadSound("resources/notification-for-game-scenes-132473.mp3");
    Sound key_sound = LoadSound("resources/click-button-140881.mp3");
    Sound dash_sound = LoadSound("resources/slash-21834.mp3");
    Sound eat_sound = LoadSound("resources/eating-sound-effect-36186.mp3");
    Sound item_sound = LoadSound("resources/short-success-sound-glockenspiel-treasure-video-game-6346.mp3");
    Sound dead_sound = LoadSound("resources/negative_beeps-6008.mp3");
    Sound delete_sound = LoadSound("resources/the-notification-email-143029.mp3");

    SetSoundVolume(click_sound, 0.4); 
    SetSoundVolume(delete_sound, 0.4);

    // Load font
    Font font = LoadFontEx("resources/HelloSunday.otf", 32, 0, 250);

    // Load music stream and start playing music
    Music music = LoadMusicStream("resources/very-lush-and-swag-loop-74140.mp3");
    SetMusicVolume(music, 0.4);
    PlayMusicStream(music);

    // Defind current screen
    GameScreen currentScreen = TITLE;

    // Defind game variables
    bool pause = false;
    bool gameOver = false;
    int score = 0;
    float text_position_y = 780;
    float currentStateText = 0.2;

    // Defind card variables
    bool cardInitState = true;
    int cardGood[3] = { 0 };
    int cardBad[3] = { 0 };

    // Defind time variables
    int millisec = 3600;
    int day = 1;
    int countDownDash = 0;
    int countDownItem = 0;

    // Defind player variables
    std::string player_name;
    player.rec = {1200, 130, 240, 639};
    player.speed = {7, 7};

    // Defind food variables
    int foodCheck[4] = { 0 };
    Rectangle foodRec = {526, 450, 547, 276};

    // Defind card
    std::string card[15] = {
        "Increases\nplayer\nmovement\nspeed", // Good card 0 - 9
        "Decreases\nenemy\nmovement\nspeed",
        "Increases\nmaximum\nhealth\nof food",
        "Decreases\nmaximum\nhealth\nof enemy",
        "Double\npoints in\nnext day",
        "Triple\npoints in\nnext day",
        "Increases\npoints\nwhen killing\nbrown flies",
        "Increases\npoints\nwhen killing\ngreen flies",
        "Increases\npoints\nwhen killing\nred flies",
        "Decreases\nenemies\nwill appear",
        "Decreases\nplayer\nmovement\nspeed", // Bad card 10 - 14
        "Increases\nenemy\nmovement\nspeed",
        "Decreases\nmaximum\nhealth\nfood",
        "Increases\nmaximum\nhealth\nenemy",
        "Increases\nenemies\nwill appear"
    };

    // Init enemies variables
    randomEnemy(0, NUM_MAX_ENEMIES, 3, 5, 1);
    for (int i = 0; i < NUM_MAX_ENEMIES; i++)
    {
        enemy[i].rec.width = 89;
        enemy[i].rec.height = 44;
    }

    // Init item variables
    bool check_countdown_item = false;
    item.open = false;
    item.active = false;
    item.speed = {3, 3};
    item.rec.y = -120;
    item.rec.width = 105;
    item.rec.height = 106;

    // Init file
    std::vector<Data> scores_data(5);
    std::string line;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        //----------------------------------
        // Update game
        //----------------------------------
        UpdateMusicStream(music);

        switch (currentScreen)
        {
        case TITLE:
            if (IsKeyPressed(KEY_ENTER)) {
                PlaySound(click_sound);
                currentScreen = TUTORIAL;
            }
            break;

        case TUTORIAL:
            if (IsKeyPressed(KEY_ENTER)) {
                PlaySound(click_sound);
                currentScreen = GAMEPLAY;
            }
            break;

        case GAMEPLAY:
            // Game logic
            if (IsKeyPressed('O') || foodLife < 0) gameOver = true;
            if (!gameOver)
            {
                if (IsKeyPressed('P')) pause = !pause;
                if (!pause)
                {
                    // Countdown day time
                    (millisec <= 0) ? currentScreen = ENDDAY_1 : millisec--;
                    if (countDownDash > 0) countDownDash--;

                    // Dash system : part 1
                    if (IsKeyDown(KEY_LEFT_SHIFT) && countDownDash == 0)
                    {
                        player.speed.x += 50;
                        player.speed.y += 30;
                        countDownDash = 60;
                        PlaySound(dash_sound);
                    }

                    // Player movement
                    if (IsKeyDown(KEY_RIGHT) || IsKeyDown('D')) player.rec.x += player.speed.x;
                    if (IsKeyDown(KEY_LEFT) || IsKeyDown('A')) player.rec.x -= player.speed.x;
                    if (IsKeyDown(KEY_UP) || IsKeyDown('W')) player.rec.y -= player.speed.y;
                    if (IsKeyDown(KEY_DOWN) || IsKeyDown('S')) player.rec.y += player.speed.y;

                    // Dash system : part 2
                    if (countDownDash==55)
                    {
                        player.speed.x -= 50;
                        player.speed.y -= 30;
                    }

                    // Loop alive enemy
                    for (int i = 0; i < activeEnemies; i++)
                    {
                        if (!enemy[i].active) continue;

                        // Player collision with enemy
                        if (CheckCollisionRecs({player.rec.x, player.rec.y, 240, 181}, enemy[i].rec) && IsKeyPressed(KEY_SPACE))
                        {
                            if (enemy[i].life.y > 0)
                            {
                                enemy[i].life.y--;
                            }
                            else
                            {
                                // Get score
                                if (enemy[i].life.x == 1)
                                {
                                    score += brownPoint * pointMultiply_Item * pointMultiply_Day;
                                }
                                else if (enemy[i].life.x == 2)
                                {
                                    score += greenPoint * pointMultiply_Item * pointMultiply_Day;
                                }
                                else
                                {
                                    score += redPoint * pointMultiply_Item * pointMultiply_Day;
                                }

                                // Create new enemy
                                randomEnemy(i, i+1, minSpeed, maxSpeed, maxLife);

                                PlaySound(hit_sound);
                            }
                        }

                        // Enemy collision with food
                        if (CheckCollisionRecs(foodRec, enemy[i].rec) && millisec%40==0)
                        {
                            foodLife--;
                        }

                        // Food sound
                        if (foodLife == 75 && foodCheck[0] == 0) {
                            PlaySound(eat_sound);
                            foodCheck[0] = 1;
                        } else if (foodLife == 50 && foodCheck[1] == 0) {
                            PlaySound(eat_sound);
                            foodCheck[1] = 1;
                        } else if (foodLife == 25 && foodCheck[2] == 0) {
                            PlaySound(eat_sound);
                            foodCheck[2] = 1;
                        } else if (foodLife == 10 && foodCheck[3] == 0) {
                            PlaySound(eat_sound);
                            foodCheck[3] = 1;
                        }

                        // Add enemy time
                        enemy[i].time.now++;

                        // Enemy behaviour
                        if (enemy[i].time.countdown == enemy[i].time.now)
                        {
                            (enemy[i].rec.x < 770) ? enemy[i].direction.x = 0 : enemy[i].direction.x = 1;
                            (enemy[i].rec.y < 500) ? enemy[i].direction.y = 0 : enemy[i].direction.y = 1;
                            enemy[i].direction.x = GetRandomValue(0, 1);
                            enemy[i].direction.y = GetRandomValue(0, 1);
                            enemy[i].time.now = 0;
                            enemy[i].time.countdown = GetRandomValue(30, 180);
                        }
                        else
                        {
                            if (enemy[i].rec.x < 0) enemy[i].direction.x = 0;
                            if (enemy[i].rec.x > 1600) enemy[i].direction.x = 1;
                            if (enemy[i].rec.y < 0) enemy[i].direction.y = 0;
                            if (enemy[i].rec.y > 900) enemy[i].direction.y = 1;

                            switch (int(enemy[i].direction.x))
                            {
                            case 0:
                                enemy[i].rec.x += enemy[i].speed.x; // right
                                break;
                            case 1:
                                enemy[i].rec.x -= enemy[i].speed.x; // left
                                break;
                            }

                            switch (int(enemy[i].direction.y))
                            {
                            case 0:
                                enemy[i].rec.y += enemy[i].speed.y; // down
                                break;
                            case 1:
                                enemy[i].rec.y -= enemy[i].speed.y; // up
                                break;
                            }
                        }
                    }

                    // Item open -> Item appear on screen
                    if (item.time.countdown == millisec) {
                        check_countdown_item = true;
                    }
                    if (item.open && check_countdown_item)
                    {
                        // Item movement
                        item.rec.y += item.speed.y;
                        item.time.countdown--;

                        // Check collision -> If we hit buff will active
                        if (CheckCollisionRecs({player.rec.x, player.rec.y, 240, 181}, item.rec) && IsKeyPressed(KEY_SPACE))
                        {
                            item.active = true;
                            item.open = false;
                            PlaySound(item_sound);
                        }
                    }

                    // Item active -> player have buff
                    if (item.active && item.time.now > 0)
                    {
                        pointMultiply_Item = 2;
                        item.time.now--;
                    }

                    // Item stop active
                    if (item.time.now == 0)
                    {
                        item.active = false;
                        pointMultiply_Item = 1;
                    }
                }
                else
                {
                    pause = true;
                }
            }
            else
            {
                PlaySound(dead_sound);
                currentScreen = ENDING;
            }
            break;

        case ENDDAY_1:
            // Random good card
            if (cardInitState)
            {
                do {
                    cardGood[0] = GetRandomValue(0, 9);
                    cardGood[1] = GetRandomValue(0, 9);
                    cardGood[2] = GetRandomValue(0, 9);
                } while (cardGood[0] == cardGood[1] || cardGood[0] == cardGood[2] || cardGood[1] == cardGood[2]);

                cardInitState = false;
            }
            else
            {
                if (IsKeyPressed('A'))
                {
                    PlaySound(click_sound);
                    effectCard(cardGood[0]);
                    cardInitState = true;
                    currentScreen = ENDDAY_2;
                }
                if (IsKeyPressed('S'))
                {
                    PlaySound(click_sound);
                    effectCard(cardGood[1]);
                    cardInitState = true;
                    currentScreen = ENDDAY_2;
                }
                if (IsKeyPressed('D'))
                {
                    PlaySound(click_sound);
                    effectCard(cardGood[2]);
                    cardInitState = true;
                    currentScreen = ENDDAY_2;
                }
            }
            break;
        
        case ENDDAY_2:
            // Random bad card
            if (cardInitState)
            {
                do {
                    cardBad[0] = GetRandomValue(10, 14);
                    cardBad[1] = GetRandomValue(10, 14);
                    cardBad[2] = GetRandomValue(10, 14);
                } while (cardBad[0] == cardBad[1] || cardBad[0] == cardBad[2] || cardBad[1] == cardBad[2]);
                
                cardInitState = false;
            }
            else
            {
                if (IsKeyPressed('A'))
                {
                    PlaySound(click_sound);
                    effectCard(cardBad[0]);
                    cardInitState = true;
                    currentScreen = NEWDAY;
                }
                if (IsKeyPressed('S'))
                {
                    PlaySound(click_sound);
                    effectCard(cardBad[1]);
                    cardInitState = true;
                    currentScreen = NEWDAY;
                }
                if (IsKeyPressed('D'))
                {
                    PlaySound(click_sound);
                    effectCard(cardBad[2]);
                    cardInitState = true;
                    currentScreen = NEWDAY;
                }
            }
            break;

        case NEWDAY:
            // Create enemy
            randomEnemy(0, activeEnemies, minSpeed, maxSpeed, maxLife);

            // Create item
            item.open = true;
            item.active = false;
            item.rec.x = GetRandomValue(100, 1450); // position for item appear
            item.rec.y = -120;
            item.time.now = 900; // time item can active
            item.time.countdown = GetRandomValue(1600, 3600); // time item will appear on screen
            check_countdown_item = false;

            // Set food
            foodLife = 100;
            foodCheck[4] = { 0 };

            // Set point
            pointMultiply_Day = 1;
            pointMultiply_Item = 1;

            // Set time
            millisec = 3600;
            day++;
            countDownItem = 0;

            // Set screen
            currentScreen = GAMEPLAY;
            break;

        case ENDING:
            // Input player name -> 3-20 char
            if (IsKeyPressed(KEY_ENTER) && player_name.length() > 0)
            {
                // Write file
                FILE *scoreFile = fopen("scores.txt", "a");
                fprintf(scoreFile, "%s,%d\n", player_name.c_str(), score);
                fclose(scoreFile);

                PlaySound(click_sound);

                // Read file
                std::ifstream inputFile("scores.txt");

                while (std::getline(inputFile, line)) {
                    Data score_data;
                    size_t spacePos = line.find(',');
                    if (spacePos != std::string::npos) {
                        score_data.name = line.substr(0, spacePos);
                        score_data.score = std::stod(line.substr(spacePos + 1));
                        scores_data.push_back(score_data);
                    }
                }

                inputFile.close();

                std::sort(scores_data.begin(), scores_data.end(), compareByScore);

                currentScreen = SCORE;
            }
            else
            {
                int key = GetCharPressed();
                if (key > 0 && key != KEY_BACKSPACE && key != KEY_ENTER && key != KEY_KP_ENTER && key != KEY_SPACE && player_name.length() < 9)
                {
                    player_name += static_cast<char>(key);
                    PlaySound(key_sound);
                }
                else if (IsKeyPressed(KEY_BACKSPACE) && !player_name.empty())
                {
                    player_name.pop_back();
                    PlaySound(delete_sound);
                }
                else if (IsKeyPressed(KEY_ENTER)) {
                    PlaySound(delete_sound);
                }
            }
            break;

        case SCORE:
            // something
            break;
        }
        
        //----------------------------------
        // Draw
        //----------------------------------

        BeginDrawing();

        ClearBackground(WHITE);

        // Draw background
        DrawTexture(bg_pic, 0, 0, WHITE);

        switch (currentScreen)
            {
                case TITLE:
                    DrawTexture(title_pic, 0, 0, WHITE);
                    if (text_position_y > 780) currentStateText = -0.2;
                    else if (text_position_y < 765) currentStateText = 0.2;
                    text_position_y += currentStateText;
                    DrawTextEx(font, "Enter to continue", {590, text_position_y}, 70, 1, MYBROWN);
                    break;
                
                case TUTORIAL:
                    DrawTexture(tutorial_pic, 0, 0, WHITE);
                    if (text_position_y > 780) currentStateText = -0.2;
                    else if (text_position_y < 765) currentStateText = 0.2;
                    text_position_y += currentStateText;
                    DrawTextEx(font, "Enter to continue", {590, text_position_y}, 70, 1, MYBROWN);
                    break;

                case GAMEPLAY:
                    // Food
                    if (foodLife > 75) DrawTexture(food_pic, 526, 450, WHITE);
                    else if (foodLife > 50) DrawTexture(food75_pic, 526, 450, WHITE);
                    else if (foodLife > 25) DrawTexture(food50_pic, 526, 450, WHITE);
                    else if (foodLife > 10) DrawTexture(food25_pic, 526, 450, WHITE);
                    else DrawTexture(food10_pic, 526, 450, WHITE);

                    // Enemy
                    for (int i = 0; i < activeEnemies; i++)
                    {
                        if (!enemy[i].active) continue;
                        if (enemy[i].life.x == 1)
                        {
                            DrawTexture(enemybrown_pic, enemy[i].rec.x, enemy[i].rec.y, WHITE);
                        }
                        else if (enemy[i].life.x == 2)
                        {
                            DrawTexture(enemygreen_pic, enemy[i].rec.x, enemy[i].rec.y, WHITE);
                        }
                        else
                        {
                            DrawTexture(enemyred_pic, enemy[i].rec.x, enemy[i].rec.y, WHITE);
                        }
                    }

                    // Item
                    if (item.open && check_countdown_item) DrawTexture(item_pic, item.rec.x, item.rec.y, WHITE);
                    if (item.active) countDownItem++;
                    if (item.active && countDownItem < 30) {
                        DrawTextEx(font, "special time", {630, 150}, 80, 2, MYBROWN);
                    } else {
                        if (countDownItem == 50) countDownItem = 0;
                    }

                    // PLayer
                    DrawTexture(player_pic, player.rec.x, player.rec.y, WHITE);
                    
                    // Time
                    DrawTexture(boxtime_pic, 0, 50, WHITE);
                    if (millisec/60 < 10) {
                        DrawTextEx(font, ("00:0" + std::to_string(millisec/60)).c_str(), {80, 75}, 70, 2, MYBROWN);
                    } else {
                        DrawTextEx(font, ("00:" + std::to_string(millisec/60)).c_str(), {80, 75}, 70, 2, MYBROWN);
                    }
                    DrawTextEx(font, (std::to_string(score)).c_str(), {120, 780}, 70, 2, MYBROWN);
                    DrawTextEx(font, ("Day " + std::to_string(day)).c_str(), {1350, 75}, 70, 2, MYBROWN);

                    break;

                case ENDDAY_1:
                    // Draw picture
                    DrawTexture(topbrown_pic, 488, 80, WHITE);
                    DrawTexture(boxcard_pic, 230, 320, WHITE);
                    DrawTexture(boxcard_pic, 630, 320, WHITE);
                    DrawTexture(boxcard_pic, 1030, 320, WHITE);

                    // Show text -> from random card
                    DrawTextEx(font, "Buff Card", {670, 110}, 70, 1, MYBROWN);
                    DrawTextEx(font, card[cardGood[0]].c_str(), {300, 410}, 50, 1, MYBROWN);
                    DrawTextEx(font, card[cardGood[1]].c_str(), {700, 410}, 50, 1, MYBROWN);
                    DrawTextEx(font, card[cardGood[2]].c_str(), {1100, 410}, 50, 1, MYBROWN);

                    break;

                case ENDDAY_2:
                    // Draw picture
                    DrawTexture(toppink_pic, 488, 80, WHITE);
                    DrawTexture(boxcard_pic, 230, 320, WHITE);
                    DrawTexture(boxcard_pic, 630, 320, WHITE);
                    DrawTexture(boxcard_pic, 1030, 320, WHITE);

                    // Show text -> from random card
                    DrawTextEx(font, "Debuff Card", {650, 100}, 70, 1, MYBROWN);
                    DrawTextEx(font, card[cardBad[0]].c_str(), {300, 410}, 50, 1, MYBROWN);
                    DrawTextEx(font, card[cardBad[1]].c_str(), {700, 410}, 50, 1, MYBROWN);
                    DrawTextEx(font, card[cardBad[2]].c_str(), {1100, 410}, 50, 1, MYBROWN);

                    break;
                
                case NEWDAY:
                    break;

                case ENDING:
                    DrawTexture(topbrown_pic, 488, 80, WHITE);
                    DrawTextEx(font, "Game over", {680, 110}, 70, 1, MYBROWN);
                    DrawTextEx(font, "Please enter your name", {530, 370}, 70, 1, MYBROWN);
                    DrawTextEx(font, ("Name: " + player_name).c_str(), {530, 470}, 80, 2, MYBROWN);
                    if (text_position_y > 780) currentStateText = -0.2;
                    else if (text_position_y < 765) currentStateText = 0.2;
                    text_position_y += currentStateText;
                    DrawTextEx(font, "Enter to continue", {590, text_position_y}, 70, 1, MYBROWN);

                    break;
                    
                case SCORE:
                    DrawTexture(topgreen_pic, 488, 80, WHITE);
                    DrawTextEx(font, "Score Board", {660, 100}, 70, 1, MYBROWN);
                    DrawTextEx(font, ("1) " + scores_data[0].name).c_str(), {360, 320}, 80, 2, MYBROWN);
                    DrawTextEx(font, (std::to_string(static_cast<int>(scores_data[0].score))).c_str(), {1100, 320}, 80, 2, MYBROWN);
                    DrawTextEx(font, ("2) " + scores_data[1].name).c_str(), {360, 410}, 80, 2, MYBROWN);
                    DrawTextEx(font, (std::to_string(static_cast<int>(scores_data[1].score))).c_str(), {1100, 410}, 80, 2, MYBROWN);
                    DrawTextEx(font, ("3) " + scores_data[2].name).c_str(), {360, 500}, 80, 2, MYBROWN);
                    DrawTextEx(font, (std::to_string(static_cast<int>(scores_data[2].score))).c_str(), {1100, 500}, 80, 2, MYBROWN);
                    DrawTextEx(font, ("4) " + scores_data[3].name).c_str(), {360, 590}, 80, 2, MYBROWN);
                    DrawTextEx(font, (std::to_string(static_cast<int>(scores_data[3].score))).c_str(), {1100, 590}, 80, 2, MYBROWN);
                    DrawTextEx(font, ("5) " + scores_data[4].name).c_str(), {360, 680}, 80, 2, MYBROWN);
                    DrawTextEx(font, (std::to_string(static_cast<int>(scores_data[4].score))).c_str(), {1100, 680}, 80, 2, MYBROWN);
                    break;
            }

        EndDrawing();
    }

    //----------------------------------
    // Unlode game
    //----------------------------------

    // Unload textures
    UnloadTexture(title_pic);
    UnloadTexture(tutorial_pic);
    UnloadTexture(food_pic);
    UnloadTexture(food75_pic);
    UnloadTexture(food50_pic);
    UnloadTexture(food25_pic);
    UnloadTexture(food10_pic);
    UnloadTexture(item_pic);
    UnloadTexture(boxtime_pic);
    UnloadTexture(boxcard_pic);
    UnloadTexture(topbrown_pic);
    UnloadTexture(toppink_pic);
    UnloadTexture(topgreen_pic);
    UnloadTexture(enemybrown_pic);
    UnloadTexture(enemygreen_pic);
    UnloadTexture(enemyred_pic);
    UnloadTexture(player_pic);

    // Unload sounds
    UnloadSound(hit_sound);
    UnloadSound(click_sound);
    UnloadSound(key_sound);
    UnloadSound(dash_sound);
    UnloadSound(eat_sound);
    UnloadSound(item_sound);
    UnloadSound(dead_sound);
    UnloadSound(delete_sound);

    UnloadMusicStream(music);
    CloseAudioDevice();
    
    CloseWindow();
    return 0;
}