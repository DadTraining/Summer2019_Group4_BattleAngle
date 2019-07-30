#include "PlayGameScene2.h"
#include "ui\UIButton.h"
#include "SimpleAudioEngine.h"
#include "MenuScene.h"
#include "OverScene.h"

using namespace CocosDenshion;

USING_NS_CC;



Scene* PlayGameScene2::createScene()
{
	auto scene = Scene::createWithPhysics();
	//scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
	scene->getPhysicsWorld()->setGravity(Vec2(0, -500));
	//scene->getPhysicsWorld()->setSpeed(3);
	auto layer = PlayGameScene2::create();
	layer->setPhysicsWorld(scene->getPhysicsWorld());
	scene->addChild(layer, 0);
	return scene;
}
bool PlayGameScene2::init()
{
	if (!Scene::init())
	{
		return false;
	}
	visibleSize = Director::getInstance()->getVisibleSize();
	mCurrentTouchState = ui::Widget::TouchEventType::ENDED;
	mCurrentTouchPoint = Point(-1, -1);
	createMC();
	createHub();
	createMapPhysics();
	addListener();
	createController();
	createCamera();
	createPause();

	//Kaisa_Monster *kaisa = new Kaisa_Monster(this);
	//Murad_Monster *murad = new Murad_Monster(this);

	createMonsters();
	createKillLabel();
	scheduleUpdate();
	return true;
}

void PlayGameScene2::update(float deltaTime) {
	auto x_alita = m_Alita->getSprite()->getPosition().x;
	m_Alita->Update(deltaTime);
	updateKillLabel();
	updateCenterView();
	mMcHudBlood->setPercent((m_Alita->getHP() * 100) / mHP);
	setTurn_Monster(x_alita);
	UpdateMonster(x_alita);
	if (m_Alita->getHP() <= 0)
	{
		createLose();
	}
}



void PlayGameScene2::createMapPhysics() {
	createMap();
	createPhysics();
}
void PlayGameScene2::createMap()
{
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	//map = TMXTiledMap::create("res/map/ok.tmx");
	map = TMXTiledMap::create("map/map3.tmx"); 
	map->setAnchorPoint(Vec2(0, 0));
	map->setScale(0.5);
	map->setPosition(0, 0);

}
void PlayGameScene2::createPhysics()
{
	mObjectGroup = map->getObjectGroup("Monster");
	auto mPhysicsLayer = map->getLayer("Lane");
	Size layerSize = mPhysicsLayer->getLayerSize();
	for (int i = 0; i < layerSize.width; i++)
	{
		for (int j = 0; j < layerSize.height; j++)
		{
			auto tileSet = mPhysicsLayer->getTileAt(Vec2(i, j));
			if (tileSet != NULL)
			{
				auto physics = PhysicsBody::createBox(tileSet->getContentSize(), PhysicsMaterial(1.0f, 0.0f, 1.0f));
				physics->setCategoryBitmask(3);
				physics->setCollisionBitmask(Objects::BITMASK_GROUND);
				physics->setContactTestBitmask(true);
				physics->setDynamic(false);
				//physics->setMass(100);
				tileSet->setPhysicsBody(physics);
			}
		}
	}
	addChild(map, -1);
	auto egdeBody = PhysicsBody::createEdgeBox(map->getContentSize()/2, PHYSICSBODY_MATERIAL_DEFAULT, 3);
	egdeBody->setDynamic(false);
	//egdeBody->;
	egdeNode = Node::create();
	egdeNode->setPosition(map->getContentSize().width / 4, map->getContentSize().height / 4);
	egdeNode->setPhysicsBody(egdeBody);
	addChild(egdeNode);
}
void PlayGameScene2::createCamera()
{
	camera = this->getDefaultCamera();
	camera->setAnchorPoint(Vec2(1, 1));
	camera->setPosition(visibleSize.width / 2, visibleSize.height / 2);
}
void PlayGameScene2::createMonsters() {

	//add effect
	auto paricleEffect = ParticleSnow::create();
	paricleEffect->setPosition(Vec2(visibleSize.width / 2, visibleSize.height));
	addChild(paricleEffect);
	auto objects = mObjectGroup->getObjects();
	int kaisa_count = 0;
	int murad_count = 0;
	for (int i = 0; i < objects.size(); i++)
	{
		auto object = objects.at(i);
		auto properties = object.asValueMap();
		int posX = properties.at("x").asInt();
		int posY = properties.at("y").asInt();
		int type = object.asValueMap().at("type").asInt();
		if (type == 0)
		{
			m_Alita->getSprite()->setPosition(Vec2(posX, posY));
		}
		else if (type == 1)
		{
			kaisa = new Kaisa_Monster(this);
			kaisa->getSprite()->setPosition(Vec2(posX, posY));
			kaisa->setIndex(kaisa_count);
			kaisa->getBullet()->setIndex(kaisa_count);
			mKaisa.push_back(kaisa);
			kaisa_count++;
			sizeMonster++;
		}
		else if (type == 2)
		{
			murad = new Murad_Monster(this);
			murad->getSprite()->setPosition(Vec2(posX, posY));
			murad->setIndex(murad_count++);
			murad->FPSMurad = murad_count * 3;
			if (murad->FPSMurad > 18) {
				murad->FPSMurad -= 18;
			}
			mMurad.push_back(murad);
			sizeMonster++;
		}
	}
}

void PlayGameScene2::createKillLabel()
{
	tempCount = CCString::createWithFormat("Kill: %i / %i", countMonster, sizeMonster);
	labelMonster = Label::createWithTTF(tempCount->getCString(), "fonts/Marker Felt.ttf", 24);
	labelMonster->setColor(Color3B::RED);
	labelMonster->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 1.2));
	addChild(labelMonster, 20);
}

void PlayGameScene2::createHub()
{
	hud_bg = Sprite::create("res/BloodMc/hud_bg.png");
	hud_bg->setAnchorPoint(Vec2(0, 0.5));
	hud_bg->setScale(0.5);
	hud_bg->setPosition(Vec2(50, visibleSize.height - 30));
	addChild(hud_bg, 20);

	mMcHudBlood = ui::LoadingBar::create("res/BloodMc/hud_blood.png");
	mMcHudBlood->setAnchorPoint(Vec2(0, 0.5));
	mMcHudBlood->setScale(0.5);
	mMcHudBlood->setPosition(hud_bg->getPosition());
	mMcHudBlood->setDirection(ui::LoadingBar::Direction::LEFT);
	mMcHudBlood->setPercent(m_Alita->getHP());
	addChild(mMcHudBlood,20);

	hud = Sprite::create("res/BloodMc/hud.png");
	hud->setAnchorPoint(Vec2(0, 0.5));
	hud->setScale(0.5);
	hud->setPosition(hud_bg->getPosition() - Vec2(12, 0));
	addChild(hud,20);
}
void PlayGameScene2::createPause()
{
		mPauseLayer = Sprite::create("res/Pause_UI/alpha.png");
		mPauseLayer->setContentSize(visibleSize);
		mPauseLayer->setAnchorPoint(Vec2(0.5, 0.5));
		mPauseLayer->setPosition(visibleSize / 2);
		mPauseLayer->setVisible(false);
		addChild(mPauseLayer, 30);

		auto bg = Sprite::create("res/Setting_UI/bg.png");
		bg->setAnchorPoint(Vec2(0.5, 0.5));
		bg->setScale(0.2);
		bg->setPosition(visibleSize / 2);
		mPauseLayer->addChild(bg, 0);
		auto bg1 = Sprite::create("res/Setting_UI/table.png");
		bg1->setAnchorPoint(Vec2(0.5, 0.5));
		bg1->setScale(0.2);
		bg1->setPosition(bg->getPosition());
		mPauseLayer->addChild(bg1, 2);
		auto bg2 = Sprite::create("res/Pause_UI/header.png");
		bg2->setAnchorPoint(Vec2(0.5, 0.5));
		bg2->setScale(0.15);
		bg2->setPosition(bg1->getPosition() + Vec2(0, 70));
		mPauseLayer->addChild(bg2, 3);
		auto text = Sprite::create("res/Pause_UI/text.png");
		text->setAnchorPoint(Vec2(0.5, 0.5));
		text->setScale(0.2);
		text->setPosition(bg2->getPosition() - Vec2(0, 70));
		mPauseLayer->addChild(text, 3);
		//Button Ok

		auto btnOk = ui::Button::create("res/Pause_UI/ok.png");
		btnOk->setAnchorPoint(Vec2(0.5, 0.5));
		btnOk->setPosition(text->getPosition() - Vec2(50, 50));
		btnOk->setScale(0.2);
		mPauseLayer->addChild(btnOk, 7);
		btnOk->addClickEventListener([&](Ref* event) {
			auto audio = SimpleAudioEngine::getInstance();
			audio->playEffect("res/Music/buttonclick.mp3", false);
			SimpleAudioEngine::getInstance()->resumeAllEffects();
			SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
			Director::getInstance()->resume();
			mPauseLayer->setVisible(false);
		});
		//Button Menu

		auto btnMenu = ui::Button::create("res/Pause_UI/menu.png");
		btnMenu->setAnchorPoint(Vec2(0.5, 0.5));
		btnMenu->setPosition(btnOk->getPosition() + Vec2(100, 0));
		btnMenu->setScale(0.2);
		mPauseLayer->addChild(btnMenu, 7);
		btnMenu->addClickEventListener([&](Ref* event) {
			auto audio = SimpleAudioEngine::getInstance();
			audio->playEffect("res/Music/buttonclick.mp3", false);
			SimpleAudioEngine::getInstance()->resumeAllEffects();
			SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
			Director::getInstance()->resume();
			Director::getInstance()->replaceScene(MenuScene::createScene());
		});



	
}
void PlayGameScene2::createLose()
{
	Director::getInstance()->replaceScene(OverScene::createScene());
}
void PlayGameScene2::UpdateMonster(float x_alita)
{
	//	auto PositionAlita = m_Alita->getSprite()->();
	for (auto i : mMurad) {
		i->UpdateAttack(x_alita,m_Alita);
		i->Update(0);
	}
	for (auto i : mKaisa) {
		i->Update(x_alita);
	}
}
void PlayGameScene2::updateKillLabel()
{
	for (auto i : mMurad) {
		if (i->getHP() <=0 && i->getKill==false) {
			countMonster++;
			i->getKill = true;
		}
	}
	for (auto i : mKaisa) {
		if (i->getHP() <= 0 && i->getKill==false) {
			countMonster++;
			i->getKill = true;
		}
	}
	tempCount = CCString::createWithFormat("Kill: %i / %i", countMonster, sizeMonster);
	labelMonster->setString(tempCount->getCString());
}
bool PlayGameScene2::onContactBegin(cocos2d::PhysicsContact & contact)
{
	PhysicsBody* a = contact.getShapeA()->getBody();
	PhysicsBody* b = contact.getShapeB()->getBody();

	//dark vs mKaisa
	if ((a->getCollisionBitmask() == Objects::BITMASK_DART && b->getCollisionBitmask() == Objects::BITMASK_KAISA)
		|| (a->getCollisionBitmask() == Objects::BITMASK_KAISA && b->getCollisionBitmask() == Objects::BITMASK_DART))
	{
		if (a->getCollisionBitmask() == Objects::BITMASK_DART)
		{
			mKaisa.at(b->getGroup())->DarkCollision(m_Alita->getDarts()->getDame());
			m_Alita->getDarts()->setAlive(false);
		}
		else if (b->getCollisionBitmask() == Objects::BITMASK_DART)
		{
			mKaisa.at(a->getGroup())->DarkCollision(m_Alita->getDarts()->getDame());
			m_Alita->getDarts()->setAlive(false);
		}
		auto paricleEffect = ParticleSystemQuad::create("particles/collision.plist");
		paricleEffect->setPosition(a->getPosition());
		addChild(paricleEffect);
	}

	//dark vs mMurad
	if ((a->getCollisionBitmask() == Objects::BITMASK_DART && b->getCollisionBitmask() == Objects::BITMASK_MURAD)
		|| (a->getCollisionBitmask() == Objects::BITMASK_MURAD && b->getCollisionBitmask() == Objects::BITMASK_DART))
	{
		if (a->getCollisionBitmask() == Objects::BITMASK_DART&& b->getContactTestBitmask() == 3)
		{
			mMurad.at(b->getGroup())->DarkCollision(m_Alita->getDarts()->getDame());
			m_Alita->getDarts()->setAlive(false);
		}
		else if (b->getCollisionBitmask() == Objects::BITMASK_DART &&a->getContactTestBitmask() == 3)
		{
			mMurad.at(a->getGroup())->DarkCollision(m_Alita->getDarts()->getDame());
			m_Alita->getDarts()->setAlive(false);
		}
		auto paricleEffect = ParticleSystemQuad::create("particles/collision.plist");
		paricleEffect->setPosition(a->getPosition());
		addChild(paricleEffect);
	}

	//dark vs ground
	if ((a->getCollisionBitmask() == Objects::BITMASK_DART && b->getCollisionBitmask() == Objects::BITMASK_GROUND)
		|| (a->getCollisionBitmask() == Objects::BITMASK_GROUND && b->getCollisionBitmask() == Objects::BITMASK_DART))
	{
		m_Alita->getDarts()->setAlive(false);
	}

	//bullet vs Alita 
	if ((a->getCollisionBitmask() == Objects::BITMASK_BULLET && b->getCollisionBitmask() == Objects::BITMASK_ALITA)
		|| (a->getCollisionBitmask() == Objects::BITMASK_ALITA && b->getCollisionBitmask() == Objects::BITMASK_BULLET))
	{
		if (a->getCollisionBitmask() == Objects::BITMASK_BULLET)
		{
			m_Alita->BulletCollision();
			mKaisa.at(a->getGroup())->getBullet()->getSprite()->setVisible(false);
			//mKaisa.at(a->getGroup())->getBullet()->getSprite()->setVisible(false);
		}
		else if (b->getCollisionBitmask() == Objects::BITMASK_BULLET)
		{
			m_Alita->BulletCollision();
			mKaisa.at(b->getGroup())->getBullet()->getSprite()->setVisible(false);
			//mKaisa.at(a->getGroup())->getBullet()->getSprite()->setVisible(false);
		}
		auto paricleEffect = ParticleSystemQuad::create("particles/collision.plist");
		paricleEffect->setPosition(a->getPosition());
		addChild(paricleEffect);
	}

	//Alita vs Round
	if ((a->getCollisionBitmask() == Objects::BITMASK_GROUND && b->getCollisionBitmask() == Objects::BITMASK_ALITA)
		|| (a->getCollisionBitmask() == Objects::BITMASK_ALITA && b->getCollisionBitmask() == Objects::BITMASK_GROUND))
	{
		m_Alita->setJumping(false);
	}
	return true;
}
void PlayGameScene2::setTurn_Monster(float xAlita)
{
	for (auto i : mMurad) {
		i->setTurn_Murad(xAlita);
	}
	for (auto i : mKaisa) {
		i->setTurnKaisa(xAlita);
	}
}
void PlayGameScene2::updateCenterView()
{
	auto x_alita = m_Alita->getSprite()->getPosition().x;
	if (x_alita>STATIC_Position_Alita && x_alita<(map->getContentSize().width/2 - visibleSize.width / 2)) {
		camera->setPosition(camera->getPosition().x + (x_alita - x_positon_Alita), visibleSize.height / 2);
		//egdeNode->setPosition(egdeNode->getPosition().x + (x_alita - x_positon_Alita), visibleSize.height / 2);
		mMoveLeftController->setPosition(Vec2(mMoveLeftController->getPosition().x + (x_alita - x_positon_Alita), mMoveLeftController->getPosition().y));
		mMoveRightController->setPosition(Vec2(mMoveRightController->getPosition().x + (x_alita - x_positon_Alita), mMoveRightController->getPosition().y));
		mJumpController->setPosition(Vec2(mJumpController->getPosition().x + (x_alita - x_positon_Alita), mJumpController->getPosition().y));
		mAttackController->setPosition(Vec2(mAttackController->getPosition().x + (x_alita - x_positon_Alita), mAttackController->getPosition().y));
		mThrowController->setPosition(Vec2(mThrowController->getPosition().x + (x_alita - x_positon_Alita), mThrowController->getPosition().y));

		hud_bg->setPosition(Vec2(hud_bg->getPosition().x + (x_alita - x_positon_Alita), hud_bg->getPosition().y));
		hud->setPosition(hud_bg->getPosition() - Vec2(15, 0));
		mMcHudBlood->setPosition(hud_bg->getPosition());
		btnPause->setPosition(Vec2(btnPause->getPosition().x + (x_alita - x_positon_Alita), btnPause->getPosition().y));
		mPauseLayer->setPosition(Vec2(mPauseLayer->getPosition().x + (x_alita - x_positon_Alita), mPauseLayer->getPosition().y));
		
		//Update Kill Label 
		labelMonster->setPosition(Vec2(labelMonster->getPosition().x + (x_alita - x_positon_Alita), labelMonster->getPosition().y));
		x_positon_Alita = x_alita;
	}
}
void PlayGameScene2::addListener()
{
	//touch
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->setSwallowTouches(true);
	touchListener->onTouchBegan = CC_CALLBACK_2(PlayGameScene2::onTouchBegan, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(PlayGameScene2::onTouchEnded, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

	//add contact event listener
	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(PlayGameScene2::onContactBegin, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);
}
bool PlayGameScene2::onTouchBegan(Touch* touch, Event  *event)
{
	//mCurrentTouchState = ui::Widget::TouchEventType::BEGAN;
	mCurrentTouchPoint = touch->getLocation();
	return true;
}


bool PlayGameScene2::onTouchEnded(Touch* touch, Event  *event)
{
	//mCurrentTouchState = ui::Widget::TouchEventType::ENDED;
	mCurrentTouchPoint = Point(-1, -1);
	return true;
}

void PlayGameScene2::createController()
{
	auto ScreenSize = Director::getInstance()->getVisibleSize();
	//MoveLeft
	mMoveLeftController = ui::Button::create("res/Main_UI/left.png", "res/Main_UI/left.png");
	mMoveLeftController->setPosition(Vec2(50, 50));
	mMoveLeftController->setScale(0.7);
	mMoveLeftController->setOpacity(160);
	mMoveLeftController->addTouchEventListener(CC_CALLBACK_2(PlayGameScene2::moveLeft, this));
	addChild(mMoveLeftController, 10);
	//MoveRight
	mMoveRightController = ui::Button::create("res/Main_UI/left.png", "res/Main_UI/left.png");
	mMoveRightController->setFlippedX(true);
	mMoveRightController->setPosition(mMoveLeftController->getPosition() + Vec2(mMoveLeftController->getContentSize().width, 0));
	mMoveRightController->setScale(0.7);
	mMoveRightController->setOpacity(160);
	mMoveRightController->addTouchEventListener(CC_CALLBACK_2(PlayGameScene2::moveRight, this));
	addChild(mMoveRightController, 10);
	//Jump
	mJumpController = ui::Button::create("res/Main_UI/jump.png", "res/Main_UI/jump.png");
	mJumpController->setScale(0.7);
	mJumpController->setOpacity(160);
	mJumpController->setPosition(Vec2(visibleSize.width - mJumpController->getContentSize().width +20, mJumpController->getContentSize().height +20));
	mJumpController->addTouchEventListener(CC_CALLBACK_2(PlayGameScene2::jump, this));
	addChild(mJumpController, 10);
	//Attack
	mAttackController = ui::Button::create("res/Main_UI/attack.png", "res/Main_UI/attack.png");
	mAttackController->setScale(0.7);
	mAttackController->setOpacity(160);
	mAttackController->setPosition(Vec2(visibleSize.width - mJumpController->getContentSize().width + 20, mJumpController->getContentSize().height - 40));
	mAttackController->addTouchEventListener(CC_CALLBACK_2(PlayGameScene2::attack, this));
	addChild(mAttackController, 10);
	//Throw
	mThrowController = ui::Button::create("res/Main_UI/skill.png", "res/Main_UI/skill.png");
	mThrowController->setScale(0.7);
	mThrowController->setOpacity(160);
	mThrowController->setPosition(Vec2(visibleSize.width - mJumpController->getContentSize().width - 32, mJumpController->getContentSize().height-5));
	mThrowController->addTouchEventListener(CC_CALLBACK_2(PlayGameScene2::throws, this));
	addChild(mThrowController, 10);
	//Pause
	btnPause = ui::Button::create("res/Pause_UI/pause.png");
	btnPause->setAnchorPoint(Vec2(0, 0.5));
	btnPause->setScale(0.2);
	btnPause->setPosition(Vec2(visibleSize.width - 50, visibleSize.height - 30));
	btnPause->addTouchEventListener(CC_CALLBACK_2(PlayGameScene2::pause, this));
	addChild(btnPause, 20);
}

void PlayGameScene2::createMC()
{
	m_Alita = new Alita(this);
	STATIC_Position_Alita = m_Alita->getSprite()->getPosition().x;
	x_positon_Alita = m_Alita->getSprite()->getPosition().x;
}

void PlayGameScene2::moveRight(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
	switch (type)
	{
	case ui::Widget::TouchEventType::BEGAN:
		m_Alita->MoveRight();
		break;
	case ui::Widget::TouchEventType::MOVED:
		/*if (!Rect(mMoveRightController->getPositionX(), mMoveRightController->getPositionY(), mMoveRightController->getContentSize().width*3, mMoveRightController->getContentSize().height*3).containsPoint(mCurrentTouchPoint)) {
		m_Alita->getSprite()->stopAllActions();
		m_Alita->setRunning(false);
		}*/
		break;
	case ui::Widget::TouchEventType::ENDED:
		m_Alita->getSprite()->stopAllActions();
		m_Alita->setRunning(false);
		break;
	}
}

void PlayGameScene2::moveLeft(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type) {

	switch (type)
	{
	case ui::Widget::TouchEventType::BEGAN:
		m_Alita->MoveLeft();
		break;
	case ui::Widget::TouchEventType::MOVED:
		/*if (!Rect(mMoveRightController->getPositionX(), mMoveRightController->getPositionY(), mMoveRightController->getContentSize().width*3, mMoveRightController->getContentSize().height*3).containsPoint(mCurrentTouchPoint)) {
		m_Alita->getSprite()->stopAllActions();
		m_Alita->setRunning(false);
		}*/
		break;
	case ui::Widget::TouchEventType::ENDED:
		m_Alita->getSprite()->stopAllActions();
		m_Alita->setRunning(false);
		break;
	}
}
void PlayGameScene2::jump(Ref* sender, ui::Widget::TouchEventType type)
{
	switch (type)
	{
	case ui::Widget::TouchEventType::BEGAN:
		m_Alita->Jump();
		break;
	case ui::Widget::TouchEventType::ENDED:
		break;
	}
}

void PlayGameScene2::attack(cocos2d::Ref * sender, cocos2d::ui::Widget::TouchEventType type)
{
	switch (type)
	{
	case ui::Widget::TouchEventType::BEGAN:
		break;
	case ui::Widget::TouchEventType::ENDED:
		if (m_Alita->Attack()) {
			// Collision Alita vs Monster
			rectAlita = m_Alita->getSprite()->getBoundingBox();
			for (auto i : mMurad) {
				rectMonster = i->getSprite()->getBoundingBox();
				if (rectAlita.intersectsRect(rectMonster)) {
				//	// Attack from Left to Right
				//	if (!m_Alita->m_LefttoRight) {
				//		if (m_Alita->getSprite()->getPositionX() < i->getSprite()->getPositionX()) {
				//			i->DarkCollision();
				//		}
				//	}
				//	else {
						// (m_Alita->getSprite()->getPositionX() > i->getSprite()->getPositionX()) {
							i->DarkCollision(m_Alita->getDame());
					/*	}
					}*/
				}
			}
			for (auto i : mKaisa) {
				rectMonster = i->getSprite()->getBoundingBox();
				if (rectAlita.intersectsRect(rectMonster)) {
					// Attack from Right to Left
					/*if (m_Alita->m_LefttoRight) {
						if (m_Alita->getSprite()->getPositionX() < i->getSprite()->getPositionX()) {
							i->DarkCollision();
						}
					}
					else {*/
						//if (m_Alita->getSprite()->getPositionX() > i->getSprite()->getPositionX()) {
							i->DarkCollision(m_Alita->getDame());
						//}
					//}
				}
			}
		}
		break;
	}
}

void PlayGameScene2::throws(cocos2d::Ref * sender, cocos2d::ui::Widget::TouchEventType type)
{
	switch (type)
	{
	case ui::Widget::TouchEventType::BEGAN:
		m_Alita->Throw();
		
		break;
	case ui::Widget::TouchEventType::ENDED:
			
		
		break;
	}
}
void PlayGameScene2::pause(Ref* sender, ui::Widget::TouchEventType type)
{
	switch (type)
	{
	case ui::Widget::TouchEventType::ENDED:
		auto funcPause = CallFunc::create([]() {
			SimpleAudioEngine::getInstance()->pauseAllEffects();
			SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
			Director::getInstance()->pause();
			auto audio = SimpleAudioEngine::getInstance();
			audio->playEffect("res/Music/buttonclick.mp3", false);
			//Director::getInstance()->replaceScene(PauseScene::createScene());
		});
		mPauseLayer->setOpacity(0);
		mPauseLayer->setVisible(true);
		auto fadeIn = FadeIn::create(0.3f);
		mPauseLayer->runAction(Sequence::create(fadeIn, funcPause, nullptr));
		break;
	}
}