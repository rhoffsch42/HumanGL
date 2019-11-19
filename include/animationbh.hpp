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
#include <json/json.hpp>
using json = nlohmann::json;

#include <sstream>
#include <string>

/*
	As this Behavior store data depending on the target state, it will act the same way on all its targets
	recreate a new instance to operate on targets independently
	bool Behavior::isPersistant = true;

	-> create and dedicate a persistant data struct for each target on the list
*/

class AnimationBH : public Behavior
{
public:
	AnimationBH(const std::string & filename);
	~AnimationBH();

	void		behaveOnTarget(BehaviorManaged *target);
	bool		isCompatible(BehaviorManaged *target) const;

	void		setFpsTick(float tick);
	std::string	getFilename() const;

	json			jsonData;
	int				loop;//n = n loops, -1 = infinite loops //persistant
	int				currentLoop;//persistant
	bool			finished;//persistant
private:
	float			_fpsTick;
	std::string		_filename;
	unsigned int	_frameMax;

	unsigned int	_currentFrame;//persistant
	std::string		_frame;//persistant
	unsigned int	_stepMax;//persistant
	unsigned int	_step;//persistant
	float			_speed;//persistant
};
