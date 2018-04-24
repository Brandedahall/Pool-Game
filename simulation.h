/*-----------------------------------------------------------
  Simulation Header File
  -----------------------------------------------------------*/
#include"vecmath.h"
#include <iostream>

/*-----------------------------------------------------------
  Macros
  -----------------------------------------------------------*/
#define TABLE_X (0.6f)
#define TABLE_Z (1.2f)
#define TABLE_Y (0.1f)
#define BALL_RADIUS (0.02f)
#define BALL_MASS (0.1f)
#define TWO_PI (6.2832f)
#define SIM_UPDATE_MS (10)
#define NUM_BALLS (1)
#define POCKET_RADIUS (0.05f)
#define NUM_CUSHIONS (11)
#define NUM_POCKETS (4)
#define MAX_PARTICLES (200)
#define NUM_PLAYERS (2)
/*-----------------------------------------------------------
  plane normals
  -----------------------------------------------------------*/
/*
extern vec2	gPlaneNormal_Left;
extern vec2	gPlaneNormal_Top;
extern vec2	gPlaneNormal_Right;
extern vec2	gPlaneNormal_Bottom;
*/

/*-----------------------------------------------------------
Player class
-----------------------------------------------------------*/

class Player
{
public:
	Player(); // Default constructor
	Player(int Score, bool Playerturn);

	// Gets and sets

	void SetScore(int Score);
	int GetScore();

	void SetTurn(bool Playerturn);
	bool GetTurn();  

	void CreatePlayer(bool turn); 

	void switchplayer();
	void ResetScore();

	bool GetPottedBall();
	void SetPottedBall(bool Potted);

	~Player();

	//Private variables which are only accessable through the gets and sets above.
private:
	int score;
	int balls_lost;
	bool playerturn;
	bool PottedBall;
};


/*-----------------------------------------------------------
  cushion class
  -----------------------------------------------------------*/
class cushion
{
public:
	vec2	vertices[2]; //2d
	vec2	centre;
	vec2	normal;

	void MakeNormal(void);
	void MakeCentre(void);

};

/*-----------------------------------------------------------
pocket class
-----------------------------------------------------------*/

// Contains the position and relavent information about the hole which the golf-ball collides with.
class Pocket
{
public:
	vec2 vertice;
	vec2 position;
	vec2 centre;
	vec2 normal;
};


/*-----------------------------------------------------------
  ball class
  -----------------------------------------------------------*/

class ball
{
	static int ballIndexCnt;
public:
	vec2	position;
	bool	ispocketed;
	vec2	velocity;
	float	radius;
	float	mass;
	int		index;
	

	ball(): position(0.0), ispocketed(false), velocity(0.0), radius(BALL_RADIUS), 
		mass(BALL_MASS) {index = ballIndexCnt++; Reset();}
	
	void Reset(void);
	void ApplyImpulse(vec2 imp);
	void ApplyFrictionForce(int ms);
	void DoPlaneCollision(const cushion &c);
	void Update(int ms);
	
	bool HasHitPlane(const cushion &c) const;
	bool HasHitBall(const ball &b) const;

	void DoPocketCollision(const Pocket &p);
	void HitPocket(const Pocket &c);
	bool HasHitPocket(const Pocket &p) const;

	void HitPlane(const cushion &c);
	void HitBall(ball &b);
};


/*-----------------------------------------------------------
particle classes
-----------------------------------------------------------*/
class particle 
{
public:
	vec3 position;
	vec3 velocity;

	particle() {position=0;velocity=0;}
	void update(int ms);
};

class particleSet 
{
public:
	particle *particles[MAX_PARTICLES];
	int num;

	particleSet()
	{
		for(int i=0;i<MAX_PARTICLES;i++) particles[i] = 0;
		num=0;
	}

	~particleSet()
	{
		for(int i=0;i<MAX_PARTICLES;i++)
		{
			if(particles[i]) delete particles[i];
		}
	}

	void AddParticle(const vec3 &pos);
	void update(int ms);
};


/*-----------------------------------------------------------
  table class
  -----------------------------------------------------------*/
class table
{
public:
	ball balls[NUM_BALLS];	
	cushion cushions[NUM_CUSHIONS];
	Pocket pockets[NUM_POCKETS];
	Player Players[NUM_PLAYERS];
	particleSet parts;
	int level = 1;

	void SetupPlayfield();
	void Changelevel(void);
	void UpdateLevel(void);

	void SetupPockets(void);
	void SetupPlayers(void);
	void Update(int ms);

	bool AnyBallsMoving(void) const;
};

/*-----------------------------------------------------------
  global table
  -----------------------------------------------------------*/
extern table gTable;