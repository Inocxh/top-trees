include "./pic/primitives.asy";

path getClusterEllipse(pair a, pair b, real scaleX=0.7, real scaleY=0.2) {
	pair center = midpoint(a--b);
	real angle = degrees(a-b);
	return shift(center)*rotate(angle)*xscale(length(a-b)*scaleX)*yscale(scaleY)*unitcircle;
}

void childCluster(pair pos, Label L) {
	filldraw(box(pos-(.35,.25), pos+(.35,.25)), white, dotted);
	label(L, pos);
}

pair a = (0,0);
pair aa = a - (1,1);
pair b = (2,0);
pair bb = b + (1,-1);

path edgeCluster = getClusterEllipse(a, b, scaleX=0.42, scaleY=0.22);
path combinedCluster = getClusterEllipse(a, b-(.42,0), scaleY=0.6);
path fullCluster = getClusterEllipse(a, b, scaleY=1);

filldraw(fullCluster, gray(0.92), dotted);
filldraw(combinedCluster, gray(0.8), dotted);
filldraw(edgeCluster, gray(0.7), dotted);

vertex(a);
vertex(b);
edge(a,b);
edge(a,aa, dotted);
edge(b,bb, dotted);

label("$x$-$y$", (a+b)/2+(0,.33));
label("$a$-$y$", (a+b)/2+(0,.75));
label("$a$-$z$", (a+b)/2+(0,1.2));

childCluster(aa, "$a,x$");
childCluster(bb, "$y,z$");
