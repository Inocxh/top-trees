include "./pic/clusters.asy";

void connectEllipse(path f, pair a, pair b) {
	draw(firstcut(f..midpoint(a--b), getEllipse(a,b)).before, dotted, Arrow);
	drawEllipse(a, b);
}
void connectCircle(path f, pair a) {
	draw(firstcut(f..a, getCircle(a)).before, dotted, Arrow);
	drawCircle(a);
}

////////////////////////////////////////////////////////////////////////////////

pair level1 = (0,2);
pair level2 = (0,4);
pair level3 = (0,5);

// 0. Level
pair a,b,c,d,e,f;
a=(0,0); b=(1,0); c=(2,0); d=(2.5,0.5);
e=(2.5,-0.5); f=(3.5,-0.5);

// 1. Level
pair ab, cd, ef;
ab = b+level1; cd = c+level1; ef = e+level1;

// 2. Level
pair ac, ef2;
ac = c+level2; ef2 = e + level2;

// 3. Level
pair af = c + level3;

////////////////////////////////////////////////////////////////////////////////

// 3. Level
vertex(af, "$a$-$f$", N);

connectEllipse(af..af+(.1,-.2), ac, ef2);

// 2. Level
vertex(ac, "$a$-$c$", NW);
vertex(ef2, "$e$-$f$", E);
edge(ac, ef2);

connectEllipse(ac..ac-(.1,.2), ab, cd);
connectCircle(ef2..ef2+(.1,-.2), ef);

// 1. Level
vertex(ab, "$a$-$b$", NW);
vertex(cd, "$c$-$d$", NE);
vertex(ef, "$e$-$f$", E);
edge(ab, cd);
edge(cd, ef);

connectEllipse(ab..(ab-(.5,.5)), a, b);
connectEllipse(cd..cd-(.1,.2), c, d);
connectEllipse(ef..ef+(.1,-.2), e, f);


//  0. Level
vertex(a, "$a$", N);
vertex(b, "$b$", NE);
vertex(c, "$c$", NW);
vertex(d, "$d$", N);
vertex(e, "$e$", SW);
vertex(f, "$f$", SE);
edge(a,b);
edge(b,c);
edge(c,d);
edge(c,e);
edge(e,f);

////////////////////////////////////////////////////////////////////////////////

path getBaseEdge(pair pos) {
	return ellipse(pos, .37, .25);
}

void baseEdge(pair pos, Label L) {
	filldraw(getBaseEdge(pos), white);
	label(L, pos);
}

pair shift = (6,0);
pair level0 = shift;
level1 += shift + (0,0.25);
level2 += shift;
level3 += shift;

pair taf, tae, tac2, tac;
pair eab, ebc, ecd, ece, eef;

// vertices
tac = level1 + (0.5, -0.5);
tac2 = level1 + (1.25, 0.25);
tae = level2 + (2.25,0.25);
taf = level3 + (3,0);

// edge clusters
eab = level0;
ecd = level0 + (2,0);
eef = level0 + (4,0);
ebc = tac + (.5,-.5);
ece = tae + (.5,-.5);

/////
path circle1 = circle((tac+tac2+ebc)/3, 1.2);
path circle2 = circle((tae+taf+ece)/3, 1.2);

path p = ((ab+cd)/2){NE}..(tac+tac2)/2;
pair begin = intersectionpoint(p, getEllipse(ab,cd));
pair end = intersectionpoint(p, circle1);
draw(begin{NE}..end, dashed, Arrow);
filldraw(circle1, gray(0.9), dotted);

p = ((ac+ef2)/2){NE}..(tae+taf)/2;
begin = intersectionpoint(p, getEllipse(ac,ef2));
end = intersectionpoint(p, circle2);
draw(begin{NE}..end, dashed, Arrow);
filldraw(circle2, gray(0.9), dotted);

p = ((a+b)/2){SE}..eab;
begin = intersectionpoint(p, getEllipse(a,b));
end = intersectionpoint(p, getBaseEdge(eab));
draw(begin{SE}..end, dashed, Arrow);

p = ((c+d)/2){E}..{NE}ecd;
begin = intersectionpoint(p, getEllipse(c,d));
end = intersectionpoint(p, getBaseEdge(ecd));
draw(begin{E}..{NE}end, dashed, Arrow);

p = ((e+f)/2){SE}..{NE}eef;
begin = intersectionpoint(p, getEllipse(e,f));
end = intersectionpoint(p, getBaseEdge(eef));
draw(begin{SE}..{NE}end, dashed, Arrow);

/////

vertex(taf, "$a,f$", N);
vertex(tae, "$a,e$", NW);
vertex(tac2, "$a,c$", NW);
vertex(tac, "$a,c$", NW);
edge(taf, tae );
edge(tae, tac2);
edge(tac2, tac);

edge(taf, eef);
edge(tae, ece);
edge(tac2, ecd);
edge(tac, ebc);
edge(tac, eab);

baseEdge(eab, "$a,b$");
baseEdge(ebc, "$b,c$");
baseEdge(ecd, "$c,d$");
baseEdge(ece, "$c,e$");
baseEdge(eef, "$e,f$");
