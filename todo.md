
MISC:
- [ ] make a logfile manager

RENDU VOGSPHERE:
- [ ] le submodule doit pointer sur un commit fixe

SimpleGL:
- [ ] FIX `Obj3d::setColor(uint8_t,uint8_t,uint8_t)` probleme de conversion, shader ?
	peut importe la valeur envoyée, au final soit cest 0, soit cest 255 (equivalent RGB)
```C++
model.setColor(120, 0, 0); // <=> RED (255,0,0) 
model.setColor(5, 70, 100); // <=> WHITE (255,255,255) 
```
