#pragma once

#include "humangl.h"

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
class AnimationBH : public TransformBH
{
public:
private:
	bool	_loop;
	float	_fpsTick;
};
