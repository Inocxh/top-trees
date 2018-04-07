include "./pic/primitives.asy";

real u = 0.95;

void hBrick(pair origin, Label a, Label b) {
	draw(origin+(u,0)--(origin+(u,-u)), currentpen+linewidth(0.5)+gray);
	draw(origin--(origin+(2u,0))--(origin+(2u,-u))--(origin+(0,-u))--origin);
	label(a, origin+(u/2,-u/2));
	label(b, origin+(u+u/2,-u/2));
}

void vBrick(pair origin, Label a, Label b) {
	origin = origin - (0.05,0);
	draw(origin+(0,-u)--(origin+(u,-u)), currentpen+linewidth(0.5)+gray);
	draw(origin--(origin+(u,0))--(origin+(u,-2u))--(origin+(0,-2u))--origin);
	label(a, origin+(u/2,-u/2));
	label(b, origin+(u/2,-u-u/2));
}

currentpen=dotted+linewidth(0.5);
hBrick((0,0),"a","b");
hBrick((2,0),"b","c");
hBrick((4,0),"c","x");
currentpen=linewidth(1.2);
hBrick((6,0),"d","y");
currentpen=dotted+linewidth(0.5);
hBrick((8,0),"c","d");
hBrick((10,0),"d","e");

draw((6.5,-1){SE}..(9.5,-1){NE}, dashed, Arrow, L=Label("\it Rake"));

pair shift = (1,-2.2);
hBrick(shift+(0,0),"a","b");
hBrick(shift+(2,0),"b","c");
hBrick(shift+(4,0),"c","x");
vBrick(shift+(7,-1),"d","y");
currentpen=linewidth(1.2);
hBrick(shift+(6,0),"c","d");
currentpen=dotted+linewidth(0.5);
hBrick(shift+(8,0),"d","e");

draw((shift+(4.5,-1)){SE}..(shift+(6.5,-1)){NE}, dashed, Arrow, L=Label("\it Rake"));

pair shift = (2,-5.5);
currentpen=linewidth(1.2);
hBrick(shift+(0,0),"a","b");
hBrick(shift+(2,0),"b","c");
hBrick(shift+(6,0),"d","e");
hBrick(shift+(4,0),"c","d");
currentpen=dotted+linewidth(0.5);
vBrick(shift+(4,-1),"c","x");
vBrick(shift+(5,-1),"d","y");
