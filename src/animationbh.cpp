#include "animationbh.hpp"
#include "human.hpp"
#include <math.h>

AnimationBH::AnimationBH(const std::string & filename) {
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

	/*
		currentFrame start at 1 (will be set in the first pass)
		currentLoop starts at 0
	*/
}

AnimationBH::~AnimationBH() {
}


static void		applyFrameToHuman(Human * human, json & jsonData, std::string frame) {
	std::cout << frame << std::endl;
	human->_head.local.setRot(jsonData[frame]["head"][0],
							jsonData[frame]["head"][1],
							jsonData[frame]["head"][2]);
	human->_trunk.local.setRot(jsonData[frame]["trunk"][0],
							jsonData[frame]["trunk"][1],
							jsonData[frame]["trunk"][2]);
	human->_leftArm.local.setRot(jsonData[frame]["leftArm"][0],
							jsonData[frame]["leftArm"][1],
							jsonData[frame]["leftArm"][2]);
	human->_rightArm.local.setRot(jsonData[frame]["rightArm"][0],
							jsonData[frame]["rightArm"][1],
							jsonData[frame]["rightArm"][2]);
	human->_leftForearm.local.setRot(jsonData[frame]["leftForearm"][0],
							jsonData[frame]["leftForearm"][1],
							jsonData[frame]["leftForearm"][2]);
	human->_rightForearm.local.setRot(jsonData[frame]["rightForearm"][0],
							jsonData[frame]["rightForearm"][1],
							jsonData[frame]["rightForearm"][2]);
	human->_leftThigh.local.setRot(jsonData[frame]["leftThigh"][0],
							jsonData[frame]["leftThigh"][1],
							jsonData[frame]["leftThigh"][2]);
	human->_rightThigh.local.setRot(jsonData[frame]["rightThigh"][0],
							jsonData[frame]["rightThigh"][1],
							jsonData[frame]["rightThigh"][2]);
	human->_leftCalf.local.setRot(jsonData[frame]["leftCalf"][0],
							jsonData[frame]["leftCalf"][1],
							jsonData[frame]["leftCalf"][2]);
	human->_rightCalf.local.setRot(jsonData[frame]["rightCalf"][0],
							jsonData[frame]["rightCalf"][1],
							jsonData[frame]["rightCalf"][2]);
}

void		AnimationBH::behaveOnTarget(BehaviorManaged *target) {
	if (this->finished)
		return ;
	if (this->_fpsTick == -1) {
		std::cout << __PRETTY_FUNCTION__ << " : Error : fps tick not set" << std::endl;
		return ;
	}
	Human *	human = dynamic_cast<Human*>(target);//should always success, as we tested it on isCompatible()
	if (!human) {
		std::cout << __PRETTY_FUNCTION__ << " : Error : data corrupted" << std::endl;
		exit(2);
	}
	this->_step++;
	if (this->_currentFrame == 0 || this->_step == this->_stepMax) {//on fini une frame et on passe a la suivante
		this->_currentFrame++;
		if (this->_currentFrame > this->_frameMax) {//loop
			this->_currentFrame = 1;
			this->currentLoop++;
		}

		this->_frame = std::string("frame") + std::to_string(this->_currentFrame);
		applyFrameToHuman(human, this->jsonData, this->_frame);
		this->_step = 0;
		float stepMaxtmp = (float)(this->jsonData[this->_frame]["time"]) / this->_fpsTick;
		if (stepMaxtmp < 1.0f) {
			std::cout << __PRETTY_FUNCTION__ << " : Error : frame time is too small" << std::endl;
			return ;
		}
		this->_stepMax = static_cast<unsigned int>(ceil(stepMaxtmp));
	}
	if (this->loop == -1 || this->currentLoop <= this->loop) {
		// modify human
		std::cout << "loop" << this->currentLoop << "/" << this->loop << ", frame" << this->_currentFrame << "/" << this->_frameMax << ", step " << this->_step << "/" << this->_stepMax << std::endl;
	} else { this->finished = true; }//end of animation
}

bool		AnimationBH::isCompatible(BehaviorManaged* target) const {
	return (dynamic_cast<Human*>(target));
}

void		AnimationBH::setFpsTick(float tick) {
	if (tick <= 0) {
		std::cout << __PRETTY_FUNCTION__ << " : Error : fps tick too low : " << tick << std::endl;
	} else {
		this->_fpsTick = tick;
	}
}
std::string	AnimationBH::getFilename() const { return this->_filename; }
