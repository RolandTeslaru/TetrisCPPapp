//
//  game.cpp
//  TetrisC++
//
//  Created by Roland Teslaru on 14.03.2022.
//  Copyright © 2022 Roland Teslaru. All rights reserved.
//
#include "game.hpp"
using namespace std;
using namespace sf;
using namespace chrono;

int FrameNum = 0;

int Movement = 0;
bool Rotate = false ;
bool NewCurrent = false;
bool ForceDown = false;
bool KeyDown = false;
bool KeyUp = false;

bool newlevel = true;

int level;
int score ;
int TotalDeletedLindes ;

Text TextScore;
Text TextLevel;
Text TextDeletedLines;
Text TextGameOver;

Image icon;


Time Timer;
RenderWindow GameWin(VideoMode(GameWinX,GameWinY), "Tetris" );
Texture BlockTexture;
Texture WinBackgroundTexture;
Sprite s_Block(BlockTexture);
Sprite WinBackgroundSprite(WinBackgroundTexture);

RectangleShape MapShape(Vector2f(MapLength * 44.f, MapHeight * 44.f));
RectangleShape TetrominoSprite(Vector2f(42.f * scale,42.f * scale));
RectangleShape PrevTetrominoSprite(Vector2f(42.f * scale , 42.f * scale));
RectangleShape StoredPiece(Vector2f(42.f * scale , 42.f * scale));

Event GameEvent;

TetrominoInf blocks[7] =
{
    {
        {255,165,0},
        {
            {0,0,1,0}, // L BLOCK
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        5,4,3,
        "L"
    },
    {
        {255,0,0}, // Z BLOCK
        {
            {1,1,0,0},
            {0,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        5,4,3,
        "Z"
    },
    {
        {224,255,255}, // I BLOCK
        {
            {0,0,0,0},
            {1,1,1,1},
            {0,0,0,0},
            {0,0,0,0}
        },
        5,4,4,
        "I"
    },
    {
        {0,0,255}, // J BLOCK
        {
            {1,0,0,0},
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        5,4,3,
        "J"
        
    },
    {
        {255,255,0}, // O BLOCK
        {
            {1,1,0,0},
            {1,1,0,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        5,4,2,
        "O"
    },
    {
        {0,0,255}, // S BLOCK
        {
            {0,1,1,0},
            {1,1,0,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        5,4,3,
        "S"
        
    },
    {
        {128,0,128}, // T BLOCK
        {
            {0,1,0,0},
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        5,4,3,
        "T"
        
    }
},current , prevCurrent;
Font TextFont;
Board GameBoard;

bool gameover()
{
    for(int j = 0; j<MapLength;j++)
    {
        if(GameBoard.matrix[19][j] == true)
        {
            return true;
        }
    }
    return false;
}

void initialize()
{
    // Initialize game window
    GameWin.clear(Color(37,37,42));
    MapShape.setFillColor(Color::Black);
    GameWin.setFramerateLimit(60);
    
    TextFont.loadFromFile(resourcePath() + "SfMonoFont.ttf");
    if(!TextFont.loadFromFile(resourcePath() + "SfMonoFont.ttf"))
    {
        cout << "Error loading text font" << endl;
    }
    TextLevel.setFont(TextFont);
    TextScore.setFont(TextFont);
    TextDeletedLines.setFont(TextFont);
    TextGameOver.setFont(TextFont);
    
    TextLevel.setCharacterSize(20 * scale);
    TextScore.setCharacterSize(20 * scale);
    TextDeletedLines.setCharacterSize(20 * scale);
    TextGameOver.setCharacterSize(20 * scale);
    
    TextLevel.setFillColor(Color::White);
    TextScore.setFillColor(Color::White);
    TextDeletedLines.setFillColor(Color::White);
    TextGameOver.setFillColor(Color::Red);
    
    TextLevel.setStyle(Text::Bold);
    TextScore.setStyle(Text::Bold);
    TextDeletedLines.setStyle(Text::Bold);
    TextGameOver.setStyle(Text::Bold);
    
    TextLevel.setPosition((MapLength * TileSize + 20 ) * scale , ( MapHeight * TileSize -40 ) * scale);
    TextScore.setPosition(MapLength * TileSize + 20 , MapHeight * TileSize -80);
    TextDeletedLines.setPosition(MapLength * TileSize * scale + 20 ,MapHeight * TileSize * scale -120);
    TextGameOver.setPosition((MapLength * TileSize * scale ) / 2 , (MapHeight * TileSize * scale) / 2);
    
    // Set the Icon
    if (!icon.loadFromFile(resourcePath() + "icon.png")) {
        return EXIT_FAILURE;
    }
    GameWin.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    current = blocks[rand()%7];
    prevCurrent = blocks[rand()%7];
}

void scoringSys(int deletedlines)
{
    // Scoring formula
    score += 40 * deletedlines * level;
    TotalDeletedLindes += deletedlines;
    
    if(TotalDeletedLindes >= 10 && TotalDeletedLindes % 10 == 0 && newlevel)
    {
        level ++;
        newlevel = false;
    }
    TextLevel.setString("Level " + to_string(level));
    TextScore.setString("Score " + to_string(score));
    TextDeletedLines.setString("Deleted lines " + to_string(TotalDeletedLindes));
}

void startgame()
{
    // Initialize tetromino
    current = blocks[rand()% 7];
    current.PozY = -3;
    Movement = 0;
    Rotate = 0;
    level = 1;
    score = 0;
    TotalDeletedLindes = 0;
}
void restart()
{
    for(int i = 0; i<MapHeight;i++)
        for(int j = 0; j<MapLength;j++)
            GameBoard.matrix[i][j] = 0;
    startgame();
}
// Check current tetromino for overlapping sprites with stored board sprites
int BoardCollison(TetrominoInf Tetromino )
{
    for(int i=0;i<Tetromino.Size;i++)
    {   for(int j=0;j<Tetromino.Size;j++)
        {
            if(Tetromino.matrix[i][j])
            {
                TetrominoSprite.setPosition(Tetromino.PozX + j, Tetromino.PozY + i);
                // check for each stored sprites if the XY position
                
                for(int i_board = 0; i_board < MapHeight; i_board++)
                {
                    for(int j_board= 0; j_board < MapLength; j_board++)
                    {
                        if(GameBoard.matrix[i_board][j_board])
                        {
                            if(TetrominoSprite.getPosition().x == j_board && TetrominoSprite.getPosition().y == i_board)
                                return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}
int DeleteLines()
{
    int deletedlines = 0;
    
    for(int i = MapHeight-1; i>0; i--)// Check if every square on the i row is filled
    {
        bool check = true;
        for(int j = 0; j < MapLength; j++)
        {
            if(GameBoard.matrix[i][j] == false)
            {
                check = false;
                break;
            }
        }
        if(check)
        {
            // Move row down
            deletedlines ++;
            newlevel = true;
            for(int y = i ; y>0 ; y--)
            {
                for(int x = 0; x < MapLength; x++)
                {
                    GameBoard.matrix[y][x] = false;
                    swap(GameBoard.matrix[y][x] , GameBoard.matrix[y - 1][x]);
                    swap(GameBoard.color[y][x] , GameBoard.color[y-1][x]);
                }
            }
        }
    }
    return deletedlines;
}
TetrominoInf rotate(TetrominoInf Tetromino)
{   // Transpose matrix
    TetrominoInf tmp = Tetromino;
    for(int i =0; i<Tetromino.Size; i++)
        for(int j =0; j<Tetromino.Size; j++)
            tmp.matrix[i][j] = Tetromino.matrix[j][i];
    // Reverse Cols
    Tetromino = tmp;
    for(int i =0; i<Tetromino.Size; i++)
        for(int j =0; j<Tetromino.Size / 2; j++)
            swap(Tetromino.matrix[i][j] , Tetromino.matrix[i][Tetromino.Size -j - 1]);
    return Tetromino;
}

bool isPossibleDown(TetrominoInf Tetromino)
{
    Tetromino.PozY++;
    for(int i = 0 ; i<Tetromino.Size; i++)
        for(int j = 0; j<=Tetromino.Size; j++)
            if(Tetromino.matrix[i][j])
                TetrominoSprite.setPosition((Tetromino.PozX + j) * TileSize, (Tetromino.PozY + i) * TileSize);
                if(TetrominoSprite.getPosition().y > 840.2f * scale || BoardCollison(Tetromino) == true )
                    return false;

    return true;
}

bool isPossibleMove(TetrominoInf Tetromino , int move)
{
    Tetromino.PozX +=  move;
    for(int i=0; i<Tetromino.Size; i++)
        for(int j=0; j<Tetromino.Size; j++)
        {
            if(Tetromino.matrix[i][j])
            {
                TetrominoSprite.setPosition((Tetromino.PozX + j) * TileSize, (Tetromino.PozY + i) * TileSize);
                // Left and right walls collision check
                if( TetrominoSprite.getPosition().x < 0.f * scale || TetrominoSprite.getPosition().x > 420.f * scale || BoardCollison(Tetromino) == true)
                    return false;
            }
        }
    return true;
}

bool isPossibleRotate(TetrominoInf Tetromino)
{
    TetrominoInf tmp = Tetromino;
    Tetromino = rotate(Tetromino);
    for(int i=0;i<Tetromino.Size;i++)
        for(int j=0;j<Tetromino.Size;j++)
            if(Tetromino.matrix[i][j])
            {
                TetrominoSprite.setPosition((Tetromino.PozX + j) * TileSize, (Tetromino.PozY + i) * TileSize);
                if( TetrominoSprite.getPosition().x < -44.f * scale || TetrominoSprite.getPosition().x >= 396.2f * scale || BoardCollison(Tetromino) == true)
                {   // revert changes
                    Tetromino = tmp;
                    return false;
                }
            }
    return true;
}


void storePiece()
{
    for(int i=0;i<current.Size;i++)
        for(int j=0;j<current.Size;j++)
            if(current.matrix[i][j])
            {
                GameBoard.matrix[int(current.PozY) + i][int(current.PozX) + j] = current.matrix[i][j];
                GameBoard.color[int(current.PozY) + i][int(current.PozX) + j] = current.color;
            }
}
void HardDrop(TetrominoInf Tetromino)
{
    while(isPossibleDown(Tetromino))
        Tetromino.PozY++;
    
    current.PozY = Tetromino.PozY;
}
void input()
{
    Movement = 0;
    Rotate = false;
    
    switch(GameEvent.key.code)
    {
        case Keyboard::Left:
        {   Movement = -1; break;}
        case Keyboard::Right:
        {   Movement = 1; break;}
        case Keyboard::Up:
        {   Rotate = true; break; }
        case Keyboard::Space:
        {   ForceDown = true; break;}
        case Keyboard::Down:
        {   KeyDown = true; break;}
        case Keyboard::R:
        {   restart(); break; }
        case Keyboard::Escape:
        {GameWin.close(); break;}
    }
}
void draw(TetrominoInf Tetromino)
{
    // draw map map background and clear the screen idk
    GameWin.clear(Color(37,37,42));
    GameWin.draw(MapShape);
    
    // draw current active Tetromino
    for(int i =0; i<Tetromino.Size; i++ )
        for(int j =0; j<Tetromino.Size; j++)
            if(Tetromino.matrix[i][j])
            {
                TetrominoSprite.setPosition((Tetromino.PozX + j) * TileSize, (Tetromino.PozY + i) * TileSize);
                TetrominoSprite.setFillColor(Tetromino.color);
                GameWin.draw(TetrominoSprite);
            }
    
    // draw stored pieces on game board
    for(int i = 0 ;i<MapHeight; i++)
        for(int j = 0;j<MapLength; j++)
            if(GameBoard.matrix[i][j])
            {
                StoredPiece.setPosition(j * TileSize, i * TileSize);
                StoredPiece.setFillColor(GameBoard.color[i][j]);
                GameWin.draw(StoredPiece);
            }
    
    GameWin.draw(TextLevel);
    GameWin.draw(TextScore);
    GameWin.draw(TextDeletedLines);
    
    prevCurrent.PozX = 12;
    prevCurrent.PozY = 1;
    
    for(int i = 0 ; i<prevCurrent.Size; i++)
        for(int j = 0 ; j<prevCurrent.Size; j++)
            if(prevCurrent.matrix[i][j])
            {
                PrevTetrominoSprite.setPosition((prevCurrent.PozX + j) * TileSize, (prevCurrent.PozY + i) * TileSize);
                PrevTetrominoSprite.setFillColor(prevCurrent.color);
                GameWin.draw(PrevTetrominoSprite);
            }
}

void update()
{
    // Movement and rotation input  +   Collision check
    
    if(Rotate)
        if(isPossibleRotate(current))
            current = rotate(current);
    
    if(Movement == -1 && isPossibleMove(current, Movement))
        current.PozX --;
    
    else if(Movement == 1 && isPossibleMove(current, Movement))
        current.PozX ++;
   
    if(KeyDown && isPossibleDown(current))
        current.PozY ++ ;
    
    if(ForceDown)
        HardDrop(current);
    
    if(FrameNum % (10 * 6)  == 0)
    {
        // To go down every 10 * x frames
        if(!isPossibleDown(current))
            NewCurrent = true;
        else
            current.PozY ++;
    }
    // Reset imput values
    Movement = 0;
    Rotate = false;
    ForceDown = false;
    KeyDown = false;
}

int game()
{
    srand(time(NULL));
    
    initialize();
    startgame();
    
    while(GameWin.isOpen())
    {
        Clock clock;
        if(NewCurrent)
        {
            storePiece();
            current = prevCurrent;
            prevCurrent = blocks[rand()%7];
            
            current.PozY = 0;
            current.PozX = 5;
            NewCurrent = false;
        }
        while(GameWin.pollEvent(GameEvent))
        {
            if(GameEvent.type == Event::Closed)
                GameWin.close();
            if(GameEvent.type == Event::KeyPressed)
                input();
        }
        Timer = clock.getElapsedTime();
        // Update screen based on input
        update();
        // Check for completed lines and calculate score
        scoringSys(DeleteLines());
        
        FrameNum ++;
        // draw and display everything
        draw(current);
        GameWin.display();
        
        clock.restart();
    }
    return 0;
}
