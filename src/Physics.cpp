/*The MIT License (MIT)

Copyright (c) 2016 Lucas Borsatto Simão

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
*/

#include "Physics.h"

Physics::Physics(int numTeams){
    this->numTeams = numTeams;
    this->numRobotsTeam = NUM_ROBOTS_TEAM;

	collisionConfig = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfig);
    broadphase = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();
    world = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfig);
    world->setGravity(btVector3(0,-9.81*SCALE_WORLD,0));

    glDebugDrawer = new GLDebugDrawer();
    world->setDebugDrawer(glDebugDrawer);
    gContactAddedCallback = callBackHitFunc;

    registBodies();
}

Physics::~Physics(){
    deleteWorldObj();

    if(collisionConfig) delete collisionConfig;
    if(dispatcher) delete dispatcher;
    if(broadphase) delete broadphase;
    if(solver) delete solver;
    if(world) delete world;
}

void Physics::deleteWorldObj(){
    for(int i = 0; i<bodies.size();i++){
        delete bodies[i];
    }

    for(int i = 0;i<genRobots.size();i++){
        delete genRobots[i];
    }
}

void Physics::registBodies(){
    addFloor();

    addBall(2.5,btVector3(SIZE_WIDTH/1.5,0,SIZE_DEPTH/1.5),0.08);

    btVector3 posTeam1[] = {btVector3(15,4,SIZE_DEPTH- 55),btVector3(35,4,30),btVector3(55,4,45)};
    btVector3 posTeam2[] = {btVector3(SIZE_WIDTH-15,4,55),btVector3(SIZE_WIDTH-25,4,SIZE_DEPTH - SIZE_DEPTH/2.5 + 20),btVector3(SIZE_WIDTH-55,4,85)};
    //Create robots here
    //Team 1
    for(int i = 0;i < numRobotsTeam;i++){
        if(numTeams >= 1){
            addRobot(Color(0.3,0.3,0.3),posTeam1[i],btVector3(0,90,0),8,0.25,clrPlayers[i],clrTeams[0]);
        }
    }

    for(int i = 0;i < numRobotsTeam;i++){
        if(numTeams == 2){
            addRobot(Color(0.3,0.3,0.3),posTeam2[i],btVector3(0,-100,0),8,0.25,clrPlayers[i],clrTeams[1]);
        }
    }

    addWall(Color(0,0,0),btVector3(SIZE_WIDTH/2+1,7.5,-2.5),SIZE_WIDTH,15,5,0);
    addWall(Color(0,0,0),btVector3(SIZE_WIDTH/2+1,7.5,SIZE_DEPTH+2.5),SIZE_WIDTH,15,5,0);

    addWall(Color(0,0,0),btVector3(-1.6,7.5,SIZE_DEPTH/6),5,15,SIZE_DEPTH/3,0);
    addWall(Color(0,0,0),btVector3(-1.6,7.5,SIZE_DEPTH-SIZE_DEPTH/6),5,15,SIZE_DEPTH/3,0);
    addWall(Color(0,0,0),btVector3(-12.5,7.5, SIZE_DEPTH/2),5,15,SIZE_DEPTH/3,0);
    addWall(Color(0,0,0),btVector3(-7,7.5,SIZE_DEPTH/3),15,15,5,0);
    addWall(Color(0,0,0),btVector3(-7,7.5,2*SIZE_DEPTH/3+2.5),15,15,5,0);

    addWall(Color(0,0,0),btVector3(SIZE_WIDTH+3.5,7.5,SIZE_DEPTH/6),5,15,SIZE_DEPTH/3,0);
    addWall(Color(0,0,0),btVector3(SIZE_WIDTH+3.5,7.5,SIZE_DEPTH-SIZE_DEPTH/6),5,15,SIZE_DEPTH/3,0);
    addWall(Color(0,0,0),btVector3(SIZE_WIDTH+13.5,7.5, SIZE_DEPTH/2),5,15,SIZE_DEPTH/3,0);
    addWall(Color(0,0,0),btVector3(SIZE_WIDTH+8.5,7.5,SIZE_DEPTH/3),15,15,5,0);
    addWall(Color(0,0,0),btVector3(SIZE_WIDTH+8.5,7.5,2*SIZE_DEPTH/3+2.5),15,15,5,0);

    addCorner(Color(0,0,0),btVector3(10,7.5,10),30,15,btVector3(0,45,0));
    addCorner(Color(0,0,0),btVector3(SIZE_WIDTH-9,7.5,10),30,15,btVector3(0,-45,0));
    addCorner(Color(0,0,0),btVector3(SIZE_WIDTH-9,7.5,SIZE_DEPTH-10),30,15,btVector3(0,45,0));
    addCorner(Color(0,0,0),btVector3(10,7.5,SIZE_DEPTH-10),30,15,btVector3(0,-45,0));
}

void Physics::stepSimulation(float timeW,float subStep, float timeStep){
    world-> stepSimulation(timeW, subStep, timeStep);
}

bool Physics::callBackHitFunc(btManifoldPoint& cp,const btCollisionObjectWrapper* obj1,int id1,int index1,const btCollisionObjectWrapper* obj2,int id2,int index2){
    string prefix = "robot";

    const btCollisionObjectWrapper* wrappers[] = {obj1, obj2};

    for(int i = 0; i < 2; i++){
        BulletObject* btObj = (BulletObject*)wrappers[i]->getCollisionObject()->getUserPointer();

        string name = btObj->name;
        if (!name.compare(0, prefix.size(), prefix) || name == "ball")
           btObj->hit = true;
    }

    return false;
}


btVector3 Physics::getBallPosition(){
	btVector3 ballPos;
	for(int i=0;i<bodies.size();i++){
		if(bodies[i]->name.compare("ball") == 0){
			btTransform t;
			bodies[i]->body->getMotionState()->getWorldTransform(t);
            ballPos = t.getOrigin();
			break;
		}
	}
	return ballPos;
}

void Physics::startDebug(){
    world->debugDrawWorld();
}

void Physics::setDebugWorld(int debugMode){
    vector<int> debugDrawMode;
    ((GLDebugDrawer*)world-> getDebugDrawer())->setDrawScenarioMode(true);
    switch (debugMode){
        case 0:{
            debugDrawMode.push_back(btIDebugDraw::DBG_NoDebug);
            world->getDebugDrawer()->setDebugMode(debugDrawMode);
        }break;
        case 1:{
            debugDrawMode.push_back(btIDebugDraw::DBG_DrawLocalProperties);
            debugDrawMode.push_back(btIDebugDraw::DBG_DrawWireframe);
            world->getDebugDrawer()->setDebugMode(debugDrawMode);
            ((GLDebugDrawer*)world-> getDebugDrawer())->setDrawScenarioMode(false);
        }break;
        case 2:{
            debugDrawMode.push_back(btIDebugDraw::DBG_DrawWireframe);
            debugDrawMode.push_back(btIDebugDraw::DBG_DrawLocalProperties);
            world-> getDebugDrawer()->setDebugMode(debugDrawMode);
        }break;
    }
}

btRigidBody* Physics::addFloor(){
	string name = "floor";
    btStaticPlaneShape* plane = new btStaticPlaneShape(btVector3(0,1,0),0);
    btRigidBody* body = addGenericBody(plane,name,Color(0.0,0.0,0.0),btVector3(1.0,0.0,0.0),0);
    return body;
}

btRigidBody* Physics::addBall(float rad,btVector3 pos,float mass)
{
    string name = "ball";
    btSphereShape* ball = new btSphereShape(rad);
    btRigidBody* body = addGenericBody(ball,name,Color(1.0,0.0,0.0), pos, mass);
    return body;
}

btRigidBody* Physics::addWall(Color clr, btVector3 pos,float width, float height, float depth, float mass){
    string name = "wall";
    btBoxShape* wall = new btBoxShape(btVector3(width/2.0,height/2.0,depth/2.0));
    btRigidBody* body = addGenericBody(wall,name,clr,pos,mass);
    return body;
}

btRigidBody* Physics::addCorner(Color clr, btVector3 pos,float width, float height,btVector3 rotation){
    float mass = 0.f;
    float depth = 0.01f;
    btVector3 rotRad = rotation*PI/180;
    string name = "corner";
    btBoxShape* corner = new btBoxShape(btVector3(width/2.0,height/2.0,depth/2.0));
    btRigidBody* body = addGenericBody(corner,name,clr,pos,mass,rotRad);
    return body;
}

RobotPhysics* Physics::addRobot(Color clr, btVector3 pos, btVector3 rotation,float sizeRobot, float mass,Color colorPlayer,Color colorTeam){
    btBoxShape* modelShape = new btBoxShape(btVector3(sizeRobot/2,sizeRobot/2,sizeRobot/2));
    btCompoundShape* compound = new btCompoundShape();

    btTransform localTrans;
    localTrans.setIdentity();
    localTrans.setOrigin(btVector3(0.0,4.0,0));

    compound->addChildShape(localTrans,modelShape);

    btTransform transRobot;
    transRobot.setIdentity();
    transRobot.setOrigin(btVector3(pos.getX(),pos.getY(),pos.getZ()));
    if(rotation.length() != 0){
        rotation *= PI/180;
        float rad = rotation.length();
        btVector3 axis = rotation.normalize();
        btQuaternion quat(axis,rad);
        transRobot.setRotation(quat);
    }

    btVector3 localInertia(0,0,0);
    compound->calculateLocalInertia(mass,localInertia);

    btMotionState* robotMotState = new btDefaultMotionState(transRobot);
    btRigidBody::btRigidBodyConstructionInfo cInfo(mass,robotMotState,compound,localInertia);
    btRigidBody* bdRobot = new btRigidBody(cInfo);
    bdRobot->setCollisionFlags(bdRobot->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

    bdRobot -> setLinearVelocity(btVector3(0,0,0));
    bdRobot -> setAngularVelocity(btVector3(0,0,0));

    world->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(
        bdRobot-> getBroadphaseHandle(),
        world -> getDispatcher()
    );

    bdRobot->setIdDebug(1);

    bodies.push_back(new BulletObject(bdRobot,"robot",clr));
    bodies[bodies.size()-1]->halfExt = modelShape->getHalfExtentsWithMargin();
    bdRobot->setUserPointer(bodies[bodies.size()-1]);

    world->addRigidBody (bdRobot);

    RobotPhysics* localRobot = new RobotPhysics(pos,0.2,bdRobot,colorPlayer,colorTeam);
    localRobot->buildRobot(world);

    world-> addVehicle(localRobot-> getRaycast());

    genRobots.push_back(localRobot);
    return localRobot;
}

btRigidBody* Physics::addGenericBody(btCollisionShape* shape,string name,Color clr, btVector3 pos, float mass,btVector3 rotation){

	btTransform t;
    t.setIdentity();
    t.setOrigin(btVector3(pos.getX(),pos.getY(),pos.getZ()));

    if(rotation.length() != 0){
        float rad = rotation.length();
        btVector3 axis = rotation.normalize();
        btQuaternion quat(axis,rad);
        t.setRotation(quat);
    }

    btVector3 inertia(0,0,0);
    if(mass!=0.0)
        shape->calculateLocalInertia(mass,inertia);

    btMotionState* motion=new btDefaultMotionState(t);
    btRigidBody::btRigidBodyConstructionInfo info(mass,motion,shape,inertia);
    btRigidBody* body=new btRigidBody(info);

    bodies.push_back(new BulletObject(body,name,clr));
    body->setUserPointer(bodies[bodies.size()-1]);
    world->addRigidBody(body);
    return body;
}
