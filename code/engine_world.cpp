/*void InitODE(game_state *GameState)

{

    // Create a new, empty world and assign its ID number to World. Most applications will only need one world.

    GameState->World = dWorldCreate();

    // Create a new collision space and assign its ID number to Space, passing 0 instead of an existing dSpaceID.

    // There are three different types of collision spaces we could create here depending on the number of objects

    // in the world but dSimpleSpaceCreate is fine for a small number of objects. If there were more objects we

    // would be using dHashSpaceCreate or dQuadTreeSpaceCreate (look these up in the ODE docs)

    GameState->Space = dSimpleSpaceCreate(0);

    // Create a joint group object and assign its ID number to contactgroup. dJointGroupCreate used to have a

    // max_size parameter but it is no longer used so we just pass 0 as its argument.

    GameState->contactgroup = dJointGroupCreate(0);

    // Create a ground plane in our collision space by passing Space as the first argument to dCreatePlane.

    // The next four parameters are the planes normal (a, b, c) and distance (d) according to the plane

    // equation a*x+b*y+c*z=d and must have length 1

    dCreatePlane(GameState->Space, 0, 1, 0, 0);

    // Now we set the gravity vector for our world by passing World as the first argument to dWorldSetGravity.

    // Earth's gravity vector would be (0, -9.81, 0) assuming that +Y is up. I found that a lighter gravity looked

    // more realistic in this case.

    dWorldSetGravity(GameState->World, 0, -1.0, 0);

    // These next two functions control how much error correcting and constraint force mixing occurs in the world.

    // Don't worry about these for now as they are set to the default values and we could happily delete them from

    // this example. Different values, however, can drastically change the behaviour of the objects colliding, so

    // I suggest you look up the full info on them in the ODE docs.

    dWorldSetERP(GameState->World, 0.2);

    dWorldSetCFM(GameState->World, 1e-5);

    // This function sets the velocity that interpenetrating objects will separate at. The default value is infinity.

    dWorldSetContactMaxCorrectingVel(GameState->World, 0.9);

    // This function sets the depth of the surface layer around the world objects. Contacts are allowed to sink into

    // each other up to this depth. Setting it to a small value reduces the amount of jittering between contacting

    // objects, the default value is 0.

    dWorldSetContactSurfaceLayer(GameState->World, 0.001);

    // To save some CPU time we set the auto disable flag to 1. This means that objects that have come to rest (based

    // on their current linear and angular velocity) will no longer participate in the simulation, unless acted upon

    // by a moving object. If you do not want to use this feature then set the flag to 0. You can also manually enable

    // or disable objects using dBodyEnable and dBodyDisable, see the docs for more info on this.

    dWorldSetAutoDisableFlag(GameState->World, 1);

    // This brings us to the end of the world settings, now we have to initialize the objects themselves.

    // Create a new body for our object in the world and get its ID.

    GameState->Object.Body = dBodyCreate(GameState->World);

    // Next we set the position of the new body

    dBodySetPosition(GameState->Object.Body, 0, 10, -5);

    // Here I have set the initial linear velocity to stationary and let gravity do the work, but you can experiment

    // with the velocity vector to change the starting behaviour. You can also set the rotational velocity for the new

    // body using dBodySetAngularVel which takes the same parameters.

    v3 tempVect = V3(0.0, 0.0, 0.0);

    dBodySetLinearVel(GameState->Object.Body, tempVect.x, tempVect.y, tempVect.z);

    // To start the object with a different rotation each time the program runs we create a new matrix called R and use

    // the function dRFromAxisAndAngle to create a random initial rotation before passing this matrix to
    // dBodySetRotation.

    dMatrix3 R;

    dRFromAxisAndAngle(R, (r32)(dRandReal() * 2.0 - 1.0),

                       (r32)(dRandReal() * 2.0 - 1.0),

                       (r32)(dRandReal() * 2.0 - 1.0),

                       (r32)(dRandReal() * 10.0 - 5.0));

    dBodySetRotation(GameState->Object.Body, R);

    // At this point we could add our own user data using dBodySetData but in this example it isn't used.

    size_t i = 0;

    dBodySetData(GameState->Object.Body, (void *)i);

    // Now we need to create a box mass to go with our geom. First we create a new dMass structure (the internals

    // of which aren't important at the moment) then create an array of 3 float (dReal) values and set them

    // to the side lengths of our box along the x, y and z axes. We then pass the both of these to dMassSetBox with a

    // pre-defined DENSITY value of 0.5 in this case.

    dMass m;

    dReal sides[3];

    sides[0] = 2.0;

    sides[1] = 2.0;

    sides[2] = 2.0;

    r32 DENSITY = 0.5f;
    dMassSetBox(&m, DENSITY, sides[0], sides[1], sides[2]);

    // We can then apply this mass to our objects body.

    dBodySetMass(GameState->Object.Body, &m);

    // Here we create the actual geom object using dCreateBox. Note that this also adds the geom to our

    // collision space and sets the size of the geom to that of our box mass.

    GameState->Object.Geom[0] = dCreateBox(GameState->Space, sides[0], sides[1], sides[2]);

    // And lastly we want to associate the body with the geom using dGeomSetBody. Setting a body on a geom automatically

    // combines the position vector and rotation matrix of the body and geom so that setting the position or orientation

    // of one will set the value for both objects. The ODE docs have a lot more to say about the geom functions.

    dGeomSetBody(GameState->Object.Geom[0], GameState->Object.Body);
}*/