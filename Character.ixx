/*
* CHARACTER and LIMB (abstract) objects to be extended in the modules that use them.
*/

module;

#include "SDL.h"
#include "SDL_image.h"

export module Character;


export enum CharacterType { Player, Hostile, Friendly }; /* NOT a CLASS because we want to use it as int. */

/*
* Very minimal parent class.
* Different Screen modules will extend this to be useful in their environments.
* We can't hold a vector of Limb objects here, because the derived Character classes must hold similarly derived Limb objects.
*/
export class Character {
	public:
		Character() {}

		Character(SDL_Texture* texture, CharacterType characterType) :
			texture(texture), characterType(characterType) { }

		~Character() {}

		SDL_Texture* getTexture() { return texture; }
		void setTexture(SDL_Texture* incomingTexture) {
			if (texture) {
				SDL_DestroyTexture(texture);
				texture = incomingTexture;
			}
		}
		int getType() { return characterType; }

	protected:
		SDL_Texture* texture;
		CharacterType characterType;
};