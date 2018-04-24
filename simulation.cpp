/*-----------------------------------------------------------
  Simulation Source File
  -----------------------------------------------------------*/
#include"stdafx.h"
#include"simulation.h"
#include<stdio.h>
#include<stdlib.h>
#include<glut.h>


  /*-----------------------------------------------------------
	macros
	-----------------------------------------------------------*/
float SMALL_VELOCITY(0.01f);

/*-----------------------------------------------------------
  globals
  -----------------------------------------------------------*/
  /*
  vec2	gPlaneNormal_Left(1.0,0.0);
  vec2	gPlaneNormal_Top(0.0,1.0);
  vec2	gPlaneNormal_Right(-1.0,0.0);
  vec2	gPlaneNormal_Bottom(0.0,-1.0);
  */

table gTable;

static const float gRackPositionX[] = { 0.0f,0.0f,(BALL_RADIUS*2.0f),(-BALL_RADIUS*2.0f),(BALL_RADIUS*4.0f) };
static const float gRackPositionZ[] = { 0.0f,0.0f,(-BALL_RADIUS*3.0f),(-BALL_RADIUS*3.0f) };

float gCoeffRestitution = 0.5f;
float gCoeffFriction = 0.03f;
float gGravityAccn = 9.8f;




/*-----------------------------------------------------------
Player class members
-----------------------------------------------------------*/
//Player default constructor
Player::Player()
{
}

Player::Player(int Score, bool Playerturn) { score = Score; playerturn = Playerturn; }

//Sets the player score when called with a parameter. Usually used when a player has potted the golf ball.
void Player::SetScore(int Score)
{
	score = score + Score;
}

//This resets the player's score to 0. Used when one of the players press 'esc'.
void Player::ResetScore()
{
	score = 0;
}

//Gets the score of the player. Returns an int.
int Player::GetScore() { return score; }

//This sets whether it is the current player's turn. Due to having two players in the game, only one will have this set to true at any one time.
void Player::SetTurn(bool Playerturn)
{
	playerturn = Playerturn;
}

// This is called to check whether it is the player's turn. Returns a bool.
bool Player::GetTurn()
{
	if (playerturn == true) { return true; }
	else { return false; }
}

//Creates a specfic player, with scores and whether it is their turn.
void Player::CreatePlayer(bool turn)
{
	Player player(0, turn);
}

//Checks whether it is a specific player's turn. If it is, set that player's turn to false. Basically reverses which player is currently playing.
void Player::switchplayer()
{
	if (playerturn == true)
	{
		playerturn = false;
	}
	else if (playerturn == false)
	{
		playerturn = true;
	}

}

//Returns whether a player's golf-ball has been potted already. Returns a boolean value.
bool Player::GetPottedBall()
{
	return PottedBall;
}

//This is used to set a specific player's ball to 'potted'. This means that when both player's balls are potted, the level is changed.
void Player::SetPottedBall(bool Potted)
{
	PottedBall = Potted;
}

//Deconstructor for the player class.
Player::~Player()
{

}

/*-----------------------------------------------------------
  cushion class members
  -----------------------------------------------------------*/
void cushion::MakeNormal(void)
{
	//can do this in 2d
	vec2 temp = vertices[1] - vertices[0];
	normal(0) = temp(1);
	normal(1) = -temp(0);
	normal.Normalise();
}

void cushion::MakeCentre(void)
{
	centre = vertices[0];
	centre += vertices[1];
	centre /= 2.0;
}

/*-----------------------------------------------------------
  ball class members
  -----------------------------------------------------------*/
int ball::ballIndexCnt = 0;

void ball::Reset(void)
{
	//set velocity to zero
	velocity = 0.0;

	//work out rack position
	if (index == 0)
	{
		position(1) = 1;
		position(0) = 0;
		return;
	}

	static const float sep = (BALL_RADIUS*3.0f);
	static const float rowSep = (BALL_RADIUS*2.5f);
	int row = 1;
	int rowIndex = index;
	while (rowIndex > row)
	{
		rowIndex -= row;
		row++;
	}
	position(1) = -(rowSep * (row - 1));
	position(0) = (((row - 1)*sep) / 2.0f) - (sep*(row - rowIndex));
}

void ball::ApplyImpulse(vec2 imp)
{
	velocity = imp;
}

void ball::ApplyFrictionForce(int ms)
{
	if (velocity.Magnitude() <= 0.0) return;

	//accelaration is opposite to direction of motion
	vec2 accelaration = -velocity.Normalised();
	//friction force = constant * mg
	//F=Ma, so accelaration = force/mass = constant*g
	accelaration *= (gCoeffFriction * gGravityAccn);
	//integrate velocity : find change in velocity
	vec2 velocityChange = ((accelaration * ms) / 1000.0f);
	//cap magnitude of change in velocity to remove integration errors
	if (velocityChange.Magnitude() > velocity.Magnitude()) velocity = 0.0;
	else velocity += velocityChange;
}


//void ball::DoBallCollision(ball &b)
//{
//	for (int i = 0; i < NUM_BALLS; i++)
//	{
//		if (!gTable.balls[i].ispocketed)
//		{
//			if (HasHitBall(b)) HitBall(b);
//		}
//	}
//}

//Checks whether the golf-ball has collided with the hole. If it has, call HitPocket().
void ball::DoPocketCollision(const Pocket &p)
{
	for (int i = 0; i < NUM_BALLS; i++)
	{
		if (!gTable.balls[i].ispocketed)
		{
			if (HasHitPocket(p)) HitPocket(p);
		}
	}
}
//This checks whether the golf-ball has collided with any of the walls of the level.
void ball::DoPlaneCollision(const cushion &b)
{
	for (int i = 0; i < NUM_BALLS; i++)
	{
		if (!gTable.balls[i].ispocketed)
		{
			if (HasHitPlane(b)) HitPlane(b);
		}
	}
}

void ball::Update(int ms)
{
	//apply friction
	ApplyFrictionForce(ms);
	//integrate position
	position += ((velocity * ms) / 1000.0f);
	//set small velocities to zero
	if (velocity.Magnitude() < SMALL_VELOCITY) velocity = 0.0;

	if (gTable.balls[0].ispocketed)
	{
		gTable.balls[0].position(0) = 0;
		gTable.balls[0].position(1) = 1;
		ball::ispocketed = false;
		velocity = 0.0;
	}
}


bool ball::HasHitPlane(const cushion &c) const
{
	//if moving away from plane, cannot hit
	if (velocity.Dot(c.normal) >= 0.0) return false;

	//if in front of plane, then have not hit
	vec2 relPos = position - c.vertices[0];
	double sep = relPos.Dot(c.normal);

	vec2 Pc = (position - (position - c.vertices[0]).Dot(c.normal));

	vec2 p1 = Pc - c.vertices[1];
	vec2 p2 = Pc - c.vertices[0];

	vec2 NormalP1 = p1.Normalised();
	vec2 NormalP2 = p2.Normalised();

	double DotP1P2 = NormalP1.Dot(NormalP2);


	// If the distance between the ball and the wall is greater than the radius of the ball.
	if (sep > radius) 
	{
		return false;
	}
	//If the distance becomes less than the radius of the ball
	else if (sep < radius)
	{
		//If the dot product of P1 and P2 is greater than 0, the ball is not about to hit the wall because it isn't in front of the wall.
		if (DotP1P2 > 0)
		{
			return false;
		}
		//The ball has collided with the wall.
		else if (DotP1P2 < 0)
		{
			return true;
		}
	}
	return false;
}

bool ball::HasHitBall(const ball &b) const
{
	//work out relative position of ball from other ball,
	//distance between balls
	//and relative velocity
	vec2 relPosn = position - b.position;
	float dist = (float)relPosn.Magnitude();
	vec2 relPosnNorm = relPosn.Normalised();
	vec2 relVelocity = velocity - b.velocity;

	//if moving apart, cannot have hit
	if (relVelocity.Dot(relPosnNorm) >= 0.0) return false;
	//if distnce is more than sum of radii, have not hit
	if (dist > (radius + b.radius)) return false;
	return true;
}

void ball::HitPlane(const cushion &c)
{
	//reverse velocity component perpendicular to plane  
	double comp = velocity.Dot(c.normal) * (1.0 + gCoeffRestitution);
	vec2 delta = -(c.normal * comp);
	velocity += delta;

	//make some particles
	int n = (rand() % 4) + 3;
	vec3 pos(position(0), radius / 2.0, position(1));
	vec3 oset(c.normal(0), 0.0, c.normal(1));
	pos += (oset*radius);
	for (int i = 0; i < n; i++)
	{
		gTable.parts.AddParticle(pos);
	}
}
//Checks whether the golf-ball has collided with the pocket. It checks whether the ball radius has intersected with the radius of the pocket. 
bool ball::HasHitPocket(const Pocket &p) const
{
	//TODO
	//if in pocket, then have not hit
	double sep = sqrt(pow(position(0) - p.vertice(0), 2) + pow(position(1) - p.vertice(1), 2));
	if (sep > POCKET_RADIUS) return false;
	return true;
}

void ball::HitPocket(const Pocket &c)
{
	//Sets the current balls 'ispocketed' variable to true. so that whoever's ball has just been pocketed doesnt interact with the game field.
	ball::ispocketed = true;

	//make some particles
	int n = (rand() % 4) + 3;
	vec3 pos(position(0), radius / 2.0, position(1));
	vec3 oset(c.normal(0), 0.0, c.normal(1));
	pos += (oset*radius);
	for (int i = 0; i < n; i++)
	{
		gTable.parts.AddParticle(pos);
	}


	//If it's player 1's turn, it sets 'setpottedball' for them to true, so they do not keep gaining points. Then it switches the player's turns around, so it's player 2's turn. This is done every time a golf-ball has been pocketed.
	if (gTable.Players[0].GetTurn())
	{
		gTable.Players[0].SetPottedBall(true);
		gTable.Players[0].SetScore(-1);
		gTable.Players[0].switchplayer();
		gTable.Players[1].switchplayer();
	}
	else if (gTable.Players[1].GetTurn())
	{
		gTable.Players[1].SetPottedBall(true);
		gTable.Players[1].SetScore(-1);
		gTable.Players[1].switchplayer();
		gTable.Players[0].switchplayer();
	}
}

void ball::HitBall(ball &b)
{
	//find direction from other ball to this ball
	vec2 relDir = (position - b.position).Normalised();

	//split velocities into 2 parts:  one component perpendicular, and one parallel to 
	//the collision plane, for both balls
	//(NB the collision plane is defined by the point of contact and the contact normal)
	float perpV = (float)velocity.Dot(relDir);
	float perpV2 = (float)b.velocity.Dot(relDir);
	vec2 parallelV = velocity - (relDir*perpV);
	vec2 parallelV2 = b.velocity - (relDir*perpV2);

	//Calculate new perpendicluar components:
	//v1 = (2*m2 / m1+m2)*u2 + ((m1 - m2)/(m1+m2))*u1;
	//v2 = (2*m1 / m1+m2)*u1 + ((m2 - m1)/(m1+m2))*u2;
	float sumMass = mass + b.mass;
	float perpVNew = (float)((perpV*(mass - b.mass)) / sumMass) + (float)((perpV2*(2.0*b.mass)) / sumMass);
	float perpVNew2 = (float)((perpV2*(b.mass - mass)) / sumMass) + (float)((perpV*(2.0*mass)) / sumMass);

	//find new velocities by adding unchanged parallel component to new perpendicluar component
	velocity = parallelV + (relDir*perpVNew);
	b.velocity = parallelV2 + (relDir*perpVNew2);


	//make some particles
	int n = (rand() % 5) + 5;
	vec3 pos(position(0), radius / 2.0, position(1));
	vec3 oset(relDir(0), 0.0, relDir(1));
	pos += (oset*radius);
	for (int i = 0; i < n; i++)
	{
		gTable.parts.AddParticle(pos);
	}
}

/*-----------------------------------------------------------
  particle class members
  -----------------------------------------------------------*/
void particle::update(int ms)
{
	position += (velocity*ms) / 1000.0;
	velocity(1) -= (4.0*ms) / 1000.0; //(9.8*ms)/1000.0;
}

/*-----------------------------------------------------------
  particle set class members
  -----------------------------------------------------------*/
void particleSet::AddParticle(const vec3 &pos)
{
	if (num >= MAX_PARTICLES) return;
	particles[num] = new particle;
	particles[num]->position = pos;

	particles[num]->velocity(0) = ((rand() % 200) - 100) / 200.0;
	particles[num]->velocity(2) = ((rand() % 200) - 100) / 200.0;
	particles[num]->velocity(1) = 2.0*((rand() % 100) / 100.0);

	num++;
}

void particleSet::update(int ms)
{
	int i = 0;
	while (i < num)
	{
		particles[i]->update(ms);
		if ((particles[i]->position(1) < 0.0) && (particles[i]->velocity(1) < 0.0))
		{
			delete particles[i];
			particles[i] = particles[num - 1];
			num--;
		}
		else i++;
	}
}

/*-----------------------------------------------------------
  table class members
  -----------------------------------------------------------*/
//This sets up level 1 at the start of the game. Each 'cushion' has two points; their positions are dictated by x and y coordinates relative to the game field.
void table::SetupPlayfield()
{
#pragma region MyRegion

	for (int i = 0; i < NUM_CUSHIONS; i++)
	{
		cushions[i].vertices[0](0) = NULL;
		cushions[i].vertices[0](1) = NULL;
		cushions[i].vertices[1](0) = NULL;
		cushions[i].vertices[1](1) = NULL;
	}
	//Null the pockets, so they can be redrawn in a different place.
	pockets[0].vertice(0) = NULL;
	pockets[0].vertice(1) = NULL;
#pragma endregion

	cushions[0].vertices[0](0) = -TABLE_X + 0.45;
	cushions[0].vertices[0](1) = TABLE_Z;
	cushions[0].vertices[1](0) = TABLE_X - 0.445;
	cushions[0].vertices[1](1) = TABLE_Z;

	cushions[1].vertices[0](0) = TABLE_X;
	cushions[1].vertices[0](1) = TABLE_Z - 0.9;
	cushions[1].vertices[1](0) = TABLE_X;
	cushions[1].vertices[1](1) = -TABLE_Z + 0.8;

	cushions[2].vertices[0](0) = TABLE_X;
	cushions[2].vertices[0](1) = -TABLE_Z + 0.8;
	cushions[2].vertices[1](0) = -TABLE_X + 0.6;
	cushions[2].vertices[1](1) = -TABLE_Z + 0.8;

	cushions[3].vertices[1](0) = TABLE_X;
	cushions[3].vertices[1](1) = -TABLE_Z + 1.5;
	cushions[3].vertices[0](0) = -TABLE_X + 0.75;
	cushions[3].vertices[0](1) = -TABLE_Z + 1.5;

	cushions[4].vertices[1](0) = -TABLE_X + 0.45;
	cushions[4].vertices[1](1) = -TABLE_Z + 1.1;
	cushions[4].vertices[0](0) = -TABLE_X + 0.6;
	cushions[4].vertices[0](1) = -TABLE_Z + 0.8;

	cushions[5].vertices[0](0) = TABLE_X - 0.444;
	cushions[5].vertices[0](1) = -TABLE_Z + 2.4;
	cushions[5].vertices[1](0) = -TABLE_X + 0.75;
	cushions[5].vertices[1](1) = -TABLE_Z + 1.5;

	cushions[6].vertices[1](0) = TABLE_X - 0.75;
	cushions[6].vertices[1](1) = -TABLE_Z + 2.4;
	cushions[6].vertices[0](0) = -TABLE_X + 0.45;
	cushions[6].vertices[0](1) = -TABLE_Z + 1.1;

	pockets[0].vertice(0) = -TABLE_X + 1.1;
	pockets[0].vertice(1) = -TABLE_Z + 1.1;


	for (int i = 0; i < NUM_CUSHIONS; i++)
	{
		cushions[i].MakeCentre();
		cushions[i].MakeNormal();
	}
}

//Sets player 1 to play first at the start of the game.
void table::SetupPlayers(void)
{
	Players[0].SetTurn(true);

	Players[1].SetTurn(false);
}

//Increments the current level, then resets the turn, so player 1 starts. This also resets whether the balls have been potted.
void table::Changelevel(void)
{
	table::level++;

	Players[0].SetPottedBall(false);
	Players[0].SetTurn(true);

	Players[1].SetPottedBall(false);
	Players[1].SetTurn(false);
}

//This contains and subsequently redraws each level when the current level has been completed. There are three levels contained within, all of them work the same as the first level.
void table::UpdateLevel()
{
	if (gTable.level == 2)
	{
		////level 2
		//NULL all of the cushions, so they can be redrawn in a different place.
#pragma region MyRegion

		for (int i = 0; i < NUM_CUSHIONS; i++)
		{
			cushions[i].vertices[0](0) = NULL;
			cushions[i].vertices[0](1) = NULL;
			cushions[i].vertices[1](0) = NULL;
			cushions[i].vertices[1](1) = NULL;
		}
		//Null the pockets, so they can be redrawn in a different place.
		pockets[0].vertice(0) = NULL;
		pockets[0].vertice(1) = NULL;
#pragma endregion

		cushions[0].vertices[0](0) = -TABLE_X + 0.45;
		cushions[0].vertices[0](1) = TABLE_Z;
		cushions[0].vertices[1](0) = TABLE_X - 0.445;
		cushions[0].vertices[1](1) = TABLE_Z;

		cushions[1].vertices[0](0) = TABLE_X - 0.444;
		cushions[1].vertices[0](1) = -TABLE_Z + 2.4;
		cushions[1].vertices[1](0) = -TABLE_X + 0.75;
		cushions[1].vertices[1](1) = -TABLE_Z + 1.5;

		cushions[2].vertices[1](0) = TABLE_X - 0.2;
		cushions[2].vertices[1](1) = -TABLE_Z + 2.4;
		cushions[2].vertices[0](0) = -TABLE_X + 0.75;
		cushions[2].vertices[0](1) = -TABLE_Z + 1.5;

		cushions[3].vertices[0](0) = TABLE_X - 0.2;
		cushions[3].vertices[0](1) = -TABLE_Z + 2.4;
		cushions[3].vertices[1](0) = -TABLE_X + 1.5;
		cushions[3].vertices[1](1) = -TABLE_Z + 1.5;

		cushions[4].vertices[1](0) = TABLE_X - 0.75;
		cushions[4].vertices[1](1) = -TABLE_Z + 2.4;
		cushions[4].vertices[0](0) = -TABLE_X + 0.4;
		cushions[4].vertices[0](1) = -TABLE_Z + 1;

		cushions[5].vertices[1](0) = -TABLE_X + 0.4;
		cushions[5].vertices[1](1) = -TABLE_Z + 1;
		cushions[5].vertices[0](0) = -TABLE_X + 1.1;
		cushions[5].vertices[0](1) = -TABLE_Z + 0.8;

		cushions[6].vertices[1](0) = -TABLE_X + 1.1;
		cushions[6].vertices[1](1) = -TABLE_Z + 0.8;
		cushions[6].vertices[0](0) = -TABLE_X + 1.5;
		cushions[6].vertices[0](1) = -TABLE_Z + 1.5;

	pockets[0].vertice(0) = -TABLE_X + 1.35;
	pockets[0].vertice(1) = -TABLE_Z + 1.7;


		for (int i = 0; i < NUM_CUSHIONS; i++)
		{
			cushions[i].MakeCentre();
			cushions[i].MakeNormal();
		}
	}

	if (gTable.level == 3)
	{
		//// level 3
		//NULL all of the cushions, so they can be redraw in a different place.
#pragma region MyRegion

		for (int i = 0; i < NUM_CUSHIONS; i++)
		{
			cushions[i].vertices[0](0) = NULL;
			cushions[i].vertices[0](1) = NULL;
			cushions[i].vertices[1](0) = NULL;
			cushions[i].vertices[1](1) = NULL;
		}
		//Null the pockets, so they can be redrawn in a different place.
		pockets[0].vertice(0) = NULL;
		pockets[0].vertice(1) = NULL;
#pragma endregion

		cushions[0].vertices[0](0) = -TABLE_X + 0.45;
		cushions[0].vertices[0](1) = TABLE_Z;
		cushions[0].vertices[1](0) = TABLE_X - 0.445;
		cushions[0].vertices[1](1) = TABLE_Z;

		cushions[1].vertices[1](0) = TABLE_X;
		cushions[1].vertices[1](1) = TABLE_Z - 0.4;
		cushions[1].vertices[0](0) = TABLE_X;
		cushions[1].vertices[0](1) = -TABLE_Z + 1.5; //

		cushions[2].vertices[0](0) = TABLE_X;
		cushions[2].vertices[0](1) = -TABLE_Z + 0.2;
		cushions[2].vertices[1](0) = -TABLE_X + 0.6;
		cushions[2].vertices[1](1) = -TABLE_Z + 0.8;

		cushions[3].vertices[1](0) = TABLE_X;
		cushions[3].vertices[1](1) = -TABLE_Z + 1.5;
		cushions[3].vertices[0](0) = -TABLE_X + 0.75;
		cushions[3].vertices[0](1) = -TABLE_Z + 1.5;

		cushions[4].vertices[1](0) = -TABLE_X + 0.45;
		cushions[4].vertices[1](1) = -TABLE_Z + 1.1;
		cushions[4].vertices[0](0) = -TABLE_X + 0.6;
		cushions[4].vertices[0](1) = -TABLE_Z + 0.8;

		cushions[5].vertices[0](0) = TABLE_X - 0.444;
		cushions[5].vertices[0](1) = -TABLE_Z + 2.4;
		cushions[5].vertices[1](0) = -TABLE_X + 0.75;
		cushions[5].vertices[1](1) = -TABLE_Z + 1.5;

		cushions[6].vertices[1](0) = TABLE_X - 0.75;
		cushions[6].vertices[1](1) = -TABLE_Z + 2.4;
		cushions[6].vertices[0](0) = -TABLE_X + 0.45;
		cushions[6].vertices[0](1) = -TABLE_Z + 1.1;

		cushions[7].vertices[1](0) = TABLE_X;
		cushions[7].vertices[1](1) = -TABLE_Z + 0.2;
		cushions[7].vertices[0](0) = -TABLE_X + 1.5;
		cushions[7].vertices[0](1) = -TABLE_Z + 0.2;

		cushions[8].vertices[0](0) = TABLE_X + 0.3;
		cushions[8].vertices[0](1) = -TABLE_Z + 1.95;
		cushions[8].vertices[1](0) = -TABLE_X + 1.5;
		cushions[8].vertices[1](1) = -TABLE_Z + 0.2;

		cushions[9].vertices[1](0) = TABLE_X + 0.3;
		cushions[9].vertices[1](1) = -TABLE_Z + 1.95;
		cushions[9].vertices[0](0) = -TABLE_X + 1.2;
		cushions[9].vertices[0](1) = -TABLE_Z + 2;

		pockets[0].vertice(0) = -TABLE_X + 1.35;
		pockets[0].vertice(1) = -TABLE_Z + 1.7;

		for (int i = 0; i < NUM_CUSHIONS; i++)
		{
			cushions[i].MakeCentre();
			cushions[i].MakeNormal();
		}
	}

	if (gTable.level == 4)
	{

#pragma region MyRegion


		for (int i = 0; i < NUM_CUSHIONS; i++)
		{
			cushions[i].vertices[0](0) = NULL;
			cushions[i].vertices[0](1) = NULL;
			cushions[i].vertices[1](0) = NULL;
			cushions[i].vertices[1](1) = NULL;
		}
		//Null the pockets, so they can be redrawn in a different place.
		pockets[0].vertice(0) = NULL;
		pockets[0].vertice(1) = NULL;

#pragma endregion


		////level 4
		cushions[0].vertices[0](0) = -TABLE_X + 0.45;
		cushions[0].vertices[0](1) = TABLE_Z;
		cushions[0].vertices[1](0) = TABLE_X - 0.445;
		cushions[0].vertices[1](1) = TABLE_Z;

		cushions[1].vertices[0](0) = TABLE_X - 0.444;
		cushions[1].vertices[0](1) = -TABLE_Z + 2.4;
		cushions[1].vertices[1](0) = -TABLE_X + 0.75;
		cushions[1].vertices[1](1) = -TABLE_Z + 1.5;

		cushions[2].vertices[1](0) = TABLE_X - 0.2;
		cushions[2].vertices[1](1) = -TABLE_Z + 2.4;
		cushions[2].vertices[0](0) = -TABLE_X + 0.75;
		cushions[2].vertices[0](1) = -TABLE_Z + 1.5;

		cushions[3].vertices[0](0) = TABLE_X - 0.2;
		cushions[3].vertices[0](1) = -TABLE_Z + 2.4;
		cushions[3].vertices[1](0) = -TABLE_X + 1.5;
		cushions[3].vertices[1](1) = -TABLE_Z + 1.5;

		cushions[4].vertices[1](0) = TABLE_X - 0.75;
		cushions[4].vertices[1](1) = -TABLE_Z + 2.4;
		cushions[4].vertices[0](0) = -TABLE_X + 0.4;
		cushions[4].vertices[0](1) = -TABLE_Z + 1;

		cushions[5].vertices[1](0) = -TABLE_X + 0.4;
		cushions[5].vertices[1](1) = -TABLE_Z + 1;
		cushions[5].vertices[0](0) = -TABLE_X + 1.1;
		cushions[5].vertices[0](1) = -TABLE_Z + 0.8;

		cushions[6].vertices[1](0) = -TABLE_X + 1.1;
		cushions[6].vertices[1](1) = -TABLE_Z + 0.8;
		cushions[6].vertices[0](0) = -TABLE_X + 1.5;
		cushions[6].vertices[0](1) = -TABLE_Z + 1.5;

		pockets[0].vertice(0) = -TABLE_X + 1.1;
		pockets[0].vertice(1) = -TABLE_Z + 2.1;

		for (int i = 0; i < NUM_CUSHIONS; i++)
		{
			cushions[i].MakeCentre();
			cushions[i].MakeNormal();
		}
	}
}


void table::Update(int ms)
{
	//check for collisions for each ball
	for (int i = 0; i < NUM_BALLS; i++)
	{
		for (int j = 0; j < NUM_CUSHIONS; j++)
		{
			balls[i].DoPlaneCollision(cushions[j]);
		}

		for (int j = 0; j < NUM_BALLS; j++)
		{
			balls[i].DoPocketCollision(pockets[j]);
		}
	}

	//This checks whether both players have completed the level, then updates scores, etc. Then, the level changes, rendering in the next level.
	if (Players[0].GetPottedBall() && Players[1].GetPottedBall())
	{
		table::Changelevel();
		table::UpdateLevel();
	}

	//updates all of the balls positions
	for (int i = 0; i < NUM_BALLS; i++) balls[i].Update(ms);

	//updates particles
	parts.update(ms);
}



bool table::AnyBallsMoving(void) const
{
	//return true if any ball has a non-zero velocity
	for (int i = 0; i < NUM_BALLS; i++)
	{
		if (balls[i].velocity(0) != 0.0) return true;
		if (balls[i].velocity(1) != 0.0) return true;
	}
	return false;

}