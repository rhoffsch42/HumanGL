#pragma once

/*
	Dans Behavior:
		la liste de pair<BehaviorManaged, bool>
		il faudrait la transformer en list de pair<BehaviorManaged, Status>
			class Status
			{
			public:
				bool	state = true; //default value
			} 
		Pour permettre une specialisation de Status pour la classe AnimationBH:
			class Step : public Status
			{
			public:
				unsigned int	step = 0; // 1ere step
			}
		Ainsi on peut tracker les step de differents Humans pour les maneuvrer selon la step de l'anim

	/!\ si on herite de TransformBH, les value de transform seront valables pour une certaine step.
	comment adapter le TransformBH pour le step de la target
		-> Faire une Anim pour chaque step?
			{
				if (animStep1.isDone()) {
					animStep1.removeTarget(&bob);
					animStep2.addTarget(&bob);
				}
			}
		/!\ pour le dernier mouvement de la step, le pas est peut etre trop grand
			- il faut mettre le Human a la t_pp exacte et pas faire un run du BH
	/!\ un TransformBH pour chaque membre et pour chaque step de l'anim:
		anim1, 3steps, 6 membres: 3 * 6 = 18 TransformBH ?!
*/

//pour HumanGL on ne fera qu'une seule target, et on changera le TransformBH selon la step!
//Il devra être capable de marcher, sauter et se tenir immobile.

#include "behavior.hpp"
#include "misc.hpp"
#include "human.hpp"
#include "math.hpp"
#include <json/json.hpp>
using json = nlohmann::json;

#include <sstream>
#include <string>
#include <map>

/*
	As this Behavior store data depending on the target state, it will act the same way on all its targets
	recreate a new instance to operate on targets independently
	bool Behavior::isPersistant = true;

	refacto -> create and dedicate a persistant data struct for each target on the list
*/

/*
	the key "time" is reserved for the keyframe duration, do not use it for members
	the key "pos" is reserved for the relative position, do not use it for members
*/
class IAnimationBH : public Behavior {
public:
	IAnimationBH(const std::string & filename);
	virtual ~IAnimationBH();

	void		behaveOnTarget(BehaviorManaged *target);
	bool		isCompatible(BehaviorManaged *target) const;

	void		setSpeed(float speed);
	void		setFpsTick(float tick);
	void		reset();
	std::string	getFilename() const;

	json			jsonData;
	int				loop;//n = n loops, -1 = infinite loops //persistant
	int				currentLoop;//persistant
	bool			finished;//persistant
	std::map<std::string, Math::Rotation>	_rotaMap;
protected:
	float			_fpsTick;
	std::string		_filename;
	unsigned int	_frameMax;

	unsigned int	_currentFrame;//persistant
	std::string		_frame;//persistant
	unsigned int	_stepMax;//persistant
	unsigned int	_step;//persistant
	float			_speed;//persistant

	void			generateRotationsDelta();

	virtual void	applyFrameToTarget(BehaviorManaged *target) = 0;
	virtual void	applyStepToTarget(BehaviorManaged *target) = 0;
};


class AnimationHumanBH : public IAnimationBH {
public:
	AnimationHumanBH(const std::string & filename) : IAnimationBH(filename) {}
	~AnimationHumanBH() {}
private:
	void			applyFrameToTarget(BehaviorManaged *target) {
		Human *	human = dynamic_cast<Human*>(target);//should always success, as we tested it on isCompatible()
		if (!human) {
			std::cout << __PRETTY_FUNCTION__ << " : Error : data corrupted" << std::endl;
			exit(2);
		}
		// std::cout << this->_frame << std::endl;
		human->_head.local.setRot(this->jsonData[this->_frame]["head"][0], this->jsonData[this->_frame]["head"][1], this->jsonData[this->_frame]["head"][2]);
		human->_trunk.local.setRot(this->jsonData[this->_frame]["trunk"][0], this->jsonData[this->_frame]["trunk"][1], this->jsonData[this->_frame]["trunk"][2]);
		human->_leftArm.local.setRot(this->jsonData[this->_frame]["leftArm"][0], this->jsonData[this->_frame]["leftArm"][1], this->jsonData[this->_frame]["leftArm"][2]);
		human->_rightArm.local.setRot(this->jsonData[this->_frame]["rightArm"][0], this->jsonData[this->_frame]["rightArm"][1], this->jsonData[this->_frame]["rightArm"][2]);
		human->_leftForearm.local.setRot(this->jsonData[this->_frame]["leftForearm"][0], this->jsonData[this->_frame]["leftForearm"][1], this->jsonData[this->_frame]["leftForearm"][2]);
		human->_rightForearm.local.setRot(this->jsonData[this->_frame]["rightForearm"][0], this->jsonData[this->_frame]["rightForearm"][1], this->jsonData[this->_frame]["rightForearm"][2]);
		human->_leftThigh.local.setRot(this->jsonData[this->_frame]["leftThigh"][0], this->jsonData[this->_frame]["leftThigh"][1], this->jsonData[this->_frame]["leftThigh"][2]);
		human->_rightThigh.local.setRot(this->jsonData[this->_frame]["rightThigh"][0], this->jsonData[this->_frame]["rightThigh"][1], this->jsonData[this->_frame]["rightThigh"][2]);
		human->_leftCalf.local.setRot(this->jsonData[this->_frame]["leftCalf"][0], this->jsonData[this->_frame]["leftCalf"][1], this->jsonData[this->_frame]["leftCalf"][2]);
		human->_rightCalf.local.setRot(this->jsonData[this->_frame]["rightCalf"][0], this->jsonData[this->_frame]["rightCalf"][1], this->jsonData[this->_frame]["rightCalf"][2]);
		float coef = human->getLenght() / 4.0f;//4 = default length
		Math::Vector3 mvt = this->_rotaMap["pos"];
		mvt.mult(coef);
		human->_trunk.local.translate(mvt);
	}
	void			applyStepToTarget(BehaviorManaged *target) {
		Human *	human = dynamic_cast<Human*>(target);//should always success, as we tested it on isCompatible()
		if (!human) {
			std::cout << __PRETTY_FUNCTION__ << " : Error : data corrupted" << std::endl;
			exit(2);
		}
		human->_head.rotateMember(this->_rotaMap["head"]);
		human->_trunk.rotateMember(this->_rotaMap["trunk"]);
		human->_leftArm.rotateMember(this->_rotaMap["leftArm"]);
		human->_leftCalf.rotateMember(this->_rotaMap["leftCalf"]);
		human->_leftForearm.rotateMember(this->_rotaMap["leftForearm"]);
		human->_leftThigh.rotateMember(this->_rotaMap["leftThigh"]);
		human->_rightArm.rotateMember(this->_rotaMap["rightArm"]);
		human->_rightCalf.rotateMember(this->_rotaMap["rightCalf"]);
		human->_rightForearm.rotateMember(this->_rotaMap["rightForearm"]);
		human->_rightThigh.rotateMember(this->_rotaMap["rightThigh"]);
		float coef = human->getLenght() / 4.0f;//4 = default length
		Math::Vector3 mvt = this->_rotaMap["pos"];
		mvt.mult(coef);
		human->_trunk.local.translate(mvt);
	}
};