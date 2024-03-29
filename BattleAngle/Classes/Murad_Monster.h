#pragma once
#include "Objects.h"
#include "Alita.h"
class Murad_Monster : public Objects
{
public:

	static const int ANIM_IDLE = 0;
	static const int ANIM_RUN = ANIM_IDLE + 1;
	static const int ANIM_ATTACK = ANIM_RUN + 1;
	static const int ANIM_DIE = ANIM_ATTACK + 1;
	static const int ANIM_TOTAl = ANIM_DIE + 1;

	Murad_Monster(Scene * scene);
	~Murad_Monster();
	bool attacked = false;
	void Init() override;
	void UpdateAttack(float xAlita, Alita * alita);
	void Update(float deltaTime) override;
	void Idle();
	void Run();
	void Attack();
	void Die();
	void setTurnRight();
	void setTurnLeft();
	bool getm_LetftoRight();
	void DarkCollision(int dame);
	void setState_Murad(float position);
	void setTurn_Murad(float position);
	void Collision();
	int FPSMurad = 0;
private:
	cocos2d::Action* mAnimation[ANIM_TOTAl];
};

#pragma once