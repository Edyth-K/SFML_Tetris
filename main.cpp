#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics.hpp>
#include <time.h>
#include <iostream>

// Game Board
const int M { 20 };
const int N { 10 };
// ? field game board is a 2d int array, 0 if empty, 1 if currently occupied by a tetris piece 
int field[M][N] = { 0 };

//offset for drawing tiles so they match the background
const float offset_y{ 30.0f };
const float offset_x{ 27.0f };

struct Point {int x, y;} a[4], b[4];

// Possible Configurations for the Tetris Blocks
int figures[7][4] =
{
    3,5,4,7, // T
    2,4,5,7, // Z
    3,5,4,6, // S
    2,3,4,5, // O
    1,3,5,7, // I
    2,3,5,7, // L
    3,5,7,6, // J
};


bool check()
{
    for (int i = 0; i < 4; i++)
        if (a[i].x < 0 || a[i].x >= N || a[i].y >= M) return 0; // check if tile is off screen (left/right) or at the bottom
        else if (field[a[i].y][a[i].x]) return 0; // ? check if board space already occupied
    return 1;
}

int main()
{
    srand(static_cast<unsigned int>(time(0)));

    // This renders the main window
    sf::RenderWindow window(sf::VideoMode(480, 480), "Tetris"); //320 x 480 original

    // imgui initialization
    ImGui::SFML::Init(window);
    sf::Clock deltaClock;

    // Load the tile sprite
    sf::Texture tile;
    tile.loadFromFile("images/tiles.png");
    sf::Sprite spriteTile(tile);

    // Load background
    sf::Texture background;
    background.loadFromFile("images/background.png");
    sf::Sprite spriteBackground(background);

    // Load frame
    sf::Texture frame;
    frame.loadFromFile("images/frame.png");
    sf::Sprite spriteFrame(frame);

    // this crops it to just the first tetris square from the tile set
    spriteTile.setTextureRect(sf::IntRect(0, 0, 18, 18));

    int dx{ 0 }; bool rotate { 0}; int colorNum{ 1 };

    // last delay is a temporary variable so that after speeding up the piece, it can go back to the speed it was at before
    float timer{ 0 }, delay{ 0.3f }, lastDelay{ 0.3f };

    sf::Clock clock;

    for (int i = 0; i < 4; i++) // set Point struct x,y arrays to the coordinates corresponding to the selected tetris piece
    {
        a[i].x = figures[0][i] % 2;
        a[i].y = figures[0][i] / 2;
    }

    // main game loop
    while (window.isOpen())
    {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;

        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed)
                if (event.key.code == sf::Keyboard::Up) rotate = true;
                else if (event.key.code == sf::Keyboard::Left) dx = -1;
                else if (event.key.code == sf::Keyboard::Right) dx = 1;

            // after releasing down key, return speed to value in temp variable
            if (event.type == sf::Event::KeyReleased)
                if (event.key.code == sf::Keyboard::Down) delay = lastDelay;
        }
        ImGui::SFML::Update(window, deltaClock.restart());
        
        ImGui::Begin("Modify Delay");
        
        // Slider to control how fast piece drops
        ImGui::SliderFloat("Delay", &delay, 0.3f, 1.0f);
        ImGui::End();

        // when down key is pressed, save current speed before speeding up to return to later
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            if (delay>0.1f) lastDelay = delay;
            delay = 0.05f;
        }
    
        // GAME LOGIC

        // Move
        for (int i = 0; i < 4; i++) { b[i] = a[i];  a[i].x += dx; }
        if (!check()) for (int i = 0; i < 4; i++) a[i] = b[i];

        // Rotate
        if (rotate)
        {
            Point p = a[1];                 // center point for rotation
            for (int i = 0; i < 4; i++)
                // rotation logic
            {
                int x{ a[i].y - p.y };
                int y{ a[i].x - p.x };
                a[i].x = p.x - x;
                a[i].y = p.y + y;
            }
            if (!check())  for (int i = 0; i < 4; i++) a[i] = b[i];
        }

        if (timer > delay)                  // Timer Tick
        // timer goes up, and when it passes delay (0.3), 
        // lower the tile (Point Struct) by 1 unit and reset timer to 0.
        {
            for (int i = 0; i < 4; i++) {b[i] = a[i]; a[i].y += 1;}
            if (!check())
            {
                for (int i = 0; i < 4; i++) field[b[i].y][b[i].x] = colorNum;
                
                int n = rand() % 7;         // changing this value changes which tetris piece is rendered
                colorNum = 1 + n;
                for (int i = 0; i < 4; i++) // set Point struct x,y arrays to the coordinates corresponding to the selected tetris piece
                {
                    a[i].x = figures[n][i] % 2;
                    a[i].y = figures[n][i] / 2;
                }
            }
            timer = 0;
        }

        // check for lines
        int k{ M - 1 };
        for (int i = M - 1; i > 0; i--)
        {
            int count{ 0 };
            for (int j = 0; j < N; j++)
            {
                if (field[i][j]) count++;
                field[k][j] = field[i][j];  // move board down
            }
            if (count < N) k--;
        }

        dx = 0; rotate = 0; //delay = 0.3f;   // reset dx and rotate every frame

        // RENDERING

        window.clear(sf::Color::White);     // white 

        // draw background
        window.draw(spriteBackground);


        // draw tiles
        for (int i = 0; i < M; i++)
            for (int j = 0; j < N; j++)
        {
                if (field[i][j] == 0) continue;                 // if cell empty, skip drawing
                spriteTile.setTextureRect(sf::IntRect(field[i][j] * 18, 0, 18, 18));
                spriteTile.setPosition(j * 18.0f+ offset_x, i * 18.0f+ offset_y);   // set position for drawing
                window.draw(spriteTile);                        // draw tile
        }

        for (int i = 0; i < 4; i++)
        {
            spriteTile.setTextureRect(sf::IntRect(colorNum * 18, 0, 18, 18));
            spriteTile.setPosition((a[i].x * 18.0f)+ offset_x, (a[i].y * 18.0f)+ offset_y);
            window.draw(spriteTile);        // draw sprite
        }

        // draw frame
        window.draw(spriteFrame);
        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
    return 0;
}