#include "animationbh.hpp"
#include <math.h>

AAnimationBH::AAnimationBH(const std::string & filename) {
	this->_filename = filename;
	this->_currentFrame = 0;
	this->_fpsTick = -1;
	this->_stepMax = -1;
	this->_step = 0;
	this->_speed = 1.0f;
	std::string content = Misc::getFileContent(filename.c_str());
	std::stringstream ss;
	ss << content;
	try { ss >> this->jsonData; }
	catch (json::type_error& e) {
		std::cout << e.what() << std::endl;
		exit(2);
	}
	this->_frameMax = this->jsonData.size() - 1;//remove 1 because of the key "loop"
	this->loop = this->jsonData["loop"];//FIX can throw..
	this->currentLoop = 0;
	this->finished = false;
	/*
		currentFrame start at 1 (will be set in the first pass)
		currentLoop starts at 0
	*/
}

AAnimationBH::AAnimationBH(const AAnimationBH & src) {
	*this = src;
}

AAnimationBH &	AAnimationBH::operator=(const AAnimationBH & src) {
	this->isActive = true;
	this->targetList.clear();

	this->jsonData = src.jsonData;
	this->loop = src.loop;
	this->currentLoop = src.currentLoop;
	this->finished = src.finished;
	this->_fpsTick = src._fpsTick;
	this->_filename = src._filename;
	this->_frameMax = src._frameMax;
	this->_currentFrame = src._currentFrame;
	this->_frame = src._frame;
	this->_stepMax = src._frameMax;
	this->_step = src._step;
	this->_speed = src._speed;
	return *this;
}

AAnimationBH::~AAnimationBH() {
}

static Math::Rotation		getRotationDelta(json & jsonData, std::string key, std::string frame, std::string nextf, unsigned int stepMax, bool lastFrame) {
	Math::Rotation	r1(jsonData[frame][key][0], jsonData[frame][key][1], jsonData[frame][key][2]);
	Math::Rotation	r2(jsonData[nextf][key][0], jsonData[nextf][key][1], jsonData[nextf][key][2]);
	r2.sub(r1);
	
	/*
		when at last frame, an obj could have done a complete rotation (at least 360),
		so we revert the rotation to get a smooth loop
	-----
		refaire le meme system que dans in_the_shadow pour ne pas à faire ca.
		trouver la rotation la plus courte de srcRot%360 vers la destRot%360.

		une autre solution serait de refaire une passe sur les données loadées du json,
		et mettre les rotations relatives par rapport a la frame precedente,
		puis utiliser le system in_the_shadow a la derniere frame, si une key "lastFrameClosestRot" est true dans le json

		danger: si on cumule trop de rotation on pourrait theoriquement overflow ou underflow / perte de precision sur float
		->faire une fonction qui %360 a partir d'une certaine valeur dans SimpleGL ?
	*/
	if (lastFrame) { 
		if (r2.x >= 360.0f)
			r2.x = fmod(r2.x, 360.0f);
		while (r2.x <= -360.0f)
			r2.x += 360.0f;
	}
	
	r2.div(stepMax);
	return r2;
}

void		AAnimationBH::generateRotationsDelta() {
	std::string nextFrame = std::string("frame") + std::to_string((this->_currentFrame) % this->_frameMax + 1);
	for (auto it = this->jsonData[this->_frame].begin(); it != this->jsonData[this->_frame].end(); ++it) {
		// std::cout << it.key() << " | " << it.value() << "\n";
		if (it.key() == "comment") {//"comment" is reserved for comments, do not use it in your keys!
			// do nothing
		} else if (it.key() == "pos") {//"pos" is reserved for the relative position, do not use it in your keys!
			this->_rotaMap[it.key()] = Math::Rotation((float)jsonData[this->_frame][it.key()][0] / (float)this->_stepMax, \
													(float)jsonData[this->_frame][it.key()][1] / (float)this->_stepMax, \
													(float)jsonData[this->_frame][it.key()][2] / (float)this->_stepMax);
		} else if (it.key() != "time") {//"time" is reserved for the keyframe duration, do not use it in your keys!
			this->_rotaMap[it.key()] = getRotationDelta(this->jsonData, it.key(), this->_frame, nextFrame, this->_stepMax, this->_currentFrame == this->_frameMax);
		}
	}
}


void		AAnimationBH::behaveOnTarget(BehaviorManaged *target) {
	if (this->finished)
		return ;
	if (this->_fpsTick == -1) {
		std::cout << __PRETTY_FUNCTION__ << " : Error : fps tick not set" << std::endl;
		return ;
	}

	this->_step++;
	if (this->_currentFrame == 0 || this->_step == this->_stepMax) {//on fini une frame et on passe a la suivante
		this->_currentFrame++;
		if (this->_currentFrame > this->_frameMax) {//loop
			this->_currentFrame = 1;
			this->currentLoop++;
		}
		this->_frame = std::string("frame") + std::to_string(this->_currentFrame);
		this->_step = 0;
		float stepMaxtmp = (float)(this->jsonData[this->_frame]["time"]) / this->_speed / this->_fpsTick;
		if (stepMaxtmp < 1.0f) {
			std::cout << __PRETTY_FUNCTION__ << " : Error : frame time is too small" << std::endl;
			return ;
		}
		this->_stepMax = static_cast<unsigned int>(ceil(stepMaxtmp));
		this->generateRotationsDelta();
		this->applyFrameToTarget(target);// = 0
	}
	if (this->loop == -1 || this->currentLoop <= this->loop) {
		// modify target
		// std::cout << "loop" << this->currentLoop << "/" << this->loop \
		// 	<< ", frame" << this->_currentFrame << "/" << this->_frameMax \
		// 	<< ", step " << this->_step << "/" << this->_stepMax << std::endl;
		this->applyStepToTarget(target);// = 0

	} else { this->finished = true; }//end of animation
}

bool		AAnimationBH::isCompatible(BehaviorManaged* target) const {
	return (dynamic_cast<Human*>(target));
}

void		AAnimationBH::setSpeed(float speed) {
	if (speed >= 0.1f && speed <= 5.0f)
		this->_speed = speed;
	else
		std::cout << __PRETTY_FUNCTION__ << " Error : value must be between 0.1f and 5.0f" << std::endl;
}


void		AAnimationBH::setFpsTick(float tick) {
	if (tick <= 0) {
		std::cout << __PRETTY_FUNCTION__ << " : Error : fps tick too low : " << tick << std::endl;
	} else {
		this->_fpsTick = tick;
	}
}

void		AAnimationBH::reset() {
	this->_currentFrame = 0;
	this->currentLoop = 0;
	this->_step = 0;
	this->finished = false;
}

std::string	AAnimationBH::getFilename() const { return this->_filename; }
