
HUMANGL:
- [X] refacto system de scale, utiliser directement les containers et le system de parenté, cest fait pour ça!
- [ ] bug random si on change le obj3d bp
- [X] bug Texture: biSizeImage = 0
- [X] Quand on cree un framebuffer, il gen une texture. Ce n'est pas forcement ce que l'on veut. Par exemple pour les UIPanel on cree un FB et on override le GL_COLOR_ATTACHMENT0. Il faudrait donc delete la texture OpenGL et remplacer le texId par la nouvelle texture.
	L'ideal est d'avoir un seul FB pour les textures offscreen sur les different attachment (elles DOIVENT avoir les memes dimensions), avoir un ptr FB dans UIPanel + un Glenum attachment. (attetntion a attachment max 8 sur nos mac, ptetre une option à activer pour en avoir +)

MISC:
- [ ] make a logfile manager

RENDU VOGSPHERE:
- [ ] fusionner les src

SimpleGL:
- [X] FIX `Obj3d::setColor(uint8_t,uint8_t,uint8_t)` probleme de conversion, shader ?
	peut importe la valeur envoyée, au final soit cest 0, soit cest 255 (equivalent RGB), probablement car cest un float 0.0f -> 1.0f dans le shader
```C++
model.setColor(120, 0, 0); // <=> RED (255,0,0) 
model.setColor(5, 70, 100); // <=> WHITE (255,255,255) 
```
- [X] FIX setOrigin/center system refacto:
	- to center obj3d: change origin during obj3d creation (offset every vertices)
	- make a `::rotateAround(Vector3 pos)` function
- [X] REFACTO replace all `NULL` by `nullptr` for pointers
- [X] FIX cursor lock with tab key
	- assign function to key/scancode (function ptr array)