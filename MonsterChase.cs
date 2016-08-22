using System;
using System.Collections.Generic;

//Monster Chase
//Alex Koumandarakis (no group partner)

namespace MonsterChase
{
    //The display on which the game is played
    public class Board
    {
        public int maxX;
        public int maxY;
        public Player player;
        public Monster[] monsters;
        public Pickup[] pickups;

        //Creates board and sets its size
        public Board(int xVal, int yVal)
        {
            this.maxX = xVal;
            this.maxY = yVal;
        }
        
        //Add player to board
        public void addPlayer(Player p)
        {
            this.player = p;
        }

        //Add Monsters to board
        public void addMonsters(Monster[] m)
        {
            this.monsters = m;
        }

        //Add pickups to board
        public void addPickups(Pickup[] p)
        {
            this.pickups = p;
        }

        //Draws out the board with the positions of the player, pickups, and monsters marked
        public void displayBoard()
        {
            int x = 0;
            Boolean objectFound = false;
            for (int y = this.maxY; y >= 0; y--)
            {
                x = 0;
                while (x <= this.maxX)
                {
                    objectFound = false;
                    foreach (Monster m in this.monsters)
                    {
                        if (y == m.yPos && x == m.xPos)
                        {
                            Console.Write("M");
                            Console.Write(" ");
                            x++;
                            objectFound = true;
                        }
                    }

                    if (y == this.player.yPos && x == this.player.xPos)
                    {
                        Console.Write("X");
                        Console.Write(" ");
                        x++;
                        objectFound = true;
                    }

                    foreach (Pickup p in this.pickups)
                    {
                        if ((y == p.yPos && x == p.xPos) && p.aquired == false && p.visible == true)
                        {
                            Console.Write("P");
                            Console.Write(" ");
                            x++;
                            objectFound = true;
                        }
                    }

                    if (objectFound == false)
                    {
                        Console.Write("+");
                        Console.Write(" ");
                        x++;
                    }
                }
                Console.WriteLine("");
            }
            Console.WriteLine("");
            Console.WriteLine("");
        }

        //Updates every object on the board and then draws the board
        public void update()
        {
            player.update();

            foreach (Monster m in monsters)
            {
                m.update();
            }

            foreach (Pickup p in pickups)
            {
                p.changeVisibility();
            }

            this.displayBoard();
        }
    }

    public class Component
    {
        public GameObject owner;
        public virtual void update() { }
    }

    class PlayerMoveComponent : Component
    {
        //Prompts the player as to which direction they want to move
        //Determines what direction the player will move based on the user's input
        public override void update()
        {
            Console.WriteLine("What direction do you want to run?");
            char direction = char.ToLower(Console.ReadKey().KeyChar);
            if (direction == 'w')
                owner.yPos += owner.speed;
            if (direction == 's')
                owner.yPos -= owner.speed;
            if (direction == 'a')
                owner.xPos -= owner.speed;
            if (direction == 'd')
                owner.xPos += owner.speed;
            Console.WriteLine();
        }
    }

    class MonsterMoveComponent : Component
    {
        //Determines the direction the monster should move based on the position of the player and the monster
        public override void update()
        {
            char direction;
            int xDiffAbs = Math.Abs(owner.xPos - owner.board.player.xPos);
            int yDiffAbs = Math.Abs(owner.yPos - owner.board.player.yPos);

            int xDiff = owner.xPos - owner.board.player.xPos;
            int yDiff = owner.yPos - owner.board.player.yPos;

            if (xDiffAbs > yDiffAbs)
            {
                if (xDiff > 0)
                {
                    direction = 'a';
                }
                else
                {
                    direction = 'd';
                }
            }
            else
            {
                if (yDiff > 0)
                {
                    direction = 's';
                }
                else
                {
                    direction = 'w';
                }
            }

            if (direction == 'w')
                owner.yPos += owner.speed;
            if (direction == 's')
                owner.yPos -= owner.speed;
            if (direction == 'a')
                owner.xPos -= owner.speed;
            if (direction == 'd')
                owner.xPos += owner.speed;
        }
    }

    class WrapAroundComponent : Component
    {
        //If the player's position is greater than the size of the board, 
        //the player wraps around to the other side of the board
        public override void update()
        {
            if (owner.xPos > owner.board.maxX)
            {
                owner.xPos = owner.board.maxX - owner.xPos + 1;
            }
            if (owner.xPos < 0)
            {
                owner.xPos = owner.board.maxX;
            }
            if (owner.yPos > owner.board.maxY)
            {
                owner.yPos = owner.board.maxY - owner.yPos + 1;
            }
            if (owner.yPos < 0)
            {
                owner.yPos = owner.board.maxY;
            }
        }
    }

    class KeepInBoundsComponent : Component
    {
        //If the monster's position is greater than the size of the board,
        //the monster's position is set to the edge of the board
        public override void update()
        {
            if (owner.xPos > owner.board.maxX)
            {
                owner.xPos = owner.board.maxX;
            }
            if (owner.xPos < 0)
            {
                owner.xPos = 0;
            }
            if (owner.yPos > owner.board.maxY)
            {
                owner.yPos = owner.board.maxY;
            }
            if (owner.yPos < 0)
            {
                owner.yPos = 0;
            }
        }
    }

    class CheckPlayerEatenComponent : Component
    {
        //If a monster and the player share the same position, then the player loses
        public override void update()
        {
            if (owner.xPos == owner.board.player.xPos && owner.yPos == owner.board.player.yPos)
            {
                owner.loss = true;
            }
        }
    }

    class CheckPickupComponent : Component
    {
        //If the player and a pickup share the same position,
        //the game displays text explaining the type of pickup
        //and adds that pickup component to the player or monster.  
        public override void update()
        {
            foreach (Pickup p in owner.board.pickups)
            {
                if ((owner.xPos == p.xPos && owner.yPos == p.yPos) && p.visible && !p.aquired)
                {
                    if (p.type ==  2)
                    {
                        AdrenalineComponent a = new AdrenalineComponent();
                        Console.WriteLine("");
                        Console.WriteLine("You've found an adrenaline shot!");
                        Console.WriteLine("Run five spaces in one turn!");
                        Console.WriteLine("");
                        p.aquired = true;
                        owner.addComponent(a);
                    }
                    else
                    {
                        CourageComponent c = new CourageComponent();
                        Console.WriteLine("");
                        Console.WriteLine("You've found your courage!");
                        Console.WriteLine("Monsters freeze for two turns!");
                        Console.WriteLine("");
                        p.aquired = true;
                        foreach (Monster m in owner.board.monsters)
                        {
                            m.addComponent(c);
                        }
                    }
                }
            }
        }
    }

    //A timer component used to determine when pickups' effects will end
    class TimerComponent : Component
    {
        int counter = 0;
        int limit;
        public Boolean over = false;

        //Sets the length of the timer
        public TimerComponent(int x)
        {
            this.limit = x;
        }

        //Sets the timer to over when the set number of turns has passed
        public override void update()
        {
            counter++;
            if (counter > limit)
            {
                over = true;
            }
        }
    }

    class AdrenalineComponent : Component
    {
        TimerComponent t = new TimerComponent(1);

        //Adds the "Adrenaline" bonus to the player
        //Sets their speed to 5 for one turn
        public override void update()
        {
            t.update();
            if (t.over == false)
            {
                owner.speed = 5;
            }
            else
            {
                owner.speed = 1;
            }
        }
    }

    class CourageComponent : Component
    {
        TimerComponent t = new TimerComponent(2);

        //Adds the "Frozen" effect to the monsters
        //Sets the monsters' speed to 0 for two turns
        public override void update()
        {
            t.update();
            if (t.over == false)
            {
                owner.speed = 0;
                Console.WriteLine("Monsters Frozen");
            }
            else
            {
                owner.speed = 2;
            }
        }
    }


    public class GameObject
    {
        public int xPos;
        public int yPos;
        public int speed;
        public String dir;
        public Board board;
        public Boolean loss = false;
        public Boolean frozen = false;

        public List<Component> components = new List<Component>();
        
        //Set the starting position and speed of the object
        public GameObject(int x, int y, int s, Board b)
        {
            this.xPos = x;
            this.yPos = y;
            this.speed = s;
            this.board = b;
        }

        public GameObject()
        {
        }

        //Returns a specific component from all an objects components
        public T getComponent<T>() where T : class
        {
            foreach(Component c in components)
            {
                T t = c as T;
                if (t != null)
                    return t;
            }
            return null;
        }

        //Adds a component to an object
        public void addComponent(Component c)
        {
            c.owner = this;
            components.Add(c);
        }

        //Updates the components of an object
        public virtual void update() {}
    }


    public class Monster : GameObject
    {
        
        //Sets the position and speed of a monster object
        //Adds monster components to the object
        public Monster(int x, int y, Board b)
        {
            this.xPos = x;
            this.yPos = y;
            this.speed = 2;
            this.board = b;

            MonsterMoveComponent m = new MonsterMoveComponent();
            KeepInBoundsComponent k = new KeepInBoundsComponent();
            CheckPlayerEatenComponent c = new CheckPlayerEatenComponent();

            this.addComponent(m);
            this.addComponent(k);
            this.addComponent(c);
        }

        //Updates the object's components
        public override void update()
        {
            foreach (Component c in components)
            {
                c.update();
            }
        }
    }


    public class Player : GameObject
    {

        //Sets the position and speed of the player object
        //Adds player components to the object
        public Player(int x, int y, int s, Board b)
        {
            this.xPos = x;
            this.yPos = y;
            this.speed = s;
            this.board = b;

            PlayerMoveComponent p = new PlayerMoveComponent();
            WrapAroundComponent w = new WrapAroundComponent();
            CheckPickupComponent c = new CheckPickupComponent();

            this.addComponent(p);
            this.addComponent(w);
            this.addComponent(c);
        }

        //Updates player's components
        public override void update()
        {
            for (int i = 0; i < this.components.Count; i++)
            {
                this.components[i].update();
            }
        }
    }


    public class Pickup : GameObject
    {
        public int type;
        public Boolean aquired;
        public Boolean visible;

        //Sets the position and type of the pickup object
        public Pickup(int x, int y, Board b, int t)
        {
            this.xPos = x;
            this.yPos = y;
            this.board = b;
            this.type = t;
            this.aquired = false;
        }

        //Randomly changes whether the player can see the pickup before they aquire it
        //After the player aquires the pickup, it remains invisible
        public void changeVisibility()
        {
            if (aquired == false)
            {
                Random rand = new Random();
                this.visible = (rand.Next() % 2 == 0);
            }
            else
            {
                this.visible = false;
            }
        }
    }



    public class MainGame
    {
        public Board board;
        public int numRounds;

        //Displays intro text
        public MainGame()
        {
            Console.WriteLine("You are being chased by monsters.");
            Console.WriteLine("Each turn you can run up (w), down (s), right (d), or left (a).");
            Console.WriteLine("You move one space a turn, the monsters move two.");
            Console.WriteLine("You can wrap around the grid, the monsters can't.");
            Console.WriteLine("");
            Console.WriteLine("Key:");
            Console.WriteLine("X- player");
            Console.WriteLine("P- pickup");
            Console.WriteLine("M- monster");
            Console.WriteLine("");
        }

        //Takes inputs to determine the size of the board, the length of the game, and the number of monsters
        //Creates the board and monsters based on these specifications
        //Also creates the player and pickups
        public void newGame()
        {
            Console.WriteLine("How wide do you want the area to be?");
            String bX = Console.ReadLine();
            int boardX = Convert.ToInt32(bX);
            Console.WriteLine("How long do you want the area to be?");
            String bY = Console.ReadLine();
            int boardY = Convert.ToInt32(bY);
            Console.WriteLine("How many rounds do you want to try and survive?");
            String r = Console.ReadLine();
            int rounds = Convert.ToInt32(r);
            Console.WriteLine("How many monsters do you want to chase you?");
            String numM = Console.ReadLine();
            int numMonst = Convert.ToInt32(numM);
            Console.WriteLine("");
            Console.WriteLine("");

            Random rand = new Random();

            Board mainBoard = new Board(boardX, boardY);

            Pickup courage = new Pickup(rand.Next(boardX), rand.Next(boardY), mainBoard, 1);
            Pickup adrenaline = new Pickup(rand.Next(boardX), rand.Next(boardY), mainBoard, 2);
            Pickup[] pickups = new Pickup[2] { courage, adrenaline };

            Monster[] monsters = new Monster[numMonst];
            for (int i = 0; i < numMonst; i++)
            {
                Monster newMonster = new Monster(rand.Next(boardX), rand.Next(boardY), mainBoard);
                monsters[i] = newMonster;
            }

            Player p1 = new Player(rand.Next(boardX), rand.Next(boardY), 1, mainBoard);

            mainBoard.addPlayer(p1);
            mainBoard.addMonsters(monsters);
            mainBoard.addPickups(pickups);

            this.board = mainBoard;

            this.numRounds = rounds;
        }

        //Main game loop
        //Displays whether the player has lost or won
        //Prompts player to start a new game or exit after a game has ended
        public void playGame()
        {
            this.newGame();
            Boolean playing = true;
            while (playing == true)
            {
                for (int i = 0; i < this.numRounds; i++)
                {
                    this.board.update();
                    foreach (Monster m in this.board.monsters)
                    {
                        if (m.loss == true)
                        {
                            Console.WriteLine("You have been violently devoured.");
                            Console.WriteLine("Play Again? (Y/N)");
                            String playAgain = Console.ReadLine();
                            playAgain = playAgain.ToLower();
                            if (playAgain.Equals("y"))
                            {
                                this.newGame();
                            }
                            else
                            {
                                playing = false;
                                i = numRounds;
                            }
                        }
                    }
                }
                if (playing == false)
                {
                    break;
                }
                Console.WriteLine("You've narrowly escaped being ripped to shreds!");
                Console.WriteLine("Play Again? (Y/N)");
                Console.WriteLine("");
                String pAgain = Console.ReadLine();
                pAgain = pAgain.ToLower();
                if (pAgain.Equals("y"))
                {
                    this.newGame();
                }
                else
                {
                    playing = false;
                }

            }
        }
    }

    public class MainClass
    {
        //Runs the main game
        static void Main()
        {
            MainGame game = new MainGame();
            game.playGame();
        }
    }
}