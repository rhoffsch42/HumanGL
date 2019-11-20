
HUMANGL:
- [X] refacto system de scale, utiliser directement les containers et le system de parenté, cest fait pour ça!
- [ ] bug random si on change le obj3d bp
- [ ] bug Texture: biSizeImage = 0
MISC:
- [ ] make a logfile manager

RENDU VOGSPHERE:
- [ ] fusionner les src

SimpleGL:
- [ ] FIX `Obj3d::setColor(uint8_t,uint8_t,uint8_t)` probleme de conversion, shader ?
	peut importe la valeur envoyée, au final soit cest 0, soit cest 255 (equivalent RGB), probablement car cest un float 0.0f -> 1.0f dans le shader
```C++
model.setColor(120, 0, 0); // <=> RED (255,0,0) 
model.setColor(5, 70, 100); // <=> WHITE (255,255,255) 
```
- [ ] FIX setOrigin/center system refacto:
	- to center obj3d: change origin during obj3d creation (offset every vertices)
	- make a `::rotateAround(Vector3 pos)` function
- [ ] REFACTO replace all `NULL` by `nullptr` for pointers
- [ ] FIX cursor lock with tab key
	- assign function to key/scancode (function ptr array)