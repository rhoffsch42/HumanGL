#include "animationbh.hpp"
#include <math.h>

IAnimationBH::IAnimationBH(const std::string & filename) {
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

IAnimationBH::~IAnimationBH() {
}

static Math::Rotation		getRotationDelta(json & jsonData, std::string key, std::string frame, std::string nextf, unsigned int stepMax) {
	Math::Rotation	r1(jsonData[frame][key][0], jsonData[frame][key][1], jsonData[frame][key][2]);
	Math::Rotation	r2(jsonData[nextf][key][0], jsonData[nextf][key][1], jsonData[nextf][key][2]);
	r2.sub(r1);
	r2.div(stepMax);
	return r2;
}

void		IAnimationBH::generateRotationsDelta() {
	std::string nextFrame = std::string("frame") + std::to_string((this->_currentFrame) % this->_frameMax + 1);
	for (auto it = this->jsonData[this->_frame].begin(); it != this->jsonData[this->_frame].end(); ++it) {
		// std::cout << it.key() << " | " << it.value() << "\n";
		if (it.key() != "time")
			this->_rotaMap[it.key()] = getRotationDelta(this->jsonData, it.key(), this->_frame, nextFrame, this->_stepMax);
	}
}


void		IAnimationBH::behaveOnTarget(BehaviorManaged *target) {
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
		std::cout << "loop" << this->currentLoop << "/" << this->loop \
			<< ", frame" << this->_currentFrame << "/" << this->_frameMax \
			<< ", step " << this->_step << "/" << this->_stepMax << std::endl;
		this->applyStepToTarget(target);// = 0

	} else { this->finished = true; }//end of animation
}

bool		IAnimationBH::isCompatible(BehaviorManaged* target) const {
	return (dynamic_cast<Human*>(target));
}

void		IAnimationBH::setSpeed(float speed) {
	if (speed >= 0.1f && speed <= 5.0f)
		this->_speed = speed;
	else
		std::cout << __PRETTY_FUNCTION__ << " Error : value must be between 0.1f and 5.0f" << std::endl;
}


void		IAnimationBH::setFpsTick(float tick) {
	if (tick <= 0) {
		std::cout << __PRETTY_FUNCTION__ << " : Error : fps tick too low : " << tick << std::endl;
	} else {
		this->_fpsTick = tick;
	}
}
std::string	IAnimationBH::getFilename() const { return this->_filename; }
