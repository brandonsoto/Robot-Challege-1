#pragma config(Sensor, S1,     leftTouch,      sensorEV3_Touch)
#pragma config(Sensor, S2,     rightTouch,     sensorEV3_Touch)
#pragma config(Motor,  motorA,           ,             tmotorEV3_Large, openLoop)
#pragma config(Motor,  motorB,          leftMotor,     tmotorEV3_Large, PIDControl, reversed, driveLeft, encoder)
#pragma config(Motor,  motorC,          rightMotor,    tmotorEV3_Large, PIDControl, reversed, driveRight, encoder)
#pragma config(Motor,  motorD,           ,             tmotorEV3_Large, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

/**************************************************************
* Title: Challenge 1
* Authors: Cody Tedrick & Brandon Soto
**************************************************************/

///////////////////////////////////////////////globals//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const unsigned int aDelay = 50;							// time to see if another bumper touches (milliseconds)
const unsigned int volume = 40;							// default volume for robot sound
const unsigned int maxPower = 40;						// the robot's max power
const unsigned int minPower = 5; 						// the robot's min power
const unsigned int powRange = maxPower - minPower; 		// the robot's power range
const unsigned int backMinTime = 1000;					// the min time the robot can back up (milliseconds)
const unsigned int backMaxTime = 2000; 					// the max time the robot can back up (milliseconds)
const unsigned int forwardMinTime = 500;				// the min time the robot can go forward (milliseconds)
const unsigned int forwardMaxTime = 4500; 				// the max time the robot can go forward (milliseconds)
const unsigned int pauseTime = 2;						// number of seconds that the robot should pause after both sensors are touched
const unsigned int bias = 4;							// the bias for the robot to move in a certain direction. The higher the number, the more influential the bias will be.
bool respondingToTouch = false; 						// lock that ensures that multiple threads don't issue robot commands at same time (NOTE: only modified in touchThread)
const unsigned int forwardTimeRange = forwardMaxTime - forwardMinTime; 		// time range the robot goes forward
const unsigned int backTimeRange = backMaxTime - backMinTime; 				// time range the robot goes backward
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ////////////////////////////////////Function definitions ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* returns a random power number within the robot's power range */
unsigned int getRandomPower() {
	return random(powRange) + minPower;
}

/* returns a larger biassed number within the robot's power range */
unsigned int getMaxBias() {
	unsigned int turnPower = minPower, i;

	for (i = 0; i < bias; i++) {
		unsigned int randPower = getRandomPower();
		if (randPower > turnPower) {
			turnPower = randPower;
		}
	}

	return turnPower;
}

/* returns a smaller biassed number within the robot's power range */
unsigned int getMinBias() {
	unsigned int turnPower = maxPower, i;

	for (i = 0; i < (int) (bias / 2); i++) {
		unsigned int randPower = getRandomPower();
		if (randPower < turnPower) {
			turnPower = randPower;
		}
	}

	return turnPower;
}

/* thread that handles touch and collision behavior */
task touchThread() {
	setSoundVolume(volume);

	while (true) {
		while (!SensorValue[touch]); // wait until a sensor is touched
		wait(aDelay, milliseconds); // wait a bit to see if other sensor is touched

		respondingToTouch = true; // signal that the robot is about to respond to touch

		/* respond to collision */
		if (SensorValue[leftTouch] && SensorValue[rightTouch]) { // both sensors touched
			setLEDColor(ledOrange);
			playSound(soundUpwardTones);
			backward(random(backTimeRange) + backMinTime, milliseconds, maxPower);
			wait(pauseTime, seconds);

			unsigned int turnTime = random(forwardTimeRange) + forwardMinTime;

			// randomly choose to go left or right
			if (random(1)) {
				turnLeft(turnTime, milliseconds, getRandomPower());
			} else {
				turnRight(turnTime, milliseconds, getRandomPower());
			}

			setLEDColor(ledGreen);

		} else if (SensorValue[leftTouch]) { 					// left touched; turn right
			backward(random(backupTimeRange) + backupMinTime, milliseconds, speed);
			turnRight(turnTime, milliseconds, speed);

		} else { 												// right touched; turn left
			backward(random(backupTimeRange) + backupMinTime, milliseconds, speed);
			turnLeft(turnTime, milliseconds, speed);
		}

		respondingToTouch = false;
	}
}


/* thread that handles the robot wandering around in a drunken sailor fashion */
task wanderThread() {
	srand(nSysTime);

	/* Represents the robot's current direction relative to its starting direction
	 * (0 = start; positive = right of start; negative = left of start)
	 */
	long posRelativeToStart = 0;
	unsigned int leftPower, rightPower;
	float sleepTime;

	while (true) {
		sleepTime = random(forwardTimeRange) + forwardMinTime;

		displayCenteredTextLine(1, "Pos: %ld", posRelativeToStart); // display position relative to start position

		if (posRelativeToStart > 0) { 					// robot is currently RIGHT of its starting direction; try turn left
				displayCenteredTextLine(3, "Status: Right of Start");
				leftPower = getMinBias();
				rightPower = getMaxBias();

		} else { 										// robot is currently LEFT of or is facing its starting direction; try turn right
				displayCenteredTextLine(3, "Status: Left of Start");
				leftPower = getMaxBias();
				rightPower = getMinBias();
		}

		posRelativeToStart += ( (leftPower - rightPower) * sleepTime ); // robot's relative pos = old pos + new pos

		displayCenteredTextLine(4, "LP: %u, RP: %u", leftPower, rightPower); // display both motor powers
		displayCenteredTextLine(5, "Time: %f", sleepTime); // display the current sleep time

		setMotorSpeed(leftMotor, leftPower);
		setMotorSpeed(rightMotor, rightPower);

		resetTimer(T1);

		float valueOfTimer =  getTimer(T1, milliseconds); // used for clarity

		while (!respondingToTouch && valueOfTimer < sleepTime) { // wait for touch thread to take over or let the timer finish
			valueOfTimer = getTimer(T1, milliseconds);
		}

		// ************** maybe place the while loop inside an if? It'll be ugly **************************************************
		while(respondingToTouch) { // wait for touchThread to finish
			posRelativeToStart = 0; // reset starting direction because the robot has bumped into something

		}
		/************************************************************************************************************************/
	}
}

/* main thread */
task main() {
	startTask(wanderThread);
	startTask(touchThread);

	while (true); // allow other threads to execute
}
