#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <Windows.h>
#include <cmath>
using namespace sf;
using namespace std;

//Global Variables 
const int mapsize = 200, tilesize = 64, tiles = 15;
int** arr, gamestate, currdialogue, numgoblins, numgoblinriders, blind, currblessing;
bool combat, gameover, keyheldW, keyheldA, keyheldS, keyheldD, keyheldenter, keyheldE, dialogue, keypicked, locked, bossroom;
RenderWindow window(VideoMode(tilesize* tiles, tilesize* tiles), "RPG", Style::Close);
Text text;
Font font;

//Function Prototypes
void Destroy();

//Classes
class Goblin;
class GoblinRider;
class Boss;

class Audio
{
private:
	Music bgmusic1, bgmusic2, bgmusic3;
	Sound footsteps, attack, textbox, playerhurt, enemyhurt, enemygrunt, wasted, door, water;
	SoundBuffer footstepsbuffer, attackbuffer, textboxbuffer, playerhurtbuffer, enemyhurtbuffer, enemygruntbuffer, wastedbuffer, doorbuffer, waterbuffer;
	bool inc, transitioning;
	int state;
	double volume;
	Clock clk, clk1;
	string dest;
public:
	Audio()
	{
		bgmusic1.openFromFile("Resources/Audios/bgmusicnormal.wav");
		bgmusic2.openFromFile("Resources/Audios/goblin fight.wav");
		bgmusic3.openFromFile("Resources/Audios/maze music.wav");
		footstepsbuffer.loadFromFile("Resources/Audios/indoor footsteps.wav");
		attackbuffer.loadFromFile("Resources/Audios/attack.wav");
		textboxbuffer.loadFromFile("Resources/Audios/textbox.wav");
		playerhurtbuffer.loadFromFile("Resources/Audios/playerhurt.wav");
		enemyhurtbuffer.loadFromFile("Resources/Audios/enemyhurt.wav");
		enemygruntbuffer.loadFromFile("Resources/Audios/enemy grunt.wav");
		wastedbuffer.loadFromFile("Resources/Audios/wasted.wav");
		doorbuffer.loadFromFile("Resources/Audios/dooropen.wav");
		waterbuffer.loadFromFile("Resources/Audios/water.wav");
		volume = 10;
		state = 100;
		inc = transitioning = false;
		bgmusic1.setLoop(true);
		bgmusic1.setVolume(20);
		bgmusic2.setLoop(true);
		bgmusic2.setVolume(20);
		bgmusic3.setLoop(true);
		bgmusic3.setVolume(20);
		bgmusic1.play();
		footsteps.setVolume(40);
		footsteps.setBuffer(footstepsbuffer);
		attack.setVolume(20);
		attack.setBuffer(attackbuffer);
		textbox.setVolume(80);
		textbox.setBuffer(textboxbuffer);
		playerhurt.setVolume(20);
		playerhurt.setBuffer(playerhurtbuffer);
		enemyhurt.setVolume(20);
		enemyhurt.setBuffer(enemyhurtbuffer);
		enemygrunt.setVolume(20);
		enemygrunt.setBuffer(enemygruntbuffer);
		wasted.setVolume(20);
		wasted.setBuffer(wastedbuffer);
		door.setVolume(20);
		door.setBuffer(doorbuffer);
		water.setVolume(50);
		water.setBuffer(waterbuffer);
	}
	void settransitioning(bool n) { transitioning = n; }
	bool gettransitioning() { return transitioning; }
	void transition(string s = "")
	{
		if (s != "")dest = s;
		if (clk1.getElapsedTime() >= Time(milliseconds(30)))
		{
			if (inc)
			{
				if (state == 100)
				{
					transitioning = false;
					inc = false;
				}
				else
				{
					state++;
					bgmusic1.setVolume(state / volume);
					bgmusic2.setVolume(state / (volume * 4));
					bgmusic3.setVolume(state / volume);
				}
			}
			else
			{
				if (state == 0)
				{
					inc = true;
					bgmusic1.stop();
					bgmusic2.stop();
					bgmusic3.stop();
					if (dest == "peace")bgmusic1.play();
					else if (dest == "fight")bgmusic2.play();
					else if (dest == "maze")bgmusic3.play();
				}
				else
				{
					state--;
					bgmusic1.setVolume(state / volume);
					bgmusic2.setVolume(state / (volume * 4));
					bgmusic3.setVolume(state / volume);
				}
			}
			clk1.restart();
		}
	}
	void playsound(string s)
	{
		if (s == "footsteps" && footsteps.getStatus() != 2)footsteps.play();
		else if (s == "attack" && attack.getStatus() != 2)attack.play();
		else if (s == "textbox" && textbox.getStatus() != 2)textbox.play();
		else if (s == "playerhurt" && playerhurt.getStatus() != 2)playerhurt.play();
		else if (s == "enemyhurt" && enemyhurt.getStatus() != 2)enemyhurt.play();
		else if (s == "enemygrunt" && enemygrunt.getStatus() != 2)enemygrunt.play();
		else if (s == "wasted" && wasted.getStatus() != 2)wasted.play();
		else if (s == "door" && door.getStatus() != 2)door.play();
		else if (s == "water" && water.getStatus() != 2)water.play();
	}
	void stopsound(string s)
	{
		if (s == "footsteps")footsteps.stop();
		else if (s == "attack")attack.stop();
		else if (s == "textbox")textbox.stop();
		else if (s == "playerhurt")playerhurt.stop();
		else if (s == "enemyhurt")enemyhurt.stop();
		else if (s == "enemygrunt")enemygrunt.stop();
		else if (s == "door")door.stop();
		else if (s == "water")water.stop();
	}
	void stopall()
	{
		footsteps.stop();
		attack.stop();
		textbox.stop();
		playerhurt.stop();
		enemyhurt.stop();
		enemygrunt.stop();
		bgmusic1.stop();
		bgmusic2.stop();
		bgmusic3.stop();
		wasted.stop();
		door.stop();
		water.stop();
	}
};
class Player
{
private:
	Sprite player;
	Texture playertexturefrontwalk, playertexturebackwalk, playertextureleftwalk, playertexturerightwalk, playertexturefrontattack, playertexturebackattack, playertextureleftattack, playertexturerightattack, playertexturedeath;
	int cordx, cordy, direction, state, attack, health, totalhealth, critchance;
	long double posx, posy, speed, gposx, gposy, cooldown;
	bool attacking, healsused[4];
	Clock clk;
public:
	Player()
	{
		playertexturefrontwalk.loadFromFile("Resources/Graphics/character/front walk.png");
		playertexturebackwalk.loadFromFile("Resources/Graphics/character/back walk.png");
		playertextureleftwalk.loadFromFile("Resources/Graphics/character/left walk.png");
		playertexturerightwalk.loadFromFile("Resources/Graphics/character/right walk.png");
		playertexturefrontattack.loadFromFile("Resources/Graphics/character/front attack.png");
		playertexturebackattack.loadFromFile("Resources/Graphics/character/back attack.png");
		playertextureleftattack.loadFromFile("Resources/Graphics/character/left attack.png");
		playertexturerightattack.loadFromFile("Resources/Graphics/character/right attack.png");
		playertexturedeath.loadFromFile("Resources/Graphics/character/death.png");
		player.setScale(tilesize / 32 * 1.5, tilesize / 32 * 1.5);
		posx = posy = direction = state = 0;
		cordx = 18;
		cordy = 17;
		gposx = cordx * tilesize;
		gposy = cordy * tilesize;
		player.setPosition(tilesize * (tiles / 2) - 64, tilesize * (tiles / 2) - 64);
		speed = 2;
		attacking = false;
		attack = 15;
		cooldown = 0.5;
		totalhealth = health = 100;
		critchance = 20;
		healsused[0] = healsused[1] = healsused[2] = healsused[3] = false;
	}
	double getposx() { return posx; }
	double getposy() { return posy; }
	double getgposx() { return gposx; }
	double getgposy() { return gposy; }
	int getcordx() { return cordx; }
	int getcordy() { return cordy; }
	void setposx(double n) { posx = n; }
	void setposy(double n) { posy = n; }
	void setgposx(double n) { gposx = n; }
	void setgposy(double n) { gposy = n; }
	void setcordx(int n) { cordx = n; }
	void setcordy(int n) { cordy = n; }
	double getspeed() { return speed; }
	void setdirection(int n) { direction = n; }
	int getstate() { return state; }
	void setstate(int n) { state = n; }
	bool getattacking() { return attacking; }
	void setattacking(bool n) { attacking = n; }
	int getattack() { return attack; }
	int gethealth() { return health; }
	int gettotalhealth() { return totalhealth; }
	void sethealth(int n) { health = n; }
	double getcooldown() { return cooldown; }
	int getcritchance() { return critchance; }
	void setspeed(double n) { speed = n; }
	Sprite& operator[](int i) { return player; }
	int getdirection() { return direction; }
	void Draw()
	{
		if (gameover) { player.setTexture(playertexturedeath); window.draw(player); return; }
		if (direction == 0)
		{
			if (!attacking)player.setTexture(playertexturefrontwalk);
			else player.setTexture(playertexturefrontattack);
		}
		else if (direction == 1)
		{
			if (!attacking)player.setTexture(playertextureleftwalk);
			else player.setTexture(playertextureleftattack);
		}
		else if (direction == 2)
		{
			if (!attacking)player.setTexture(playertexturerightwalk);
			else player.setTexture(playertexturerightattack);
		}
		else if (direction == 3)
		{
			if (!attacking)player.setTexture(playertexturebackwalk);
			else player.setTexture(playertexturebackattack);
		}
		player.setTextureRect(IntRect(state * 64, 0, 64, 64));
		window.draw(player);
	}
	void Movement(Audio& audio)
	{
		if (clk.getElapsedTime() >= Time(milliseconds(10)))
		{
			if (keyheldW || keyheldA || keyheldS || keyheldD)audio.playsound("footsteps");
			else audio.stopsound("footsteps");
			CheckDeath(audio);
			if (Keyboard::isKeyPressed(Keyboard::W))
			{
				keyheldW = true;
				if (!isblocked("up"))
				{
					posy -= speed;
					gposy -= speed;
					if (posy <= -tilesize)
					{
						posy = 0;
						cordy--;
						if (!attacking)state++;
						if (!attacking && state >= 6)state = 0;
					}
				}
				if (!attacking)direction = 3;
			}
			if (Keyboard::isKeyPressed(Keyboard::S))
			{
				keyheldS = true;
				if (!isblocked("down"))
				{
					posy += speed;
					gposy += speed;
					if (posy >= tilesize)
					{
						posy = 0;
						cordy++;
						if (!attacking)state++;
						if (!attacking && state >= 6)state = 0;
					}
				}
				if (!attacking)direction = 0;
			}
			if (Keyboard::isKeyPressed(Keyboard::A))
			{
				keyheldA = true;
				if (!isblocked("left"))
				{
					posx -= speed;
					gposx -= speed;
					if (posx <= -tilesize)
					{
						posx = 0;
						cordx--;
						if (!attacking)state++;
						if (!attacking && state >= 6)state = 0;
					}
				}
				if (!attacking)direction = 1;
			}
			if (Keyboard::isKeyPressed(Keyboard::D))
			{
				keyheldD = true;
				if (!isblocked("right"))
				{
					posx += speed;
					gposx += speed;
					if (posx >= tilesize)
					{
						posx = 0;
						cordx++;
						if (!attacking)state++;
						if (!attacking && state >= 6)state = 0;
					}
				}
				if (!attacking)direction = 2;
			}
			clk.restart();
		}
	}
	bool isblockedtile(int n)
	{
		if (n >= 976 && n < 1055)return false;
		int temp[] = { 0,48,49,314,298,325,44,50,51,52,53,54,55,56,57,64,66,156,157,158,159,164,165,166,167,172,173,174,175,301,550,558,1060,1061,1062,1063 };
		for (int i : temp)if (n == i)return false;
		return true;
	}
	bool isblocked(string dir)
	{
		if (dir == "up" && ((isblockedtile(arr[(int)((gposy - speed) / tilesize)][(int)((gposx + 32) / tilesize)])))) return true;
		if (dir == "down" && ((isblockedtile(arr[(int)((gposy + speed) / tilesize) + 1][(int)((gposx + 32) / tilesize)])))) return true;
		if (dir == "left" && ((isblockedtile(arr[(int)((gposy + 32) / tilesize)][(int)((gposx - speed) / tilesize)])))) return true;
		if (dir == "right" && ((isblockedtile(arr[(int)((gposy + 32) / tilesize)][(int)((gposx + speed) / tilesize) + 1])))) return true;
		return false;
	}
	void Attack(Clock& clk, Goblin* goblin, Audio& audio, GoblinRider* goblinrider,Boss& boss);
	bool CheckDeath(Audio& audio)
	{
		if (health <= 0 || ((int)((gposy + 32) / tilesize) == 25 && (int)((gposx + 32) / tilesize) == 19))
		{
			gameover = true;
			audio.stopall();
			audio.playsound("wasted");
			return true;
		}
		return false;
	}
	void operator+(int n)
	{
		if (health + n < totalhealth)health += n;
		else health = totalhealth;
	}
	void operator-(int n) { healsused[n] = true; }
	bool gethealsused(int n) { return healsused[n]; }
};
class Ghost
{
private:
	Sprite ghostie;
	Texture ghostietexture;
	bool following;
	Clock clk;
	int dir, state, cordy, cordx, posx, posy, lastdir, tempdir;
	double offset, gposx, gposy, pposy, pposx, ppposy, ppposx, stage, temppposx, temppposy;
	bool up, shiftup, shiftdown, shiftleft, shiftright, animating, inc;
public:
	Ghost(int i, int j)
	{
		ghostietexture.loadFromFile("Resources/Graphics/ghost/ghost.png");
		ghostie.setTexture(ghostietexture);
		following = false;
		state = 0;
		dir = 0;
		offset = 0;
		posx = posy = 0;
		pposx = pposy = ppposx = ppposy = lastdir = temppposx = temppposy = tempdir = 0;
		stage = 255;
		cordy = i;
		cordx = j;
		gposx = cordx * tilesize;
		gposy = cordy * tilesize;
		up = true;
		ghostie.setScale(2, 2);
		clk.restart();
		shiftup = shiftleft = shiftdown = shiftright = animating = inc = false;
	}
	void Draw(Player& player)
	{
		ghostie.setTextureRect(IntRect(state * 32, dir * 32, 32, 32));
		Animation(player);
		if (!following)ghostie.setPosition(gposx - player.getgposx() + 448, gposy - player.getgposy() + 448 + offset);
		else
		{
			if (player.getdirection() != lastdir && ((lastdir == 0 && !keyheldS) || (lastdir == 1 && !keyheldA) || (lastdir == 2 && !keyheldD) || (lastdir == 3 && !keyheldW)))
			{
				tempdir = player.getdirection();
				animating = true;
				if (player.getdirection() == 0) { pposy = 64, pposx = 0; }
				else if (player.getdirection() == 1) { pposy = 0, pposx = -64; }
				else if (player.getdirection() == 2) { pposy = 0, pposx = 64; }
				else if (player.getdirection() == 3) { pposy = -64, pposx = 0; }
			}
			if (animating)
			{
				if (tempdir != player.getdirection())
				{
					inc = false;
					lastdir = player.getdirection();
					ppposx = pposx;
					ppposy = pposy;
					tempdir = player.getdirection();
					animating = true;
					if (player.getdirection() == 0) { pposy = 64, pposx = 0; }
					else if (player.getdirection() == 1) { pposy = 0, pposx = -64; }
					else if (player.getdirection() == 2) { pposy = 0, pposx = 64; }
					else if (player.getdirection() == 3) { pposy = -64, pposx = 0; }
				}
				if (inc)
				{
					stage += 1;
					ghostie.setPosition(448 - pposx, 448 - pposy + offset);
				}
				else
				{
					stage -= 1;
					ghostie.setPosition(448 - ppposx, 448 - ppposy + offset);
				}
				ghostie.setColor(Color(255, 255, 255, stage));
				if (stage == 0)inc = true;
				else if (stage == 255)
				{
					animating = false;
					inc = false;
					lastdir = player.getdirection();
					ppposx = pposx;
					ppposy = pposy;
				}
			}
			else ghostie.setPosition(448 - pposx, 448 - pposy + offset);
		}

		window.draw(ghostie);
	}
	void Animation(Player& player)
	{
		if (following)dir = player.getdirection();
		if (clk.getElapsedTime() >= Time(milliseconds(100)))
		{
			if (abs(offset) >= 10)up = !up;
			if (up)offset -= 2;
			else offset += 2;
			if (following)
			{
				state++;
				if (state >= 3)state = 0;
			}
			clk.restart();
		}
	}
	void setpposy(double n) { pposy = n; }
	void setpposx(double n) { pposx = n; }
	void setppposy(double n) { ppposy = n; }
	void setppposx(double n) { ppposx = n; }
	void setfollowing(bool n) { following = n; }
	bool getfollowing() { return following; }
	void setlastdir(int n) { lastdir = n; }
	int getcordy() { return cordy; }
	int getcordx() { return cordx; }
};
class Mob
{
protected:
	Sprite mob;
	Clock clk, clk1, clk2, clk3;
	int lasthealth, health, totalhealth, attack, state, stage, healthstate, cordx, cordy;
	double gposx, gposy, speed;
	Texture downattack, downdeath, downhurt, downrun, upattack, updeath, uphurt, uprun, leftattack, leftdeath, lefthurt, leftrun, rightattack, rightdeath, righthurt, rightrun;
	bool drawtext, animating, died, drained;
public:
	Mob(int health = 0, double speed = 0, bool hostile = false) :health(health), speed(speed) { totalhealth = lasthealth = health; }
	double getgposx() { return gposx; }
	double getgposy() { return gposy; }
	bool getdied() { return died; }
	bool getanimating() { return animating; }
	bool getdrawtext() { return drawtext; }
	virtual void PathFind(Player& player)
	{
		if (clk1.getElapsedTime() >= Time(milliseconds(10)))
		{
			if (gposy - speed > player.getgposy())
			{
				gposy -= speed;
				state = 0;
			}
			else if (gposy + speed < player.getgposy())
			{
				gposy += speed;
				state = 4;
			}
			if (gposx - speed > player.getgposx())
			{
				gposx -= speed;
				state = 8;
			}
			else if (gposx + speed < player.getgposx())
			{
				gposx += speed;
				state = 12;
			}
			clk1.restart();
		}
	}
	void Grunt(Audio& audio)
	{
		if (clk3.getElapsedTime() >= Time(milliseconds(2000)))
		{
			audio.playsound("enemygrunt");
			clk3.restart();
		}
	}
	void Heal()
	{
		if (clk2.getElapsedTime() >= Time(milliseconds(5000)))
		{
			int raise = (totalhealth - health) / 2;
			health = (raise + health >= totalhealth || raise < (totalhealth * 0.1)) ? totalhealth : health + raise;
			clk2.restart();
		}
	}
	virtual void Hurt(Player& player)
	{
		clk2.restart();
		animating = true;
		int temp = rand() % 100 + 1;
		if (temp <= player.getcritchance())
		{
			text.setFillColor(Color::Red);
			text.setString(to_string(-2 * player.getattack()));
			health -= 2 * player.getattack();
		}
		else
		{
			text.setFillColor(Color::White);
			text.setString(to_string(-player.getattack()));
			health -= player.getattack();
		}
		drawtext = true;
	}
};
class Goblin : public Mob
{
private:
	int type;
	int lastposx, lastposy;
	bool goingup, goingdown, goingleft, goingright, avoidingup, avoidingdown, avoidingleft, avoidingright;
	RectangleShape healthbar;
	Clock clk4;
public:
	Goblin(int i = 0, int j = 0, int att = rand() % 11 + 15, int random = rand() % 2) : Mob(100, 1, true)
	{
		type = random;
		string character = (!random) ? "Goblin" : "Skeleton";
		downattack.loadFromFile("Resources/Graphics/goblin/" + character + "DownAttack.png");
		downdeath.loadFromFile("Resources/Graphics/goblin/" + character + "DownDeath.png");
		downhurt.loadFromFile("Resources/Graphics/goblin/" + character + "DownHurt.png");
		downrun.loadFromFile("Resources/Graphics/goblin/" + character + "DownRun.png");
		upattack.loadFromFile("Resources/Graphics/goblin/" + character + "UpAttack.png");
		updeath.loadFromFile("Resources/Graphics/goblin/" + character + "UpDeath.png");
		uphurt.loadFromFile("Resources/Graphics/goblin/" + character + "UpHurt.png");
		uprun.loadFromFile("Resources/Graphics/goblin/" + character + "UpRun.png");
		leftattack.loadFromFile("Resources/Graphics/goblin/" + character + "LeftAttack.png");
		leftdeath.loadFromFile("Resources/Graphics/goblin/" + character + "LeftDeath.png");
		lefthurt.loadFromFile("Resources/Graphics/goblin/" + character + "LeftHurt.png");
		leftrun.loadFromFile("Resources/Graphics/goblin/" + character + "LeftRun.png");
		rightattack.loadFromFile("Resources/Graphics/goblin/" + character + "RightAttack.png");
		rightdeath.loadFromFile("Resources/Graphics/goblin/" + character + "RightDeath.png");
		righthurt.loadFromFile("Resources/Graphics/goblin/" + character + "RightHurt.png");
		rightrun.loadFromFile("Resources/Graphics/goblin/" + character + "RightRun.png");
		attack = att;
		state = 5;
		stage = healthstate = 0;
		mob.setScale(1.75, 1.75);
		clk.restart();
		clk1.restart();
		animating = died = goingdown = goingleft = goingright = avoidingright = avoidingup = avoidingdown = avoidingleft = drawtext = false;
		goingup = true;
		drained = true;
		cordx = j;
		cordy = i;
		gposx = j * tilesize;
		gposy = i * tilesize;
		healthbar.setFillColor(Color(255, 0, 0, 125));
		healthbar.setSize(Vector2f(48, 5));
	}
	void Animation(Player& player, Audio& audio)
	{
		if (clk.getElapsedTime() >= Time(milliseconds(100)))
		{
			stage++;
			int max = 0;
			if (state % 4 == 0)max = 5;
			else if (state % 4 == 1) max = (type) ? 7 : 9;
			else if (state % 4 == 2) max = 3;
			else if (state % 4 == 3) max = (type) ? 7 : 8;
			if (stage >= max)
			{
				stage = 0;
				animating = false;
				if (died)
				{
					gposx = gposy = 0;
					drawtext = false;
					audio.stopsound("enemygrunt");
				}
				if (drawtext)drawtext = false;
				if (state % 4 == 1 && abs(player.getgposx() - gposx) <= 40 && abs(player.getgposy() - gposy) <= 40)
				{
					player.sethealth(player.gethealth() - attack);
					audio.playsound("playerhurt");
					if (player.CheckDeath(audio))
					{
						health -= totalhealth;
						CheckDeath();
					}
				}
			}
			clk.restart();
		}
	}
	void Draw(Player& player, Audio& audio)
	{
		if (state == 0) { mob.setTexture(uprun); }
		else if (state == 1) { mob.setTexture(upattack); }
		else if (state == 2) { mob.setTexture(uphurt); }
		else if (state == 3) { mob.setTexture(updeath); }
		else if (state == 4) { mob.setTexture(downrun); }
		else if (state == 5) { mob.setTexture(downattack); }
		else if (state == 6) { mob.setTexture(downhurt); }
		else if (state == 7) { mob.setTexture(downdeath); }
		else if (state == 8) { mob.setTexture(leftrun); }
		else if (state == 9) { mob.setTexture(leftattack); }
		else if (state == 10) { mob.setTexture(lefthurt); }
		else if (state == 11) { mob.setTexture(leftdeath); }
		else if (state == 12) { mob.setTexture(rightrun); }
		else if (state == 13) { mob.setTexture(rightattack); }
		else if (state == 14) { mob.setTexture(righthurt); }
		else if (state == 15) { mob.setTexture(rightdeath); }
		mob.setTextureRect(IntRect(stage * 48, 0, 48, 48));
		mob.setPosition(gposx - player.getgposx() + 448, gposy - player.getgposy() + 448);
		Animation(player, audio);
		window.draw(mob);
		if (health <= 0)healthbar.setSize(Vector2f(0, 5));
		else
		{
			if (lasthealth != health)
			{
				healthbar.setSize(Vector2f((lasthealth - ((lasthealth - health) / 200.0 * healthstate)) / totalhealth * 48, 5));
				healthstate++;
				drained = false;
				if (healthstate == 200)drained = true;
			}
		}
		window.draw(healthbar);
		if (drained)
		{
			lasthealth = health;
			healthstate = 0;
		}
		healthbar.setPosition(gposx - player.getgposx() + 448 + 15, gposy - player.getgposy() + 448 + 70);
		window.draw(healthbar);
	}
	void PathFind(Player& player)
	{
		if (clk1.getElapsedTime() >= Time(milliseconds(10)))
		{
			int tilex = (gposx + 32) / tilesize;
			int tiley = (gposy + 32) / tilesize;
			bool changedir = false;
			if (clk4.getElapsedTime() >= Time(seconds(0.5)))
			{
				if (lastposx == tilex || lastposy == tiley)
				{
					int temp = rand() % 2;
					if (temp == 0)
					{
						goingup = true;
						state = 0;
					}
					else if (temp == 1)
					{
						goingdown = true;
						state = 4;
					}
					if (temp == 2)
					{
						goingleft = true;
						state = 8;
					}
					else if (temp == 3)
					{
						goingright = true;
						state = 12;
					}
				}
				lastposy == tiley;
				lastposx == tilex;
				clk4.restart();
				clk1.restart();
				return;
			}
			if (goingup)
			{
				state = 0;
				if (player.isblockedtile(arr[tiley - 1][tilex]) || player.getgposy() == gposy)
				{
					changedir = true;
					goingup = false;
					gposy += speed;
				}
				else gposy -= speed;
			}
			else if (goingdown)
			{
				state = 4;
				if (player.isblockedtile(arr[tiley + 1][tilex]) || player.getgposy() == gposy)
				{
					changedir = true;
					goingdown = false;
					gposy -= speed;
				}
				else gposy += speed;
			}
			if (goingleft)
			{
				state = 8;
				if (player.isblockedtile(arr[tiley][tilex - 1]) || player.getgposx() == gposx)
				{
					changedir = true;
					goingleft = false;
					gposx += speed;
				}
				else gposx -= speed;
			}
			else if (goingright)
			{
				state = 12;
				if (player.isblockedtile(arr[tiley][tilex + 1]) || player.getgposx() == gposx)
				{
					changedir = true;
					goingright = false;
					gposx -= speed;
				}
				else gposx += speed;
			}
			if (changedir)
			{
				if (player.getgposy() <= gposy && !player.isblockedtile(arr[tiley - 1][tilex]))
				{
					goingup = true;
					state = 0;
				}
				else if (player.getgposy() > gposy && !player.isblockedtile(arr[tiley + 1][tilex]))
				{
					goingdown = true;
					state = 4;
				}
				if (player.getgposx() <= gposx && !player.isblockedtile(arr[tiley][tilex - 1]))
				{
					goingleft = true;
					state = 8;
				}
				else if (player.getgposx() > gposx && !player.isblockedtile(arr[tiley][tilex + 1]))
				{
					goingright = true;
					state = 12;
				}
				else
				{
					int temp = rand() % 2;
					if (temp == 0)
					{
						goingup = true;
						state = 0;
					}
					else if (temp == 1)
					{
						goingdown = true;
						state = 4;
					}
					if (temp == 2)
					{
						goingleft = true;
						state = 8;
					}
					else if (temp == 3)
					{
						goingright = true;
						state = 12;
					}
				}
				changedir = false;
			}
			clk1.restart();
		}
	}
	void Attack(Player& player, Audio& audio)
	{
		Grunt(audio);
		if (abs(player.getgposx() - gposx) <= 40 && abs(player.getgposy() - gposy) <= 40 && !animating)
		{
			if (player.getgposx() < gposx && abs(player.getgposx() - gposx) > abs(player.getgposy() - gposy))
			{
				state = 9;
				animating = true;
			}
			else if (player.getgposx() > gposx && abs(player.getgposx() - gposx) > abs(player.getgposy() - gposy))
			{
				state = 13;
				animating = true;
			}
			else if (player.getgposy() < gposy && abs(player.getgposx() - gposx) < abs(player.getgposy() - gposy))
			{
				state = 1;
				animating = true;
			}
			else if (player.getgposy() > gposy && abs(player.getgposx() - gposx) < abs(player.getgposy() - gposy))
			{
				state = 5;
				animating = true;
			}
		}
	}
	void CheckDeath()
	{
		if (health <= 0)
		{
			if (state == 0 || state == 1 || state == 2)state = 3;
			else if (state == 4 || state == 5 || state == 6)state = 7;
			else if (state == 8 || state == 9 || state == 10)state = 11;
			else if (state == 12 || state == 13 || state == 14)state = 15;
			stage = 0;
			animating = died = true;
		}
	}
	void Hurt(Player& player)
	{
		if (state == 0 || state == 1)state = 2, stage = 0;
		else if (state == 4 || state == 5)state = 6, stage = 0;
		else if (state == 8 || state == 9)state = 10, stage = 0;
		else if (state == 12 || state == 13)state = 14, stage = 0;
		Mob::Hurt(player);
		CheckDeath();
	}
};
class GoblinRider : public Mob
{
private:
	RectangleShape healthbar;
public:
	GoblinRider(int i = 0, int j = 0, int att = rand() % 11 + 15, int random = rand() % 2) : Mob(100, (rand() % 151 + 50) / 100.0, true)
	{
		downattack.loadFromFile("Resources/Graphics/goblin/GoblinDownAttack1.png");
		downdeath.loadFromFile("Resources/Graphics/goblin/GoblinDownDeath1.png");
		downhurt.loadFromFile("Resources/Graphics/goblin/GoblinDownHurt1.png");
		downrun.loadFromFile("Resources/Graphics/goblin/GoblinDownRun1.png");
		upattack.loadFromFile("Resources/Graphics/goblin/GoblinUpAttack1.png");
		updeath.loadFromFile("Resources/Graphics/goblin/GoblinUpDeath1.png");
		uphurt.loadFromFile("Resources/Graphics/goblin/GoblinUpHurt1.png");
		uprun.loadFromFile("Resources/Graphics/goblin/GoblinUpRun1.png");
		leftattack.loadFromFile("Resources/Graphics/goblin/GoblinLeftAttack1.png");
		leftdeath.loadFromFile("Resources/Graphics/goblin/GoblinLeftDeath1.png");
		lefthurt.loadFromFile("Resources/Graphics/goblin/GoblinLeftHurt1.png");
		leftrun.loadFromFile("Resources/Graphics/goblin/GoblinLeftRun1.png");
		rightattack.loadFromFile("Resources/Graphics/goblin/GoblinRightAttack1.png");
		rightdeath.loadFromFile("Resources/Graphics/goblin/GoblinRightDeath1.png");
		righthurt.loadFromFile("Resources/Graphics/goblin/GoblinRightHurt1.png");
		rightrun.loadFromFile("Resources/Graphics/goblin/GoblinRightRun1.png");
		attack = att;
		state = 5;
		stage = healthstate = 0;
		clk.restart();
		clk1.restart();
		animating = died = drawtext = false;
		drained = true;
		cordx = j;
		cordy = i;
		gposx = j * tilesize + 40;
		gposy = i * tilesize + 40;
		healthbar.setFillColor(Color(255, 0, 0, 125));
		healthbar.setSize(Vector2f(40, 5));
	}
	void Animation(Player& player, Audio& audio)
	{
		if (clk.getElapsedTime() >= Time(milliseconds(100)))
		{
			stage++;
			int max = 0;
			if (state % 4 == 0)max = 3;
			else if (state % 4 == 1) max = 5;
			else if (state % 4 == 2) max = 3;
			else if (state % 4 == 3) max = 9;
			if (stage >= max)
			{
				stage = 0;
				animating = false;
				if (died)
				{
					gposx = gposy = 0;
					drawtext = false;
					audio.stopsound("enemygrunt");
				}
				if (drawtext)drawtext = false;
				if (state % 4 == 1 && abs(player.getgposx() - gposx) <= 40 && abs(player.getgposy() - gposy) <= 40)
				{
					player.sethealth(player.gethealth() - attack);
					audio.playsound("playerhurt");
					if (player.CheckDeath(audio))
					{
						health -= totalhealth;
						CheckDeath();
					}
				}
			}
			clk.restart();
		}
	}
	void Draw(Player& player, Audio& audio)
	{
		if (state == 0) { mob.setTexture(uprun); }
		else if (state == 1) { mob.setTexture(upattack); }
		else if (state == 2) { mob.setTexture(uphurt); }
		else if (state == 3) { mob.setTexture(updeath); }
		else if (state == 4) { mob.setTexture(downrun); }
		else if (state == 5) { mob.setTexture(downattack); }
		else if (state == 6) { mob.setTexture(downhurt); }
		else if (state == 7) { mob.setTexture(downdeath); }
		else if (state == 8) { mob.setTexture(leftrun); }
		else if (state == 9) { mob.setTexture(leftattack); }
		else if (state == 10) { mob.setTexture(lefthurt); }
		else if (state == 11) { mob.setTexture(leftdeath); }
		else if (state == 12) { mob.setTexture(rightrun); }
		else if (state == 13) { mob.setTexture(rightattack); }
		else if (state == 14) { mob.setTexture(righthurt); }
		else if (state == 15) { mob.setTexture(rightdeath); }
		mob.setTextureRect(IntRect(stage * 80, 0, 80, 80));
		mob.setPosition(gposx - player.getgposx() + 448, gposy - player.getgposy() + 448);
		Animation(player, audio);
		window.draw(mob);
		if (health <= 0)healthbar.setSize(Vector2f(0, 5));
		else
		{
			if (lasthealth != health)
			{
				healthbar.setSize(Vector2f((lasthealth - ((lasthealth - health) / 200.0 * healthstate)) / totalhealth * 40, 5));
				healthstate++;
				drained = false;
				if (healthstate == 200)drained = true;
			}
		}
		window.draw(healthbar);
		if (drained)
		{
			lasthealth = health;
			healthstate = 0;
		}
		healthbar.setPosition(gposx - player.getgposx() + 448 + 20, gposy - player.getgposy() + 448 + 70);
		window.draw(healthbar);
	}
	void Attack(Player& player, Audio& audio)
	{
		Grunt(audio);
		if (abs(player.getgposx() - gposx) <= 40 && abs(player.getgposy() - gposy) <= 40 && !animating)
		{
			if (player.getgposx() < gposx && abs(player.getgposx() - gposx) > abs(player.getgposy() - gposy))
			{
				state = 9;
				animating = true;
			}
			else if (player.getgposx() > gposx && abs(player.getgposx() - gposx) > abs(player.getgposy() - gposy))
			{
				state = 13;
				animating = true;
			}
			else if (player.getgposy() < gposy && abs(player.getgposx() - gposx) < abs(player.getgposy() - gposy))
			{
				state = 1;
				animating = true;
			}
			else if (player.getgposy() > gposy && abs(player.getgposx() - gposx) < abs(player.getgposy() - gposy))
			{
				state = 5;
				animating = true;
			}
		}
	}
	void CheckDeath()
	{
		if (health <= 0)
		{
			if (state == 0 || state == 1 || state == 2)state = 3;
			else if (state == 4 || state == 5 || state == 6)state = 7;
			else if (state == 8 || state == 9 || state == 10)state = 11;
			else if (state == 12 || state == 13 || state == 14)state = 15;
			stage = 0;
			animating = died = true;
		}
	}
	void Hurt(Player& player)
	{
		if (state == 0 || state == 1)state = 2, stage = 0;
		else if (state == 4 || state == 5)state = 6, stage = 0;
		else if (state == 8 || state == 9)state = 10, stage = 0;
		else if (state == 12 || state == 13)state = 14, stage = 0;
		Mob::Hurt(player);
		CheckDeath();
	}
};
class Boss : public Mob
{
private:
	int phase;
	float cooldown;
	Texture frametexture, bartexture, downattack1, downattack2, downattack3, upattack1, upattack2, upattack3, leftattack1, leftattack2, leftattack3, rightattack1, rightattack2, rightattack3;
	Sprite frame, bar;
	Clock clk4;
public:
	Boss(int i = 0, int j = 0, int att = 5) : Mob(256, 3, true)
	{
		downattack1.loadFromFile("Resources/Graphics/boss/BossDownAttack1.png");
		downattack2.loadFromFile("Resources/Graphics/boss/BossDownAttack2.png");
		downattack3.loadFromFile("Resources/Graphics/boss/BossDownAttack3.png");
		downdeath.loadFromFile("Resources/Graphics/boss/BossDownDeath.png");
		downhurt.loadFromFile("Resources/Graphics/boss/BossDownHurt.png");
		downrun.loadFromFile("Resources/Graphics/boss/BossDownRun.png");
		upattack1.loadFromFile("Resources/Graphics/boss/BossUpAttack1.png");
		upattack2.loadFromFile("Resources/Graphics/boss/BossUpAttack2.png");
		upattack3.loadFromFile("Resources/Graphics/boss/BossUpAttack3.png");
		updeath.loadFromFile("Resources/Graphics/boss/BossUpDeath.png");
		uphurt.loadFromFile("Resources/Graphics/boss/BossUpHurt.png");
		uprun.loadFromFile("Resources/Graphics/boss/BossUpRun.png");
		leftattack1.loadFromFile("Resources/Graphics/boss/BossLeftAttack1.png");
		leftattack2.loadFromFile("Resources/Graphics/boss/BossLeftAttack2.png");
		leftattack3.loadFromFile("Resources/Graphics/boss/BossLeftAttack3.png");
		leftdeath.loadFromFile("Resources/Graphics/boss/BossLeftDeath.png");
		lefthurt.loadFromFile("Resources/Graphics/boss/BossLeftHurt.png");
		leftrun.loadFromFile("Resources/Graphics/boss/BossLeftRun.png");
		rightattack1.loadFromFile("Resources/Graphics/boss/BossRightAttack1.png");
		rightattack2.loadFromFile("Resources/Graphics/boss/BossRightAttack2.png");
		rightattack3.loadFromFile("Resources/Graphics/boss/BossRightAttack3.png");
		rightdeath.loadFromFile("Resources/Graphics/boss/BossRightDeath.png");
		righthurt.loadFromFile("Resources/Graphics/boss/BossRightHurt.png");
		rightrun.loadFromFile("Resources/Graphics/boss/BossRightRun.png");
		frametexture.loadFromFile("Resources/Graphics/boss/Healthbar.png");
		bartexture.loadFromFile("Resources/Graphics/boss/Healthbar1.png");
		frame.setTexture(frametexture);
		bar.setTexture(bartexture);
		attack = att;
		state = 6;
		stage = healthstate = phase =0;
		mob.setScale(2, 2);
		frame.setScale(0.5, 0.5);
		bar.setScale(0.5, 0.5);
		bar.setColor(Color(255, 255, 255, 180));
		clk.restart();
		clk1.restart();
		animating = died = drawtext = false;
		drained = true;
		gposx = j * tilesize;
		gposy = i * tilesize;
		cooldown = 0;
	}
	void Animation(Player& player, Audio& audio)
	{
		if (clk.getElapsedTime() >= Time(milliseconds(100)))
		{
			stage++;
			int max = 0;
			if (state % 6 == 0)max = 7;
			else if (state % 6 == 1) max = 6;
			else if (state % 6 == 2) max = 12;
			else if (state % 6 == 3) max = 3;
			else if (state % 6 == 4) max = 4;
			else if (state % 6 == 5) max = 8;
			if (stage >= max)
			{
				stage = 0;
				animating = false;
				if (died)
				{
					gposx = gposy = 0;
					drawtext = false;
				}
				if (drawtext)drawtext = false;
			}
			if ((stage == 5 && state %6==1 )|| ((stage == 3||stage==6||stage==9) && state % 6 == 2)|| (stage == 5 && state % 6 == 3))
			{
				if (abs(player.getgposx() - gposx) <= 40 && abs(player.getgposy() - gposy) <= 40)
				{
					audio.playsound("playerhurt");
					if (phase == 1)player.sethealth(player.gethealth() - attack);
					else player.sethealth(player.gethealth() - 2 * attack);
					if (player.CheckDeath(audio))
					{
						health -= totalhealth;
						CheckDeath();
					}
				}
			}
			clk.restart();
		}
	}
	void setstage(int n) { stage = n; }
	void Draw(Player& player, Audio& audio)
	{
		if (state == 0) { mob.setTexture(uprun); }
		else if (state == 1) { mob.setTexture(upattack1); }
		else if (state == 2) { mob.setTexture(upattack2); }
		else if (state == 3) { mob.setTexture(upattack3); }
		else if (state == 4) { mob.setTexture(uphurt); }
		else if (state == 5) { mob.setTexture(updeath); }
		else if (state == 6) { mob.setTexture(downrun); }
		else if (state == 7) { mob.setTexture(downattack1); }
		else if (state == 8) { mob.setTexture(downattack2); }
		else if (state == 9) { mob.setTexture(downattack3); }
		else if (state == 10) { mob.setTexture(downhurt); }
		else if (state == 11) { mob.setTexture(downdeath); }
		else if (state == 12) { mob.setTexture(leftrun); }
		else if (state == 13) { mob.setTexture(leftattack1); }
		else if (state == 14) { mob.setTexture(leftattack2); }
		else if (state == 15) { mob.setTexture(leftattack3); }
		else if (state == 16) { mob.setTexture(lefthurt); }
		else if (state == 17) { mob.setTexture(leftdeath); }
		else if (state == 18) { mob.setTexture(rightrun); }
		else if (state == 19) { mob.setTexture(rightattack1); }
		else if (state == 20) { mob.setTexture(rightattack2); }
		else if (state == 21) { mob.setTexture(rightattack3); }
		else if (state == 22) { mob.setTexture(righthurt); }
		else if (state == 23) { mob.setTexture(rightdeath); }
		mob.setTextureRect(IntRect(stage*80, 0, 80, 80));
		mob.setPosition(gposx - player.getgposx() + 448-40, gposy - player.getgposy() + 448-40);
		Animation(player, audio);
		window.draw(mob);
		frame.setPosition(gposx - player.getgposx() + 448, gposy - player.getgposy() + 448 + 80);
		bar.setPosition(gposx - player.getgposx() + 448, gposy - player.getgposy() + 448 + 80);
		window.draw(frame);
		if (lasthealth != health)
		{
			bar.setTextureRect(IntRect(0, 0,lasthealth-(lasthealth-health)/200.0*healthstate, 280));
			healthstate++;
			drained = false;
			if (healthstate == 200)drained = true;
		}
		window.draw(bar);
		if (drained)
		{
			lasthealth = health;
			healthstate = 0;
		}
		if (drained)
		{
			lasthealth = health;
			healthstate = 0;
		}
	}
	void Attack(Player& player, Audio& audio)
	{
		if (clk4.getElapsedTime()>=Time(seconds(cooldown)))
		{
			if (phase == 1)
			{
				if (abs(player.getgposx() - gposx) <= 40 && abs(player.getgposy() - gposy) <= 40 && !animating)
				{
					if (player.getgposx() < gposx && abs(player.getgposx() - gposx) > abs(player.getgposy() - gposy))
					{
						state = 13;
						animating = true;
					}
					else if (player.getgposx() > gposx && abs(player.getgposx() - gposx) > abs(player.getgposy() - gposy))
					{
						state = 19;
						animating = true;
					}
					else if (player.getgposy() < gposy && abs(player.getgposx() - gposx) < abs(player.getgposy() - gposy))
					{
						state = 1;
						animating = true;
					}
					else if (player.getgposy() > gposy && abs(player.getgposx() - gposx) < abs(player.getgposy() - gposy))
					{
						state = 7;
						animating = true;
					}
				}
			}
			else if (phase == 2)
			{
				if (abs(player.getgposx() - gposx) <= 40 && abs(player.getgposy() - gposy) <= 40 && !animating)
				{
					if (player.getgposx() < gposx && abs(player.getgposx() - gposx) > abs(player.getgposy() - gposy))
					{
						state = 14;
						animating = true;
					}
					else if (player.getgposx() > gposx && abs(player.getgposx() - gposx) > abs(player.getgposy() - gposy))
					{
						state = 20;
						animating = true;
					}
					else if (player.getgposy() < gposy && abs(player.getgposx() - gposx) < abs(player.getgposy() - gposy))
					{
						state = 2;
						animating = true;
					}
					else if (player.getgposy() > gposy && abs(player.getgposx() - gposx) < abs(player.getgposy() - gposy))
					{
						state = 8;
						animating = true;
					}
				}
			}
			clk4.restart();
		}
	}
	void CheckDeath()
	{
		if (health <= 0)
		{
			if (state >= 0 && state <= 4)state = 5;
			else if (state >= 6 && state <= 10)state = 11;
			else if (state >= 12 && state <= 16)state = 17;
			else if (state >= 18 && state <= 22)state = 23;
			stage = 0;
			animating = died = true;
		}
	}
	void Hurt(Player& player,Audio& audio)
	{
		if (state >= 0 && state <= 3)state = 4, stage = 0;
		else if (state >= 6 && state <= 9)state = 10, stage = 0;
		else if (state >= 12 && state <= 15)state = 16, stage = 0;
		else if (state >= 18 && state <= 21)state = 22, stage = 0;
		Mob::Hurt(player);
		if (health < totalhealth / 4&&phase!=2)
		{
				gamestate++;
				currdialogue = 22;
				dialogue = true;
				audio.stopsound("footsteps");
				audio.playsound("textbox");
				phase = 2;
				cooldown = 0.25;
				speed = 10;
				state = 6;
				attack = 2;
		}
		CheckDeath();
	}
	void PathFind(Player& player,Audio& audio)
	{
		if (phase!=0&&clk1.getElapsedTime() >= Time(milliseconds(10)))
		{
			if (gposy - speed > player.getgposy())
			{
				gposy -= speed;
				state = 0;
			}
			else if (gposy + speed < player.getgposy())
			{
				gposy += speed;
				state = 6;
			}
			if (gposx - speed > player.getgposx())gposx -= speed;
			else if (gposx + speed < player.getgposx())gposx += speed;
			clk1.restart();
		}
	}
	void setphase(int n) { phase = n; }
	void setspeed(int n) { speed = n; }
	int getspeed() { return speed; }
	void sethealth(int n) { health = n; }
	void setdiied(bool n) { died = n; }
};
class DialogueBox
{
private:
	Sprite dialoguebox, pic;
	Texture dialogueboxtexture, pictexture;
	string** dialogues;
	int* numdialogues;
	int totaldialogues, state, letters;
	Font font;
	Text text;
	Clock clk, clk1;
public:
	DialogueBox()
	{
		state = letters = 0;
		dialogueboxtexture.loadFromFile("Resources/Graphics/dialoguebox.png");
		font.loadFromFile("resources/Fonts/pixel.ttf");
		text.setFont(font);
		text.setFillColor(Color(214, 174, 53, 255));
		text.setCharacterSize(25);
		text.setPosition(320, 750);
		text.setLetterSpacing(3);
		dialoguebox.setTexture(dialogueboxtexture);
		dialoguebox.setPosition(71.2, 700);
		dialoguebox.setScale(1.4, 1);
		dialoguebox.setColor(Color(255, 255, 255, 190));
		pic.setPosition(105, 725);
		pic.setTexture(pictexture);
		totaldialogues = 25;
		numdialogues = new int[totaldialogues];
		numdialogues[0] = 9;
		numdialogues[1] = 1;
		numdialogues[2] = 1;
		numdialogues[3] = 9;
		numdialogues[4] = 1;
		numdialogues[5] = 2;
		numdialogues[6] = 2;
		numdialogues[7] = 2;
		numdialogues[8] = 3;
		numdialogues[9] = 1;
		numdialogues[10] = 7;
		numdialogues[11] = 2;
		numdialogues[12] = 5;
		numdialogues[13] = 7;
		numdialogues[14] = 4;
		numdialogues[15] = 1;
		numdialogues[16] = 2;
		numdialogues[17] = 2;
		numdialogues[18] = 2;
		numdialogues[19] = 2;
		numdialogues[20] = 3;
		numdialogues[21] = 2;
		numdialogues[22] = 2;
		numdialogues[23] = 9;
		numdialogues[24] = 2;
		dialogues = new string * [totaldialogues];
		for (int i = 0; i < totaldialogues; i++)dialogues[i] = new string[numdialogues[i]];
		dialogues[0][0] = "Hey traveller!";
		dialogues[0][1] = "Woah! Who are you?\nHow are you floating???";
		dialogues[0][2] = "I am the ghost of the farmer\nwho used to live here.";
		dialogues[0][3] = "Ggghost??";
		dialogues[0][4] = "Yes!\nI was killed in the goblin invasion...";
		dialogues[0][5] = "How come you're still here then?";
		dialogues[0][6] = "I'm here to save my daughter..\nThey took her away!";
		dialogues[0][7] = "Sounds like someone needs rescuing.\nFollow me let's go!";
		dialogues[0][8] = "Thank you brave traveller...";
		dialogues[1][0] = "West: Village\nEast: Wilderness";
		dialogues[2][0] = "Announcements:\n-> Missing Child Please Help\n-> Beware Goblins";
		dialogues[3][0] = "Tutorial:\nPress E to progress dialogue.";
		dialogues[3][1] = "Press W A S D to move.";
		dialogues[3][2] = "Press E to interact with the World.";
		dialogues[3][3] = "Press Left Click or Spacebar\nto Attack.";
		dialogues[3][4] = "Keep track of your health at the top\nleft of your screen.";
		dialogues[3][5] = "Your attacks have a cooldown based\non the bar above your head.";
		dialogues[3][6] = "Enemies heal if not attacked\nfor a while.";
		dialogues[3][7] = "Wishing at wells may grant\nyou the blessings of the gods.";
		dialogues[3][8] = "You're Ready now!\nGood luck traveller.";
		dialogues[4][0] = "Hole of Doom";
		dialogues[5][0] = "Oh? I found a key.";
		dialogues[5][1] = "This must be useful somewhere...";
		dialogues[6][0] = "Hmm...\nSeems like a locked door.\nNeed to open it somehow.";
		dialogues[6][1] = "Let's look around!";
		dialogues[7][0] = "That key helped open the door! ";
		dialogues[7][1] = "Let's move forward!";
		dialogues[8][0] = "Oh no! We've been trapped.";
		dialogues[8][1] = "This place looks like an arena...";
		dialogues[8][2] = "Better beware goblins\nmoving forward.";
		dialogues[9][0] = "Defeat the current threats\nin order to advance.";
		dialogues[10][0] = "Phew!\nThose goblins sure were tough.";
		dialogues[10][1] = "Hey, this is the door\nthey used to trap us.";
		dialogues[10][2] = "Yeah, what about it?";
		dialogues[10][3] = "It seems to be open.\nSomeone forgot to lock it...";
		dialogues[10][4] = "...";
		dialogues[10][5] = "....";
		dialogues[10][6] = "Well, onwards we go!";
		dialogues[11][0] = "Yet another locked door?";
		dialogues[11][1] = "i'm sure if we explore\nour surroundings we can\nfind a way.";
		dialogues[12][0] = "What's this weird thing?";
		dialogues[12][1] = "It seems like a pressure plate.";
		dialogues[12][2] = "I hear something opening\nin the distance.";
		dialogues[12][3] = "Seems like it won't stay\nopen for long.";
		dialogues[12][4] = "Let's hurry!\nGotta run fast!";
		dialogues[13][0] = "Wow that gave us a run\nfor our money.";
		dialogues[13][1] = "that goblin came outta nowhere.";
		dialogues[13][2] = "Yeah!";
		dialogues[13][3] = "Is it me or is it becoming dark?";
		dialogues[13][4] = "hmm...";
		dialogues[13][5] = "I don't like this.";
		dialogues[13][6] = "We've still got a long\njourney ahead of us.\nLet's move.";
		dialogues[14][0] = "Phew!\nThat was too scary!";
		dialogues[14][1] = "You know, you are a ghost yourself.\nShould't you be the scary one?";
		dialogues[14][2] = "I'm not used to this life.\nOrrr death?...\n";
		dialogues[14][3] = "Anyway, I sense the end of our\njourney is near.\nLet's move.";
		dialogues[15][0] = "You feel a set of eyes on you.\nYou cannot proceed yet.";
		dialogues[16][0] = "You wish in the well.";
		dialogues[17][0] = "You wish in the well.";
		dialogues[17][1] = "The gods rejected your offering.";
		dialogues[18][0] = "The sound of gushing water fills\nthe air.";
		dialogues[18][1] = "You are filled with determination.";
		dialogues[19][0] = "The walls reward those who show\ncourage.";
		dialogues[19][1] = "May the light guide you.";
		dialogues[20][0] = "You've made it really far\ntraveller.";
		dialogues[20][1] = "But to face me, you have to\nface my minions.";
		dialogues[20][2] = "Get ready now.";
		dialogues[21][0] = "Wow so you really killed them.\nImpressive...";
		dialogues[21][1] = "Fine.\nI'll step up now.";
		dialogues[22][0] = "How are you doing this???";
		dialogues[22][1] = "I'LL GET YOU";
		dialogues[23][0] = "Wow, that boss was tough.";
		dialogues[23][1] = "Yeah.\nLet's go find your daughter.";
		dialogues[23][2] = "About that...";
		dialogues[23][3] = "???";
		dialogues[23][4] = "I kinda...\nMay have lied....";
		dialogues[23][5] = "What do you mean?";
		dialogues[23][6] = "Yeah my daughter died 500 \nyears ago. I was just lonely so \nI saw company in you.";
		dialogues[23][7] = "(- _ -)";
		dialogues[23][8] = "Hehehe";
		dialogues[24][0] = "Looks like an old doll.";
		dialogues[24][1] = "Wonder who it belonged to...";
	}
	void Draw(int i, Player& player, Ghost& ghost, Audio& audio)
	{
		window.draw(dialoguebox);
		if (i == 0 || i == 8 || i == 9 || i == 10 || i == 11 || i == 12 || i == 13 || i == 14||i==23)
		{
			if (state % 2 == 0)
			{
				pictexture.loadFromFile("Resources/Graphics/ghost/ghost.png");
				pic.setTextureRect(IntRect(0, 0, 32, 23));
				pic.setScale(5, 5);
				pic.setPosition(105, 725);
			}
			else
			{
				pictexture.loadFromFile("Resources/Graphics/character/front walk.png");
				pic.setTextureRect(IntRect(16, 24, 32, 12));
				pic.setScale(7, 9.6);
				pic.setPosition(80, 725);
			}
		}
		else if (i == 1)
		{
			pictexture.loadFromFile("Resources/Graphics/tileset.png");
			pic.setTextureRect(IntRect(229 % 8 * 32, 229 / 8 * 32, 32, 32));
			pic.setScale(5, 5);
			pic.setPosition(105, 680);
		}
		else if (i == 2)
		{
			pictexture.loadFromFile("Resources/Graphics/tileset.png");
			pic.setTextureRect(IntRect(230 % 8 * 32, 230 / 8 * 32, 64, 64));
			pic.setScale(2.5, 2.5);
			pic.setPosition(105, 680);
		}
		else if (i == 3 || i == 4 || i == 19)
		{
			pictexture.loadFromFile("Resources/Graphics/tileset.png");
			pic.setTextureRect(IntRect(235 % 8 * 32, 235 / 8 * 32, 32, 32));
			pic.setScale(5, 5);
			pic.setPosition(105, 690);
		}
		else if (i == 5 || i == 6 || i == 7||i==24)
		{
			pictexture.loadFromFile("Resources/Graphics/character/front walk.png");
			pic.setTextureRect(IntRect(16, 24, 32, 12));
			pic.setScale(7, 9.6);
			pic.setPosition(80, 725);
		}
		else if (i == 16)
		{
			pictexture.loadFromFile("Resources/Graphics/tileset.png");
			pic.setTextureRect(IntRect(665 % 8 * 32, 665 / 8 * 32, 32, 32));
			pic.setScale(5, 5);
			pic.setPosition(105, 680);
			dialogues[16][1] = "You were blessed with " + to_string(currblessing) + " health.";
		}
		else if (i == 20 || i == 21 || i == 22)
		{
			pictexture.loadFromFile("Resources/Graphics/boss/BossDownRun.png");
			pic.setTextureRect(IntRect(0, 0, 80, 40));
			pic.setScale(3.5, 3.5);
			pic.setPosition(50, 705);
		}
		window.draw(pic);
		Animate(i, dialogues[i][state], audio);
		if ((Keyboard::isKeyPressed(Keyboard::E) && !keyheldE))
		{
			letters = 0;
			keyheldE = true;
			state++;
			if (state >= numdialogues[i])
			{
				state = 0;
				dialogue = false;
				if (i == 0)
				{
					ghost.setfollowing(true);
					if (player.getdirection() == 0) { ghost.setpposy(64); ghost.setppposy(64); }
					else if (player.getdirection() == 1) { ghost.setpposx(-64); ghost.setppposx(-64); }
					else if (player.getdirection() == 2) { ghost.setpposx(64); ghost.setppposx(64); }
					else if (player.getdirection() == 3) { ghost.setpposy(-64); ghost.setppposy(-64); }
					ghost.setlastdir(player.getdirection());
				}
				if (i == 20||i==21||i==22||i==7)gamestate++;
				audio.stopsound("textbox");
			}
			else audio.playsound("textbox");
			if (i == 5)
			{
				keypicked = true;
				arr[7][7] = 0;
			}
			if (i == 10 && state == 3)
			{
				audio.playsound("door");
				arr[16][73] = 550;
				arr[17][73] = 558;
				arr[16][48] = 550;
				arr[17][48] = 558;
			}
			clk1.restart();
		}
	}
	void Animate(int i, string s, Audio& audio)
	{
		if (clk.getElapsedTime() >= Time(milliseconds(50)) && letters < dialogues[i][state].length())
		{
			letters++;
			clk.restart();
		}
		if (letters == dialogues[i][state].length())audio.stopsound("textbox");
		text.setString(dialogues[i][state].substr(0, letters));
		window.draw(text);
	}
	~DialogueBox()
	{
		for (int i = 0; i < totaldialogues; i++)delete[] dialogues[i];
		delete[] dialogues;
		delete[] numdialogues;
	}
	int getstate() { return state; }
};
class Blindness
{
private:
	CircleShape blindness;
	int state = 0;
	Clock clk;
public:
	Blindness()
	{
		blindness.setFillColor(Color(0, 0, 0, 0));
		blindness.setRadius(680);
		blindness.setPosition(-200, -200);
		blindness.setPointCount(360);
	}
	void Draw()
	{
		if (blind == 2 && state == 0)
		{
			blind = 0;
			return;
		}
		if ((blind == 1 && state < 255) || (blind == 2 && state > 0))Animate();
		window.draw(blindness);
	}
	void Animate()
	{
		int time = (blind == 1) ? 100 : 25;
		if (clk.getElapsedTime() >= Time(milliseconds(time)))
		{
			if (blind == 1)state++;
			else state -= 15;
			blindness.setOutlineThickness(-200 - state);
			blindness.setOutlineColor(Color(0, 0, 0, state));
			clk.restart();
		}
	}
};
class TitleScreen
{
private:
	Sprite title, titletext;
	Texture titletexture0, titletexture1, titletexture2, titletexture3, titletexttexture;
	int state, offset, fade;
	Clock clk, clk1;
	Font font;
	Text text;
	bool inc;
public:
	TitleScreen()
	{
		titletexture0.loadFromFile("Resources/Graphics/title/0.jpg");
		titletexture1.loadFromFile("Resources/Graphics/title/1.jpg");
		titletexture2.loadFromFile("Resources/Graphics/title/2.jpg");
		titletexture3.loadFromFile("Resources/Graphics/title/3.jpg");
		titletexttexture.loadFromFile("Resources/Graphics/title/titletext.png");
		titletext.setTexture(titletexttexture);
		titletext.setScale(2, 2);
		titletext.setPosition(80, -60);
		font.loadFromFile("Resources/Fonts/knight.otf");
		text.setFont(font);
		text.setString("Press Enter to Start");
		text.setCharacterSize(64);
		offset = 0;
		state = 0;
		fade = 0;
		title.setTexture(titletexture0);
		title.setScale(1.7, 2.5);
		title.setPosition(0, 0);
		inc = true;
	}
	void Draw()
	{
		Animation();
		window.draw(title);
		window.draw(titletext);
		window.draw(text);
	}
	void Animation()
	{
		if (clk.getElapsedTime() >= Time(milliseconds(200)))
		{
			state++;
			if (state == 0)title.setTexture(titletexture0);
			else if (state == 1)title.setTexture(titletexture1);
			else if (state == 2)title.setTexture(titletexture2);
			else if (state == 3)title.setTexture(titletexture3);
			if (state == 3)state = 0;
			clk.restart();
		}
		if (clk1.getElapsedTime() >= Time(milliseconds(50)))
		{
			if (inc)offset++, fade += 6;
			else offset--, fade -= 6;
			if (offset == 40)inc = false;
			else if (offset == 0)inc = true;
			text.setPosition(240, 700 - offset);
			text.setFillColor(Color(255, 255, 255, fade));
			clk1.restart();
		}
	}
};
class UI
{
private:
	Sprite frame, bar;
	CircleShape death;
	Texture frametexture, bartexture, deathtexture;
	Player* ptr;
	int lasthealth, state;
	bool drained;
	RectangleShape cooldownbar;
	DialogueBox dialoguebox;
	Blindness blindness;
	Text healthtext;
	Clock clk1;
public:
	UI(Player& player)
	{
		ptr = &player;
		lasthealth = ptr->gethealth();
		frametexture.loadFromFile("Resources/Graphics/Healthbar.png");
		bartexture.loadFromFile("Resources/Graphics/Healthbar2.png");
		deathtexture.loadFromFile("Resources/Graphics/death.png");
		frame.setTexture(frametexture);
		bar.setTexture(bartexture);
		death.setTexture(&deathtexture);
		frame.setScale(0.25, 0.25);
		bar.setScale(0.25, 0.25);
		frame.setPosition(64, 64);
		bar.setPosition(64, 64);
		death.setPosition(248, 248);
		healthtext.setPosition(140, 115);
		healthtext.setCharacterSize(10);
		healthtext.setFillColor(Color::Black);
		healthtext.setStyle(Text::Italic);
		healthtext.setStyle(Text::Bold);
		state = 0;
		drained = true;
		cooldownbar.setFillColor(Color(0, 0, 0, 125));
		cooldownbar.setPosition(458, 450);
		healthtext.setFont(font);
	}
	void Draw(Player& player, Clock& clk, Ghost& ghost, Audio& audio)
	{
		if (!player.getattacking())cooldownbar.setSize(Vector2f((((clk.getElapsedTime().asSeconds() >= player.getcooldown()) ? 1 : clk.getElapsedTime().asSeconds() / player.getcooldown())) * 48, 3));
		window.draw(cooldownbar);
		window.draw(frame);
		if (clk1.getElapsedTime() >= Time(milliseconds(10)))
		{
			if (lasthealth != ptr->gethealth())
			{
				bar.setTextureRect(IntRect(0, 0, (lasthealth * 5) - (state * ((lasthealth * 5) - (ptr->gethealth() * 5)) / 100), 280));
				state++;
				drained = false;
				if (state == 100)drained = true;
			}
			if (drained)
			{
				lasthealth = ptr->gethealth();
				state = 0;
			}
			clk1.restart();
		}
		window.draw(bar);
		string temp = to_string(((player.gethealth() < 0) ? 0 : player.gethealth())) + "/" + to_string(player.gettotalhealth());
		if (player.gethealth() < 100)temp = "  " + temp;
		if (player.gethealth() < 10)temp = "  " + temp;
		healthtext.setString(temp);
		window.draw(healthtext);
		if (blind != 0)blindness.Draw();
		if (dialogue)dialoguebox.Draw(currdialogue, player, ghost, audio);
	}
	void DrawDeath(int i)
	{
		death.setPosition(488 - i, 488 - i);
		death.setRadius(i);
		death.setFillColor(Color(0, 0, 0, (i > 255) ? 255 : i));
		window.draw(death);
	}
};
class Board
{
private:
	Sprite map;
	RectangleShape border;
	Texture maptexture;
	UI ui;
public:
	Board(Player& player, const string filename = "Resources/Graphics/tileset.png") :ui(player)
	{
		maptexture.loadFromFile(filename);
		map.setTexture(maptexture);
		map.setScale(tilesize / 32, tilesize / 32);
		border.setFillColor(Color(44, 96, 21, 255));
		border.setSize(Vector2f(tilesize, tilesize));
	}
	void Draw(Player& player, Goblin* goblin, Clock& clk, Ghost& ghost, Boss& boss, Audio& audio, GoblinRider* goblinrider)
	{
		window.clear();
		DrawInner(player, goblin, clk, ghost, boss, audio, goblinrider);
		window.display();
		if (gameover)
		{
			for (int i = 0; i < 8; i++)
			{
				window.clear();
				player[0].setTextureRect(IntRect(i * 64, 0, 64, 64));
				DrawInner(player, goblin, clk, ghost, boss, audio, goblinrider);
				window.display();
				Sleep(100);
			}
			for (int i = 0; i <= 260; i++)
			{
				window.clear();
				DrawInner(player, goblin, clk, ghost, boss, audio, goblinrider);
				ui.DrawDeath(i);
				window.display();
				Sleep(5);
			}
			while (!Mouse::isButtonPressed(Mouse::Left));
			Destroy();
			window.close();
			exit(0);
		}
	}
	void DrawInner(Player& player, Goblin* goblin, Clock& clk, Ghost& ghost, Boss& boss, Audio& audio, GoblinRider* goblinrider)
	{
		for (int i = 0; i < tiles; i++)for (int j = 0; j < tiles; j++)
		{
			int x = player.getcordx() - (tiles / 2) + j;
			int y = player.getcordy() - (tiles / 2) + i;
			map.setPosition(-player.getposx() + j * tilesize, -player.getposy() + i * tilesize);
			if (y >= 7 && y <= 26 && x >= 49 && x <= 72) map.setTextureRect(IntRect(325 % 8 * 32, 325 / 8 * 32, 32, 32));
			else if (y >= 23 && y <= 26 && x >= 9 && x <= 12)
			{
				map.setTextureRect(IntRect(0, 0, 32, 32));
				window.draw(map);
				if (y == 23)
				{
					if (x == 9)map.setTextureRect(IntRect(1064 % 8 * 32, 1064 / 8 * 32, 32, 32));
					else if (x == 10 || x == 11)map.setTextureRect(IntRect(1065 % 8 * 32, 1065 / 8 * 32, 32, 32));
					else map.setTextureRect(IntRect(1066 % 8 * 32, 1066 / 8 * 32, 32, 32));
				}
				else if (y == 24 || y == 25)
				{
					if (x == 9)map.setTextureRect(IntRect(1072 % 8 * 32, 1072 / 8 * 32, 32, 32));
					else if (x == 10 || x == 11)map.setTextureRect(IntRect(1073 % 8 * 32, 1073 / 8 * 32, 32, 32));
					else map.setTextureRect(IntRect(1074 % 8 * 32, 1074 / 8 * 32, 32, 32));
				}
				else
				{
					if (x == 9)map.setTextureRect(IntRect(1080 % 8 * 32, 1080 / 8 * 32, 32, 32));
					else if (x == 10 || x == 11)map.setTextureRect(IntRect(1081 % 8 * 32, 1081 / 8 * 32, 32, 32));
					else map.setTextureRect(IntRect(1082 % 8 * 32, 1082 / 8 * 32, 32, 32));
				}
			}
			else if (((x == 29 || x == 48 || x == 73) && (y == 16 || y == 17)) || ((x == 99 || x == 100) && (y == 51 || y == 52)))map.setTextureRect(IntRect(298 % 8 * 32, 298 / 8 * 32, 32, 32));
			else if (x >= 11 && x <= 35 && y >= 56 && y <= 84)map.setTextureRect(IntRect(312 % 8 * 32, 312 / 8 * 32, 32, 32));
			else map.setTextureRect(IntRect(0, 0, 32, 32));
			window.draw(map);
			if (arr[y][x] != 0)
			{
				map.setTextureRect(IntRect(arr[y][x] % 8 * 32, arr[y][x] / 8 * 32, 32, 32));
				window.draw(map);
			}
		}
		ghost.Draw(player);
		if (combat)
		{
			for (int i = 0; i < numgoblins; i++)
			{
				goblin[i].Draw(player, audio);
				if (goblin[i].getdrawtext())
				{
					text.setPosition(goblin[i].getgposx() - player.getgposx() + 448 + 30, goblin[i].getgposy() - player.getgposy() + 448 - 15);
					window.draw(text);
				}
			}
			for (int i = 0; i < numgoblinriders; i++)
			{
				goblinrider[i].Draw(player, audio);
				if (goblinrider[i].getdrawtext())
				{
					text.setPosition(goblinrider[i].getgposx() - player.getgposx() + 448 + 30, goblinrider[i].getgposy() - player.getgposy() + 448 - 15);
					window.draw(text);
				}
			}
		}
		if (bossroom)
		{
			boss.Draw(player, audio);
			if (boss.getdrawtext())
			{
				text.setPosition(boss.getgposx() - player.getgposx() + 448 + 30, boss.getgposy() - player.getgposy() + 448 - 15);
				window.draw(text);
			}
		}
		player.Draw();
		ui.Draw(player, clk, ghost, audio);
		for (int i = 0; i < tiles; i++)for (int j = 0; j < tiles; j++)
		{
			border.setPosition(j * tilesize, i * tilesize);
			if (i == 0 || i == tiles - 1 || j == 0 || j == tiles - 1)window.draw(border);
		}
	}
};

//Functions
void LoadMap()
{
	string temp;
	ifstream fin("map.csv");
	getline(fin, temp, '\n');
	for (int i = 0; i < mapsize; i++)
	{
		for (int j = 0; j < mapsize; j++)
		{
			if (!fin.eof())
			{
				getline(fin, temp, ',');
				while (temp == "")
					getline(fin, temp, ',');
				arr[i][j] = stoi(temp);
			}
		}
	}
	fin.close();
}
void Initialize()
{
	arr = new int* [mapsize];
	for (int i = 0; i < mapsize; i++)arr[i] = new int[mapsize];
	LoadMap();
	gamestate = 0;
	numgoblins = 4;
	numgoblinriders = 2;
	currblessing = 0;
	currdialogue = 3;
	combat = keyheldW = keyheldA = keyheldS = keyheldD = keyheldenter = keyheldE = locked = false;
	keypicked = false;
	dialogue = true;
	bossroom = false;
	blind = 0;
	font.loadFromFile("Resources/Fonts/montserrat.ttf");
	text.setFont(font);
	text.setCharacterSize(20);
	text.setFillColor(Color::Red);
}
void Destroy()
{
	for (int i = 0; i < mapsize; i++)delete[] arr[i];
	delete[] arr;
}
bool CheckFacing(int i, int j, Player& player)
{
	int playercordx = (player.getgposx() + 32) / tilesize, playercordy = (player.getgposy() + 32) / tilesize;
	int dir = player.getdirection();
	if (playercordx == j - 1 && playercordy == i && dir == 2)return true;
	if (playercordx == j + 1 && playercordy == i && dir == 1)return true;
	if (playercordy == i - 1 && playercordx == j && dir == 0)return true;
	if (playercordy == i + 1 && playercordx == j && dir == 3)return true;
	if (playercordy == i && playercordx == j)return true;
	return false;
}
void CheckDialogue(Player& player, Ghost& ghost, Audio& audio, Board& board)
{
	if (Keyboard::isKeyPressed(Keyboard::E) && !keyheldE && !dialogue)
	{
		keyheldE = true;
		int ghostcordx = ghost.getcordx(), ghostcordy = ghost.getcordy();
		if (CheckFacing(15, 23, player) && !ghost.getfollowing())
		{
			currdialogue = 0;
			dialogue = true;
			audio.playsound("textbox");
			return;
		}
		if (CheckFacing(15, 26, player))
		{
			currdialogue = 1;
			dialogue = true;
			audio.playsound("textbox");
			return;
		}
		if ((CheckFacing(14, 20, player) || CheckFacing(14, 21, player)))
		{
			currdialogue = 2;
			dialogue = true;
			audio.playsound("textbox");
			return;
		}
		if (CheckFacing(15, 18, player))
		{
			currdialogue = 3;
			dialogue = true;
			audio.playsound("textbox");
			return;
		}
		if (CheckFacing(25, 18, player))
		{
			currdialogue = 4;
			dialogue = true;
			audio.playsound("textbox");
			return;
		}
		if (CheckFacing(7, 7, player) && !keypicked)
		{
			currdialogue = 5;
			dialogue = true;
			audio.playsound("textbox");
			return;
		}
		if ((CheckFacing(16, 29, player) || CheckFacing(17, 29, player)) && arr[16][29] == 551)
		{
			if (!keypicked)currdialogue = 6;
			else if (!ghost.getfollowing())currdialogue = 15;
			else
			{
				audio.playsound("door");
				arr[16][29] = 550;
				arr[17][29] = 558;
				currdialogue = 7;
			}
			dialogue = true;
			audio.playsound("textbox");
			return;
		}
		if ((CheckFacing(16, 73, player) || CheckFacing(17, 73, player)) && arr[16][73] == 551)
		{
			if (gamestate == 2)currdialogue = 9;
			else currdialogue = 10;
			dialogue = true;
			audio.playsound("textbox");
			return;
		}
		if ((CheckFacing(51, 99, player) || CheckFacing(51, 100, player)) && arr[51][99] == 551)
		{
			currdialogue = 11;
			dialogue = true;
			audio.playsound("textbox");
			return;
		}
		if (CheckFacing(8, 105, player))
		{
			currdialogue = 12;
			dialogue = true;
			audio.playsound("textbox");
			return;
		}
		if ((CheckFacing(18, 21, player) && !player.gethealsused(0)) || (CheckFacing(15, 83, player) && !player.gethealsused(1)) || (CheckFacing(57, 98, player) && !player.gethealsused(2)) || (CheckFacing(123, 64, player) && !player.gethealsused(3)))
		{
			int temp = rand() % 61 + 20;
			currblessing = temp;
			player + temp;
			currdialogue = 16;
			dialogue = true;
			audio.playsound("textbox");
			if (CheckFacing(18, 21, player))player - 0;
			else if (CheckFacing(15, 83, player))player - 1;
			else if (CheckFacing(57, 98, player))player - 2;
			else if (CheckFacing(123, 64, player))player - 3;
			return;
		}
		if ((CheckFacing(18, 21, player) && player.gethealsused(0)) || (CheckFacing(15, 83, player) && player.gethealsused(1)) || (CheckFacing(57, 98, player) && player.gethealsused(2)) || (CheckFacing(123, 64, player) && player.gethealsused(3)))
		{
			currdialogue = 17;
			dialogue = true;
			audio.playsound("textbox");
			return;
		}
		if (CheckFacing(12, 99, player) || CheckFacing(13, 99, player) || CheckFacing(14, 99, player) || CheckFacing(12, 100, player) || CheckFacing(14, 100, player) || CheckFacing(12, 101, player) || CheckFacing(13, 101, player) || CheckFacing(14, 101, player))
		{
			currdialogue = 18;
			dialogue = true;
			audio.playsound("textbox");
			return;
		}
		if (CheckFacing(83, 97, player))
		{
			currdialogue = 19;
			dialogue = true;
			audio.playsound("textbox");
			return;
		}
		if (CheckFacing(46, 52, player)||CheckFacing(47, 52, player))
		{
			currdialogue = 24;
			dialogue = true;
			audio.playsound("textbox");
			return;
		}
	}
	if (gamestate == 7 && !dialogue)
	{
		int playercordx = (player.getgposx() + 32) / tilesize, playercordy = (player.getgposy() + 32) / tilesize;
		if (playercordx >= 11 && playercordx <= 21 && playercordy >= 56 && playercordy <= 66)
		{
			currdialogue = 20;
			dialogue = true;
			audio.stopsound("footsteps");
			audio.playsound("textbox");
			return;
		}
	}
}
void CheckLocked(Player& player, Audio& audio)
{
	int playercordx = (player.getgposx() + 32) / tilesize, playercordy = (player.getgposy() + 32) / tilesize;
	if (playercordx == 49 && (playercordy == 16 || playercordy == 17) && !locked)
	{
		audio.stopsound("footsteps");
		audio.transition("fight");
		audio.settransitioning(true);
		audio.playsound("textbox");
		locked = true;
		audio.playsound("door");
		arr[16][48] = 551;
		arr[17][48] = 559;
		currdialogue = 8;
		dialogue = true;
		return;
	}
}
void KeyHeldCalc()
{
	if (!Keyboard::isKeyPressed(Keyboard::E)&&keyheldE)
	{
		keyheldE = false;
	}
	if (!Keyboard::isKeyPressed(Keyboard::Enter) && keyheldenter)
	{
		keyheldenter = false;
	}
	if (!Keyboard::isKeyPressed(Keyboard::W) && keyheldW)
	{
		keyheldW = false;
	}
	if (!Keyboard::isKeyPressed(Keyboard::A) && keyheldA)
	{
		keyheldA = false;
	}
	if (!Keyboard::isKeyPressed(Keyboard::S) && keyheldS)
	{
		keyheldS = false;
	}
	if (!Keyboard::isKeyPressed(Keyboard::D) && keyheldD)
	{
		keyheldD = false;
	}
}
void Close()
{
	Event event;
	while (window.pollEvent(event))if (event.type == Event::Closed)
	{
		Destroy();
		window.close();
		exit(0);
	}
}
bool ButtonPress(Player& player, Clock& clk1, Audio& audio)
{
	static bool gatesopen = false;
	int playercordx = (player.getgposx() + 32) / tilesize, playercordy = (player.getgposy() + 32) / tilesize;
	if (playercordy == 8 && playercordx == 105)
	{
		if (!gatesopen)audio.playsound("door");
		arr[51][99] = arr[51][100] = 550;
		arr[52][99] = arr[52][100] = 558;
		gatesopen = true;
		clk1.restart();
	}
	if (!((playercordx == 99 || playercordx == 100) && (playercordy == 51 || playercordy == 52)) && (clk1.getElapsedTime() >= Time(seconds(22)) || combat) && gatesopen)
	{
		audio.playsound("door");
		arr[51][99] = arr[51][100] = 551;
		arr[52][99] = arr[52][100] = 559;
		gatesopen = false;
	}
	return playercordy >= 51;
}
void Maze(Player& player, Audio& audio)
{
	int playercordx = (player.getgposx() + 32) / tilesize, playercordy = (player.getgposy() + 32) / tilesize;
	if (playercordx == 42 && (playercordy == 101 || playercordy == 102))
	{
		blind = 2;
		currdialogue = 14;
		dialogue = true;
		gamestate++;
		audio.transition("peace");
		audio.stopsound("footsteps");
		audio.settransitioning(true);
	}
}
void checkfountain(Player& player, Audio& audio)
{
	int playercordx = (player.getgposx() + 32) / tilesize, playercordy = (player.getgposy() + 32) / tilesize;
	if (playercordx >= 93 && playercordx <= 106 && playercordy >= 7 && playercordy <= 50)audio.playsound("water");
	else audio.stopsound("water");
}
void checkboss(Player& player, Audio& audio)
{
	int playercordx = (player.getgposx() + 32) / tilesize, playercordy = (player.getgposy() + 32) / tilesize;
	if ((playercordx == 11 || playercordx == 12) && playercordy == 85)
	{
		gamestate++;
		audio.transition("fight");
		audio.settransitioning(true);
		bossroom = true;
	}
}
int main()
{
	srand(time(0));
	Initialize();
	Player player;
	Board board(player);
	Clock clk, clk1;
	Goblin* goblins = new Goblin[numgoblins]{ Goblin(9, 67, 15,0) ,Goblin(14, 67, 15, 1), Goblin(19, 67, 15, 0), Goblin(24, 67, 15, 1) };
	GoblinRider* goblinriders = new GoblinRider[numgoblinriders]{ GoblinRider(13, 65, 10), GoblinRider(18, 65, 10) };
	Ghost ghost(15, 23);
	Boss boss(61, 16);
	TitleScreen title;
	Audio audio;
	//Game Loop
	while (window.isOpen())
	{
		if (gamestate == 0)
		{
			window.clear();
			title.Draw();
			window.display();
			if (Keyboard::isKeyPressed(Keyboard::Enter))
			{
				audio.playsound("textbox");
				gamestate++;
			}
		}
		else if (gamestate == 1)
		{
			board.Draw(player, goblins, clk, ghost, boss, audio, goblinriders);
			if (!dialogue)player.Movement(audio);
			player.Attack(clk, goblins, audio, goblinriders,boss);
			CheckDialogue(player, ghost, audio, board);
			KeyHeldCalc();
		}
		else if (gamestate == 2)
		{
			if (!combat)
			{
				for (int i = 0; i < numgoblins; i++)if (abs(goblins[i].getgposx() - player.getgposx()) <= 416 && abs(goblins[i].getgposy() - player.getgposy()) <= 416 && !goblins[i].getdied())combat = true;
				for (int i = 0; i < numgoblinriders; i++)if (abs(goblinriders[i].getgposx() - player.getgposx()) <= 416 && abs(goblinriders[i].getgposy() - player.getgposy()) <= 416 && !goblinriders[i].getdied())combat = true;
			}
			board.Draw(player, goblins, clk, ghost, boss, audio, goblinriders);
			if (!dialogue)player.Movement(audio);
			player.Attack(clk, goblins, audio, goblinriders,boss);
			if (combat)
			{
				bool alldead = true;
				for (int i = 0; i < numgoblins; i++)
				{
					if (!goblins[i].getdied() && !goblins[i].getanimating())
					{
						if (!goblins[i].getanimating())goblins[i].PathFind(player);
						goblins[i].Attack(player, audio);
						goblins[i].Heal();
					}
					if (!(goblins[i].getdied() && !goblins[i].getanimating()))alldead = false;;
				}
				for (int i = 0; i < numgoblinriders; i++)
				{
					if (!goblinriders[i].getdied() && !goblinriders[i].getanimating())
					{
						if (!goblinriders[i].getanimating())goblinriders[i].PathFind(player);
						goblinriders[i].Attack(player, audio);
						goblinriders[i].Heal();
					}
					if (!(goblinriders[i].getdied() && !goblinriders[i].getanimating()))alldead = false;;
				}
				if (alldead)
				{
					combat = false;
					audio.transition("peace");
					audio.settransitioning(true);
					gamestate++;
				}
			}
			CheckLocked(player, audio);
			CheckDialogue(player, ghost, audio, board);
			KeyHeldCalc();
		}
		else if (gamestate == 3)
		{
			delete[] goblinriders;
			numgoblinriders = 1;
			goblinriders = new GoblinRider(47,100,30);
			gamestate++;
		}
		else if (gamestate == 4)
		{
			if (!combat)
			{
				if (abs(goblinriders->getgposx() - player.getgposx()) <= 416 && abs(goblinriders->getgposy() - player.getgposy()) <= 416 && !goblinriders->getdied())combat = true;
			}
			board.Draw(player, goblins, clk, ghost, boss, audio, goblinriders);
			if (!dialogue)player.Movement(audio);
			player.Attack(clk, goblins, audio, goblinriders,boss);
			if (combat)
			{
				bool alldead = true;
				if (!goblinriders->getdied() && !goblinriders->getanimating())
				{
					if (!goblinriders->getanimating())goblinriders->PathFind(player);
					goblinriders->Attack(player, audio);
					goblinriders->Heal();
				}
				if (!(goblinriders->getdied() && !goblinriders->getanimating()))alldead = false;
				if (alldead)combat = false;
			}
			if (ButtonPress(player, clk1, audio))
			{
				gamestate++;
				currdialogue = 13;
				blind = 1;
				dialogue = true;
				audio.stopsound("footsteps");
				audio.transition("maze");
				audio.settransitioning(true);
			}
			CheckDialogue(player, ghost, audio, board);
			KeyHeldCalc();
		}
		else if (gamestate == 5)
		{
			board.Draw(player, goblins, clk, ghost, boss, audio, goblinriders);
			if (!dialogue)player.Movement(audio);
			player.Attack(clk, goblins, audio, goblinriders,boss);
			CheckDialogue(player, ghost, audio, board);
			KeyHeldCalc();
			Maze(player, audio);
		}
		else if (gamestate == 6)
		{
			board.Draw(player, goblins, clk, ghost, boss, audio, goblinriders);
			if (!dialogue)player.Movement(audio);
			player.Attack(clk, goblins, audio, goblinriders,boss);
			CheckDialogue(player, ghost, audio, board);
			KeyHeldCalc();
			checkboss(player, audio);
		}
		else if (gamestate == 7)
		{
			board.Draw(player, goblins, clk, ghost, boss, audio, goblinriders);
			if (!dialogue)player.Movement(audio);
			player.Attack(clk, goblins, audio, goblinriders,boss);
			CheckDialogue(player, ghost, audio, board);
			KeyHeldCalc();
		}
		else if (gamestate == 8)
		{
			delete[] goblins;
			numgoblins = 5;
			goblins = new Goblin[numgoblins]{ Goblin(59, 15, 5) ,Goblin(60, 19, 5),Goblin(61, 15, 5),Goblin(62, 19, 5),Goblin(63, 15, 5) };
			delete goblinriders;
			numgoblinriders = 5;
			goblinriders = new GoblinRider[numgoblinriders]{ GoblinRider(59, 19, 5) ,GoblinRider(60, 15, 5),GoblinRider(61, 19, 5),GoblinRider(62, 15, 5),GoblinRider(63, 19, 5) };
			gamestate++;
		}
		else if (gamestate == 9)
		{
			if (!combat)
			{
				for (int i = 0; i < numgoblins; i++)if (abs(goblins[i].getgposx() - player.getgposx()) <= 416 && abs(goblins[i].getgposy() - player.getgposy()) <= 416 && !goblins[i].getdied())combat = true;
				for (int i = 0; i < numgoblinriders; i++)if (abs(goblinriders[i].getgposx() - player.getgposx()) <= 416 && abs(goblinriders[i].getgposy() - player.getgposy()) <= 416 && !goblinriders[i].getdied())combat = true;
			}
			board.Draw(player, goblins, clk, ghost, boss, audio, goblinriders);
			if (!dialogue)player.Movement(audio);
			player.Attack(clk, goblins, audio, goblinriders,boss);
			if (combat)
			{
				bool alldead = true;
				for (int i = 0; i < numgoblins; i++)
				{
					if (!goblins[i].getdied() && !goblins[i].getanimating())
					{
						if (!goblins[i].getanimating())goblins[i].PathFind(player);
						goblins[i].Attack(player, audio);
						goblins[i].Heal();
					}
					if (!(goblins[i].getdied() && !goblins[i].getanimating()))alldead = false;
				}
				for (int i = 0; i < numgoblinriders; i++)
				{
					if (!goblinriders[i].getdied() && !goblinriders[i].getanimating())
					{
						if (!goblinriders[i].getanimating())goblinriders[i].PathFind(player);
						goblinriders[i].Attack(player, audio);
						goblinriders[i].Heal();
					}
					if (!(goblinriders[i].getdied() && !goblinriders[i].getanimating()))alldead = false;
				}
				if (alldead)
				{
					combat = false;
					currdialogue = 21;
					dialogue = true;
					audio.playsound("textbox");
				}
			}
			CheckLocked(player, audio);
			CheckDialogue(player, ghost, audio, board);
			KeyHeldCalc();
		}
		else if (gamestate == 10)
		{
			gamestate++;
			boss.setphase(1);
		}
		else if (gamestate == 11)
		{
			board.Draw(player, goblins, clk, ghost, boss, audio, goblinriders);
			if (!dialogue)player.Movement(audio);
			player.Attack(clk, goblins, audio, goblinriders,boss);
			if (!boss.getdied() && !boss.getanimating())
			{
				if (!boss.getanimating())boss.PathFind(player,audio);
				boss.Attack(player, audio);
			}
			CheckDialogue(player, ghost, audio, board);
			KeyHeldCalc();
		}
		else if (gamestate == 12)
		{
			board.Draw(player, goblins, clk, ghost, boss, audio, goblinriders);
			if (!dialogue)player.Movement(audio);
			player.Attack(clk, goblins, audio, goblinriders, boss);
			CheckDialogue(player, ghost, audio, board);
			KeyHeldCalc();
		}
		else if (gamestate == 13)
		{
			board.Draw(player, goblins, clk, ghost, boss, audio, goblinriders);
			if (!dialogue)player.Movement(audio);
			player.Attack(clk, goblins, audio, goblinriders, boss);
			if (!boss.getdied() && !boss.getanimating())
			{
				if (!boss.getanimating())boss.PathFind(player, audio);
				boss.Attack(player, audio);
			}
			if (boss.getdied() && !boss.getanimating())
			{
				dialogue = true;
				audio.transition("peace");
				audio.settransitioning(true);
				currdialogue = 23;
				gamestate++;
			}
			CheckDialogue(player, ghost, audio, board);
			KeyHeldCalc();
		}
		else if (gamestate == 14)
		{
			board.Draw(player, goblins, clk, ghost, boss, audio, goblinriders);
			if (!dialogue)player.Movement(audio);
			player.Attack(clk, goblins, audio, goblinriders, boss);
			CheckDialogue(player, ghost, audio, board);
			KeyHeldCalc();
		}
		if (audio.gettransitioning() && !gameover)audio.transition();
		checkfountain(player, audio);
		Close();
	}
	return 0;
}

//Member Functions
void Player::Attack(Clock& clk, Goblin* goblin, Audio& audio, GoblinRider* goblinrider,Boss& boss)
{
	if ((Mouse::isButtonPressed(Mouse::Left) || Keyboard::isKeyPressed(Keyboard::Space)) && !attacking && clk.getElapsedTime() >= Time(seconds(cooldown)))
	{
		attacking = true;
		state = 0;
		for (int i = 0; i < numgoblins; i++)if (abs(goblin[i].getgposx() - gposx) <= 40 && abs(goblin[i].getgposy() - gposy) <= 40)
		{
			audio.playsound("enemyhurt");
			goblin[i].Hurt(*this);
		}
		for (int i = 0; i < numgoblinriders; i++)if (abs(goblinrider[i].getgposx() - gposx) <= 40 && abs(goblinrider[i].getgposy() - gposy) <= 40)
		{
			audio.playsound("enemyhurt");
			goblinrider[i].Hurt(*this);
		}
		if (abs(boss.getgposx() - gposx) <= 40 && abs(boss.getgposy() - gposy) <= 40&&(gamestate==11||gamestate==13))
		{
			audio.playsound("enemyhurt");
			boss.Hurt(*this,audio);
		}
		audio.playsound("attack");
		clk.restart();
	}
	if (attacking)
	{
		if (clk.getElapsedTime() >= Time(milliseconds(50)))
		{
			state++;
			if (state > 6)
			{
				state = 0;
				attacking = false;
			}
			clk.restart();
		}
	}
}
